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

#include <bf_svtools/svdde.hxx>
#include <tools/urlobj.hxx>
#include <bf_svtools/pathoptions.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "app.hxx"
#include "appdata.hxx"
#include "objsh.hxx"
#include "sfxsids.hrc"

#include <bf_sfx2/sfxuno.hxx>

namespace binfilter {

//========================================================================

/*?*/ long SfxObjectShell::DdeExecute
/*?*/ (
/*?*/ 	const String&	rCmd		// in unserer BASIC-Syntax formuliert
/*?*/ )

/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------

/*?*/ long SfxObjectShell::DdeGetData
/*?*/ (
/*?*/ 	const String&	rItem,						// das anzusprechende Item
/*?*/ 	const String& rMimeType,					// in: Format
/*?*/ 	::com::sun::star::uno::Any & rValue 	// out: angeforderte Daten
/*?*/ )
{DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------

/*?*/ long SfxObjectShell::DdeSetData
/*?*/ (
/*?*/ 	const String&	rItem,						// das anzusprechende Item
/*?*/ 	const String& rMimeType,					// in: Format
/*?*/ 	const ::com::sun::star::uno::Any & rValue 	// out: angeforderte Daten
/*?*/ )
/*?*/ 
/*	[Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxObjectShell-Subklasse gerichtete DDE-Daten
    zu empfangen.

    Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/
/*?*/ 
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------

/*?*/ ::binfilter::SvLinkSource* SfxObjectShell::DdeCreateLinkSource
/*?*/ (
/*?*/ 	const String&	rItem	   // das zu erzeugende Item
/*?*/ )

/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }

//--------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
