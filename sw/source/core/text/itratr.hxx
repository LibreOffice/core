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
#ifndef _ITRATR_HXX
#define _ITRATR_HXX
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
class ViewShell;
class SwTxtFrm;

/*************************************************************************
 *                      class SwAttrIter
 *************************************************************************/

class SwAttrIter
{
    friend class SwFontSave;
protected:

    SwAttrHandler aAttrHandler;
    ViewShell *pShell;
    SwFont *pFnt;
    SwpHints  *pHints;
    const SwAttrSet* pAttrSet;       // The char attribute set
    SwScriptInfo* pScriptInfo;

private:
    OutputDevice *pLastOut;
    MSHORT nChgCnt;
    SwRedlineItr *pRedln;
    xub_StrLen nStartIndex, nEndIndex, nPos;
    sal_uInt8 nPropFont;
    void SeekFwd( const xub_StrLen nPos );
    inline void SetFnt( SwFont* pNew ) { pFnt = pNew; }
    const void* aMagicNo[ SW_SCRIPTS ];
    MSHORT aFntIdx[ SW_SCRIPTS ];
    const SwTxtNode* m_pTxtNode;

protected:
    void Chg( SwTxtAttr *pHt );
    void Rst( SwTxtAttr *pHt );
    void CtorInitAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf, SwTxtFrm* pFrm = 0 );
    inline SwAttrIter(SwTxtNode* pTxtNode)
        : pShell(0), pFnt(0), pHints(0), pAttrSet(0), pScriptInfo(0), pLastOut(0), nChgCnt(0), pRedln(0), nPropFont(0), m_pTxtNode(pTxtNode) {
            aMagicNo[SW_LATIN] = aMagicNo[SW_CJK] = aMagicNo[SW_CTL] = NULL;
        }

    /// Implementation of the considering public methods (to avoid recursion)
    sal_Bool ImplSeekAndChgAttrIter( const xub_StrLen nNewPos, OutputDevice* pOut );
    sal_Bool ImplSeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont );
public:
    // Constructor, destructor
    inline SwAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf )
        : pShell(0), pFnt(0), pHints(0), pScriptInfo(0), pLastOut(0), nChgCnt(0), pRedln(0),nPropFont(0), m_pTxtNode(&rTxtNode)
        { CtorInitAttrIter( rTxtNode, rScrInf ); }

    virtual ~SwAttrIter();

    inline SwRedlineItr *GetRedln() { return pRedln; }
    // The parameter returns the position of the next change before or at the
    // char position.
    xub_StrLen GetNextAttr( ) const;
    /// Enables the attributes used at char pos nPos in the logical font
    sal_Bool Seek( const xub_StrLen nPos );
    // Creates the font at the specified position via Seek() and checks
    // if it's a symbol font.
    sal_Bool IsSymbol( const xub_StrLen nPos );

    /** Executes ChgPhysFnt if Seek() returns sal_True
     *  and change font to merge character border with neighbours.
    **/
    sal_Bool SeekAndChgAttrIter( const xub_StrLen nPos, OutputDevice* pOut );
    sal_Bool SeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont = sal_False );

    /** Merge character border with removing left/right border of the font if the
     *  the neighbours of the current position (nPos) has the same height
     *  and same kind of border.
     *  @param      bStart  true if it is called from SeekStartAndChgAttrIter
     *                      false, otherwise
     *  @return     true,   if font change (removing some of its borders)
     *              false,  otherwise
    **/
    bool MergeCharBorder( const bool bStart );

    // Do we have an attribute change at all?
    inline sal_Bool HasHints() const { return 0 != pHints; }

    // Returns the attribute for a position
    SwTxtAttr *GetAttr( const xub_StrLen nPos ) const;

    inline const SwAttrSet* GetAttrSet() const { return pAttrSet; }

    inline const SwpHints *GetHints() const { return pHints; }

    inline SwFont *GetFnt() { return pFnt; }
    inline const SwFont *GetFnt() const { return pFnt; }

    inline sal_uInt8 GetPropFont() const { return nPropFont; }
    inline void SetPropFont( const sal_uInt8 nNew ) { nPropFont = nNew; }

    inline SwAttrHandler& GetAttrHandler() { return aAttrHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
