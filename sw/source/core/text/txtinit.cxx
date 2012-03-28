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


#include "swcache.hxx"
#include "fntcache.hxx"     // pFntCache ( SwFont/ScrFont-PrtFont cache )
#include "swfntcch.hxx"     // pSwFontCache ( SwAttrSet/SwFont cache )
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
#include "blink.hxx"    // Blink manager
#include "init.hxx"     // Declarations for _TextInit() and _TextFinit()
#include "txtfly.hxx"   // SwContourCache
#include "dbg_lay.hxx"  // Layout Debug file output

SwCache *SwTxtFrm::pTxtCache = 0;
long SwTxtFrm::nMinPrtLine = 0;
SwContourCache *pContourCache = 0;
SwDropCapCache *pDropCapCache = 0;

IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtLine )
IMPL_FIXEDMEMPOOL_NEWDEL( SwParaPortion ) // Paragraphs
IMPL_FIXEDMEMPOOL_NEWDEL( SwLineLayout ) // Lines
IMPL_FIXEDMEMPOOL_NEWDEL( SwHolePortion ) // e.g. Blanks at the line end
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtPortion ) // Attribute change

/*************************************************************************
 *                  _TextInit(), _TextFinit()
 *************************************************************************/

// Are ONLY used in init.cxx.
// There we have extern void _TextFinit()
// and extern void _TextInit(...)

void _TextInit()
{
    pFntCache = new SwFntCache; // Cache for SwSubFont -> SwFntObj = { Font aFont, Font* pScrFont, Font* pPrtFont, OutputDevice* pPrinter, ... }
    pSwFontCache = new SwFontCache; // Cache for SwTxtFmtColl -> SwFontObj = { SwFont aSwFont, SfxPoolItem* pDefaultArray }
    SwCache *pTxtCache = new SwCache( 250 // Cache for SwTxtFrm -> SwTxtLine = { SwParaPortion* pLine }
#ifdef DBG_UTIL
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
