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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FNTCACHE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FNTCACHE_HXX

#include <vcl/font.hxx>
#include <vcl/vclptr.hxx>
#include <tools/mempool.hxx>
#include <swtypes.hxx>
#include "swcache.hxx"

class OutputDevice;
class FontMetric;
class SwFntObj;
class SwDrawTextInfo;
class SwViewShell;
class SwSubFont;
class MapMode;
class Point;

class SwFntCache : public SwCache
{
public:
    SwFntCache() : SwCache(50
#ifdef DBG_UTIL
    , OString(RTL_CONSTASCII_STRINGPARAM("Global Font-Cache pFntCache"))
#endif
    ) {}

    inline SwFntObj *First( );
    static inline SwFntObj *Next( SwFntObj *pFntObj);
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
    friend void InitCore();
    friend void FinitCore();

    vcl::Font m_aFont;
    vcl::Font *m_pScrFont;
    vcl::Font *m_pPrtFont;
    VclPtr<OutputDevice> m_pPrinter;
    sal_uInt16 m_nGuessedLeading;
    sal_uInt16 m_nExtLeading;
    sal_uInt16 m_nScrAscent;
    sal_uInt16 m_nPrtAscent;
    sal_uInt16 m_nScrHeight;
    sal_uInt16 m_nPrtHeight;
    sal_uInt16 m_nPropWidth;
    sal_uInt16 m_nZoom;
    bool m_bSymbol : 1;
    bool m_bPaintBlank : 1;

    static long nPixWidth;
    static MapMode *pPixMap;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFntObj)

    SwFntObj( const SwSubFont &rFont, const void* pOwner,
              SwViewShell const *pSh );

    virtual ~SwFntObj() override;

    vcl::Font *GetScrFont()     { return m_pScrFont; }
    vcl::Font& GetFont()        { return m_aFont; }
    const vcl::Font& GetFont() const  { return m_aFont; }

    sal_uInt16 GetGuessedLeading() const  { return m_nGuessedLeading; }
    sal_uInt16 GetExternalLeading() const  { return m_nExtLeading; }

    sal_uInt16 GetFontAscent( const SwViewShell *pSh, const OutputDevice& rOut );
    sal_uInt16 GetFontHeight( const SwViewShell *pSh, const OutputDevice& rOut );
    sal_uInt16 GetFontLeading( const SwViewShell *pSh, const OutputDevice& rOut );

    void GuessLeading( const SwViewShell& rSh, const FontMetric& rMet );

    void SetDevFont( const SwViewShell *pSh, OutputDevice& rOut );
    OutputDevice* GetPrt() const { return m_pPrinter; }
    sal_uInt16   GetZoom() const { return m_nZoom; }
    sal_uInt16   GetPropWidth() const { return m_nPropWidth; }
    bool     IsSymbol() const { return m_bSymbol; }

    void   DrawText( SwDrawTextInfo &rInf );
    /// determine the TextSize (of the printer)
    Size  GetTextSize( SwDrawTextInfo &rInf );
    sal_Int32 GetCursorOfst( SwDrawTextInfo &rInf );

    void CreateScrFont( const SwViewShell& rSh, const OutputDevice& rOut );
    void CreatePrtFont( const OutputDevice& rOut );
};

SwFntObj *SwFntCache::First( )
{
    return static_cast<SwFntObj *>(SwCache::First());
}

SwFntObj *SwFntCache::Next( SwFntObj *pFntObj)
{
    return static_cast<SwFntObj *>(SwCache::Next( pFntObj ));
}

class SwFntAccess : public SwCacheAccess
{
    SwViewShell const *pShell;
protected:
    virtual SwCacheObj *NewObj( ) override;

public:
    SwFntAccess( const void * &rMagic, sal_uInt16 &rIndex, const void *pOwner,
                 SwViewShell const *pShell,
                 bool bCheck = false  );
    SwFntObj* Get() { return static_cast<SwFntObj*>( SwCacheAccess::Get() ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
