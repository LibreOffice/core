/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _ITRATR_HXX
#define _ITRATR_HXX
#include <atrhndl.hxx>

#include "txttypes.hxx"
#include "swfont.hxx"
#include "porlay.hxx"

#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_USHORTS
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
    const SwAttrSet* pAttrSet;       // das Char-Attribut-Set
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
        : pShell(0), pFnt(0), pLastOut(0),  nChgCnt(0), pRedln(0), nPropFont(0), m_pTxtNode(pTxtNode) {}

public:
    // Konstruktor, Destruktor
    inline SwAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf )
        : pShell(0), pFnt(0), pHints(0), pScriptInfo(0), pLastOut(0), nChgCnt(0), pRedln(0),nPropFont(0), m_pTxtNode(&rTxtNode)
        { CtorInitAttrIter( rTxtNode, rScrInf ); }

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
    sal_Bool SeekAndChgAttrIter( const xub_StrLen nPos, OutputDevice* pOut );
    sal_Bool SeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont = sal_False );

    // Gibt es ueberhaupt Attributwechsel ?
    inline sal_Bool HasHints() const { return 0 != pHints; }

    // liefert fuer eine Position das Attribut
    SwTxtAttr *GetAttr( const xub_StrLen nPos ) const;

    inline const SwAttrSet* GetAttrSet() const { return pAttrSet; }

    inline const SwpHints *GetHints() const { return pHints; }

    inline SwFont *GetFnt() { return pFnt; }
    inline const SwFont *GetFnt() const { return pFnt; }

    inline sal_uInt8 GetPropFont() const { return nPropFont; }
    inline void SetPropFont( const sal_uInt8 nNew ) { nPropFont = nNew; }

    inline SwAttrHandler& GetAttrHandler() { return aAttrHandler; }

#if OSL_DEBUG_LEVEL > 1
    void Dump( SvStream &rOS ) const;
#endif
};

#endif
