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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <bf_offmgr/app.hxx>

#include <comphelper/processfactory.hxx>
#include <bf_svx/unolingu.hxx>

#include <pagefrm.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <viscrs.hxx>
#include <fntcache.hxx>
#include <swfntcch.hxx>
#include <hffrm.hxx>
#include <colfrm.hxx>
#include <bodyfrm.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <swtblfmt.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <sectfrm.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::comphelper;

#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#ifdef _MSC_VER
#pragma code_seg( "SWSTATICS" )
#endif
#endif

//STRIP001 ByteString aEmptyByteStr;     // Konstante Strings
String aEmptyStr;				// Konstante Strings
String aDotStr('.');			// Konstante Strings

IMPL_FIXEDMEMPOOL_NEWDEL( SwAttrSet, 25, 25 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwStartNode, 20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwEndNode,   20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBox, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPaM, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCursor, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtNode, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwpHints, 25, 25 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFntObj, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFontObj, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFrmFmt,		20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFlyFrmFmt,	10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwDrawFrmFmt,	10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBorderAttrs, 100, 100 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCellFrm,	50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwRowFrm,		10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwColumnFrm,	40, 40 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwSectionFrm,	20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTabFrm,	10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPageFrm,	20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBodyFrm,	20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwHeaderFrm,	20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFooterFrm,	20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtFrm, 	50,  50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableFmt, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableLineFmt, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBoxFmt, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos, 20, 20 )


#ifndef PROFILE
#ifdef _MSC_VER
#pragma code_seg()
#endif
#endif

/*N*/ Size GetGraphicSizeTwip( const Graphic& rGraphic, OutputDevice* pOutDev )
/*N*/ {
/*N*/ 	const MapMode aMapTwip( MAP_TWIP );
/*N*/  	Size aSize( rGraphic.GetPrefSize() );
/*N*/ 	if( MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit() )
/*N*/ 	{
/*N*/ 		if( !pOutDev )
/*N*/ 			pOutDev = Application::GetDefaultDevice();
/*N*/ 		aSize = pOutDev->PixelToLogic( aSize, aMapTwip );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aSize = OutputDevice::LogicToLogic( aSize,
/*N*/ 										rGraphic.GetPrefMapMode(), aMapTwip );
/*N*/ 	return aSize;
/*N*/ }


/*N*/ Locale CreateLocale( LanguageType eLanguage )
/*N*/ {
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); Locale temp; return temp;//STRIP001 	String aLangStr, aCtryStr;
/*N*/ }


/*N*/ Reference< XHyphenator >  GetHyphenator()
/*N*/ {
/*N*/ 	return LinguMgr::GetHyphenator();
/*N*/ }


/*N*/ Reference< XDictionaryList >  GetDictionaryList()
/*N*/ {
/*N*/ 	return LinguMgr::GetDictionaryList();
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
