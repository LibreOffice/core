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
#ifndef INCLUDED_SFX2_SOURCE_INC_SFXTYPES_HXX
#define INCLUDED_SFX2_SOURCE_INC_SFXTYPES_HXX

#include <sal/config.h>

#include <sal/log.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include <osl/mutex.hxx>

#ifndef DELETEZ
#define DELETEZ(pPtr) ( delete pPtr, pPtr = 0 )
#endif

#ifndef DELETEX
#ifdef DBG_UTIL
#define DELETEX(T, pPtr) \
    ( delete pPtr, pPtr = reinterpret_cast<T *>(sal_IntPtr(-1)) )
#else
#define DELETEX(T, pPtr) delete pPtr
#endif
#endif


#if defined(DBG_UTIL)

class SfxStack
{
    static unsigned nLevel;

public:
    SfxStack( const char *pName )
    {
        ++nLevel;
        SAL_INFO("sfx.control", "STACK: enter " << nLevel << " " << pName);
    }
    ~SfxStack()
    {
        SAL_INFO("sfx.control", "STACK: leave " << nLevel);
        --nLevel;
    }
};

#define SFX_STACK(s) SfxStack aSfxStack_( #s )
#else
#define SFX_STACK(s)
#endif


struct StringList_Impl : private Resource
{

    ResId aResId;

    StringList_Impl( const ResId& rErrIdP,  sal_uInt16 nId)
        : Resource( rErrIdP ), aResId(nId, *rErrIdP.GetResMgr()) {}
    ~StringList_Impl() { FreeResource(); }

};

#endif // INCLUDED_SFX2_SOURCE_INC_SFXTYPES_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
