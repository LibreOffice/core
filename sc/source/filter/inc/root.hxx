/*************************************************************************
 *
 *  $RCSfile: root.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:28:11 $
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
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
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

class SvStorage;
class SvNumberFormatter;
class ScRangeName;

class NameBuffer;
class RangeNameBuffer;
class RangeNameBufferWK3;
class ShrfmlaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class ExcExternDup;
class ScExtDocOptions;

class XclImpPivotCacheList;

class XclExpChTrTabId;
class XclExpUserBViewList;
class XclExpCellMerging;

class ExcNameList;
class XclObjList;
class XclEscher;
class XclPivotCacheList;
class SfxStyleSheet;
class ExcRecordList;
class XclExpHyperlink;

class XclImpRoot;
class XclExpRoot;

// ---------------------------------------------------------- Excel Imp~/Exp~ -

struct RootData     // -> Inkarnation jeweils im ImportExcel-Objekt!
{
    SvStorage*          pRootStorage;           // THE storage
    double              fRowScale;              //  Spaltenbreiten / Zeilenhoehen
    ScDocument*         pDoc;
    ScRangeName*        pScRangeName;

    String              aStandard;              // Schluessel fuer Exc-Standard-Format
    BiffTyp             eDateiTyp;              // feine Differenzierung
    BiffTyp             eHauptDateiTyp;         // grobe Klassifizierung
    ExtSheetBuffer*     pExtSheetBuff;
    NameBuffer*         pTabNameBuff;
    RangeNameBuffer*    pRNameBuff;
    ShrfmlaBuffer*      pShrfmlaBuff;
    ExtNameBuff*        pExtNameBuff;
    const CharSet*      pCharset;               // actual charset (im- / export!)
    ScExtDocOptions*    pExtDocOpt;

    BOOL                bCellCut;               // bei Ueberlauf max. Cols / Rows
    BOOL                bBreakSharedFormula;    // SharedFormula bei Export aufbrechen?
    BOOL                bDefaultPage;           // Landscape etc. nicht gesetzt
    BOOL                bChartTab;              // Tabelle mit einem einzigen Chart

    // Biff8
    SvStorage*              pPivotCacheStorage;
    XclImpPivotCacheList*   pImpPivotCacheList;     // pivot caches for import

    // Erweiterungen fuer Export
    XclExpChTrTabId*        pTabId;             // pointer to rec list, do not destroy
    XclExpUserBViewList*    pUserBViewList;     // pointer to rec list, do not destroy
    XclExpCellMerging*      pCellMerging;       // pointer to rec list, do not destroy

    ExcNameList*        pNameList;
    ScRangeName*        pScNameList;        // stores range names and DB ranges
    ExcExternDup*       pExtSheetCntAndRecs;
    UINT16              nColMax;
    UINT16              nRowMax;
    // Biff8
    XclObjList*         pObjRecs;
    String              sAddNoteText;       // text to append at current note (multiple hyperlinks)
    XclEscher*          pEscher;

    XclPivotCacheList*  pPivotCacheList;

    BOOL                bWriteVBAStorage;

    SfxStyleSheet*      pStyleSheet;
    SfxItemSet*         pStyleSheetItemSet;

//  const SvxURLField*  pLastHlink;             // last found hyperlink
    XclExpHyperlink*    pLastHlink;

    XclImpRoot*         pIR;
    XclExpRoot*         pER;

                        RootData( void );       // -> exctools.cxx
                        ~RootData();            // -> exctools.cxx
};




class ExcRoot
{
private:
protected:
    RootData*       pExcRoot;
//    inline          ExcRoot( void );              //#94039# prevent empty rootdata
    inline          ExcRoot( RootData* pNexExcRoot ) : pExcRoot( pNexExcRoot ) {}
    inline          ExcRoot( const ExcRoot& rCopy ) : pExcRoot( rCopy.pExcRoot ) {}
public:
//    inline void     Set( RootData* pExcRoot );    //#94039# prevent empty rootdata
};



//#94039# prevent empty rootdata

//inline ExcRoot::ExcRoot( void )
//{
//#ifdef DBG_UTIL
//    pExcRoot = NULL;
//#endif
//}


//#94039# prevent empty rootdata

//inline void ExcRoot::Set( RootData* pRD )
//{
//    pExcRoot = pRD;
//}




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

