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
#ifndef _FNTCACHE_HXX
#define _FNTCACHE_HXX

#include <vcl/font.hxx>
#include <tools/mempool.hxx>

#include "swtypes.hxx"
#include "swcache.hxx"

class Printer;
class OutputDevice;
class FontMetric;
class SwFntObj;
class SwDrawTextInfo;   // DrawText
class ViewShell;
class SwSubFont;
class MapMode;

/*************************************************************************
 *                      class SwFntCache
 *************************************************************************/

class SwFntCache : public SwCache
{
public:

    inline SwFntCache() : SwCache(50,50
#if OSL_DEBUG_LEVEL > 1
    , ByteString( RTL_CONSTASCII_STRINGPARAM(
                        "Globaler Font-Cache pFntCache" ))
#endif
    ) {}

    inline SwFntObj *First( ) { return (SwFntObj *)SwCache::First(); }
    inline SwFntObj *Next( SwFntObj *pFntObj)
        { return (SwFntObj *)SwCache::Next( (SwCacheObj *)pFntObj ); }
    void Flush();
};

// Font-Cache, globale Variable, in txtinit.Cxx angelegt/zerstoert
extern SwFntCache *pFntCache;
extern SwFntObj *pLastFont;
extern sal_uInt8 *pMagicNo;
extern Color *pWaveCol;

/*************************************************************************
 *                      class SwFntObj
 *************************************************************************/

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

/*************************************************************************
 *                      class SwFntAccess
 *************************************************************************/


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
