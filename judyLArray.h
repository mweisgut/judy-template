#ifndef JUDYLARRAY_CPP_H
#define JUDYLARRAY_CPP_H

/****************************************************************************//**
* \file judyLArray.h C++ wrapper for judyL array implementation
*
*    A judyL array maps a set of ints to corresponding memory cells.
*    Each cell must be set to a non-zero value by the caller.
*
*    Author: Mark Pictor. Public domain.
*
********************************************************************************/

#include "judy64.h"
#include "assert.h"
#include <string.h>

template< typename JudyKey, typename JudyValue >
struct judylKVpair {
    JudyKey key;
    JudyValue value;
};
template< typename JudyKey, typename JudyValue >
class judyLArray {
protected:
    Judy * _judyarray;
    unsigned int _maxKeyLen, _depth;
    JudyValue * _lastSlot;
    unsigned char *_buff; //TODO change type for judyL?
    bool _success;
public:
    typedef judylKVpair< JudyKey, JudyValue > pair;
    judyLArray(): _maxKeyLen( sizeof( JudyKey ) ), _depth( sizeof( JudyKey ) ), _success( true ) {
        _judyarray = judy_open( _maxKeyLen, _depth );
        _buff = new unsigned char[_maxKeyLen];
        assert( sizeof( JudyValue ) == sizeof( this ) && "JudyValue *must* be the same size as a pointer!" );
    }

    explicit judyLArray( const judyLArray< JudyKey, JudyValue > & other ): _maxKeyLen( other._maxKeyLen ), _depth( other._depth ), _success( other._success ) {
        _judyarray = judy_clone( other._judyarray );
        _buff = new unsigned char[_maxKeyLen];
        strncpy( _buff, other._buff, _maxKeyLen );
        _buff[ _maxKeyLen ] = '\0'; //ensure that _buff is null-terminated, since strncpy won't necessarily do so
        find( _buff ); //set _lastSlot
    }

    ~judyLArray() {
        judy_close( _judyarray );
        delete[] _buff;
    }

    JudyValue getLastValue() {
        return &_lastSlot;
    }

    void setLastValue( JudyValue value ) {
        &_lastSlot = value;
    }

    bool success() {
        return _success;
    }
    //TODO
    // allocate data memory within judy array for external use.
    // void *judy_data (Judy *judy, unsigned int amt);

    //can this overwrite?
    void insert( JudyKey key, JudyValue value ) {
        assert( value != 0 );
        _lastSlot = (JudyValue *) judy_cell( _judyarray, (const unsigned char *) &key, _maxKeyLen );
        if( _lastSlot ) {
            *_lastSlot = value;
            _success = true;
        } else {
            _success = false;
        }
    }

    /// retrieve the cell pointer greater than or equal to given key
    /// NOTE what about an atOrBefore function?
    const pair atOrAfter( JudyKey key ) {
        _lastSlot = (JudyValue *) judy_strt( _judyarray, (const unsigned char *) &key, _maxKeyLen );
        return mostRecentPair();
    }

    /// retrieve the cell pointer, or return NULL for a given key.
    JudyValue find( JudyKey key ) {
        _lastSlot = (JudyValue *) judy_slot( _judyarray, (const unsigned char *) &key, _maxKeyLen );
        if( _lastSlot ) {
            _success = true;
            return *_lastSlot;
        } else {
            _success = false;
            return 0;
        }
    }

    /// retrieve the key-value pair for the most recent judy query.
    inline const pair mostRecentPair() {
        pair kv;
        judy_key( _judyarray, _buff, _maxKeyLen );
        if( _lastSlot ) {
            kv.value = *_lastSlot;
            _success = true;
        } else {
            kv.value = (JudyValue) 0;
            _success = false;
        }
        kv.key = ( JudyKey ) *_buff;
        return kv;
    }

    /// retrieve the last key-value pair in the array
    const pair & end() {
        _lastSlot = (JudyValue *) judy_end( _judyarray );
        return mostRecentPair();
    }

    /// retrieve the key-value pair for the next string in the array.
    const pair & next() {
        _lastSlot = (JudyValue *) judy_nxt( _judyarray );
        return mostRecentPair();
    }

    /// retrieve the key-value pair for the prev string in the array.
    const pair & previous() {
        _lastSlot = (JudyValue *) judy_prv( _judyarray );
        return mostRecentPair();
    }

    /// delete the key and cell for the current stack entry.
    void removeEntry() {
        judy_del ( _judyarray );
    }
};
#endif //JUDYLARRAY_CPP_H
