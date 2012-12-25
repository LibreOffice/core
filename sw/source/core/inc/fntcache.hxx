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

#ifndef _FNTCACHE_HXX
#define _FNTCACHE_HXX

#include <vcl/font.hxx>
#include <tools/mempool.hxx>
#include "swtypes.hxx"
#include "swcache.hxx"

class OutputDevice;
class FontMetric;
class SwFntObj;
class SwDrawTextInfo;
class ViewShell;
class SwSubFont;
class MapMode;
class Point;

class SwFntCache : public SwCache
{
public:
    inline SwFntCache() : SwCache(50
#ifdef DBG_UTIL
    , rtl::OString(RTL_CONSTASCII_STRINGPARAM("Global Font-Cache pFntCache"))
#endif
    ) {}

    inline SwFntObj *First( ) { return (SwFntObj *)SwCache::First(); }
    inline SwFntObj *Next( SwFntObj *pFntObj)
        { return (SwFntObj *)SwCache::Next( (SwCacheObj *)pFntObj ); }
    void Flush();
};

// Font cache, global variable, created/destroyed in txtinit.cxx
extern SwFntCache *pFntCache;
extern SwFntObj *pLastFont;
extern sal_uInt8 *pMagicNo;
extern Color *pWaveCol;

class SwFntObj : public SwCacheObj
{
    friend class SwFntAccess;
    friend void _InitCore();
    friend void _FinitCore();

    Font aFont;
    Font *pScrFont;
    Font *pPrtFont;
    OutputDevice* pPrinter;
    sal_uInt16 nGuessedLeading;
    sal_uInt16 nExtLeading;
    sal_uInt16 nScrAscent;
    sal_uInt16 nPrtAscent;
    sal_uInt16 nScrHeight;
    sal_uInt16 nPrtHeight;
    sal_uInt16 nPropWidth;
    sal_uInt16 nZoom;
    sal_Bool bSymbol : 1;
    sal_Bool bPaintBlank : 1;

    static long nPixWidth;
    static MapMode *pPixMap;
    static OutputDevice *pPixOut;

    // SMARTTAGS
    void calcLinePos(SwDrawTextInfo& rInf, Point& aStart, Point& aEnd, xub_StrLen nStart,
       xub_StrLen nWrLen, xub_StrLen nCnt, const sal_Bool bSwitchH2V, const sal_Bool bSwitchL2R,
       long nHalfSpace, long* pKernArray, const sal_Bool bBidiPor);

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFntObj)

    SwFntObj( const SwSubFont &rFont, const void* pOwner,
              ViewShell *pSh );

    virtual ~SwFntObj();

    inline       Font *GetScrFont()     { return pScrFont; }
    inline       Font *GetFont()        { return &aFont; }
    inline const Font *GetFont() const  { return &aFont; }

    inline sal_uInt16 GetGuessedLeading() const  { return nGuessedLeading; }
    inline sal_uInt16 GetExtLeading() const  { return nExtLeading; }

    sal_uInt16 GetFontAscent( const ViewShell *pSh, const OutputDevice& rOut );
    sal_uInt16 GetFontHeight( const ViewShell *pSh, const OutputDevice& rOut );
    sal_uInt16 GetFontLeading( const ViewShell *pSh, const OutputDevice& rOut );

    void GuessLeading( const ViewShell& rSh, const FontMetric& rMet );

    void SetDevFont( const ViewShell *pSh, OutputDevice& rOut );
    inline OutputDevice* GetPrt() const { return pPrinter; }
    inline sal_uInt16   GetZoom() const { return nZoom; }
    inline sal_uInt16   GetPropWidth() const { return nPropWidth; }
    inline sal_Bool     IsSymbol() const { return bSymbol; }

    void   DrawText( SwDrawTextInfo &rInf );
    Size  GetTextSize( SwDrawTextInfo &rInf );
    xub_StrLen GetCrsrOfst( SwDrawTextInfo &rInf );

    void CreateScrFont( const ViewShell& rSh, const OutputDevice& rOut );
    void CreatePrtFont( const OutputDevice& rOut );
};

class SwFntAccess : public SwCacheAccess
{
    ViewShell *pShell;
protected:
    virtual SwCacheObj *NewObj( );

public:
    SwFntAccess( const void * &rMagic, sal_uInt16 &rIndex, const void *pOwner,
                 ViewShell *pShell,
                 sal_Bool bCheck = sal_False  );
    inline SwFntObj* Get() { return (SwFntObj*) SwCacheAccess::Get(); };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
