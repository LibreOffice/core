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
#ifdef DBG_UTIL
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
extern BYTE *pMagicNo;
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
    USHORT nGuessedLeading;
    USHORT nExtLeading;
    USHORT nScrAscent;
    USHORT nPrtAscent;
    USHORT nScrHeight;
    USHORT nPrtHeight;
    USHORT nPropWidth;
    USHORT nZoom;
    BOOL bSymbol : 1;
    BOOL bPaintBlank : 1;

    static long nPixWidth;
    static MapMode *pPixMap;
    static OutputDevice *pPixOut;

    // SMARTTAGS
    void calcLinePos(SwDrawTextInfo& rInf, Point& aStart, Point& aEnd, xub_StrLen nStart,
       xub_StrLen nWrLen, xub_StrLen nCnt, const BOOL bSwitchH2V, const BOOL bSwitchL2R,
       long nHalfSpace, long* pKernArray, const BOOL bBidiPor);

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFntObj)

    SwFntObj( const SwSubFont &rFont, const void* pOwner,
              ViewShell *pSh );

    virtual ~SwFntObj();

    inline       Font *GetScrFont()     { return pScrFont; }
    inline       Font *GetFont()        { return &aFont; }
    inline const Font *GetFont() const  { return &aFont; }

    inline USHORT GetGuessedLeading() const  { return nGuessedLeading; }
    inline USHORT GetExtLeading() const  { return nExtLeading; }

    USHORT GetFontAscent( const ViewShell *pSh, const OutputDevice& rOut );
    USHORT GetFontHeight( const ViewShell *pSh, const OutputDevice& rOut );
    USHORT GetFontLeading( const ViewShell *pSh, const OutputDevice& rOut );

    void GuessLeading( const ViewShell& rSh, const FontMetric& rMet );

    void SetDevFont( const ViewShell *pSh, OutputDevice& rOut );
    inline OutputDevice* GetPrt() const { return pPrinter; }
    inline USHORT   GetZoom() const { return nZoom; }
    inline USHORT   GetPropWidth() const { return nPropWidth; }
    inline BOOL     IsSymbol() const { return bSymbol; }

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
    SwFntAccess( const void * &rMagic, USHORT &rIndex, const void *pOwner,
                 ViewShell *pShell,
                 BOOL bCheck = FALSE  );
    inline SwFntObj* Get() { return (SwFntObj*) SwCacheAccess::Get(); };
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
