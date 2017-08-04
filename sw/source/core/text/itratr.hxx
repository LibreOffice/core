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
#include <atrhndl.hxx>

#include "swtypes.hxx"
#include "swfont.hxx"
#include "porlay.hxx"

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
    SwFont *m_pFont;
    SwpHints  *m_pHints;
    const SwAttrSet* m_pAttrSet;       // The char attribute set
    SwScriptInfo* m_pScriptInfo;

private:
    VclPtr<OutputDevice> m_pLastOut;
    short m_nChgCnt;
    SwRedlineItr *m_pRedline;
    size_t m_nStartIndex;
    size_t m_nEndIndex;
    sal_Int32 m_nPosition;
    sal_uInt8 m_nPropFont;
    o3tl::enumarray<SwFontScript, const void*> m_aMagicNo;
    o3tl::enumarray<SwFontScript, sal_uInt16> m_aFontIdx;
    const SwTextNode* m_pTextNode;

    void SeekFwd( const sal_Int32 nPos );
    void SetFnt( SwFont* pNew ) { m_pFont = pNew; }

protected:
    void Chg( SwTextAttr const *pHt );
    void Rst( SwTextAttr const *pHt );
    void CtorInitAttrIter( SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame const * pFrame = nullptr );
    explicit SwAttrIter(SwTextNode const * pTextNode)
        : m_pViewShell(nullptr)
        , m_pFont(nullptr)
        , m_pHints(nullptr)
        , m_pAttrSet(nullptr)
        , m_pScriptInfo(nullptr)
        , m_pLastOut(nullptr)
        , m_nChgCnt(0)
        , m_pRedline(nullptr)
        , m_nStartIndex(0)
        , m_nEndIndex(0)
        , m_nPosition(0)
        , m_nPropFont(0)
        , m_pTextNode(pTextNode)
        {
            m_aMagicNo[SwFontScript::Latin] = m_aMagicNo[SwFontScript::CJK] = m_aMagicNo[SwFontScript::CTL] = nullptr;
        }

public:
    // Constructor, destructor
    SwAttrIter( SwTextNode& rTextNode, SwScriptInfo& rScrInf )
        : m_pViewShell(nullptr), m_pFont(nullptr), m_pHints(nullptr), m_pScriptInfo(nullptr), m_pLastOut(nullptr), m_nChgCnt(0), m_pRedline(nullptr),m_nPropFont(0), m_pTextNode(&rTextNode)
        { CtorInitAttrIter( rTextNode, rScrInf ); }

    virtual ~SwAttrIter();

    SwRedlineItr *GetRedln() { return m_pRedline; }
    // The parameter returns the position of the next change before or at the
    // char position.
    sal_Int32 GetNextAttr( ) const;
    /// Enables the attributes used at char pos nPos in the logical font
    bool Seek( const sal_Int32 nPos );
    // Creates the font at the specified position via Seek() and checks
    // if it's a symbol font.
    bool IsSymbol( const sal_Int32 nPos );

    /** Executes ChgPhysFnt if Seek() returns true
     *  and change font to merge character border with neighbours.
    **/
    bool SeekAndChgAttrIter( const sal_Int32 nPos, OutputDevice* pOut );
    bool SeekStartAndChgAttrIter( OutputDevice* pOut, const bool bParaFont );

    // Do we have an attribute change at all?
    bool HasHints() const { return nullptr != m_pHints; }

    // Returns the attribute for a position
    SwTextAttr *GetAttr( const sal_Int32 nPos ) const;

    const SwpHints *GetHints() const { return m_pHints; }

    SwFont *GetFnt() { return m_pFont; }
    const SwFont *GetFnt() const { return m_pFont; }

    sal_uInt8 GetPropFont() const { return m_nPropFont; }
    void SetPropFont( const sal_uInt8 nNew ) { m_nPropFont = nNew; }

    SwAttrHandler& GetAttrHandler() { return m_aAttrHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
