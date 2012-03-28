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
#ifndef _ITRATR_HXX
#define _ITRATR_HXX
#include <atrhndl.hxx>

#include "txttypes.hxx"
#include "swfont.hxx"
#include "porlay.hxx"

#include <svl/svstdarr.hxx>

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

public:
    // Constructor, destructor
    inline SwAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf )
        : pShell(0), pFnt(0), pHints(0), pScriptInfo(0), pLastOut(0), nChgCnt(0), pRedln(0),nPropFont(0), m_pTxtNode(&rTxtNode)
        { CtorInitAttrIter( rTxtNode, rScrInf ); }

    virtual ~SwAttrIter();

    inline SwRedlineItr *GetRedln() { return pRedln; }
    // The parameter returns the position of the next change before or at the
    // char position.
    // Returns sal_False, if there's no change before or at the positon,
    // else sal_True.
    xub_StrLen GetNextAttr( ) const;
    // Enables the attributes used at char pos nPos in the logical font
    sal_Bool Seek( const xub_StrLen nPos );
    // Creates the font at the specified position via Seek() and checks
    // if it's a symbol font.
    sal_Bool IsSymbol( const xub_StrLen nPos );

    // Executes ChgPhysFnt if Seek() returns sal_True
    sal_Bool SeekAndChgAttrIter( const xub_StrLen nPos, OutputDevice* pOut );
    sal_Bool SeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont = sal_False );

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

#ifdef DBG_UTIL
    void Dump( SvStream &rOS ) const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
