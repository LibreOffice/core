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

#include <swcache.hxx>
#include <fntcache.hxx>
#include <swfntcch.hxx>
#include <txtfrm.hxx>
#include "pordrop.hxx"
#include <blink.hxx>
#include <init.hxx>
#include <txtfly.hxx>
#include <dbg_lay.hxx>

SwCache *SwTextFrame::s_pTextCache = nullptr;
SwContourCache *pContourCache = nullptr;
SwDropCapCache *pDropCapCache = nullptr;

// Are ONLY used in init.cxx.
// There we have extern void TextFinit()
// and extern void TextInit_(...)

void TextInit_()
{
    pFntCache = new SwFntCache; // Cache for SwSubFont -> SwFntObj = { Font aFont, Font* pScrFont, Font* pPrtFont, OutputDevice* pPrinter, ... }
    pSwFontCache = new SwFontCache; // Cache for SwTextFormatColl -> SwFontObj = { SwFont aSwFont, SfxPoolItem* pDefaultArray }
    SwCache *pTextCache = new SwCache( 250 // Cache for SwTextFrame -> SwTextLine = { SwParaPortion* pLine }
#ifdef DBG_UTIL
    , "static SwTextFrame::s_pTextCache"
#endif
    );
    SwTextFrame::SetTextCache( pTextCache );
    PROTOCOL_INIT
}

void TextFinit()
{
    PROTOCOL_STOP
    delete SwTextFrame::GetTextCache();
    delete pSwFontCache;
    delete pFntCache;
    delete pBlink;
    delete pContourCache;
    SwDropPortion::DeleteDropCapCache();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
