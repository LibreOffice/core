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

#include <IDocumentRedlineAccess.hxx>

#include <swfont.hxx>

#include <vcl/commandevent.hxx>

#include <cstddef>
#include <deque>
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
    /// position of start of SwExtTextInput
    sal_uLong const m_nNode;
    sal_Int32 const m_nStart;
    /// current position (inside)
    sal_Int32 m_nPos;
    /// position of end of SwExtTextInput (in same node as start)
    sal_Int32 const m_nEnd;
    bool Leave_(SwFont& rFnt, sal_uLong nNode, sal_Int32 nNew);
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
    bool Leave(SwFont& rFnt, sal_uLong const nNode, sal_Int32 const nNew)
        { return m_pFont && Leave_(rFnt, nNode, nNew); }
    short Enter(SwFont& rFnt, sal_uLong nNode, sal_Int32 nNew);
    sal_Int32 Next(sal_uLong nNode, sal_Int32 nNext);
    SwFont* GetFont() { return m_pFont.get(); }
    void UpdateFont(SwFont &rFont) { ActualizeFont(rFont, m_rArr[m_nPos - m_nStart]); }
};

class SwRedlineItr
{
    std::deque<SwTextAttr *> m_Hints;
    const SwDoc& m_rDoc;
    SwAttrHandler& m_rAttrHandler;
    std::unique_ptr<SfxItemSet> m_pSet;
    std::unique_ptr<SwExtend> m_pExt;
    // note: this isn't actually used in the merged-para (Hide) case
    sal_uLong const m_nNdIdx;
    SwRedlineTable::size_type const m_nFirst;
    SwRedlineTable::size_type m_nAct;
    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;
    bool m_bOn;
public:
    enum class Mode { Show, Ignore, Hide };
private:
    Mode const m_eMode;

    void Clear_( SwFont* pFnt );
    bool ChkSpecialUnderline_() const;
    void FillHints( std::size_t nAuthor, RedlineType eType );
    short EnterExtend(SwFont& rFnt, sal_uLong const nNode, sal_Int32 const nNew)
    {
        if (m_pExt) return m_pExt->Enter(rFnt, nNode, nNew);
        return 0;
    }
    sal_Int32 NextExtend(sal_uLong const nNode, sal_Int32 const nNext) {
        if (m_pExt) return m_pExt->Next(nNode, nNext);
        return nNext;
    }
public:
    SwRedlineItr( const SwTextNode& rTextNd, SwFont& rFnt, SwAttrHandler& rAH,
        sal_Int32 nRedlPos, Mode mode,
        const std::vector<ExtTextInputAttr> *pArr = nullptr,
        SwPosition const* pExtInputStart = nullptr);
    ~SwRedlineItr() COVERITY_NOEXCEPT_FALSE;
    SwRedlineTable::size_type GetAct() const { return m_nAct; }
    bool IsOn() const { return m_bOn || (m_pExt && m_pExt->IsOn()); }
    void Clear( SwFont* pFnt ) { if (m_bOn) Clear_( pFnt ); }
    void ChangeTextAttr( SwFont* pFnt, SwTextAttr const &rHt, bool bChg );
    short Seek(SwFont& rFnt, sal_uLong nNode, sal_Int32 nNew, sal_Int32 nOld);
    void Reset() {
        if (m_nAct != m_nFirst) m_nAct = SwRedlineTable::npos;
        if (m_pExt) m_pExt->Reset();
    }
    std::pair<sal_Int32, std::pair<SwRangeRedline const*, size_t>> GetNextRedln(
        sal_Int32 nNext, SwTextNode const* pNode, SwRedlineTable::size_type & rAct);
    bool ChkSpecialUnderline() const
        { return IsOn() && ChkSpecialUnderline_(); }
    bool CheckLine(sal_uLong nStartNode, sal_Int32 nChkStart, sal_uLong nEndNode,
        sal_Int32 nChkEnd, OUString& rRedlineText, bool& bRedlineEnd,
        RedlineType& eRedlineEnd, size_t* pAuthorAtPos = nullptr);
    bool LeaveExtend(SwFont& rFnt, sal_uLong const nNode, sal_Int32 const nNew)
        { return m_pExt->Leave(rFnt, nNode, nNew); }
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
