/*************************************************************************
 *
 *  $RCSfile: ww8par3.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:43:55 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
#include <com/sun/star/form/XFormControllerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONNECTABLESHAPE_HPP_
#include <com/sun/star/drawing/XConnectableShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONNECTORSHAPE_HPP_
#include <com/sun/star/drawing/XConnectorShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEALIGNER_HPP_
#include <com/sun/star/drawing/XShapeAligner.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUP_HPP_
#include <com/sun/star/drawing/XShapeGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XUNIVERSALSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XUniversalShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEMIRROR_HPP_
#include <com/sun/star/drawing/XShapeMirror.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEARRANGER_HPP_
#include <com/sun/star/drawing/XShapeArranger.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#ifndef __SGI_STL_FUNCTIONAL
#include <functional>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
#ifndef _UNOOBJ_HXX //autogen
#include <unoobj.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _CHARATR_HXX //autogen
#include <charatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FLDDROPDOWN_HXX
#include <flddropdown.hxx>
#endif

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif
#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"  // wg. Listen-Attributen in Styles
#endif

using namespace com::sun::star;
using namespace sw::util;
//-----------------------------------------
//            UNO-Controls
//-----------------------------------------

//cmc, OCX i.e. word 97 form controls
eF_ResT SwWW8ImplReader::Read_F_OCX( WW8FieldDesc*, String& )
{
    if( bObj && nPicLocFc )
        nObjLocFc = nPicLocFc;
    bEmbeddObj = true;
    return FLD_TEXT;
}

eF_ResT SwWW8ImplReader::Read_F_FormTextBox( WW8FieldDesc* pF, String& rStr )
{
    WW8FormulaEditBox aFormula(*this);

    if (0x01 == rStr.GetChar( pF->nLCode-1 ))
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_EDIT);

    /* #80205#
    Here we have a small complication. This formula control contains
    the default text that is displayed if you edit the form field in
    the "default text" area. But MSOffice does not display that
    information, instead it display the result of the field,
    MSOffice just uses the default text of the control as its
    initial value for the displayed default text. So we will swap in
    the field result into the formula here in place of the default
    text.
    */
    aFormula.sDefault = GetFieldResult(pF);

    //substituting Unicode spacing 0x2002 with double space for layout
    aFormula.sDefault.SearchAndReplaceAll(String(0x2002),
        CREATE_CONST_ASC("  "));

    SwInputField aFld((SwInputFieldType*)rDoc.GetSysFldType( RES_INPUTFLD ),
        aFormula.sDefault , aFormula.sTitle , INP_TXT, 0 );
    rDoc.Insert(*pPaM, SwFmtFld(aFld));

    return FLD_OK;
}

eF_ResT SwWW8ImplReader::Read_F_FormCheckBox( WW8FieldDesc* pF, String& rStr )
{
    WW8FormulaCheckBox aFormula(*this);

    if( !pFormImpl )
        pFormImpl = new SwMSConvertControls(rDoc.GetDocShell(),pPaM);

    if (0x01 == rStr.GetChar(pF->nLCode-1))
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_CHECKBOX);

    pFormImpl->InsertFormula(aFormula);
    return FLD_OK;
}

eF_ResT SwWW8ImplReader::Read_F_FormListBox( WW8FieldDesc* pF, String& rStr)
{
    WW8FormulaListBox aFormula(*this);

    if (0x01 == rStr.GetChar(pF->nLCode-1))
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_DROPDOWN);

    SwDropDownField aFld(
        (SwDropDownFieldType*)rDoc.GetSysFldType(RES_DROPDOWN));

    aFld.SetName(aFormula.sTitle);

    if (!aFormula.maListEntries.empty())
    {
        aFld.SetItems(aFormula.maListEntries);
        int nIndex = aFormula.fDropdownIndex  < aFormula.maListEntries.size()
            ? aFormula.fDropdownIndex : 0;
        aFld.SetSelectedItem(aFormula.maListEntries[nIndex]);
    }

    rDoc.Insert(*pPaM, SwFmtFld(aFld));

    return FLD_OK;
}

void SwWW8ImplReader::DeleteFormImpl()
{
    delete pFormImpl, pFormImpl = 0;
}

//----------------------------------------------------------------------------
//          WW8ListManager          oeffentliche Methoden stehen ganz am Ende
//------------------------- ============ --------------- ============ --------



// Hilfs-Deklarationen ///////////////////////////////////////////////////////
//
// Style Id's for each level
typedef sal_uInt16 WW8aIdSty[WW8ListManager::nMaxLevel];
// Zeichenattribute aus GrpprlChpx
typedef SfxItemSet* WW8aISet[WW8ListManager::nMaxLevel];
// Zeichen Style Pointer
typedef SwCharFmt* WW8aCFmt[WW8ListManager::nMaxLevel];

struct WW8LST   // nur DIE Eintraege, die WIR benoetigen!
{
    WW8aIdSty   aIdSty;     // Style Id's for each level,
                            //   nIStDNil if no style linked
    sal_uInt32      nIdLst;     // Unique List ID
    sal_uInt32      nTplC;      // Unique template code - Was ist das bloss?
    BYTE bSimpleList:1; // Flag: Liste hat nur EINEN Level
    BYTE bRestartHdn:1; // WW6-Kompatibilitaets-Flag:
                                                        //   true if the list should start numbering over
};                                                      //   at the beginning of each section

struct WW8LFO   // nur DIE Eintraege, die WIR benoetigen!
{
    SwNumRule*      pNumRule;   // Parent NumRule
    sal_uInt32      nIdLst;     // Unique List ID
    sal_uInt8       nLfoLvl;    // count of levels whose format is overridden
    bool bSimpleList;
};

#define nIStDNil 0x0FFF // d.h. KEIN Style ist an den Level gelinkt

struct WW8LVL   // nur DIE Eintraege, die WIR benoetigen!
{
    long    nStartAt;       // start at value for this value
    long    nV6DxaSpace;// Ver6-Compatible: min Space between Num anf ::com::sun::star::text::Paragraph
    long    nV6Indent;  // Ver6-Compatible: Breite des Prefix Textes; ggfs. zur
                        // Definition d. Erstzl.einzug nutzen!
    // Absatzattribute aus GrpprlPapx
    sal_uInt16  nDxaLeft;               // linker Einzug
    short   nDxaLeft1;          // Erstzeilen-Einzug

    sal_uInt8   nNFC;               // number format code
    // Offset der Feldkodes im Num-X-String
    sal_uInt8   aOfsNumsXCH[WW8ListManager::nMaxLevel];
    sal_uInt8   nLenGrpprlChpx; // length, in bytes, of the LVL's grpprlChpx
    sal_uInt8   nLenGrpprlPapx; // length, in bytes, of the LVL's grpprlPapx
    sal_uInt8   nAlign: 2;  // alignment (left, right, centered) of the number
    BYTE bLegal:    1;  // egal
    BYTE bNoRest:1; // egal
    BYTE bV6Prev:1; // Ver6-Compatible: number will include previous levels
    BYTE bV6PrSp:1; // Ver6-Compatible: egal
    BYTE bV6:       1;  // falls true , beachte die V6-Compatible Eintraege!
    sal_uInt8   bDummy: 1;  // (macht das Byte voll)

};

struct WW8LFOLVL
{
    long nStartAt;          // start-at value if bFormat==false and bStartAt == true
                                            // (if bFormat==true, the start-at is stored in the LVL)
    sal_uInt8 nLevel;               // the level to be overridden
    // dieses Byte ist _absichtlich_ nicht in das folgende Byte hineingepackt   !!
    // (siehe Kommentar unten bei struct WW8LFOInfo)

    BYTE bStartAt :1;       // true if the start-at value is overridden
    BYTE bFormat    :1;     // true if the formatting is overriden
};

typedef std::vector<sal_uInt8> levelsprms;

// in den ListenInfos zu speichernde Daten ///////////////////////////////////
//
struct WW8LSTInfo   // sortiert nach nIdLst (in WW8 verwendete Listen-Id)
{
    std::vector<levelsprms> maParaSprms;
    WW8aIdSty   aIdSty;          // Style Id's for each level
    WW8aISet    aItemSet;        // Zeichenattribute aus GrpprlChpx
    WW8aCFmt    aCharFmt;        // Zeichen Style Pointer

    SwNumRule*  pNumRule;        // Zeiger auf entsprechende Listenvorlage im Writer
    sal_uInt32      nIdLst;          // WW8Id dieser Liste
    BYTE bSimpleList:1;// Flag, ob diese NumRule nur einen Level verwendet
    BYTE bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte

    WW8LSTInfo(SwNumRule* pNumRule_, WW8LST& aLST)
        : pNumRule(pNumRule_), nIdLst(aLST.nIdLst),
        bSimpleList(aLST.bSimpleList), bUsedInDoc(0)
    {
        memcpy( aIdSty, aLST.aIdSty, sizeof( aIdSty   ));
        memset(&aItemSet, 0,  sizeof( aItemSet ));
        memset(&aCharFmt, 0,  sizeof( aCharFmt ));
    }

};

// in den ListenFormatOverrideInfos zu speichernde Daten /////////////////////
//
struct WW8LFOInfo   // unsortiert, d.h. Reihenfolge genau wie im WW8 Stream
{
    std::vector<levelsprms> maParaSprms;
    SwNumRule* pNumRule;         // Zeiger auf entsprechende Listenvorlage im Writer
                                                     // entweder: Liste in LSTInfos oder eigene Liste
                                                     // (im Ctor erstmal die aus den LSTInfos merken)

    sal_uInt32  nIdLst;          // WW8-Id der betreffenden Liste
    sal_uInt8   nLfoLvl;             // count of levels whose format is overridden
    // Ja, ich natuerlich koennten wir nLfoLvl (mittels :4) noch in das folgende
    // Byte mit hineinpacken, doch waere das eine ziemliche Fehlerquelle,
    // an dem Tag, wo MS ihr Listenformat auf mehr als 15 Level aufbohren.

    BYTE bOverride  :1;// Flag, ob die NumRule nicht in maLSTInfos steht,
                                                     //   sondern fuer pLFOInfos NEU angelegt wurde
    BYTE bSimpleList:1;// Flag, ob diese NumRule nur einen Level verwendet
    BYTE bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte
    BYTE bLSTbUIDSet    :1;// Flag, ob bUsedInDoc in maLSTInfos gesetzt wurde,
                                                     //   und nicht nochmals gesetzt zu werden braucht
    WW8LFOInfo(const WW8LFO& rLFO)
        : pNumRule(rLFO.pNumRule), // hier bloss die Parent NumRule
          nIdLst(rLFO.nIdLst), nLfoLvl(rLFO.nLfoLvl),
          bOverride(rLFO.nLfoLvl ? true : false), bSimpleList(rLFO.bSimpleList),
          bUsedInDoc(0), bLSTbUIDSet(0) {}
};

SV_IMPL_PTRARR( WW8LFOInfos, WW8LFOInfo_Ptr );


// Hilfs-Methoden ////////////////////////////////////////////////////////////
//

// finden der Sprm-Parameter-Daten, falls Sprm im Grpprl enthalten
sal_uInt8* WW8ListManager::GrpprlHasSprm(sal_uInt16 nId, sal_uInt8& rSprms,
    sal_uInt8 nLen)
{
    sal_uInt8* pSprms = &rSprms;
    USHORT i=0;
    while (i < nLen)
    {
        sal_uInt16 nAktId = maSprmParser.GetSprmId(pSprms);
        if( nAktId == nId ) // Sprm found
            return pSprms + maSprmParser.DistanceToData(nId);

        // gib Zeiger auf Daten
        USHORT x = maSprmParser.GetSprmSize(nAktId, pSprms);
        i += x;
        pSprms += x;
    }
    return 0;                           // Sprm not found
}

class ListWithId : public ::std::unary_function<const WW8LSTInfo *, bool>
{
private:
    sal_uInt32 mnIdLst;
public:
    explicit ListWithId(sal_uInt32 nIdLst) : mnIdLst(nIdLst) {}
    bool operator() (const WW8LSTInfo *pEntry) const
        { return (pEntry->nIdLst == mnIdLst); }
};

// Zugriff ueber die List-Id des LST Eintrags
WW8LSTInfo* WW8ListManager::GetLSTByListId( sal_uInt32 nIdLst ) const
{
    std::vector<WW8LSTInfo *>::const_iterator aResult =
        std::find_if(maLSTInfos.begin(),maLSTInfos.end(),ListWithId(nIdLst));
    if (aResult == maLSTInfos.end())
        return 0;
    return *aResult;
}

void lcl_CopyGreaterEight(String &rDest, String &rSrc,
    xub_StrLen nStart, xub_StrLen nLen = STRING_LEN)
{
    if (nLen == STRING_LEN)
        nLen = rSrc.Len();
    for (xub_StrLen nI = nStart; nI < nLen; ++nI)
    {
        sal_Unicode nChar = rSrc.GetChar(nI);
        if (nChar > WW8ListManager::nMaxLevel)
            rDest.Append(nChar);
    }
}

bool WW8ListManager::ReadLVL(SwNumFmt& rNumFmt, SfxItemSet*& rpItemSet,
    sal_uInt16 nLevelStyle, bool bSetStartNo,
    std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
    levelsprms &rParaSprms)
{
    sal_uInt8       aBits1;
    sal_uInt16      nStartNo    = 0;    // Start-Nr. fuer den Writer
    SvxExtNumType   eType;              // Writer-Num-Typ
    SvxAdjust       eAdj;               // Ausrichtung (Links/rechts/zent.)
    sal_Unicode     cBullet(0x2190);    // default safe bullet
    String          sPrefix;
    String          sPostfix;
    WW8LVL          aLVL;
    //
    // 1. LVLF einlesen
    //
    memset(&aLVL, 0, sizeof( aLVL ));
    rSt >> aLVL.nStartAt;
    rSt >> aLVL.nNFC;
    rSt >> aBits1;
    if( 0 != rSt.GetError() ) return false;
    aLVL.nAlign = (aBits1 & 0x03);
    if( aBits1 & 0x10 ) aLVL.bV6Prev    = true;
    if( aBits1 & 0x20 ) aLVL.bV6PrSp    = true;
    if( aBits1 & 0x40 ) aLVL.bV6        = true;
    bool bLVLOkB = true;
    sal_uInt8 nLevelB = 0;
    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        rSt >> aLVL.aOfsNumsXCH[ nLevelB ];
        if( 0 != rSt.GetError() )
        {
            bLVLOkB = false;
            break;
        }
    }

    if( !bLVLOkB )
        return false;

    rSt.SeekRel( 1 );
    rSt >> aLVL.nV6DxaSpace;
    rSt >> aLVL.nV6Indent;
    rSt >> aLVL.nLenGrpprlChpx;
    rSt >> aLVL.nLenGrpprlPapx;
    rSt.SeekRel( 2 );
    if( 0 != rSt.GetError()) return false;

    //
    // 2. ggfs. PAPx einlesen und nach Einzug-Werten suchen
    //
    if( aLVL.nLenGrpprlPapx )
    {
        sal_uInt8 aGrpprlPapx[ 255 ];
        if(aLVL.nLenGrpprlPapx != rSt.Read(&aGrpprlPapx,aLVL.nLenGrpprlPapx))
            return false;
        // "sprmPDxaLeft"  pap.dxaLeft;dxa;word;
        sal_uInt8* pSprm;
        if ((pSprm = GrpprlHasSprm(0x840F,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)))
        {
            sal_uInt8 *pBegin = pSprm-2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            short nDxaLeft = SVBT16ToShort( pSprm );
            aLVL.nDxaLeft = (0 < nDxaLeft) ? (sal_uInt16)nDxaLeft
                            : (sal_uInt16)(-nDxaLeft);
        }

        // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
        if( (pSprm = GrpprlHasSprm(0x8411,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
        {
            sal_uInt8 *pBegin = pSprm-2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            aLVL.nDxaLeft1 = SVBT16ToShort(  pSprm );
        }

        // If there is a tab setting with a larger value, then use that.
        // Ideally we would allow tabs to be used in numbering fields and set
        // this on the containing paragraph which would make it actually work
        // most of the time.
        if( (pSprm = GrpprlHasSprm(0xC615,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
        {
            bool bDone=false;
            if (*(pSprm-1) == 5)
            {
                if (*pSprm++ == 0) //nDel
                {
                    if (*pSprm++ == 1) //nIns
                    {
                        short nTabPos = SVBT16ToShort(pSprm);
                        pSprm+=2;
                        if (*pSprm == 6) //type
                        {
                            USHORT nDesired = aLVL.nDxaLeft + aLVL.nDxaLeft1;

                               aLVL.nDxaLeft = (0 < nTabPos) ? (sal_uInt16)nTabPos
                                : (sal_uInt16)(-nTabPos);

                            aLVL.nDxaLeft1 = nDesired - aLVL.nDxaLeft;

                            bDone=true;
                        }
                    }
                }
            }
            ASSERT(bDone, "tab setting in numbering is "
                "of unexpected configuration");
        }

    }
    //
    // 3. ggfs. CHPx einlesen und
    //
    if( aLVL.nLenGrpprlChpx )
    {
        sal_uInt8 aGrpprlChpx[ 255 ];
        memset(&aGrpprlChpx, 0, sizeof( aGrpprlChpx ));
        if(aLVL.nLenGrpprlChpx != rSt.Read(&aGrpprlChpx, aLVL.nLenGrpprlChpx))
            return false;
        // neues ItemSet fuer die Zeichenattribute anlegen
        rpItemSet = new SfxItemSet( rDoc.GetAttrPool(), RES_CHRATR_BEGIN,
            RES_CHRATR_END - 1 );

        // Reader-ItemSet-Pointer darauf zeigen lassen
        rReader.SetAktItemSet( rpItemSet );
        // Reader-Style auf den Style dieses Levels setzen
        sal_uInt16 nOldColl = rReader.GetNAktColl();
        sal_uInt16 nNewColl = nLevelStyle;
        if( nIStDNil == nNewColl )
            nNewColl = 0;
        rReader.SetNAktColl( nNewColl );

        // Nun den GrpprlChpx einfach durchnudeln: die Read_xy() Methoden
        // in WW8PAR6.CXX rufen ganz normal ihr NewAttr() oder GetFmtAttr()
        // und diese merken am besetzten Reader-ItemSet-Pointer, dass dieser
        // spezielle ItemSet relevant ist - und nicht ein Stack oder Style!
        short nLen      = aLVL.nLenGrpprlChpx;
        sal_uInt8* pSprms1  = &aGrpprlChpx[0];
        while( 0 < nLen )
        {
            sal_uInt16 nL1 = rReader.ImportSprm( pSprms1 );
            nLen      -= nL1;
            pSprms1   += nL1;
        }
        // Reader-ItemSet-Pointer und Reader-Style zuruecksetzen
        rReader.SetAktItemSet( 0 );
        rReader.SetNAktColl( nOldColl );
    }
    //
    // 4. den Nummerierungsstring einlesen: ergibt Prefix und Postfix
    //
    String sNumString(WW8Read_xstz(rSt, 0, false));

    //
    // 5. gelesene Werte in Writer Syntax umwandeln
    //
    if( 0 <= aLVL.nStartAt )
        nStartNo = (sal_uInt16)aLVL.nStartAt;

    switch( aLVL.nNFC )
    {
        case   0:
            eType = SVX_NUM_ARABIC;
            break;
        case   1:
            eType = SVX_NUM_ROMAN_UPPER;
            break;
        case   2:
            eType = SVX_NUM_ROMAN_LOWER;
            break;
        case   3:
            eType = SVX_NUM_CHARS_UPPER_LETTER_N;
            break;
        case   4:
            eType = SVX_NUM_CHARS_LOWER_LETTER_N;
            break;
        case   5:
            // eigentlich: ORDINAL
            eType = SVX_NUM_ARABIC;
            break;
        case  23:
            eType = SVX_NUM_CHAR_SPECIAL;
            break;
        case 255:
            eType = SVX_NUM_NUMBER_NONE;
            break;
         default:
            // take default
            eType = SVX_NUM_ARABIC;
            break;
    }

    //If a number level is not going to be used, then record this fact
    if (SVX_NUM_NUMBER_NONE == eType)
        rNotReallyThere[nLevel] = true;

    /*
     If a number level was not used (i.e. is in NotReallyThere), and that
     number level appears at one of the positions in the display string of the
     list, then it effectively is not there at all. So remove that level entry
     from a copy of the aOfsNumsXCH.
    */
    std::vector<sal_uInt8> aOfsNumsXCH;
    typedef std::vector<sal_uInt8>::iterator myIter;
    aOfsNumsXCH.reserve(nMaxLevel);

    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
        aOfsNumsXCH.push_back(aLVL.aOfsNumsXCH[nLevelB]);

    for(nLevelB = 0; nLevelB <= nLevel; ++nLevelB)
    {
        sal_uInt8 nPos = aOfsNumsXCH[nLevelB];
        if (nPos && sNumString.GetChar(nPos-1) < nMaxLevel)
        {
            if (rNotReallyThere[nLevelB])
                aOfsNumsXCH[nLevelB] = 0;
        }
    }

    myIter aIter = std::remove(aOfsNumsXCH.begin(), aOfsNumsXCH.end(), 0);
    myIter aEnd = aOfsNumsXCH.end();
    while (aIter++ != aEnd)
        (*aIter) = 0;

    sal_uInt8 nUpperLevel = 0;  // akt. Anzeigetiefe fuer den Writer
    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        if (!nUpperLevel && !aOfsNumsXCH[nLevelB])
            nUpperLevel = nLevelB;
    }

    // falls kein NULL als Terminierungs-Char kam,
    // ist die Liste voller Indices, d.h. alle Plaetze sind besetzt,
    // also sind alle Level anzuzeigen
    if (!nUpperLevel)
        nUpperLevel = nMaxLevel;

    if (SVX_NUM_CHAR_SPECIAL == eType)
    {
        cBullet = sNumString.Len() ? sNumString.GetChar(0) : 0x2190;

        if (!cBullet)  // unsave control code?
            cBullet = 0x2190;
    }
    else
    {
        /*
        #83154#, #82192#, #i173#, #109158#
        Our aOfsNumsXCH seems generally to be an array that contains the
        offset into sNumString of locations where the numbers should be
        filled in, so if the first "fill in a number" slot is greater than
        1 there is a "prefix" before the number
        */
        //First number appears at
        sal_uInt8 nOneBasedFirstNoIndex = aOfsNumsXCH[0];
        xub_StrLen nFirstNoIndex =
            nOneBasedFirstNoIndex > 0 ? nOneBasedFirstNoIndex -1 : STRING_LEN;
        lcl_CopyGreaterEight(sPrefix, sNumString, 0, nFirstNoIndex);

        //Next number appears at
        if (nUpperLevel)
        {
            sal_uInt8 nOneBasedNextNoIndex = aOfsNumsXCH[nUpperLevel-1];
            xub_StrLen nNextNoIndex =
                nOneBasedNextNoIndex > 0 ? nOneBasedNextNoIndex -1 : STRING_LEN;
            if (nNextNoIndex != STRING_LEN)
                ++nNextNoIndex;
            if (sNumString.Len() > nNextNoIndex)
                lcl_CopyGreaterEight(sPostfix, sNumString, nNextNoIndex);
        }

        /*
         We use lcl_CopyGreaterEight because once if we have removed unused
         number indexes from the aOfsNumsXCH then placeholders remain in
         sNumString which must not be copied into the final numbering strings
        */
    }

    switch( aLVL.nAlign )
    {
        case 0:
            eAdj = SVX_ADJUST_LEFT;
            break;
        case 1:
            eAdj = SVX_ADJUST_CENTER;
            break;
        case 2:
            eAdj = SVX_ADJUST_RIGHT;
            break;
        case 3:
            // Writer here cannot do block justification
            eAdj = SVX_ADJUST_LEFT;
            break;
         default:
            // undefied value
            ASSERT( !this, "Value of aLVL.nAlign is not supported" );
            // take default
            eAdj = SVX_ADJUST_LEFT;
            break;
    }

    // 6. entsprechendes NumFmt konfigurieren
    if( bSetStartNo )
        rNumFmt.SetStart( nStartNo );
    rNumFmt.SetNumberingType(eType);
    rNumFmt.SetNumAdjust( eAdj );

    if( SVX_NUM_CHAR_SPECIAL == eType )
    {
        // first character of the Prefix-Text is the Bullet
        rNumFmt.SetBulletChar(cBullet);
        // Don't forget: unten, nach dem Bauen eventueller Styles auch noch
        // SetBulletFont() rufen !!!
    }
    else
    {
        // reminder: Garnix ist default Prefix
        if( sPrefix.Len() )
            rNumFmt.SetPrefix( sPrefix );
        // reminder: Point is default Postfix
        rNumFmt.SetSuffix( sPostfix );
        rNumFmt.SetIncludeUpperLevels( nUpperLevel );
    }

    if( 0 < aLVL.nDxaLeft1 )
        aLVL.nDxaLeft1 = aLVL.nDxaLeft1 * -1;

    if (eAdj == SVX_ADJUST_RIGHT)
    {
        rNumFmt.SetAbsLSpace(aLVL.nDxaLeft);
        rNumFmt.SetFirstLineOffset(-aLVL.nDxaLeft);
        rNumFmt.SetCharTextDistance(-aLVL.nDxaLeft1);
    }
    else
    {
        rNumFmt.SetAbsLSpace( aLVL.nDxaLeft );
        rNumFmt.SetFirstLineOffset(aLVL.nDxaLeft1);
    }

    return true;
}

void WW8ListManager::AdjustLVL( sal_uInt8 nLevel, SwNumRule& rNumRule,
    WW8aISet& rListItemSet, WW8aCFmt& rCharFmt, bool& bNewCharFmtCreated,
    String sPrefix )
{
    bNewCharFmtCreated = false;
    SfxItemSet* pThisLevelItemSet;
    SfxItemSet* pLowerLevelItemSet;
    sal_uInt8        nIdenticalItemSetLevel;
    const SfxPoolItem* pItem;

    SwNumFmt aNumFmt  = rNumRule.Get( nLevel );

    pThisLevelItemSet = rListItemSet[ nLevel ];

    if( pThisLevelItemSet && pThisLevelItemSet->Count())
    {
        nIdenticalItemSetLevel = nMaxLevel;
        SfxItemIter aIter( *pThisLevelItemSet );
        for (sal_uInt8 nLowerLevel = 0; nLowerLevel < nLevel; ++nLowerLevel)
        {
            pLowerLevelItemSet = rListItemSet[ nLowerLevel ];
            if(     pLowerLevelItemSet
                && (pLowerLevelItemSet->Count() == pThisLevelItemSet->Count()) )
            {
                nIdenticalItemSetLevel = nLowerLevel;
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                while (true)
                {
                    if(  // ggfs. passenden pItem im pLowerLevelItemSet finden
                         (SFX_ITEM_SET != pLowerLevelItemSet->GetItemState(
                                            nWhich, false, &pItem ) )
                        || // virtuellen "!=" Operator anwenden
                         (*pItem != *aIter.GetCurItem() ) )
                    // falls kein Item mit gleicher nWhich gefunden oder Werte
                    // der Items ungleich, Ungleichheit merken und abbrechen!
                    {
                        nIdenticalItemSetLevel = nMaxLevel;
                        break;
                    }
                    if( aIter.IsAtEnd() )
                        break;
                    nWhich = aIter.NextItem()->Which();
                }

                if( nIdenticalItemSetLevel != nMaxLevel )
                    break;
            }
        }

        SwCharFmt* pFmt;
        if (nMaxLevel == nIdenticalItemSetLevel)
        {
            // Style definieren
            String aName( sPrefix.Len() ? sPrefix : rNumRule.GetName() );
            (aName += 'z') += String::CreateFromInt32( nLevel );

            // const Wegcasten
            pFmt = rDoc.MakeCharFmt(aName, (SwCharFmt*)rDoc.GetDfltCharFmt());
            bNewCharFmtCreated = true;
            // Attribute reinsetzen
            pFmt->SetAttr( *pThisLevelItemSet );
        }
        else
        {
            // passenden Style hier anhaengen
            pFmt = rCharFmt[ nIdenticalItemSetLevel ];
        }

        // merken
        rCharFmt[ nLevel ] = pFmt;

        //
        // Style an das NumFormat haengen
        //
        aNumFmt.SetCharFmt( pFmt );
    }
    //
    // ggfs. Bullet Font an das NumFormat haengen
    //
    if( SVX_NUM_CHAR_SPECIAL == aNumFmt.GetNumberingType() )
    {
        SwCharFmt* pFmt = aNumFmt.GetCharFmt();
        Font aFont;
        if( !pFmt )
            aFont = SwNumRule::GetDefBulletFont();
        else
        {
            const SvxFontItem& rFontItem = pFmt->GetFont();
            aFont.SetFamily(    rFontItem.GetFamily()     );
            aFont.SetName(      rFontItem.GetFamilyName() );
            aFont.SetStyleName( rFontItem.GetStyleName()  );
            aFont.SetPitch(     rFontItem.GetPitch()      );
            aFont.SetCharSet(   rFontItem.GetCharSet()    );
        }
        aNumFmt.SetBulletFont( &aFont );
    }
    //
    // und wieder rein in die NumRule
    //
    rNumRule.Set(nLevel, aNumFmt);
}

SwNumRule* WW8ListManager::CreateNextRule(bool bSimple)
{
    // wird erstmal zur Bildung des Style Namens genommen
    String sPrefix(CREATE_CONST_ASC("WW8Num"));
    sPrefix += String::CreateFromInt32(nUniqueList++);
    sal_uInt16 nRul = rDoc.MakeNumRule(rDoc.GetUniqueNumRuleName(&sPrefix));
    SwNumRule* pMyNumRule = rDoc.GetNumRuleTbl()[nRul];
    pMyNumRule->SetAutoRule(false);
    pMyNumRule->SetContinusNum(bSimple);
    return pMyNumRule;
}

// oeffentliche Methoden /////////////////////////////////////////////////////
//
WW8ListManager::WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_)
    : maSprmParser(rReader_.GetFib().nVersion), rReader(rReader_),
    rDoc(rReader.GetDoc()), rFib(rReader.GetFib()), rSt(rSt_), pLFOInfos(0),
    nUniqueList(1)
{
    // LST und LFO gibts erst ab WW8
    if(    ( 8 > rFib.nVersion )
            || ( rFib.fcPlcfLst == rFib.fcPlfLfo )
            || ( !rFib.lcbPlcfLst )
            || ( !rFib.lcbPlfLfo ) ) return; // offensichtlich keine Listen da

    // Arrays anlegen
    pLFOInfos = new WW8LFOInfos;
    bool bLVLOk = true;
    sal_uInt8  aBits1;

    long nOriginalPos = rSt.Tell();
    //
    // 1. PLCF LST auslesen und die Listen Vorlagen im Writer anlegen
    //
    rSt.Seek( rFib.fcPlcfLst );
    sal_uInt16 nListCount;
    rSt >> nListCount;
    bool bOk = 0 < nListCount;
    if( bOk )
    {
        WW8LST aLST;
        //
        // 1.1 alle LST einlesen
        //
        for (sal_uInt16 nList=0; nList < nListCount; ++nList)
        {
            bOk = false;
            memset(&aLST, 0, sizeof( aLST ));
            sal_uInt16 nLevel;
            //
            // 1.1.1 Daten einlesen
            //
            rSt >> aLST.nIdLst;
            rSt >> aLST.nTplC;
            for (nLevel = 0; nLevel < nMaxLevel; ++nLevel)
                rSt >> aLST.aIdSty[ nLevel ];


            rSt >> aBits1;

            rSt.SeekRel( 1 );

            if (rSt.GetError())
                break;

            if( aBits1 & 0x01 )
                aLST.bSimpleList = true;
            if( aBits1 & 0x02 )
                aLST.bRestartHdn = true;

            // 1.1.2 new NumRule inserted in Doc and  WW8LSTInfo marked

            /*
            #i1869#
            In word 2000 microsoft got rid of creating new "simple lists" with
            only 1 level, all new lists are created with 9 levels. To hack it
            so that the list types formerly known as simple lists still have
            their own tab page to themselves one of the reserved bits is used
            to show that a given list is to be in the simple list tabpage.
            This has now nothing to do with the actual number of list level a
            list has, only how many will be shown in the user interface.

            i.e. create a simple list in 2000 and open it in 97 and 97 will
            claim (correctly) that it is an outline list. We can set our
            continous flag in these lists to store this information.
            */
            SwNumRule* pMyNumRule = CreateNextRule(
                aLST.bSimpleList || (aBits1 & 0x10));

            WW8LSTInfo* pLSTInfo = new WW8LSTInfo(pMyNumRule, aLST);
            maLSTInfos.push_back(pLSTInfo);
            bOk = true;
        }
    }

    if( bOk )
    {
        //
        // 1.2 alle LVL aller aLST einlesen
        //
        sal_uInt8 nLevel;
        sal_uInt16 nLSTInfos = maLSTInfos.size();
        for (sal_uInt16 nList = 0; nList < nLSTInfos; ++nList)
        {
            bOk = false;
            WW8LSTInfo* pListInfo = maLSTInfos[nList];
            if( !pListInfo || !pListInfo->pNumRule ) break;
            SwNumRule& rMyNumRule = *pListInfo->pNumRule;
            //
            // 1.2.1 betreffende(n) LVL(s) fuer diese aLST einlesen
            //
            sal_uInt16 nLvlCount=pListInfo->bSimpleList ? nMinLevel : nMaxLevel;
            std::deque<bool> aNotReallyThere;
            aNotReallyThere.resize(nMaxLevel);
            pListInfo->maParaSprms.resize(nMaxLevel);
            for (nLevel = 0; nLevel < nLvlCount; ++nLevel)
            {
                SwNumFmt aNumFmt( rMyNumRule.Get( nLevel ) );
                // LVLF einlesen
                bLVLOk = ReadLVL( aNumFmt, pListInfo->aItemSet[nLevel],
                    pListInfo->aIdSty[nLevel], true, aNotReallyThere, nLevel,
                    pListInfo->maParaSprms[nLevel]);
                if( !bLVLOk )
                    break;
                // und in die rMyNumRule aufnehmen
                rMyNumRule.Set( nLevel, aNumFmt );
            }
            if( !bLVLOk )
                break;
            //
            // 1.2.2 die ItemPools mit den CHPx Einstellungen der verschiedenen
            //       Level miteinander vergleichen und ggfs. Style(s) erzeugen
            //
            bool bDummy;
            for (nLevel = 0; nLevel < nLvlCount; ++nLevel)
            {
                AdjustLVL( nLevel, rMyNumRule, pListInfo->aItemSet,
                                               pListInfo->aCharFmt, bDummy );
            }
            //
            // 1.2.3 ItemPools leeren und loeschen
            //
            for (nLevel = 0; nLevel < nLvlCount; ++nLevel)
                delete pListInfo->aItemSet[ nLevel ];
            bOk = true;
        }
    }
    if( !bOk )
    {
        // Fehler aufgetreten - LSTInfos abraeumen !!!

        ;
    }

    //
    // 2. PLF LFO auslesen und speichern
    //
    long nLfoCount(0);
    if (bOk)
    {
        rSt.Seek(rFib.fcPlfLfo);
        rSt >> nLfoCount;
        if (0 >= nLfoCount)
            bOk = false;
    }

    if(bOk)
    {
        WW8LFO aLFO;
        //
        // 2.1 alle LFO einlesen
        //
        for (sal_uInt16 nLfo = 0; nLfo < nLfoCount; ++nLfo)
        {
            bOk = false;
            memset(&aLFO, 0, sizeof( aLFO ));
            rSt >> aLFO.nIdLst;
            rSt.SeekRel( 8 );
            rSt >> aLFO.nLfoLvl;
            rSt.SeekRel( 3 );
            // soviele Overrides existieren
            if ((nMaxLevel < aLFO.nLfoLvl) && rSt.GetError())
                break;

            // die Parent NumRule der entsprechenden Liste ermitteln
            WW8LSTInfo* pParentListInfo = GetLSTByListId(aLFO.nIdLst);
            if (pParentListInfo)
            {
                // hier, im ersten Schritt, erst mal diese NumRule festhalten
                aLFO.pNumRule = pParentListInfo->pNumRule;

                // hat die Liste mehrere Level ?
                aLFO.bSimpleList = pParentListInfo->bSimpleList;
            }
            // und rein ins Merk-Array mit dem Teil
            WW8LFOInfo* pLFOInfo = new WW8LFOInfo(aLFO);
            if (pParentListInfo)
            {
                //Copy the basic paragraph properties for each level from the
                //original list into the list format override levels.
                int nMaxSize = pParentListInfo->maParaSprms.size();
                pLFOInfo->maParaSprms.resize(nMaxSize);
                for (int i = 0; i < nMaxSize; ++i)
                    pLFOInfo->maParaSprms[i] = pParentListInfo->maParaSprms[i];
            }
            pLFOInfos->Insert(pLFOInfo, pLFOInfos->Count());
            bOk = true;
        }
    }
    if( bOk )
    {
        //
        // 2.2 fuer alle LFO die zugehoerigen LFOLVL einlesen
        //
        WW8LFOLVL aLFOLVL;
        sal_uInt16 nLFOInfos = pLFOInfos ? pLFOInfos->Count() : 0;
        for (sal_uInt16 nLfo = 0; nLfo < nLFOInfos; ++nLfo)
        {
            bOk = false;
            WW8LFOInfo* pLFOInfo = pLFOInfos->GetObject( nLfo );
            if (!pLFOInfo)
                break;
            // stehen hierfuer ueberhaupt LFOLVL an ?
            if( pLFOInfo->bOverride )
            {
                WW8LSTInfo* pParentListInfo = GetLSTByListId(pLFOInfo->nIdLst);
                ASSERT(pParentListInfo, "ww: Impossible lists, please report");
                if (!pParentListInfo)
                    break;
                //
                // 2.2.1 eine neue NumRule fuer diese Liste anlegen
                //
                SwNumRule* pParentNumRule = pLFOInfo->pNumRule;
                ASSERT(pParentNumRule, "ww: Impossible lists, please report");
                if( !pParentNumRule )
                    break;
                // Nauemsprefix aufbauen: fuer NumRule-Name (eventuell)
                // und (falls vorhanden) fuer Style-Name (dann auf jeden Fall)
                String sPrefix(CREATE_CONST_ASC( "WW8NumSt" ));
                sPrefix += String::CreateFromInt32( nLfo + 1 );
                // jetzt dem pNumRule seinen RICHTIGEN Wert zuweisen !!!
                // (bis dahin war hier die Parent NumRule vermerkt )
                //
                // Dazu erst mal nachsehen, ob ein Style diesen LFO
                // referenziert:
                if( USHRT_MAX > rReader.StyleUsingLFO( nLfo ) )
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName( &sPrefix ), pParentNumRule);
                    pLFOInfo->pNumRule = rDoc.GetNumRuleTbl()[ nRul ];
                    pLFOInfo->pNumRule->SetAutoRule(false);
                }
                else
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName(), pParentNumRule);
                    pLFOInfo->pNumRule = rDoc.GetNumRuleTbl()[ nRul ];
                    pLFOInfo->pNumRule->SetAutoRule(true);  // = default
                }
                //
                // 2.2.2 alle LFOLVL (und ggfs. LVL) fuer die neue NumRule
                // einlesen
                //
                WW8aISet aItemSet;       // Zeichenattribute aus GrpprlChpx
                WW8aCFmt aCharFmt;       // Zeichen Style Pointer
                memset(&aItemSet, 0,  sizeof( aItemSet ));
                memset(&aCharFmt, 0,  sizeof( aCharFmt ));

                std::deque<bool> aNotReallyThere;
                aNotReallyThere.resize(WW8ListManager::nMaxLevel);
                pLFOInfo->maParaSprms.resize(WW8ListManager::nMaxLevel);

                for (sal_uInt8 nLevel = 0; nLevel < pLFOInfo->nLfoLvl; ++nLevel)
                {
                    bLVLOk = false;
                    memset(&aLFOLVL, 0, sizeof( aLFOLVL ));

                    //
                    // 2.2.2.0 fuehrende 0xFF ueberspringen
                    //
                    sal_uInt8 n1;
                    do
                    {
                        rSt >> n1;
                        if (rSt.GetError())
                            break;
                    }
                    while( 0xFF == n1 );
                    rSt.SeekRel( -1 );

                    //
                    // 2.2.2.1 den LFOLVL einlesen
                    //
                    rSt >> aLFOLVL.nStartAt;
                    rSt >> aBits1;
                    rSt.SeekRel( 3 );
                    if (rSt.GetError())
                        break;

                    // beachte: Die Witzbolde bei MS quetschen die
                    // Override-Level-Nummer in vier Bits hinein, damit sie
                    // wieder einen Grund haben, ihr Dateiformat zu aendern,
                    // falls ihnen einfaellt, dass sie eigentlich doch gerne
                    // bis zu 16 Listen-Level haetten.  Wir tun das *nicht*
                    // (siehe Kommentar oben bei "struct
                    // WW8LFOInfo")
                    aLFOLVL.nLevel = aBits1 & 0x0F;
                    if( (0xFF > aBits1) &&
                        (nMaxLevel > aLFOLVL.nLevel) )
                    {
                        if( aBits1 & 0x10 )
                            aLFOLVL.bStartAt = true;
                        //
                        // 2.2.2.2 eventuell auch den zugehoerigen LVL einlesen
                        //
                        SwNumFmt aNumFmt(
                            pLFOInfo->pNumRule->Get( aLFOLVL.nLevel ) );
                        if( aBits1 & 0x20 )
                        {
                            aLFOLVL.bFormat = true;
                            // falls bStartup true, hier den Startup-Level
                            // durch den im LVL vermerkten ersetzen LVLF
                            // einlesen
                            bLVLOk = ReadLVL(aNumFmt, aItemSet[nLevel],
                                pParentListInfo->aIdSty[nLevel],
                                aLFOLVL.bStartAt, aNotReallyThere, nLevel,
                                pLFOInfo->maParaSprms[nLevel]);

                            if( !bLVLOk )
                                break;
                        }
                        else if( aLFOLVL.bStartAt )
                            aNumFmt.SetStart( (sal_uInt16)aLFOLVL.nStartAt );
                        //
                        // 2.2.2.3 das NumFmt in die NumRule aufnehmen
                        //
                        pLFOInfo->pNumRule->Set( nLevel, aNumFmt );
                    }
                    bLVLOk = true;
                }
                if( !bLVLOk )
                    break;
                //
                // 2.2.3 die LVL der neuen NumRule anpassen
                //
                sal_uInt16 aFlagsNewCharFmt = 0;
                bool bNewCharFmtCreated = false;
                for (nLevel = 0; nLevel < pLFOInfo->nLfoLvl; ++nLevel)
                {
                    AdjustLVL( nLevel, *pLFOInfo->pNumRule, aItemSet, aCharFmt,
                        bNewCharFmtCreated, sPrefix );
                    if( bNewCharFmtCreated )
                        aFlagsNewCharFmt += (1 << nLevel);
                }
                //
                // 2.2.4 ItemPools leeren und loeschen
                //
                for (nLevel = 0; nLevel < pLFOInfo->nLfoLvl; ++nLevel)
                    delete aItemSet[ nLevel ];
                bOk = true;
            }
        }
    }
    if( !bOk )
    {
        // Fehler aufgetreten - LSTInfos und LFOInfos abraeumen !!!
        ;
    }
    // und schon sind wir fertig!
    rSt.Seek( nOriginalPos );
}

WW8ListManager::~WW8ListManager()
{
    /*
    named lists remain in doc!!!
    unnamed lists are deleted when unused
    pLFOInfos are in any case destructed
    */
    for(::std::vector<WW8LSTInfo *>::iterator aIter = maLSTInfos.begin();
        aIter != maLSTInfos.end(); ++aIter)
    {
        if ((*aIter)->pNumRule && !(*aIter)->bUsedInDoc &&
            (*aIter)->pNumRule->IsAutoRule())
        {
            rDoc.DelNumRule((*aIter)->pNumRule->GetName());
        }
//        delete *aIter, *aIter=0;
        delete *aIter;
    }

    if (pLFOInfos)
    {
        for(sal_uInt16 nInfo = pLFOInfos->Count(); nInfo; )
        {
            WW8LFOInfo *pActInfo = pLFOInfos->GetObject(--nInfo);
            if (pActInfo->bOverride && pActInfo->pNumRule
                && !pActInfo->bUsedInDoc && pActInfo->pNumRule->IsAutoRule())
            {
                rDoc.DelNumRule( pActInfo->pNumRule->GetName() );
            }
        }
        delete pLFOInfos;
    }
}

SwNumRule* WW8ListManager::GetNumRuleForActivation(sal_uInt16 nLFOPosition,
    sal_uInt8 nLevel, std::vector<sal_uInt8> &rParaSprms) const
{
    sal_uInt16 nLFOInfos = pLFOInfos ? pLFOInfos->Count() : 0;
    if( nLFOInfos <= nLFOPosition )
        return 0;

    WW8LFOInfo* pLFOInfo = pLFOInfos->GetObject( nLFOPosition );

    if( !pLFOInfo )
        return 0;

    pLFOInfo->bUsedInDoc = true;

    if( !pLFOInfo->pNumRule )
        return 0;

    /*
    #i1869#
    If this list has had its bits set in word 2000 to pretend that it is a
    simple list from the point of view of the user, then it is almost
    certainly a simple continous list, and we will try to keep it like that.
    Otherwise when we save again it will be shown as the true outline list
    that it is, confusing the user that just wanted what they thought was a
    simple list. On the otherhand it is possible that some of the other levels
    were used by the user, in which case we will not pretend anymore that it
    is a simple list. Something that word 2000 does anyway, that 97 didn't, to
    my bewilderment.
    */
    if (nLevel && pLFOInfo->pNumRule->IsContinusNum())
        pLFOInfo->pNumRule->SetContinusNum(false);

    if( (!pLFOInfo->bOverride) && (!pLFOInfo->bLSTbUIDSet) )
    {
        WW8LSTInfo* pParentListInfo = GetLSTByListId( pLFOInfo->nIdLst );
        if( pParentListInfo )
            pParentListInfo->bUsedInDoc = true;
        pLFOInfo->bLSTbUIDSet = true;
    }

    if (pLFOInfo->maParaSprms.size() > nLevel)
        rParaSprms = pLFOInfo->maParaSprms[nLevel];

    return pLFOInfo->pNumRule;
}

//----------------------------------------------------------------------------
//          SwWW8ImplReader:  anhaengen einer Liste an einen Style oder Absatz
//----------------------------------------------------------------------------
bool SwWW8ImplReader::SetTxtFmtCollAndListLevel(const SwPaM& rRg,
    SwWW8StyInf& rStyleInfo)
{
    bool bRes = true;
    if( rStyleInfo.pFmt && rStyleInfo.bColl )
    {
        bRes = rDoc.SetTxtFmtColl(rRg, (SwTxtFmtColl*)rStyleInfo.pFmt)
            ? true : false;
        SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode();
        ASSERT( pTxtNode, "No Text-Node at PaM-Position" );
        if( !IsInvalidOrToBeMergedTabCell() )
            pTxtNode->SwCntntNode::ResetAttr( RES_PARATR_NUMRULE );

        if( !rStyleInfo.pOutlineNumrule )
        {
            if (
                 (USHRT_MAX > rStyleInfo.nLFOIndex) &&
                 (WW8ListManager::nMaxLevel > rStyleInfo.nListLevel)
               )
            {
                RegisterNumFmtOnTxtNode(rStyleInfo.nLFOIndex,
                    rStyleInfo.nListLevel, false);
            }

            if (rStyleInfo.bHasStyNumRule && pTxtNode)
                pTxtNode->SetNumLSpace(false);
        }
    }
    return bRes;
}

void UseListIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt)
{
    rStyle.nLeftParaMgn = rFmt.GetAbsLSpace();
    rStyle.nTxtFirstLineOfst = GetListFirstLineIndent(rFmt);
    SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.pFmt, RES_LR_SPACE));
    aLR.SetTxtLeft(rStyle.nLeftParaMgn);
    aLR.SetTxtFirstLineOfst(rStyle.nTxtFirstLineOfst);
    rStyle.pFmt->SetAttr(aLR);
    rStyle.bListReleventIndentSet = true;
}

void SetStyleIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt)
{
    if (rStyle.bListReleventIndentSet)
    {
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.pFmt, RES_LR_SPACE));
        SyncStyleIndentWithList(aLR, rFmt);
        rStyle.pFmt->SetAttr(aLR);
    }
    else
    {
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.pFmt, RES_LR_SPACE));
        aLR.SetTxtLeft(0);
        aLR.SetTxtFirstLineOfst(0);
        rStyle.pFmt->SetAttr(aLR);
    }
}

void SwWW8ImplReader::SetStylesList(sal_uInt16 nStyle, sal_uInt16 nActLFO,
    sal_uInt8 nActLevel)
{
    SwWW8StyInf &rStyleInf = pCollA[nStyle];
    if (rStyleInf.bValid)
    {
        ASSERT(pAktColl, "Cannot be called outside of style import");
        // Phase 1: Nummerierungsattribute beim Einlesen einer StyleDef
        if( pAktColl )
        {
            // jetzt nur die Parameter vermerken: die tatsaechliche Liste wird
            // spaeter drangehaengt, wenn die Listendefinitionen gelesen sind...
            if (
                 (USHRT_MAX > nActLFO) &&
                 (WW8ListManager::nMaxLevel > nActLevel)
               )
            {
                rStyleInf.nLFOIndex  = nActLFO;
                rStyleInf.nListLevel = nActLevel;

                if (
                    (USHRT_MAX > nActLFO) &&
                    (WW8ListManager::nMaxLevel > nActLevel)
                   )
                {
                    std::vector<sal_uInt8> aParaSprms;
                    SwNumRule *pNmRule =
                        pLstManager->GetNumRuleForActivation(nActLFO,nActLevel, aParaSprms);
                    if (pNmRule)
                        UseListIndent(rStyleInf, pNmRule->Get(nActLevel));
                }
            }
        }
    }
}


void SwWW8ImplReader::RegisterNumFmtOnStyle(sal_uInt16 nStyle)
{
    SwWW8StyInf &rStyleInf = pCollA[ nStyle ];
    if (rStyleInf.bValid)
    {
        // Phase 2: aktualisieren der StyleDef nach einlesen aller Listen
        SwNumRule* pNmRule = 0;
        sal_uInt16 nLFO = rStyleInf.nLFOIndex;
        sal_uInt8  nLevel = rStyleInf.nListLevel;
        if (
             (USHRT_MAX > nLFO) &&
             (WW8ListManager::nMaxLevel > nLevel)
           )
        {
            std::vector<sal_uInt8> aParaSprms;
            pNmRule = pLstManager->GetNumRuleForActivation(nLFO, nLevel,
                aParaSprms);

            if (pNmRule)
            {
                if( MAXLEVEL > rStyleInf.nOutlineLevel )
                    rStyleInf.pOutlineNumrule = pNmRule;
                else
                {
                    rStyleInf.pFmt->SetAttr(
                        SwNumRuleItem( pNmRule->GetName() ) );
                    rStyleInf.bHasStyNumRule = true;
                }
            }
        }

        if (pNmRule)
            SetStyleIndent(rStyleInf, pNmRule->Get(nLevel));
    }
}

void SwWW8ImplReader::RegisterNumFmtOnTxtNode(sal_uInt16 nActLFO,
    sal_uInt8 nActLevel, bool bSetAttr)
{
    // beachte: die Methode haengt die NumRule an den Text Node, falls
    // bSetAttr (dann muessen natuerlich vorher die Listen gelesen sein)
    // stellt sie NUR den Level ein, im Vertrauen darauf, dass am STYLE eine
    // NumRule haengt - dies wird NICHT ueberprueft !!!

    if (pLstManager) // sind die Listendeklarationen gelesen?
    {
        std::vector<sal_uInt8> aParaSprms;
        const SwNumRule* pRule = bSetAttr ?
            pLstManager->GetNumRuleForActivation( nActLFO, nActLevel,
                aParaSprms) : 0;

        if( pRule || !bSetAttr )
        {
            SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode();
            ASSERT( pTxtNode, "Kein Text-Node an PaM-Position" );

            if (bSetAttr)
                pTxtNode->SwCntntNode::SetAttr(SwNumRuleItem(pRule->GetName()));

            pTxtNode->SetNumLSpace(bSetAttr);

            pTxtNode->UpdateNum(SwNodeNum(nActLevel));

            /*
             Take the original paragraph sprms attached to this list level
             formatting and apply them to the paragraph. I'm convinced that
             this is exactly what word does.
            */
            if (short nLen = aParaSprms.size())
            {
                SfxItemSet* pOldAktItemSet = pAktItemSet;
                sal_uInt8* pSprms1  = &aParaSprms[0];
                SfxItemSet aListIndent(rDoc.GetAttrPool(),
                    RES_LR_SPACE, RES_LR_SPACE);
                SetAktItemSet(&aListIndent);
                while (0 < nLen)
                {
                    sal_uInt16 nL1 = ImportSprm(pSprms1);
                    nLen -= nL1;
                    pSprms1 += nL1;
                }
                SetAktItemSet(pOldAktItemSet);
                pTxtNode->SwCntntNode::SetAttr(aListIndent);
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFmt(sal_uInt16 nActLFO, sal_uInt8 nActLevel)
{
    // sind wir erst beim Einlesen der StyleDef ?
    if (pAktColl)
        SetStylesList( nAktColl , nActLFO, nActLevel);
    else
        RegisterNumFmtOnTxtNode(nActLFO, nActLevel);
}

void SwWW8ImplReader::Read_ListLevel(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // aktuelle Liste ist hier zu Ende, was ist zu tun ???
        nListLevel = WW8ListManager::nMaxLevel;
        if (pStyles && !bVer67)
            pStyles->nWwNumLevel = 0;
    }
    else
    {
        // Sicherheitspruefung auf NIL Pointer
        if( !pData ) return;
        // die Streamdaten sind hier Null basiert, so wie wir es brauchen
        nListLevel = *pData;

        if (pStyles && !bVer67)
        {
            /*
            #94672#
            if this is the case, then if the numbering is actually stored in
            winword 6 format, and its likely that sprmPIlvl has been abused
            to set the ww6 list level information which we will need when we
            reach the true ww6 list def.  So set it now
            */
            pStyles->nWwNumLevel = nListLevel;
        }

        if (WW8ListManager::nMaxLevel <= nListLevel )
            nListLevel = WW8ListManager::nMaxLevel;
        else if
           (
             (USHRT_MAX > nLFOPosition) &&
             (WW8ListManager::nMaxLevel > nListLevel)
           )
        {
            RegisterNumFmt(nLFOPosition, nListLevel);
            nLFOPosition = USHRT_MAX;
            nListLevel  = WW8ListManager::nMaxLevel;
        }
    }
}

void SwWW8ImplReader::Read_LFOPosition(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // aktueller Level ist hier zu Ende, was ist zu tun ???
        nLFOPosition = USHRT_MAX;
        nListLevel = WW8ListManager::nMaxLevel;
    }
    else
    {
        // Sicherheitspruefung auf NIL Pointer
        if( !pData )
            return;
        short nData = SVBT16ToShort( pData );
        if( 0 >= nData )
        {
            /*
            #94672# discussion
            If you have a paragraph in word with left and/or hanging indent
            and remove its numbering, then the indentation appears to get
            reset, but not back to the base style, instead its goes to a blank
            setting.
            Unless its a broken ww6 list in 97 in which case more hackery is
            required, some more details about that in
            ww8par6.cxx#SwWW8ImplReader::Read_LR
            */

            if (pAktColl)
            {
                pAktColl->SetAttr(*GetDfltAttr( RES_PARATR_NUMRULE));
                pAktColl->SetAttr(SvxLRSpaceItem());    //#94672#
            }
            else if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
            {
                if (pTxtNode->GetNum())
                {
                    pTxtNode->SwCntntNode::SetAttr(
                        *GetDfltAttr(RES_PARATR_NUMRULE));
                    pTxtNode->UpdateNum(SwNodeNum(NO_NUMBERING));
                }
                if (pTxtNode->GetOutlineNum())
                    pTxtNode->UpdateOutlineNum(SwNodeNum(NO_NUM));
                pTxtNode->SwCntntNode::SetAttr(SvxLRSpaceItem()); //#94672#
            }
            nLFOPosition = USHRT_MAX;
        }
        else
        {
            nLFOPosition = (sal_uInt16)nData-1;
            /*
            #94672#
            If we are a ww8+ style with ww7- style lists then there is a
            bizarre broken word bug where when the list is removed from a para
            the ww6 list first line indent still affects the first line
            indentation.  Setting this flag will allow us to recover from this
            braindeadness
            */
            if (pAktColl && (nLFOPosition == 2047-1))
                pCollA[nAktColl].bHasBrokenWW6List = true;

            // die Streamdaten sind hier 1 basiert, wir ziehen EINS ab
            if (USHRT_MAX > nLFOPosition)
            {
                if (nLFOPosition != 2047-1) //Normal ww8+ list behaviour
                {
                    if (WW8ListManager::nMaxLevel == nListLevel)
                        nListLevel = 0;
                    else if (WW8ListManager::nMaxLevel > nListLevel)
                    {
                        RegisterNumFmt(nLFOPosition, nListLevel);
                        nLFOPosition = USHRT_MAX;
                        nListLevel = WW8ListManager::nMaxLevel;
                    }
                }
                else if (pPlcxMan && pPlcxMan->HasParaSprm(0xC63E))
                {
                    /*
                     #i8114# Horrific backwards compatible ww7- lists in ww8+
                     docs
                    */
                    Read_ANLevelNo(13 /*equiv ww7- sprm no*/, &nListLevel, 1);
                }
            }
        }
    }
}

// -------------------------------------------------------------------
// ------------------------- Reading Controls ------------------------
// -------------------------------------------------------------------

bool SwWW8ImplReader::ImportFormulaControl(WW8FormulaControl &aFormula,
    WW8_CP nStart, SwWw8ControlType nWhich )
{
    bool bRet=false;
    /*
     * Save the reader state and process the sprms for this anchor cp.
     * Doing so will set the nPicLocFc to the offset to find the hypertext
     * data in the data stream.
     */
    WW8_CP nEndCp = nStart+1; //Only interested in the single 0x01 character

    WW8ReaderSave aSave(this,nStart);

    WW8PLCFManResult aRes;
    nStart = pPlcxMan->Where();
    while(nStart <= nEndCp)
    {
        if ( pPlcxMan->Get(&aRes)
            && aRes.pMemPos && aRes.nSprmId )
        {
            //only interested in sprms which would set nPicLocFc
            if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
            {
                Read_PicLoc( aRes.nSprmId, aRes.pMemPos +
                    mpSprmParser->DistanceToData(aRes.nSprmId), 4);
                break;
            }
        }
        (*pPlcxMan)++;
        nStart = pPlcxMan->Where();
    }
    ULONG nOffset = nPicLocFc;
    aSave.Restore(this);

    ULONG nOldPos = pDataStream->Tell();
    WW8_PIC aPic;
    pDataStream->Seek( nOffset);
    PicRead( pDataStream, &aPic, bVer67);

    if((aPic.lcb > 0x3A) && !pDataStream->GetError() )
    {
        pDataStream->Seek( nPicLocFc + aPic.cbHeader );
        aFormula.FormulaRead(nWhich,pDataStream);
        bRet = true;
    }

    /*
     There is a problem with aPic, the WW8_PIC is always used even though it
     is too big for the WW95 files, it needs to be modified to check the
     version C.
     */
    pDataStream->Seek( nOldPos );
    return(bRet);
}

sal_Bool SwMSConvertControls::InsertFormula(WW8FormulaControl &rFormula)
{
    sal_Bool bRet = sal_False;

    const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
        GetServiceFactory();

    if(!rServiceFactory.is())
        return sal_False;

    awt::Size aSz;
    uno::Reference< form::XFormComponent> xFComp;

    if ((bRet = rFormula.Import(rServiceFactory, xFComp, aSz)))
    {
        uno::Reference <drawing::XShape> xShapeRef;
        if ((bRet = InsertControl(xFComp, aSz, &xShapeRef, false)))
            GetShapes()->add(xShapeRef);
    }
    return bRet;
}

void WW8FormulaControl::FormulaRead(SwWw8ControlType nWhich,
    SvStream *pDataStream)
{
    UINT8 nField;
    UINT8 nHeaderByte;

    int nType=0;
    *pDataStream >> nHeaderByte;
    if (nHeaderByte == 0xFF) //Guesswork time, difference between 97 and 95 ?
    {
        pDataStream->SeekRel(3);
        *pDataStream >> nHeaderByte;
        nType=1;
    }
    fUnknown = nHeaderByte & 0x3;
    fDropdownIndex = (nHeaderByte & 0xFC) >> 2;
    *pDataStream >> nField;
    fToolTip = nField & 0x01;
    fNoMark = (nField & 0x02)>>1;
    fUseSize = (nField & 0x04)>>2;
    fNumbersOnly= (nField & 0x08)>>3;
    fDateOnly = (nField & 0x10)>>4;
    fUnused = (nField & 0xE0)>>5;
    *pDataStream >> nSize;

    *pDataStream >> hpsCheckBox;
    if (nType == 0)
        pDataStream->SeekRel(2); //Guess

    rtl_TextEncoding eEnc = rRdr.eStructCharSet;
    sTitle = !nType ? WW8ReadPString(*pDataStream, eEnc, true)
                    : WW8Read_xstz(*pDataStream, 0, true);

    if (nWhich == WW8_CT_CHECKBOX)
    {
        *pDataStream >> nDefaultChecked;
        nChecked = nDefaultChecked;
        //Don't know the details yet
        switch (nHeaderByte)
        {
            case 0x65: //01100101
                //use defaults ?
                break;
            case 0x1: //00000001
                ASSERT(nChecked, "expected to see checked, report to cmc (minor)");
                //swap to unchecked from checked ?
                nChecked = false;
                break;
            case 0x5: //00000101
                //change to checked
                nChecked = true;
                break;
            default:
                ASSERT(!this, "unknown option, please report to cmc");
                break;
        }
    }
    else if (nWhich == WW8_CT_DROPDOWN)
        *pDataStream >> nChecked;
    else
        sDefault = !nType ? WW8ReadPString(*pDataStream, eEnc, true)
                          : WW8Read_xstz(*pDataStream, 0, true);

    sFormatting = !nType ? WW8ReadPString(*pDataStream, eEnc, true)
                           : WW8Read_xstz(*pDataStream, 0, true);

    sHelp = !nType ? WW8ReadPString(*pDataStream, eEnc, true)
                     : WW8Read_xstz(*pDataStream, 0, true);

    if (nWhich == WW8_CT_DROPDOWN)      //is this the case ?
        fToolTip = true;

    if( fToolTip )
        sToolTip = !nType ? WW8ReadPString(*pDataStream, eEnc, true)
                               : WW8Read_xstz(*pDataStream, 0, true);

    if (nWhich == WW8_CT_DROPDOWN)
    {
        bool bAllOk = true;
        pDataStream->SeekRel(4 * (nType ? 2 : 1));
        sal_uInt16 nDummy;
        *pDataStream >> nDummy;
        sal_uInt32 nNoStrings;
        if (!nType)
        {
            sal_uInt16 nWord95NoStrings;
            *pDataStream >> nWord95NoStrings;
            nNoStrings = nWord95NoStrings;
            *pDataStream >> nWord95NoStrings;
            if (nNoStrings != nWord95NoStrings)
                bAllOk = false;
            nNoStrings = nWord95NoStrings;
            sal_uInt16 nDummy2;
            *pDataStream >> nDummy2;
            if (nDummy2 != 0)
                bAllOk = false;
            *pDataStream >> nDummy2;
            if (nDummy2 != 0xA)
                bAllOk = false;
            if (!bAllOk)    //Not as expected, don't risk it at all.
                nNoStrings = 0;
            for (sal_uInt16 nI = 0; nI < nNoStrings; ++nI)
                pDataStream->SeekRel(2);
        }
        else
        {
            if (nDummy != 0xFFFF)
                bAllOk = false;
            *pDataStream >> nNoStrings;
        }
        ASSERT(bAllOk,
            "Unknown formfield dropdown list structure. Report to cmc");
        if (!bAllOk)    //Not as expected, don't risk it at all.
            nNoStrings = 0;
        maListEntries.reserve(nNoStrings);
        for (sal_uInt32 nI = 0; nI < nNoStrings; ++nI)
        {
            String sEntry = !nType ? WW8ReadPString(*pDataStream, eEnc, false)
                           : WW8Read_xstz(*pDataStream, 0, false);
            maListEntries.push_back(sEntry);
        }
    }
}

WW8FormulaListBox::WW8FormulaListBox(SwWW8ImplReader &rR)
    : WW8FormulaControl( CREATE_CONST_ASC(SL::aListBox), rR)
{
}

//Miserable hack to get a hardcoded guesstimate of the size of a list dropdown
//box's first entry to set as the lists default size
awt::Size SwWW8ImplReader::MiserableDropDownFormHack(const String &rString,
    uno::Reference<beans::XPropertySet>& rPropSet)
{
    awt::Size aRet;
    const struct CtrlFontMapEntry
    {
        USHORT nWhichId;
        const sal_Char* pPropNm;
    }
    aMapTable [] =
    {
        RES_CHRATR_COLOR,           "TextColor",
        RES_CHRATR_FONT,            "FontName",
        RES_CHRATR_FONTSIZE,        "FontHeight",
        RES_CHRATR_WEIGHT,          "FontWeight",
        RES_CHRATR_UNDERLINE,       "FontUnderline",
        RES_CHRATR_CROSSEDOUT,      "FontStrikeout",
        RES_CHRATR_POSTURE,         "FontSlant",
        0,                          0
    };


    Font aFont;
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        rPropSet->getPropertySetInfo();

    uno::Any aTmp;
    for( const CtrlFontMapEntry* pMap = aMapTable; pMap->nWhichId; ++pMap )
    {
        BOOL bSet = TRUE;
        const SfxPoolItem* pItem = GetFmtAttr( pMap->nWhichId );
        ASSERT(pItem, "Impossible");
        if (!pItem)
            continue;

        switch ( pMap->nWhichId )
        {
        case RES_CHRATR_COLOR:
            {
                String pNm;
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("TextColor")))
                {
                    aTmp <<= (sal_Int32)((SvxColorItem*)pItem)->GetValue().GetColor();
                    rPropSet->setPropertyValue(pNm, aTmp);
                }
            }
            aFont.SetColor(((SvxColorItem*)pItem)->GetValue());
            break;
        case RES_CHRATR_FONT:
            {
                const SvxFontItem *pFontItem = (SvxFontItem *)pItem;
                String pNm;
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontStyleName")))
                {
                    aTmp <<= rtl::OUString( pFontItem->GetStyleName());
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontFamily")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetFamily();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontCharset")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetCharSet();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontPitch")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetPitch();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }

                aTmp <<= rtl::OUString( pFontItem->GetFamilyName());
                aFont.SetName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetFamily( pFontItem->GetFamily() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }
            break;

        case RES_CHRATR_FONTSIZE:
            {
                Size aSize( aFont.GetSize().Width(),
                            ((SvxFontHeightItem*)pItem)->GetHeight() );
                aTmp <<= ((float)aSize.Height()) / 20.0;

                aFont.SetSize(OutputDevice::LogicToLogic(aSize, MAP_TWIP,
                    MAP_100TH_MM));
            }
            break;

        case RES_CHRATR_WEIGHT:
            aTmp <<= (float)VCLUnoHelper::ConvertFontWeight(
                                        ((SvxWeightItem*)pItem)->GetWeight() );
            aFont.SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
            break;

        case RES_CHRATR_UNDERLINE:
            aTmp <<= (sal_Int16)(((SvxUnderlineItem*)pItem)->GetUnderline());
            aFont.SetUnderline(((SvxUnderlineItem*)pItem)->GetUnderline());
            break;

        case RES_CHRATR_CROSSEDOUT:
            aTmp <<= (sal_Int16)( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
            aFont.SetStrikeout( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
            break;

        case RES_CHRATR_POSTURE:
            aTmp <<= (sal_Int16)( ((SvxPostureItem*)pItem)->GetPosture() );
            aFont.SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
            break;

        default:
            bSet = FALSE;
            break;
        }

        if (bSet && xPropSetInfo->hasPropertyByName(C2U(pMap->pPropNm)))
            rPropSet->setPropertyValue(C2U(pMap->pPropNm), aTmp);
    }
    // now calculate the size of the control
    OutputDevice* pOut = Application::GetDefaultDevice();
    ASSERT(pOut, "Impossible");
    if (pOut)
    {
        pOut->Push( PUSH_FONT | PUSH_MAPMODE );
        pOut->SetMapMode( MapMode( MAP_100TH_MM ));
        pOut->SetFont( aFont );
        aRet.Width  = pOut->GetTextWidth(rString);
        aRet.Width += 500; //plus size of button, total hack territory
        aRet.Height = pOut->GetTextHeight();
        pOut->Pop();
    }
    return aRet;
}

sal_Bool WW8FormulaListBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference<uno::XInterface> xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.form.component.ComboBox"));
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference<form::XFormComponent>(xCreate, uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference<beans::XPropertySet> xPropSet(xCreate, uno::UNO_QUERY);

    uno::Any aTmp;
    if (sTitle.Len())
        aTmp <<= rtl::OUString(sTitle);
    else
        aTmp <<= rtl::OUString(sName);
    xPropSet->setPropertyValue(C2U("Name"), aTmp );

    if (sToolTip.Len())
    {
        aTmp <<= rtl::OUString(sToolTip);
        xPropSet->setPropertyValue(C2U("HelpText"), aTmp );
    }

    sal_Bool bDropDown(sal_True);
    xPropSet->setPropertyValue(C2U("Dropdown"), cppu::bool2any(bDropDown));

    if (!maListEntries.empty())
    {
        sal_uInt32 nLen = maListEntries.size();
        uno::Sequence< ::rtl::OUString > aListSource(nLen);
        for (sal_uInt32 nI = 0; nI < nLen; ++nI)
            aListSource[nI] = rtl::OUString(maListEntries[nI]);
        aTmp <<= aListSource;
        xPropSet->setPropertyValue(C2U("StringItemList"), aTmp );

        aTmp <<= aListSource[0];
        xPropSet->setPropertyValue(C2U("DefaultText"), aTmp );

        rSz = rRdr.MiserableDropDownFormHack(maListEntries[0], xPropSet);
    }
    else
    {
        static const sal_Unicode aBlank[] =
        {
            0x2002,0x2002,0x2002,0x2002,0x2002
        };
        rSz = rRdr.MiserableDropDownFormHack(String(aBlank), xPropSet);
    }

    return sal_True;
}

WW8FormulaCheckBox::WW8FormulaCheckBox(SwWW8ImplReader &rR)
    : WW8FormulaControl( CREATE_CONST_ASC(SL::aCheckBox), rR)
{
}

sal_Bool WW8FormulaCheckBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.form.component.CheckBox"));
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent >( xCreate, uno::UNO_QUERY );
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = 16 * hpsCheckBox;
    rSz.Height = 16 * hpsCheckBox;

    uno::Any aTmp;
    if (sTitle.Len())
        aTmp <<= rtl::OUString(sTitle);
    else
        aTmp <<= rtl::OUString(sName);
    xPropSet->setPropertyValue(C2U("Name"), aTmp );

#if 0
    aTmp <<= (sal_Int16)nDefaultChecked;
    xPropSet->setPropertyValue(C2U("DefaultState"), aTmp);

    aTmp <<= (sal_Int16)nChecked;
    xPropSet->setPropertyValue(C2U("State"), aTmp);
#else
    aTmp <<= (sal_Int16)nChecked;
    xPropSet->setPropertyValue(C2U("DefaultState"), aTmp);
#endif

    if( sToolTip.Len() )
    {
        aTmp <<= rtl::OUString(sToolTip);
        xPropSet->setPropertyValue(C2U("HelpText"), aTmp );
    }

    return sal_True;

}

WW8FormulaEditBox::WW8FormulaEditBox(SwWW8ImplReader &rR)
    : WW8FormulaControl( CREATE_CONST_ASC(SL::aTextField) ,rR)
{
}

sal_Bool SwMSConvertControls::InsertControl(
    const uno::Reference< form::XFormComponent > & rFComp,
    const awt::Size& rSize, uno::Reference< drawing::XShape > *pShape,
    BOOL bFloatingCtrl)
{
    const uno::Reference< container::XIndexContainer > &rComps = GetFormComps();
    uno::Any aTmp( &rFComp, ::getCppuType((const uno::Reference<
        form::XFormComponent >*)0) );
    rComps->insertByIndex( rComps->getCount(), aTmp );

    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory =
        GetServiceFactory();
    if( !rServiceFactory.is() )
        return sal_False;

    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.drawing.ControlShape"));
    if( !xCreate.is() )
        return sal_False;

    uno::Reference< drawing::XShape > xShape =
        uno::Reference< drawing::XShape >(xCreate, uno::UNO_QUERY);

    DBG_ASSERT(xShape.is(), "XShape nicht erhalten")
    xShape->setSize(rSize);

    uno::Reference< beans::XPropertySet > xShapePropSet(
        xCreate, uno::UNO_QUERY );

    //I lay a small bet that this will change to
    //INT16 nTemp=TextContentAnchorType::AS_CHARACTER;
    INT16 nTemp;
    if (bFloatingCtrl)
        nTemp= text::TextContentAnchorType_AT_PARAGRAPH;
    else
        nTemp= text::TextContentAnchorType_AS_CHARACTER;

    aTmp <<= nTemp;
    xShapePropSet->setPropertyValue(C2U("AnchorType"), aTmp );

    nTemp= text::VertOrientation::TOP;
    aTmp <<= nTemp;
    xShapePropSet->setPropertyValue(C2U("VertOrient"), aTmp );

    uno::Reference< text::XText >  xDummyTxtRef;
    uno::Reference< text::XTextRange >  xTxtRg =
        new SwXTextRange( *pPaM, xDummyTxtRef );

    aTmp.setValue(&xTxtRg,::getCppuType((
        uno::Reference< text::XTextRange >*)0));
    xShapePropSet->setPropertyValue(C2U("TextRange"), aTmp );

    // Das Control-Model am Control-Shape setzen
    uno::Reference< drawing::XControlShape >  xControlShape( xShape,
        uno::UNO_QUERY );
    uno::Reference< awt::XControlModel >  xControlModel( rFComp,
        uno::UNO_QUERY );
    xControlShape->setControl( xControlModel );

    if (pShape)
        *pShape = xShape;

    return sal_True;
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
