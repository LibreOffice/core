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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_REDLNITR_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_REDLNITR_HXX

#include <ndhints.hxx>

#include <IDocumentRedlineAccess.hxx>

#include <swfont.hxx>

#include <cstddef>
#include <memory>
#include <vector>

class SwTextNode;
class SwDoc;
class SfxItemSet;
class SwAttrHandler;

class SwExtend
{
    std::unique_ptr<SwFont> pFnt;
    const std::vector<ExtTextInputAttr> &rArr;
    sal_Int32 nStart;
    sal_Int32 nPos;
    sal_Int32 nEnd;
    bool Leave_( SwFont& rFnt, sal_Int32 nNew );
    bool Inside() const { return ( nPos >= nStart && nPos < nEnd ); }
    static void ActualizeFont( SwFont &rFnt, ExtTextInputAttr nAttr );
public:
    SwExtend( const std::vector<ExtTextInputAttr> &rA, sal_Int32 nSt ) : rArr( rA ),
        nStart( nSt ), nPos( COMPLETE_STRING ), nEnd( nStart + rA.size() ) {}
    bool IsOn() const { return pFnt != nullptr; }
    void Reset() { pFnt.reset(); nPos = COMPLETE_STRING; }
    bool Leave( SwFont& rFnt, sal_Int32 nNew )
        { return pFnt && Leave_( rFnt, nNew ); }
    short Enter( SwFont& rFnt, sal_Int32 nNew );
    sal_Int32 Next( sal_Int32 nNext );
    SwFont* GetFont()  { return pFnt.get(); }
    void UpdateFont( SwFont &rFnt ) { ActualizeFont( rFnt, rArr[ nPos - nStart ] ); }
};

class SwRedlineItr
{
    std::deque<SwTextAttr *> m_Hints;
    const SwDoc& rDoc;
    SwAttrHandler& rAttrHandler;
    std::unique_ptr<SfxItemSet> pSet;
    SwExtend *pExt;
    sal_uLong nNdIdx;
    sal_Int32 nFirst;
    sal_Int32 nAct;
    sal_Int32 nStart;
    sal_Int32 nEnd;
    bool bOn;
    bool bShow;

    void Clear_( SwFont* pFnt );
    bool ChkSpecialUnderline_() const;
    void FillHints( std::size_t nAuthor, RedlineType_t eType );
    short Seek_( SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld );
    sal_Int32 GetNextRedln_( sal_Int32 nNext );
    short EnterExtend( SwFont& rFnt, sal_Int32 nNew ) {
        if( pExt ) return pExt->Enter( rFnt, nNew );
        return 0;
    }
    sal_Int32 NextExtend( sal_Int32 nNext ) {
        if( pExt ) return pExt->Next( nNext );
        return nNext;
    }
public:
    SwRedlineItr( const SwTextNode& rTextNd, SwFont& rFnt, SwAttrHandler& rAH,
        sal_Int32 nRedlPos, bool bShw, const std::vector<ExtTextInputAttr> *pArr = nullptr,
        sal_Int32 nExtStart = COMPLETE_STRING );
    ~SwRedlineItr() COVERITY_NOEXCEPT_FALSE;
    bool IsOn() const { return bOn || ( pExt && pExt->IsOn() ); }
    void Clear( SwFont* pFnt ) { if( bOn ) Clear_( pFnt ); }
    void ChangeTextAttr( SwFont* pFnt, SwTextAttr const &rHt, bool bChg );
    short Seek( SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld ) {
        if( bShow || pExt ) return Seek_( rFnt, nNew, nOld );
        return 0;
    }
    void Reset() { if( nAct != nFirst ) nAct = COMPLETE_STRING;
                          if( pExt ) pExt->Reset(); }
    sal_Int32 GetNextRedln( sal_Int32 nNext ) {
        if( bShow || pExt ) return GetNextRedln_( nNext );
        return nNext;
    }
    bool ChkSpecialUnderline() const
        { return IsOn() && ChkSpecialUnderline_(); }
    bool CheckLine( sal_Int32 nChkStart, sal_Int32 nChkEnd );
    bool LeaveExtend( SwFont& rFnt, sal_Int32 nNew )
        { return pExt->Leave(rFnt, nNew ); }
    bool ExtOn() {
        if( pExt ) return pExt->IsOn();
        return false;
    }
    void UpdateExtFont( SwFont &rFnt ) {
        OSL_ENSURE( ExtOn(), "UpdateExtFont without ExtOn" );
        pExt->UpdateFont( rFnt ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
