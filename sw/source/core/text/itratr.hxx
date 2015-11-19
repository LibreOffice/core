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

    SwAttrHandler aAttrHandler;
    SwViewShell *pShell;
    SwFont *pFnt;
    SwpHints  *pHints;
    const SwAttrSet* pAttrSet;       // The char attribute set
    SwScriptInfo* pScriptInfo;

private:
    VclPtr<OutputDevice> pLastOut;
    short nChgCnt;
    SwRedlineItr *pRedln;
    size_t nStartIndex;
    size_t nEndIndex;
    sal_Int32 nPos;
    sal_uInt8 nPropFont;
    const void* aMagicNo[ SW_SCRIPTS ];
    sal_uInt16 aFntIdx[ SW_SCRIPTS ];
    const SwTextNode* m_pTextNode;

    void SeekFwd( const sal_Int32 nPos );
    void SetFnt( SwFont* pNew ) { pFnt = pNew; }

protected:
    void Chg( SwTextAttr *pHt );
    void Rst( SwTextAttr *pHt );
    void CtorInitAttrIter( SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame* pFrame = nullptr );
    explicit SwAttrIter(SwTextNode* pTextNode)
        : pShell(nullptr)
        , pFnt(nullptr)
        , pHints(nullptr)
        , pAttrSet(nullptr)
        , pScriptInfo(nullptr)
        , pLastOut(nullptr)
        , nChgCnt(0)
        , pRedln(nullptr)
        , nStartIndex(0)
        , nEndIndex(0)
        , nPos(0)
        , nPropFont(0)
        , m_pTextNode(pTextNode)
        {
            aMagicNo[SW_LATIN] = aMagicNo[SW_CJK] = aMagicNo[SW_CTL] = nullptr;
        }

public:
    // Constructor, destructor
    SwAttrIter( SwTextNode& rTextNode, SwScriptInfo& rScrInf )
        : pShell(nullptr), pFnt(nullptr), pHints(nullptr), pScriptInfo(nullptr), pLastOut(nullptr), nChgCnt(0), pRedln(nullptr),nPropFont(0), m_pTextNode(&rTextNode)
        { CtorInitAttrIter( rTextNode, rScrInf ); }

    virtual ~SwAttrIter();

    SwRedlineItr *GetRedln() { return pRedln; }
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
    bool SeekStartAndChgAttrIter( OutputDevice* pOut, const bool bParaFont = false );

    // Do we have an attribute change at all?
    bool HasHints() const { return nullptr != pHints; }

    // Returns the attribute for a position
    SwTextAttr *GetAttr( const sal_Int32 nPos ) const;

    const SwpHints *GetHints() const { return pHints; }

    SwFont *GetFnt() { return pFnt; }
    const SwFont *GetFnt() const { return pFnt; }

    sal_uInt8 GetPropFont() const { return nPropFont; }
    void SetPropFont( const sal_uInt8 nNew ) { nPropFont = nNew; }

    SwAttrHandler& GetAttrHandler() { return aAttrHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
