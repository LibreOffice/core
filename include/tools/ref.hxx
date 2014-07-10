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

#include <sal/config.h>

#include <cassert>

#include <tools/toolsdllapi.h>
#include <vector>

namespace tools {

/** T must be a class that extends SvRefBase */
template<typename T> class SvRef {
public:
    SvRef(): pObj(0) {}

    SvRef(SvRef const & rObj): pObj(rObj.pObj)
    {
        if (pObj != 0) pObj->AddNextRef();
    }

    SvRef(T * pObjP): pObj(pObjP)
    {
        if (pObj != 0) pObj->AddRef();
    }

    ~SvRef()
    {
        if (pObj != 0) pObj->ReleaseRef();
    }

    void Clear()
    {
        if (pObj != 0) {
            T * pRefObj = pObj;
            pObj = 0;
            pRefObj->ReleaseRef();
        }
    }

    SvRef & operator =(SvRef const & rObj)
    {
        if (rObj.pObj != 0) {
            rObj.pObj->AddNextRef();
        }
        T * pRefObj = pObj;
        pObj = rObj.pObj;
        if (pRefObj != 0) {
            pRefObj->ReleaseRef();
        }
        return *this;
    }

    bool Is()         const { return pObj != 0; }

    T * operator &()  const { return pObj; }

    T * operator ->() const { assert(pObj != 0); return pObj; }

    T & operator *()  const { assert(pObj != 0); return *pObj; }

    operator T *()    const { return pObj; }

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
                  p->ReleaseRef();
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
            p->ReleaseRef();
        return p;
    }
};


/** Classes that want to be referenced-counted via SvRef<T>, should extend this base class */
class TOOLS_DLLPUBLIC SvRefBase
{
    static const sal_uIntPtr SV_NO_DELETE_REFCOUNT = 0x80000000;
    sal_uIntPtr nRefCount;

protected:
    virtual         ~SvRefBase();
    virtual void    QueryDelete();

public:
                    SvRefBase()
                    { nRefCount = SV_NO_DELETE_REFCOUNT; }

                    SvRefBase( const SvRefBase & /* rObj */ )
                    { nRefCount = SV_NO_DELETE_REFCOUNT; }

    SvRefBase &     operator = ( const SvRefBase & )
                    { return *this; }

    void            RestoreNoDelete()
                    {
                        if( nRefCount < SV_NO_DELETE_REFCOUNT )
                            nRefCount += SV_NO_DELETE_REFCOUNT;
                    }

    sal_uIntPtr     AddNextRef()
                    { return ++nRefCount; }

    sal_uIntPtr     AddRef()
                    {
                        if( nRefCount >= SV_NO_DELETE_REFCOUNT )
                            nRefCount -= SV_NO_DELETE_REFCOUNT;
                        return ++nRefCount;
                    }

    void            ReleaseRef()
                    {
                        if( !--nRefCount )
                            QueryDelete();
                    }

    sal_uIntPtr     GetRefCount() const
                    { return nRefCount; }
};

/** We only have one weak reference in LO, in include/sfx2/frame.hxx, class SfxFrameWeak.
    This acts as a intermediary between SfxFrameWeak and SfxFrame_Impl.
*/
class SvCompatWeakHdl : public SvRefBase
{
    friend class SvCompatWeakBase;
    void* _pObj;

    SvCompatWeakHdl( void* pObj ) : _pObj( pObj ) {}

public:
    void  ResetWeakBase( ) { _pObj = 0; }
    void* GetObj()        { return _pObj; }
};

/** We only have one place that extends this, in sfx2/source/view/impframe.hxx, class SfxFrame_Impl,
    its function is to notify the SvCompatWeakHdl when an SfxFrame_Impl object is deleted.
*/
class SvCompatWeakBase
{
    tools::SvRef<SvCompatWeakHdl> _xHdl;

public:
    // Does not use initializer due to compiler warnings,
    // because the lifetime of the _xHdl object can exceed the lifetime of this class.
    SvCompatWeakBase( void* pObj ) { _xHdl = new SvCompatWeakHdl( pObj ); }

    ~SvCompatWeakBase() { _xHdl->ResetWeakBase(); }

    SvCompatWeakHdl* GetHdl() { return _xHdl; }
};

#define SV_DECL_COMPAT_WEAK_REF( ClassName )                        \
class ClassName##WeakRef                                            \
{                                                                   \
    tools::SvRef<SvCompatWeakHdl> _xHdl;                            \
public:                                                             \
    inline               ClassName##WeakRef( ) {}                   \
    inline               ClassName##WeakRef( ClassName* pObj ) {    \
        if( pObj ) _xHdl = pObj->GetHdl(); }                        \
    inline ClassName##WeakRef& operator = ( ClassName * pObj ) {    \
        _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }          \
    inline bool            Is() const {                         \
        return _xHdl.Is() && _xHdl->GetObj(); }                     \
    inline ClassName *     operator -> () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName *     operator &  () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline operator ClassName * () const {                          \
        return (ClassName*) (_xHdl.Is() ? _xHdl->GetObj() : 0 ); }  \
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
