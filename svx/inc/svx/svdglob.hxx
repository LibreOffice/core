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
//   Vorausdeklarationen
//************************************************************

class Link;
class ResMgr;
class SdrEngineDefaults;
class ImpSdrStrCache;

//************************************************************
//   Defines
//************************************************************

// Den zur App passenden Resourcemanager holen
ResMgr* ImpGetResMgr();

class SdrResId: ResId {
public:
    SdrResId(sal_uInt16 nId): ResId(nId,*ImpGetResMgr()) {}
};

// ResourceCache fuer haeuffig benoetigte Strings
// gecachet werden globale String-Resourcen mit den
// IDs von SDR_StringCacheBegin (256) bis
// SDR_StringCacheEnd (weiss ich grad' nicht).
// Hierzu siehe auch SvdStr.Hrc
SVX_DLLPUBLIC String ImpGetResStr(sal_uInt16 nResID);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDGLOB_HXX

