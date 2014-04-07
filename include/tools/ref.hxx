/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_TOOLS_REF_HXX
#define INCLUDED_TOOLS_REF_HXX

#include <tools/toolsdllapi.h>
#include <vector>

namespace tools {

template<typename T> class SvRef {
public:
    SvRef(): pObj(0) {}

    SvRef(SvRef const & rObj): pObj(rObj.pObj)
    { if (pObj != 0) pObj->AddNextRef(); }

    SvRef(T * pObjP): pObj(pObjP) { if (pObj != 0) pObj->AddRef(); }

    ~SvRef() { if (pObj != 0) pObj->ReleaseReference(); }

    void Clear() {
        if (pObj != 0) {
            T * pRefObj = pObj;
            pObj = 0;
            pRefObj->ReleaseReference();
        }
    }

    SvRef & operator =(SvRef const & rObj) {
        if (rObj.pObj != 0) {
            rObj.pObj->AddNextRef();
        }
        T * pRefObj = pObj;
        pObj = rObj.pObj;
        if (pRefObj != 0) {
            pRefObj->ReleaseReference();
        }
        return *this;
    }

    bool Is() const { return pObj != 0; }

    T * operator &() const { return pObj; }

    T * operator ->() const { return pObj; }

    T & operator *() const { return *pObj; }

    operator T *() const { return pObj; }

protected:
    T * pObj;
};

}

template<typename T>
class SvRefMemberList : private std::vector<T>
{
private:
    typedef typename std::vector<T> base_t;

public:
    using base_t::size;
    using base_t::front;
    using base_t::back;
    using base_t::operator[];
    using base_t::begin;
    using base_t::end;
    using typename base_t::iterator;
    using typename base_t::const_iterator;
    using base_t::rbegin;
    using base_t::rend;
    using typename base_t::reverse_iterator;
    using base_t::empty;

    inline ~SvRefMemberList() { clear(); }
    inline void clear()
    {
        for( typename base_t::const_iterator it = base_t::begin(); it != base_t::end(); ++it )
        {
              T p = *it;
              if( p )
                  p->ReleaseReference();
        }
        base_t::clear();
    }

    inline void push_back( T p )
    {
        base_t::push_back( p );
        p->AddRef();
    }

    inline void insert(const SvRefMemberList& rOther)
    {
       for( typename base_t::const_iterator it = rOther.begin(); it != rOther.end(); ++it )
       {
           push_back(*it);
       }
    }

    inline T pop_back()
    {
        T p = base_t::back();
        base_t::pop_back();
        if( p )
            p->ReleaseReference();
        return p;
    }
};

#define SV_NO_DELETE_REFCOUNT  0x80000000

class TOOLS_DLLPUBLIC SvRefBase
{
    sal_uIntPtr nRefCount;

protected:
    virtual         ~SvRefBase();
    virtual void    QueryDelete();

public:
                    SvRefBase() { nRefCount = SV_NO_DELETE_REFCOUNT; }
                    SvRefBase( const SvRefBase & /* rObj */ )
                    { nRefCount = SV_NO_DELETE_REFCOUNT; }
    SvRefBase &     operator = ( const SvRefBase & ) { return *this; }

    void            RestoreNoDelete()
                    {
                        if( nRefCount < SV_NO_DELETE_REFCOUNT )
                            nRefCount += SV_NO_DELETE_REFCOUNT;
                    }
    sal_uIntPtr     AddNextRef() { return ++nRefCount; }
    sal_uIntPtr     AddRef()
                    {
                        if( nRefCount >= SV_NO_DELETE_REFCOUNT )
                            nRefCount -= SV_NO_DELETE_REFCOUNT;
                        return ++nRefCount;
                    }
    void            ReleaseReference()
                    {
                        if( !--nRefCount )
                            QueryDelete();
                    }
    sal_uIntPtr     ReleaseRef()
                    {
                        sal_uIntPtr n = --nRefCount;
                        if( !n )
                            QueryDelete();
                        return n;
                    }
    sal_uIntPtr     GetRefCount() const { return nRefCount; }
};

typedef tools::SvRef<SvRefBase> SvRefBaseRef;

class SvCompatWeakHdl : public SvRefBase
{
    friend class SvCompatWeakBase;
    void* _pObj;
    SvCompatWeakHdl( void* pObj ) : _pObj( pObj ) {}

public:
    void ResetWeakBase( ) { _pObj = 0; }
    void* GetObj() { return _pObj; }
};

typedef tools::SvRef<SvCompatWeakHdl> SvCompatWeakHdlRef;

class SvCompatWeakBase
{
    SvCompatWeakHdlRef _xHdl;

public:
    SvCompatWeakHdl* GetHdl() { return _xHdl; }

    // does not use Initalizer due to compiler warnings
    SvCompatWeakBase( void* pObj ) { _xHdl = new SvCompatWeakHdl( pObj ); }
    ~SvCompatWeakBase() { _xHdl->ResetWeakBase(); }
};

#define SV_DECL_COMPAT_WEAK( ClassName )                            \
class ClassName##Weak                                               \
{                                                                   \
    SvCompatWeakHdlRef _xHdl;                                       \
public:                                                             \
    inline               ClassName##Weak( ) {}                      \
    inline               ClassName##Weak( ClassName* pObj ) {       \
        if( pObj ) _xHdl = pObj->GetHdl(); }                        \
    inline void          Clear() { _xHdl.Clear(); }                 \
    inline ClassName##Weak& operator = ( ClassName * pObj ) {       \
        _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }          \
    inline bool            Is() const {                         \
        return _xHdl.Is() && _xHdl->GetObj(); }                     \
    inline ClassName *     operator &  () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName *     operator -> () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName &     operator *  () const {                   \
        return *(ClassName*) _xHdl->GetObj(); }                     \
    inline operator ClassName * () const {                          \
        return (ClassName*) (_xHdl.Is() ? _xHdl->GetObj() : 0 ); }  \
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
