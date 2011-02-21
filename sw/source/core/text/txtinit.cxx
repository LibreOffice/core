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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "txtcfg.hxx"
#include "swcache.hxx"
#include "fntcache.hxx"     // pFntCache  ( SwFont/ScrFont-PrtFont Cache )
#include "swfntcch.hxx"     // pSwFontCache  ( SwAttrSet/SwFont Cache )
#include "txtfrm.hxx"
#include "txtcache.hxx"
#include "porlay.hxx"
#include "porglue.hxx"
#include "porexp.hxx"
#include "porrst.hxx"
#include "portab.hxx"
#include "porfly.hxx"
#include "portox.hxx"
#include "porref.hxx"
#include "porftn.hxx"
#include "porhyph.hxx"
#include "pordrop.hxx"
#include "blink.hxx"    // Blink-Manager
#include "init.hxx"   // Deklarationen fuer _TextInit() und _TextFinit()
#include "txtfly.hxx"   // SwContourCache
#include "dbg_lay.hxx"  // Layout Debug Fileausgabe

SwCache *SwTxtFrm::pTxtCache = 0;
long SwTxtFrm::nMinPrtLine = 0;
SwContourCache *pContourCache = 0;
SwDropCapCache *pDropCapCache = 0;

IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtLine,      50,  50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwParaPortion,  50,  50 ) //Absaetze
IMPL_FIXEDMEMPOOL_NEWDEL( SwLineLayout,  150, 150 ) //Zeilen
IMPL_FIXEDMEMPOOL_NEWDEL( SwHolePortion, 150, 150 ) //z.B. Blanks am Zeilenende
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtPortion,  200, 100 ) //Attributwechsel

/*************************************************************************
 *                  _TextInit(), _TextFinit()
 *************************************************************************/

// Werden _nur_ in init.cxx verwendet, dort stehen extern void _TextFinit()
// und extern void _TextInit(...)

void _TextInit()
{
    pFntCache = new SwFntCache;                     // Cache for SwSubFont -> SwFntObj = { Font aFont, Font* pScrFont, Font* pPrtFont, OutputDevice* pPrinter, ... }
    pSwFontCache = new SwFontCache;                 // Cache for SwTxtFmtColl -> SwFontObj = { SwFont aSwFont, SfxPoolItem* pDefaultArray }
    SwCache *pTxtCache = new SwCache( 250, 100      // Cache for SwTxtFrm -> SwTxtLine = { SwParaPortion* pLine }
#if OSL_DEBUG_LEVEL > 1
    , "static SwTxtFrm::pTxtCache"
#endif
    );
    SwTxtFrm::SetTxtCache( pTxtCache );
    pWaveCol = new Color( COL_GRAY );
    PROTOCOL_INIT
}

void _TextFinit()
{
    PROTOCOL_STOP
    delete SwTxtFrm::GetTxtCache();
    delete pSwFontCache;
    delete pFntCache;
    delete pBlink;
    delete pWaveCol;
    delete pContourCache;
    SwDropPortion::DeleteDropCapCache();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
