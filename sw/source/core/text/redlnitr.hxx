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
#ifndef _REDLNITR_HXX
#define _REDLNITR_HXX

#include "ndhints.hxx"

#include <IDocumentRedlineAccess.hxx>

#include "swfont.hxx"
#include <vector>

class SwTxtNode;
class SwDoc;
class SfxItemSet;
class SwAttrHandler;

class SwExtend
{
    SwFont *pFnt;
    const std::vector<sal_uInt16> &rArr;    // XAMA: Array of xub_StrLen
    xub_StrLen nStart;
    xub_StrLen nPos;
    xub_StrLen nEnd;
    sal_Bool _Leave( SwFont& rFnt, xub_StrLen nNew );
    sal_Bool Inside() const { return ( nPos >= nStart && nPos < nEnd ); }
    void ActualizeFont( SwFont &rFnt, xub_StrLen nAttr );
public:
    SwExtend( const std::vector<sal_uInt16> &rA, xub_StrLen nSt ) : pFnt(0), rArr( rA ),
        nStart( nSt ), nPos( STRING_LEN ), nEnd( nStart + rA.size() ) {}
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
    std::deque<SwTxtAttr *> m_Hints;
    const SwDoc& rDoc;
    SwAttrHandler& rAttrHandler;
    SfxItemSet *pSet;
    SwExtend *pExt;
    sal_uLong nNdIdx;
    xub_StrLen nFirst;
    xub_StrLen nAct;
    xub_StrLen nStart;
    xub_StrLen nEnd;
    sal_Bool bOn;
    sal_Bool bShow;

    void _Clear( SwFont* pFnt );
    sal_Bool _ChkSpecialUnderline() const;
    void FillHints( MSHORT nAuthor, RedlineType_t eType );
    short _Seek( SwFont& rFnt, xub_StrLen nNew, xub_StrLen nOld );
    xub_StrLen _GetNextRedln( xub_StrLen nNext );
    inline short EnterExtend( SwFont& rFnt, xub_StrLen nNew )
        { if( pExt ) return pExt->Enter( rFnt, nNew ); return 0; }
    inline xub_StrLen NextExtend( xub_StrLen nNext )
        { if( pExt ) return pExt->Next( nNext ); return nNext; }
public:
    SwRedlineItr( const SwTxtNode& rTxtNd, SwFont& rFnt, SwAttrHandler& rAH,
        xub_StrLen nRedlPos, sal_Bool bShw, const std::vector<sal_uInt16> *pArr = 0,
        xub_StrLen nStart = STRING_LEN );
    ~SwRedlineItr();
    inline sal_Bool IsOn() const { return bOn || ( pExt && pExt->IsOn() ); }
    inline void Clear( SwFont* pFnt ) { if( bOn ) _Clear( pFnt ); }
    void ChangeTxtAttr( SwFont* pFnt, SwTxtAttr &rHt, sal_Bool bChg );
    inline short Seek( SwFont& rFnt, xub_StrLen nNew, xub_StrLen nOld )
        { if( bShow || pExt ) return _Seek( rFnt, nNew, nOld ); return 0; }
    inline void Reset() { if( nAct != nFirst ) nAct = STRING_LEN;
                          if( pExt ) pExt->Reset(); }
    inline xub_StrLen GetNextRedln( xub_StrLen nNext )
        { if( bShow || pExt ) return _GetNextRedln( nNext ); return nNext; }
    inline sal_Bool ChkSpecialUnderline() const
        { if ( IsOn() ) return _ChkSpecialUnderline(); return sal_False; }
    sal_Bool CheckLine( xub_StrLen nChkStart, xub_StrLen nChkEnd );
    inline sal_Bool LeaveExtend( SwFont& rFnt, xub_StrLen nNew )
        { return pExt->Leave(rFnt, nNew ); }
    inline sal_Bool ExtOn() { if( pExt ) return pExt->IsOn(); return sal_False; }
    inline void UpdateExtFont( SwFont &rFnt ) {
        OSL_ENSURE( ExtOn(), "UpdateExtFont without ExtOn" );
        pExt->UpdateFont( rFnt ); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
