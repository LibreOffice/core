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


#include "copy.hxx"
#include "destr.hxx"

using namespace cppu;

#include <map>
#include <memory>
#include <com/sun/star/uno/Reference.hxx>

extern "C"
{
static struct bar {
    bool bAvoidReenter = false;
    com::sun::star::uno::XInterface * pTracked = 0;
    int nAcquire = 0;
    int nRelease = 0;
    std::map<com::sun::star::uno::BaseReference const*, short const*> tracking;
    ~bar()
    {
        SAL_DEBUG(" ££££££££££££ tracking leaks: " << tracking.size() );
        SAL_DEBUG(" ££££££££££££ nAcquire " << nAcquire << " nRelease " << nRelease);
    }
} g_bar;
    struct ReenterGuard {
        ReenterGuard() {g_bar.bAvoidReenter = true;}
        ~ReenterGuard() {g_bar.bAvoidReenter = false;}
    };
CPPU_DLLPUBLIC void hack_acquire(com::sun::star::uno::BaseReference const* pRef)
{
    if (g_bar.bAvoidReenter || !g_bar.pTracked) return;
    ReenterGuard g;
    if (*pRef != g_bar.pTracked) return;
    short const *pTracking = new short;
    assert(g_bar.tracking.insert(std::make_pair(pRef, pTracking)).second);
    g_bar.nAcquire++;
}
CPPU_DLLPUBLIC void hack_release(com::sun::star::uno::BaseReference const* pRef)
{
    if (g_bar.bAvoidReenter || !g_bar.pTracked) return;
    ReenterGuard g;
    if (*pRef != g_bar.pTracked) return;
    auto iter = g_bar.tracking.find(pRef);
    assert(iter != g_bar.tracking.end());
    delete iter->second;
    g_bar.tracking.erase(iter);
    g_bar.nRelease++;

}
CPPU_DLLPUBLIC void hack_track(com::sun::star::uno::XInterface * pIfc)
{
    g_bar.pTracked = pIfc;
    SAL_DEBUG(" ££££££££££££ pTracked " << g_bar.pTracked);
}

CPPU_DLLPUBLIC void SAL_CALL uno_type_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pDest, release );
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pDest, release );
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_type_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, 0, mapping );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, 0, mapping );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}

CPPU_DLLPUBLIC void SAL_CALL uno_any_destruct( uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pValue, release );
}

CPPU_DLLPUBLIC void SAL_CALL uno_any_clear( uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pValue, release );
    CONSTRUCT_EMPTY_ANY( pValue );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
