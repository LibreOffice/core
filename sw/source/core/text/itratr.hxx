/*************************************************************************
 *
 *  $RCSfile: itratr.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:56:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <svtools/svstdarr.hxx>

class OutputDevice;
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
    const SwAttrSet* pAttrSet;       // das Char-Attribut-Set
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
        : pFnt(0), pLastOut(0), nChgCnt(0), nPropFont(0), pShell(0), pRedln(0){}

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
    sal_Bool SeekAndChg( const xub_StrLen nPos, OutputDevice* pOut );
    sal_Bool SeekStartAndChg( OutputDevice* pOut, const sal_Bool bParaFont = sal_False );

    // Gibt es ueberhaupt Attributwechsel ?
    inline sal_Bool HasHints() const { return 0 != pHints; }

    // liefert fuer eine Position das Attribut
    SwTxtAttr *GetAttr( const xub_StrLen nPos ) const;

    inline const SwAttrSet* GetAttrSet() const { return pAttrSet; }

    inline const SwpHints *GetHints() const { return pHints; }

    inline SwFont *GetFnt() { return pFnt; }
    inline const SwFont *GetFnt() const { return pFnt; }

    inline const BYTE GetPropFont() const { return nPropFont; }
    inline void SetPropFont( const BYTE nNew ) { nPropFont = nNew; }

    inline SwAttrHandler& GetAttrHandler() { return aAttrHandler; }

#if OSL_DEBUG_LEVEL > 1
    void Dump( SvStream &rOS ) const;
#endif
};

/*************************************************************************
 *                      class SwFontIter
 *
 * A minimal attribute iterator class. In opposite to the other iterators
 * it does not need an SwScriptInfo object.
 *************************************************************************/

class SwFontIter
{
    SwFont aFnt;
    SwAttrHandler& rAttrHandler;
    const SwpHints* pHints;
    xub_StrLen nStartIndex, nEndIndex, nCurrPos, nEndPos;

public:
    // Konstruktor, Destruktor
    // be sure to pass a fully initialised AttrHandler
    // (i.e., it has to have a valid pFnt member )
    SwFontIter( const SwTxtNode& rNode, SwAttrHandler& rAH,
                xub_StrLen nStt, xub_StrLen nEnd );
    ~SwFontIter();

    // returns the next position the font changes and changes the font
    xub_StrLen NextFontChg() const;

    // return the current cont
    const SwFont& GetCurrFont( xub_StrLen nNewPos );
};

#endif
