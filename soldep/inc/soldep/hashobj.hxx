
#ifndef _FMRWRK_HASHOBJ_HXX
#define _FMRWRK_HASHOBJ_HXX

#include <soldep/hashtbl.hxx>

class ObjectWin;

class MyHashObject
{
    ObjectWin* mpWin;
    ULONG mnId;
public:
    MyHashObject( ULONG nId, ObjectWin* pWin );
    ULONG GetId() { return mnId; };
};

#endif

