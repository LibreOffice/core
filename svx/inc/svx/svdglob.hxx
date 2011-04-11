/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVDGLOB_HXX
#define _SVDGLOB_HXX

#include <tools/contnr.hxx>

#include <tools/resid.hxx>
#include <tools/string.hxx>

#include <svx/svxdllapi.h>

//************************************************************
//   forward declaration
//************************************************************

class Link;
class ResMgr;
class SdrEngineDefaults;
class ImpSdrStrCache;

//************************************************************
//   Defines
//************************************************************

// Get the resource manager for the app
ResMgr* ImpGetResMgr();

class SdrResId: ResId {
public:
    SdrResId(sal_uInt16 nId): ResId(nId,*ImpGetResMgr()) {}
};

// ResourceCache for frequently used strings.
// Global string resources with the IDs from
// SDR_StringCacheBegin (256) to SDR_StringCacheEnd
// are cached.
// See also SvdStr.Hrc
SVX_DLLPUBLIC String ImpGetResStr(sal_uInt16 nResID);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDGLOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
