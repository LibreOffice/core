/*************************************************************************
 *
 *  $RCSfile: root.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: gt $ $Date: 2001-02-20 15:23:42 $
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

#ifndef _ROOT_HXX
#define _ROOT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef SC_SCGLOB_HXX
#include <global.hxx>
#endif

#ifndef _FLTTYPES_HXX
#include "flttypes.hxx"
#endif
#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

class SvStorage;
class SvNumberFormatter;
class ScRangeName;

class ColorBuffer;
class NameBuffer;
class RangeNameBuffer;
class RangeNameBufferWK3;
class ShrfmlaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class FontBuffer;
class ExcExternDup;
class ScExtDocOptions;
class ScEditEngineDefaulter;
class XF_Buffer;
class FilterProgressBar;
class XclImpPivotCacheList;

class XclImpXtiBuffer;
class XclImpSupbookBuffer;
class XclImpSupbook;
class XclImpTabIdBuffer;

class ExcETabNumBuffer;
class XclExpChTrTabId;
class XclExpUserBViewList;
class ExcNameList;
class ExcPalette2;
class UsedFontList;
class UsedFormList;
class XclSstList;
class XclSupbookList;
class XclExternsheetList;
class XclObjList;
class XclNoteList;
class XclEscher;
class XclPivotCacheList;
class SfxStyleSheet;
class ExcRecordList;
//class SvxURLField;
class XclHlink;
class XclAddInNameTranslator;

// ---------------------------------------------------------- Excel Imp~/Exp~ -

struct RootData     // -> Inkarnation jeweils im ImportExcel-Objekt!
{
    SvStorage*          pRootStorage;           // THE storage
    double              fColScale;              // Skalierungs-Faktoren fuer
    double              fRowScale;              //  Spaltenbreiten / Zeilenhoehen
    ScDocument*         pDoc;
    SvNumberFormatter*  pFormTable;
    ScRangeName*        pScRangeName;
    ColorBuffer*        pColor;
    XF_Buffer*          pXF_Buffer;
    FontBuffer*         pFontBuffer;
    LanguageType        eDefLanguage;
    String              aStandard;              // Schluessel fuer Exc-Standard-Format
    BiffTyp             eDateiTyp;              // feine Differenzierung
    BiffTyp             eHauptDateiTyp;         // grobe Klassifizierung
    BiffTyp             eGlobalDateiTyp;        // was war's ganz am Anfang?
    ExtSheetBuffer*     pExtSheetBuff;
    NameBuffer*         pTabNameBuff;
    RangeNameBuffer*    pRNameBuff;
    ShrfmlaBuffer*      pShrfmlaBuff;
    ExtNameBuff*        pExtNameBuff;
    USHORT*             pAktTab;                // Zeiger auf aktuelle Tab-Nummer
    const CharSet*      pCharset;               // actual charset (im- / export!)
    ScExtDocOptions*    pExtDocOpt;
    FilterProgressBar*  pProgress;              // aktueller Progressbar

    ScEditEngineDefaulter*  pEdEng;             // for rstrings, etc
    ScEditEngineDefaulter*  pEdEngHF;           // special for header and footer

    BOOL                bCellCut;               // bei Ueberlauf max. Cols / Rows
    BOOL                bBreakSharedFormula;    // SharedFormula bei Export aufbrechen?
    BOOL                bDefaultPage;           // Landscape etc. nicht gesetzt
    BOOL                bChartTab;              // Tabelle mit einem einzigen Chart

    // Biff8
    XclImpXtiBuffer*    pXtiBuffer;             // Elemente der Externsheet-Records
    XclImpSupbookBuffer* pSupbookBuffer;        // Liste Suporting External Workbooks
    XclImpSupbook*      pCurrSupbook;           // current supbook in pSupbookBuffer
    XclImpTabIdBuffer*  pImpTabIdBuffer;        // table id's for change tracking

    SvStorage*          pPivotCacheStorage;
    XclImpPivotCacheList* pImpPivotCacheList;       // pivot caches for import

    SvStorageRef        xCtrlStorage;           // SvxMSConvertOCXControls compatibel storage
//  SvStorage*          pCtrlStorage;           // SvxMSConvertOCXControls compatibel storage

    ScEditEngineDefaulter&  GetEdEng( void );       // -> exctools.cxx
    ScEditEngineDefaulter&  GetEdEngForHF( void );  // -> exctools.cxx

    UINT32              nCondRangeCnt;
    UINT16              nLastCond;
    String              GetCondFormStyleName( const UINT16 nCondCnt );  // -> exctools.cxx

    XclAddInNameTranslator* pAddInNameTranslator;

    // Erweiterungen fuer Export
    ExcETabNumBuffer*   pTabBuffer;
    XclExpChTrTabId*    pTabId;             // pointer to rec list, do not destroy
    XclExpUserBViewList* pUserBViewList;    // pointer to rec list, do not destroy
    ExcNameList*        pNameList;
    ExcPalette2*        pPalette2;
    UsedFontList*       pFontRecs;
    UsedFormList*       pFormRecs;
    ExcExternDup*       pExtSheetCntAndRecs;
    UINT16              nColMax;
    UINT16              nRowMax;
    UINT16              nRangeNameIndex;
    // Biff8
    XclSstList*         pSstRecs;
    XclExternsheetList* pExternsheetRecs;
    XclObjList*         pObjRecs;
    XclNoteList*        pNoteRecs;
    String              sAddNoteText;       // text to append at current note (multiple hyperlinks)
    XclEscher*          pEscher;

    XclPivotCacheList*  pPivotCacheList;

    BOOL                bWriteVBAStorage;

    UINT16              nCodenames;

    SfxStyleSheet*      pStyleSheet;
    SfxItemSet*         pStyleSheetItemSet;

//  const SvxURLField*  pLastHlink;             // last found hyperlink
    XclHlink*           pLastHlink;
    BOOL                bStoreRel;
    String*             pBasePath;

                        RootData( void );       // -> exctools.cxx
                        ~RootData();            // -> exctools.cxx
};




class ExcRoot
{
private:
protected:
    RootData*       pExcRoot;
    inline          ExcRoot( void );
    inline          ExcRoot( RootData* pNexExcRoot );
public:
    inline void     Set( RootData* pExcRoot );
};




inline ExcRoot::ExcRoot( void )
{
#ifdef DBG_UTIL
    pExcRoot = NULL;
#endif
}


inline ExcRoot::ExcRoot( RootData* pNexExcRoot )
{
    pExcRoot = pNexExcRoot;
}


inline void ExcRoot::Set( RootData* pRD )
{
    pExcRoot = pRD;
}




// ---------------------------------------------------------- Lotus Imp~/Exp~ -

class LotusRangeList;
class LotusFontBuffer;
class LotAttrTable;


struct LOTUS_ROOT
{
    ScDocument*         pDoc;
    LotusRangeList*     pRangeNames;
    ScRangeName*        pScRangeName;
    CharSet             eCharsetQ;
    Lotus123Typ         eFirstType;
    Lotus123Typ         eActType;
    ScRange             aActRange;
    RangeNameBufferWK3* pRngNmBffWK3;
    LotusFontBuffer*    pFontBuff;
    LotAttrTable*       pAttrTable;
};

extern LOTUS_ROOT*      pLotusRoot; // -> Inkarn. in filter.cxx

#endif
