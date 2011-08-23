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

#include "fntcache.hxx"     // pFntCache  ( SwFont/ScrFont-PrtFont Cache )
#include "swfntcch.hxx"     // pSwFontCache  ( SwAttrSet/SwFont Cache )
#include "txtfrm.hxx"
#include "txtcache.hxx"
#include "porrst.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "pordrop.hxx"
#include "txtfly.hxx"	// SwContourCache
#include "dbg_lay.hxx"  // Layout Debug Fileausgabe
namespace binfilter { 

/*N*/ SwCache *SwTxtFrm::pTxtCache = 0;
/*N*/ long SwTxtFrm::nMinPrtLine = 0;
/*N*/ SwContourCache *pContourCache = 0;

#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#ifdef _MSC_VER
#pragma code_seg( "SWSTATICS" )
#endif
#endif

/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtLine, 	  50,  50 )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( SwParaPortion,  50,  50 )	//Absaetze
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( SwLineLayout,  150, 150 )	//Zeilen
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( SwHolePortion, 150, 150 )	//z.B. Blanks am Zeilenende
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtPortion,  200, 100 )	//Attributwechsel

#ifndef PROFILE
#ifdef _MSC_VER
#pragma code_seg()
#endif
#endif

/*************************************************************************
 *                  _TextInit(), _TextFinit()
 *************************************************************************/

// Werden _nur_ in init.cxx verwendet, dort stehen extern void _TextFinit()
// und extern void _TextInit(...)

/*N*/ void _TextInit()
/*N*/ {
/*N*/ 	pFntCache = new SwFntCache;
/*N*/ 	pSwFontCache = new SwFontCache;
/*N*/ 	pWaveCol = new Color( COL_GRAY );
/*N*/ 
/*N*/ 	//Pauschale groesse 250, plus 100 pro Shell
/*N*/ 	SwCache *pTxtCache = new SwCache( 250, 100
/*N*/ #ifdef DBG_UTIL
/*N*/ 	, "static SwTxtFrm::pTxtCache"
/*N*/ #endif
/*N*/ 	);
/*N*/ 	SwTxtFrm::SetTxtCache( pTxtCache );
/*N*/ 	PROTOCOL_INIT
/*N*/ }

/*N*/ void _TextFinit()
/*N*/ {
/*N*/ 	PROTOCOL_STOP
/*N*/ 	delete SwTxtFrm::GetTxtCache();
/*N*/ 	delete pSwFontCache;
/*N*/ 	delete pFntCache;
/*N*/ 	delete pWaveCol;
/*N*/ 	delete pContourCache;
/*N*/ }



}
