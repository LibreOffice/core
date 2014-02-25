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

#include "txttypes.hxx"
#include "swfont.hxx"
#include "porlay.hxx"

class OutputDevice;
class SwFont;
class SwpHints;
class SwTxtAttr;
class SwAttrSet;
class SwTxtNode;
class SwRedlineItr;
class SwViewShell;
class SwTxtFrm;

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
    OutputDevice *pLastOut;
    MSHORT nChgCnt;
    SwRedlineItr *pRedln;
    sal_Int32 nStartIndex, nEndIndex, nPos;
    sal_uInt8 nPropFont;
    const void* aMagicNo[ SW_SCRIPTS ];
    MSHORT aFntIdx[ SW_SCRIPTS ];
    const SwTxtNode* m_pTxtNode;

    void SeekFwd( const sal_Int32 nPos );
    void SetFnt( SwFont* pNew ) { pFnt = pNew; }

protected:
    void Chg( SwTxtAttr *pHt );
    void Rst( SwTxtAttr *pHt );
    void CtorInitAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf, SwTxtFrm* pFrm = 0 );
    SwAttrIter(SwTxtNode* pTxtNode)
        : pShell(0)
        , pFnt(0)
        , pHints(0)
        , pAttrSet(0)
        , pScriptInfo(0)
        , pLastOut(0)
        , nChgCnt(0)
        , pRedln(0)
        , nStartIndex(0)
        , nEndIndex(0)
        , nPos(0)
        , nPropFont(0)
        , m_pTxtNode(pTxtNode)
        {
            aMagicNo[SW_LATIN] = aMagicNo[SW_CJK] = aMagicNo[SW_CTL] = NULL;
        }

public:
    // Constructor, destructor
    SwAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf )
        : pShell(0), pFnt(0), pHints(0), pScriptInfo(0), pLastOut(0), nChgCnt(0), pRedln(0),nPropFont(0), m_pTxtNode(&rTxtNode)
        { CtorInitAttrIter( rTxtNode, rScrInf ); }

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
    bool HasHints() const { return 0 != pHints; }

    // Returns the attribute for a position
    SwTxtAttr *GetAttr( const sal_Int32 nPos ) const;

    const SwAttrSet* GetAttrSet() const { return pAttrSet; }

    const SwpHints *GetHints() const { return pHints; }

    SwFont *GetFnt() { return pFnt; }
    const SwFont *GetFnt() const { return pFnt; }

    sal_uInt8 GetPropFont() const { return nPropFont; }
    void SetPropFont( const sal_uInt8 nNew ) { nPropFont = nNew; }

    SwAttrHandler& GetAttrHandler() { return aAttrHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
