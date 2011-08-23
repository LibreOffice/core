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

#ifndef	_REDLNITR_HXX
#define	_REDLNITR_HXX

#include "ndhints.hxx"
#include "redlenum.hxx"	// SwRedlineType
#include "swfont.hxx"
#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {
class SfxItemSet; 

class SwTxtNode;
class SwDoc;

class SwAttrHandler;

class SwExtend
{
    SwFont *pFnt;
    const SvUShorts &rArr;	// XAMA: Array of xub_StrLen
    xub_StrLen nStart;
    xub_StrLen nPos;
    xub_StrLen nEnd;
    sal_Bool _Leave( SwFont& rFnt, xub_StrLen nNew );
    sal_Bool Inside() const { return ( nPos >= nStart && nPos < nEnd ); }
    void ActualizeFont( SwFont &rFnt, xub_StrLen nAttr );
public:
    SwExtend( const SvUShorts &rA, xub_StrLen nSt ) : rArr( rA ), pFnt(0),
        nStart( nSt ), nPos( STRING_LEN ), nEnd( nStart + rA.Count() ) {}
    ~SwExtend() { delete pFnt; }
    sal_Bool IsOn() const { return pFnt != 0; }
    void Reset() { if( pFnt ) { delete pFnt; pFnt = NULL; } nPos = STRING_LEN; }
    sal_Bool Leave( SwFont& rFnt, xub_StrLen nNew )
        { if( pFnt ) return _Leave( rFnt, nNew ); return sal_False; }
    short Enter( SwFont& rFnt, xub_StrLen nNew );
    xub_StrLen Next( xub_StrLen nNext );
    SwFont* GetFont()  { return pFnt; }
    void UpdateFont( SwFont &rFnt ) { ActualizeFont( rFnt, rArr[ nPos - nStart ] ); }
};

class SwRedlineItr
{
    SwExtend *pExt;
    sal_Bool bOn;
public:
SwRedlineItr( const SwTxtNode& rTxtNd, SwFont& rFnt, SwAttrHandler& rAH,//STRIP001 SwRedlineItr( const SwTxtNode& rTxtNd, SwFont& rFnt, SwAttrHandler& rAH,
xub_StrLen nRedlPos, sal_Bool bShw, const SvUShorts *pArr = 0,//STRIP001         xub_StrLen nRedlPos, sal_Bool bShw, const SvUShorts *pArr = 0,
xub_StrLen nStart = STRING_LEN ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001        xub_StrLen nStart = STRING_LEN );
    inline sal_Bool IsOn() const { return bOn || ( pExt && pExt->IsOn() ); }
            sal_Bool CheckLine( xub_StrLen nChkStart, xub_StrLen nChkEnd ){DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 	sal_Bool CheckLine( xub_StrLen nChkStart, xub_StrLen nChkEnd );
    inline sal_Bool ExtOn() { if( pExt ) return pExt->IsOn(); return sal_False; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
