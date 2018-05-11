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
    std::unique_ptr<SwFont> m_pFont;
    const std::vector<ExtTextInputAttr> &m_rArr;
    sal_uLong m_nNode;
    sal_Int32 m_nStart;
    sal_Int32 m_nPos;
    sal_Int32 m_nEnd;
    bool Leave_( SwFont& rFnt, sal_Int32 nNew );
    bool Inside() const { return (m_nPos >= m_nStart && m_nPos < m_nEnd); }
    static void ActualizeFont( SwFont &rFnt, ExtTextInputAttr nAttr );
public:
    SwExtend(const std::vector<ExtTextInputAttr> &rArr,
             sal_uLong const nNode, sal_Int32 const nStart)
        : m_rArr(rArr)
        , m_nNode(nNode)
        , m_nStart(nStart)
        , m_nPos(COMPLETE_STRING)
        , m_nEnd(m_nStart + rArr.size())
    {}
    bool IsOn() const { return m_pFont != nullptr; }
    void Reset() { m_pFont.reset(); m_nPos = COMPLETE_STRING; }
    bool Leave( SwFont& rFnt, sal_Int32 nNew )
        { return m_pFont && Leave_( rFnt, nNew ); }
    short Enter( SwFont& rFnt, sal_Int32 nNew );
    sal_Int32 Next( sal_Int32 nNext );
    SwFont* GetFont() { return m_pFont.get(); }
    void UpdateFont(SwFont &rFont) { ActualizeFont(rFont, m_rArr[m_nPos - m_nStart]); }
};

class SwRedlineItr
{
    std::deque<SwTextAttr *> m_Hints;
    const SwDoc& m_rDoc;
    SwAttrHandler& m_rAttrHandler;
    std::unique_ptr<SfxItemSet> m_pSet;
    SwExtend *m_pExt;
    sal_uLong m_nNdIdx;
    SwRedlineTable::size_type const m_nFirst;
    SwRedlineTable::size_type m_nAct;
    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;
    bool m_bOn;
    bool m_bShow;

    void Clear_( SwFont* pFnt );
    bool ChkSpecialUnderline_() const;
    void FillHints( std::size_t nAuthor, RedlineType_t eType );
    short Seek_( SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld );
    short EnterExtend( SwFont& rFnt, sal_Int32 nNew ) {
        if (m_pExt) return m_pExt->Enter( rFnt, nNew );
        return 0;
    }
    sal_Int32 NextExtend( sal_Int32 nNext ) {
        if (m_pExt) return m_pExt->Next( nNext );
        return nNext;
    }
public:
    SwRedlineItr( const SwTextNode& rTextNd, SwFont& rFnt, SwAttrHandler& rAH,
        sal_Int32 nRedlPos, bool bShw, const std::vector<ExtTextInputAttr> *pArr = nullptr,
        SwPosition const* pExtInputStart = nullptr);
    ~SwRedlineItr() COVERITY_NOEXCEPT_FALSE;
    SwRedlineTable::size_type GetAct() const { return m_nAct; }
    bool IsOn() const { return m_bOn || (m_pExt && m_pExt->IsOn()); }
    void Clear( SwFont* pFnt ) { if (m_bOn) Clear_( pFnt ); }
    void ChangeTextAttr( SwFont* pFnt, SwTextAttr const &rHt, bool bChg );
    short Seek( SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld ) {
        if (m_bShow || m_pExt) return Seek_( rFnt, nNew, nOld );
        return 0;
    }
    void Reset() {
        if (m_nAct != m_nFirst) m_nAct = SwRedlineTable::npos;
        if (m_pExt) m_pExt->Reset();
    }
    std::pair<sal_Int32, SwRangeRedline const*> GetNextRedln(sal_Int32 nNext, SwTextNode const* pNode, SwRedlineTable::size_type & rAct);
    bool ChkSpecialUnderline() const
        { return IsOn() && ChkSpecialUnderline_(); }
    bool CheckLine( sal_Int32 nChkStart, sal_Int32 nChkEnd );
    bool LeaveExtend( SwFont& rFnt, sal_Int32 nNew )
        { return m_pExt->Leave(rFnt, nNew ); }
    bool ExtOn() {
        if (m_pExt) return m_pExt->IsOn();
        return false;
    }
    void UpdateExtFont( SwFont &rFnt ) {
        OSL_ENSURE( ExtOn(), "UpdateExtFont without ExtOn" );
        m_pExt->UpdateFont( rFnt );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
