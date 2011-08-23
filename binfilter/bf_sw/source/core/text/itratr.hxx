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


#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _ATRHNDL_HXX
#include <atrhndl.hxx>
#endif

#include "txttypes.hxx"
#include "swfont.hxx"
#include "porlay.hxx"

#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
class OutputDevice; 
namespace binfilter {


class SwFont;
class SwpHints;
class SwTxtAttr;
class SwAttrSet;
class SwTxtNode;
class SwRedlineItr;
class ViewShell;

#ifdef VERTICAL_LAYOUT
class SwTxtFrm;
#endif

/*************************************************************************
 *						class SwAttrIter
 *************************************************************************/

class SwAttrIter
{
    friend class SwFontSave;
protected:

    SwAttrHandler aAttrHandler;
    ViewShell *pShell;
    SwFont *pFnt;
    SwpHints  *pHints;
    const SwAttrSet* pAttrSet;		 // das Char-Attribut-Set
    SwScriptInfo* pScriptInfo;

private:
    OutputDevice *pLastOut;
    MSHORT nChgCnt;
    SwRedlineItr *pRedln;
    xub_StrLen nStartIndex, nEndIndex, nPos;
    BYTE nPropFont;
    void SeekFwd( const xub_StrLen nPos );
    inline void SetFnt( SwFont* pNew ) { pFnt = pNew; }
    const void* aMagicNo[ SW_SCRIPTS ];
    MSHORT aFntIdx[ SW_SCRIPTS ];

protected:
    void Chg( SwTxtAttr *pHt );
    void Rst( SwTxtAttr *pHt );
#ifdef VERTICAL_LAYOUT
    void CtorInit( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf, SwTxtFrm* pFrm = 0 );
#else
    void CtorInit( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf );
#endif
    inline SwAttrIter()
        : pFnt(0), pLastOut(0),	nChgCnt(0), nPropFont(0), pShell(0), pRedln(0){}

public:
    // Konstruktor, Destruktor
    inline SwAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf )
        : pFnt(0), pLastOut(0), nChgCnt(0), nPropFont(0), pShell(0), pRedln(0)
        { CtorInit( rTxtNode, rScrInf ); }

    virtual ~SwAttrIter();

    inline SwRedlineItr *GetRedln() { return pRedln; }
    // Liefert im Parameter die Position des naechsten Wechsels vor oder an
    // der uebergebenen Characterposition zurueck. Liefert sal_False, wenn vor
    // oder an dieser Position kein Wechsel mehr erfolgt, sal_True sonst.
    xub_StrLen GetNextAttr( ) const;
    // Macht die an der Characterposition i gueltigen Attribute im
    // logischen Font wirksam.
    sal_Bool Seek( const xub_StrLen nPos );
    // Bastelt den Font an der gew. Position via Seek und fragt ihn,
    // ob er ein Symbolfont ist.
    sal_Bool IsSymbol( const xub_StrLen nPos );

    // Fuehrt ChgPhysFnt aus, wenn Seek() sal_True zurueckliefert.
    sal_Bool SeekAndChg( const xub_StrLen nPos, OutputDevice *pOut );
    sal_Bool SeekStartAndChg( OutputDevice *pOut, const sal_Bool bParaFont = sal_False );

    // Gibt es ueberhaupt Attributwechsel ?
    inline sal_Bool HasHints() const { return 0 != pHints; }

    // liefert fuer eine Position das Attribut
    SwTxtAttr *GetAttr( const xub_StrLen nPos ) const;

    inline const SwAttrSet* GetAttrSet() const { return pAttrSet; }

    inline const SwpHints *GetHints() const { return pHints; }

    inline SwFont *GetFnt() { return pFnt; }
    inline const SwFont *GetFnt() const { return pFnt; }

    inline BYTE GetPropFont() const { return nPropFont; }
    inline void SetPropFont( const BYTE nNew ) { nPropFont = nNew; }

    inline SwAttrHandler& GetAttrHandler() { return aAttrHandler; }
};

} //namespace binfilter
#endif
