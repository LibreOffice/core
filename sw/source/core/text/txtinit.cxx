/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "swcache.hxx"
#include "fntcache.hxx"
#include "swfntcch.hxx"
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
#include "blink.hxx"
#include "init.hxx"
#include "txtfly.hxx"
#include "dbg_lay.hxx"

SwCache *SwTxtFrm::pTxtCache = 0;
long SwTxtFrm::nMinPrtLine = 0;
SwContourCache *pContourCache = 0;
SwDropCapCache *pDropCapCache = 0;

IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtLine )
IMPL_FIXEDMEMPOOL_NEWDEL( SwParaPortion ) 
IMPL_FIXEDMEMPOOL_NEWDEL( SwLineLayout ) 
IMPL_FIXEDMEMPOOL_NEWDEL( SwHolePortion ) 
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtPortion ) 

/*************************************************************************
 *                  _TextInit(), _TextFinit()
 *************************************************************************/





void _TextInit()
{
    pFntCache = new SwFntCache; 
    pSwFontCache = new SwFontCache; 
    SwCache *pTxtCache = new SwCache( 250 
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
