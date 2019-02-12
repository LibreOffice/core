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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_ITRATR_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_ITRATR_HXX
#include <o3tl/deleter.hxx>
#include "atrhndl.hxx"
#include <swtypes.hxx>
#include <swfont.hxx>
#include "porlay.hxx"

namespace sw { struct MergedPara; }
class OutputDevice;
class SwFont;
class SwpHints;
class SwTextAttr;
class SwAttrSet;
class SwTextNode;
class SwRedlineItr;
class SwViewShell;
class SwTextFrame;

class SwAttrIter
{
    friend class SwFontSave;
protected:

    SwAttrHandler m_aAttrHandler;
    SwViewShell *m_pViewShell;
    SwFont* m_pFont;
    SwScriptInfo* m_pScriptInfo;

private:
    VclPtr<OutputDevice> m_pLastOut;
    /// count currently open hints, redlines, ext-input
    short m_nChgCnt;
    std::unique_ptr<SwRedlineItr, o3tl::default_delete<SwRedlineItr>> m_pRedline;
    /// current iteration index in HintStarts
    size_t m_nStartIndex;
    /// current iteration index in HintEnds
    size_t m_nEndIndex;
    /// current iteration index in text node
    sal_Int32 m_nPosition;
    sal_uInt8 m_nPropFont;
    o3tl::enumarray<SwFontScript, const void*> m_aFontCacheIds;
    o3tl::enumarray<SwFontScript, sal_uInt16> m_aFontIdx;
    /// input: the current text node
    const SwTextNode* m_pTextNode;
    sw::MergedPara const* m_pMergedPara;

    void SeekFwd(sal_Int32 nOldPos, sal_Int32 nNewPos);
    void SetFnt( SwFont* pNew ) { m_pFont = pNew; }
    void InitFontAndAttrHandler(
        SwTextNode const& rPropsNode, SwTextNode const& rTextNode,
        OUString const& rText, bool const* pbVertLayout,
        bool const* pbVertLayoutLRBT);

protected:
    void Chg( SwTextAttr const *pHt );
    void Rst( SwTextAttr const *pHt );
    void CtorInitAttrIter(SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame const* pFrame = nullptr);
    explicit SwAttrIter(SwTextNode const * pTextNode);

public:
    /// All subclasses of this always have a SwTextFrame passed to the
    /// constructor, but SwAttrIter itself may be created without a
    /// SwTextFrame in certain special cases via this ctor here
    SwAttrIter(SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame const*const pFrame = nullptr);

    virtual ~SwAttrIter();

    SwRedlineItr *GetRedln() { return m_pRedline.get(); }
    // The parameter returns the position of the next change before or at the
    // char position.
    TextFrameIndex GetNextAttr() const;
    /// Enables the attributes used at char pos nPos in the logical font
    bool Seek(TextFrameIndex nPos);
    // Creates the font at the specified position via Seek() and checks
    // if it's a symbol font.
    bool IsSymbol(TextFrameIndex nPos);

    /** Executes ChgPhysFnt if Seek() returns true
     *  and change font to merge character border with neighbours.
    **/
    bool SeekAndChgAttrIter(TextFrameIndex nPos, OutputDevice* pOut);
    bool SeekStartAndChgAttrIter( OutputDevice* pOut, const bool bParaFont );

    // Do we possibly have nasty things like footnotes?
    bool MaybeHasHints() const;

    // Returns the attribute for a position
    SwTextAttr *GetAttr(TextFrameIndex nPos) const;

    SwFont *GetFnt() { return m_pFont; }
    const SwFont *GetFnt() const { return m_pFont; }

    sal_uInt8 GetPropFont() const { return m_nPropFont; }
    void SetPropFont( const sal_uInt8 nNew ) { m_nPropFont = nNew; }

    SwAttrHandler& GetAttrHandler() { return m_aAttrHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
