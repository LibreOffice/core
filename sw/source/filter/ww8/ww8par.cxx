/*************************************************************************
 *
 *  $RCSfile: ww8par.cxx,v $
 *
 *  $Revision: 1.138 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:12:30 $
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
 *  Contributor(s): cmc@openoffice.org, tono@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifdef PCH
#include "filt_pch.hxx"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#include <svtools/docpasswdrequest.hxx>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <svx/cscoitem.hxx>
#endif
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx> // SvxLRSpaceItem
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif
#ifndef SVX_MSCODEC_HXX
#include <svx/mscodec.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif

#include <svtools/fltrcfg.hxx>

#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // class SwPageDesc
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>           // ReadFilterFlags
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _DOCUFLD_HXX //autogen
#include <docufld.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif
#ifndef _VIEWSH_HXX     // for the pagedescname from the ShellRes
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX   // for the pagedescname from the ShellRes
#include <shellres.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>            // ERR_WW8_...
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>          // class SwTableLines, ...
#endif
// #i18732#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif


#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif

#include <algorithm>
#include <functional>

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif


#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"          // class WW8RStyle, class WW8AnchorPara
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif

#define MM_250 1417             // WW-Default fuer Hor. Seitenraender: 2.5 cm
#define MM_200 1134             // WW-Default fuer u.Seitenrand: 2.0 cm

using namespace sw::util;
using namespace sw::types;

SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr )
    : SvxMSDffManager(*rRdr.pTableStream, rRdr.pWwFib->fcDggInfo,
        rRdr.pDataStream, 0, 0, COL_WHITE, 12, rRdr.pStrm,
        rRdr.maTracer.GetTrace()),
    rReader(rRdr), pFallbackStream(0), pOldEscherBlipCache(0)
{
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

UINT32 SwMSDffManager::GetFilterFlags()
{
    UINT32 nFlags(0);
    if (const SvtFilterOptions* pOpt = SvtFilterOptions::Get())
    {
        if (pOpt->IsMathType2Math())
            nFlags |= OLE_MATHTYPE_2_STARMATH;
        if (pOpt->IsExcel2Calc())
            nFlags |= OLE_EXCEL_2_STARCALC;
        if (pOpt->IsPowerPoint2Impress())
            nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
        if (pOpt->IsWinWord2Writer())
            nFlags |= OLE_WINWORD_2_STARWRITER;
    }
    return nFlags;
}

/*
 * I would like to override the default OLE importing to add a test
 * and conversion of OCX controls from their native OLE type into our
 * native nonOLE Form Control Objects.
 *
 * cmc
 */
SdrObject* SwMSDffManager::ImportOLE( long nOLEId, const Graphic& rGrf,
                                        const Rectangle& rBoundRect ) const
{
    SdrObject* pRet = 0;
    String sStorageName;
    SvStorageRef xSrcStg, xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        SvStorageRef xSrc = xSrcStg->OpenStorage( sStorageName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );
        ASSERT(rReader.pFormImpl, "No Form Implementation!");
        STAR_REFERENCE( drawing::XShape ) xShape;
        if ( (!(rReader.bIsHeader || rReader.bIsFooter)) &&
            rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,true))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
        {
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, pStData, nSvxMSDffOLEConvFlags );
        }
    }
    return pRet;
}

void SwMSDffManager::DisableFallbackStream()
{
    ASSERT(!pFallbackStream || !pOldEscherBlipCache,
        "if you're recursive, you're broken");
    pFallbackStream = pStData2;
    pOldEscherBlipCache = pEscherBlipCache;
    pEscherBlipCache = 0;
    pStData2 = 0;
}

void SwMSDffManager::EnableFallbackStream()
{
    pStData2 = pFallbackStream;
    pEscherBlipCache = pOldEscherBlipCache;
    pOldEscherBlipCache = 0;
    pFallbackStream = 0;
}

USHORT SwWW8ImplReader::GetToggleAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleAttrFlags() : 0;
}

USHORT SwWW8ImplReader::GetToggleBiDiAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleBiDiAttrFlags() : 0;
}

void SwWW8ImplReader::SetToggleAttrFlags(USHORT nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleAttrFlags(nFlags);
}

void SwWW8ImplReader::SetToggleBiDiAttrFlags(USHORT nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleBiDiAttrFlags(nFlags);
}


SdrObject* SwMSDffManager::ProcessObj(SvStream& rSt,
                                       DffObjData& rObjData,
                                       void* pData,
                                       Rectangle& rTextRect,
                                       SdrObject* pObj
                                       )
{
    if( !rTextRect.IsEmpty() )
    {
        SvxMSDffImportData& rImportData = *(SvxMSDffImportData*)pData;
        SvxMSDffImportRec* pImpRec = new SvxMSDffImportRec;
        SvxMSDffImportRec* pTextImpRec = pImpRec;

        // fill Import Record with data
        pImpRec->nShapeId   = rObjData.nShapeId;
        pImpRec->eShapeType = rObjData.eShapeType;

        MSO_WrapMode eWrapMode( (MSO_WrapMode)GetPropertyValue(
                                                            DFF_Prop_WrapText,
                                                            mso_wrapSquare ) );
        rObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientAnchor,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientAnchor )
            ProcessClientAnchor( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientAnchorBuffer, pImpRec->nClientAnchorLen );

        rObjData.bClientData = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientData,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientData )
            ProcessClientData( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientDataBuffer, pImpRec->nClientDataLen );


        // process user (== Winword) defined parameters in 0xF122 record
        if(    maShapeRecords.SeekToContent( rSt,
                                             DFF_msofbtUDefProp,
                                             SEEK_FROM_CURRENT_AND_RESTART )
            && maShapeRecords.Current()->nRecLen )
        {
            UINT32  nBytesLeft = maShapeRecords.Current()->nRecLen;
            UINT32  nUDData;
            UINT16  nPID;
            while( 5 < nBytesLeft )
            {
                rSt >> nPID;
                if ( rSt.GetError() != 0 )
                    break;
                rSt >> nUDData;
                switch( nPID )
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390: pImpRec->nXRelTo = nUDData; break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392: pImpRec->nYRelTo = nUDData; break;
                    case 0x03BF: pImpRec->nLayoutInTableCell = nUDData; break;
                }
                if ( rSt.GetError() != 0 )
                    break;
                pImpRec->bHasUDefProp = TRUE;
                nBytesLeft  -= 6;
            }
        }

        //  Textrahmen, auch Title oder Outline
        SdrObject*  pOrgObj  = pObj;
        SdrRectObj* pTextObj = 0;
        UINT32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
        if( nTextId )
        {
            SfxItemSet aSet( pSdrModel->GetItemPool() );

            //Originally anything that as a mso_sptTextBox was created as a
            //textbox, this was changed for #88277# to be created as a simple
            //rect to keep impress happy. For the rest of us we'd like to turn
            //it back into a textbox again.
            FASTBOOL bTextFrame = (pImpRec->eShapeType == mso_sptTextBox);
            if (!bTextFrame)
            {
                //Either
                //a) its a simple text object or
                //b) its a rectangle with text and square wrapping.
                bTextFrame =
                (
                    (pImpRec->eShapeType == mso_sptTextSimple) ||
                    (
                        (pImpRec->eShapeType == mso_sptRectangle)
                        && (eWrapMode == mso_wrapSquare)
                        && ShapeHasText(pImpRec->nShapeId, rObjData.rSpHd.GetRecBegFilePos() )
                    )
                );
            }

            if (bTextFrame)
            {
                delete pObj;
                pObj = pOrgObj = 0;
            }

            // Distance of Textbox to it's surrounding Autoshape
            INT32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            INT32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            INT32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            INT32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            INT32 nTextRotationAngle=0;
            bool bVerticalText = false;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)(GetPropertyValue(
                    DFF_Prop_txflTextFlow) & 0xFFFF);
                switch( eTextFlow )
                {
                    case mso_txflBtoT:
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflVertN:
                    case mso_txflTtoBN:
                        nTextRotationAngle = 27000;
                        break;
                    case mso_txflTtoBA:
                        bVerticalText = true;
                    break;
                    case mso_txflHorzA:
                        bVerticalText = true;
                        nTextRotationAngle = 9000;
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                while (nTextRotationAngle > 360000)
                    nTextRotationAngle-=9000;
                switch (nTextRotationAngle)
                {
                    case 9000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            INT32 nOldTextLeft = nTextLeft;
                            INT32 nOldTextRight = nTextRight;
                            INT32 nOldTextTop = nTextTop;
                            INT32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextBottom;
                            nTextRight = nOldTextTop;
                            nTextTop = nOldTextLeft;
                            nTextBottom = nOldTextRight;
                        }
                        break;
                    case 27000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            INT32 nOldTextLeft = nTextLeft;
                            INT32 nOldTextRight = nTextRight;
                            INT32 nOldTextTop = nTextTop;
                            INT32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextTop;
                            nTextRight = nOldTextBottom;
                            nTextTop = nOldTextRight;
                            nTextBottom = nOldTextLeft;
                        }
                        break;
                    default:
                        break;
                }
            }

            pTextObj = new SdrRectObj(OBJ_TEXT, rTextRect);
            pTextImpRec = new SvxMSDffImportRec(*pImpRec);

            // Die vertikalen Absatzeinrueckungen sind im BoundRect mit drin,
            // hier rausrechnen
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            // Nur falls es eine einfache Textbox ist, darf der Writer
            // das Objekt durch einen Rahmen ersetzen, ansonsten
            if( bTextFrame )
            {
                SvxMSDffShapeInfo aTmpRec( 0, pImpRec->nShapeId );
                aTmpRec.bSortByShapeId = TRUE;

                USHORT nFound;
                if( GetShapeInfos()->Seek_Entry( &aTmpRec, &nFound ) )
                {
                    SvxMSDffShapeInfo& rInfo = *GetShapeInfos()->GetObject(nFound);
                    pTextImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                    pTextImpRec->bLastBoxInChain = rInfo.bLastBoxInChain;
                }
            }

            if( !pObj )
                ApplyAttributes( rSt, aSet, rObjData.eShapeType, rObjData.nSpFlags );

            bool bFitText = false;
            if (GetPropertyValue(DFF_Prop_FitTextToShape) & 2)
            {
                aSet.Put( SdrTextAutoGrowHeightItem( TRUE ) );
                aSet.Put( SdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( SdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
                bFitText = true;
            }
            else
            {
                aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                aSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
                    pTextImpRec->bAutoWidth = true;
                break;
                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( TRUE ) );
                break;
            }

            // Abstaende an den Raendern der Textbox setzen
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pTextImpRec->nDxTextLeft    = nTextLeft;
            pTextImpRec->nDyTextTop     = nTextTop;
            pTextImpRec->nDxTextRight   = nTextRight;
            pTextImpRec->nDyTextBottom  = nTextBottom;

            // Textverankerung lesen
            if ( IsProperty( DFF_Prop_anchorText ) )
            {
                MSO_Anchor eTextAnchor =
                    (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText );

                SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_CENTER;
                BOOL bTVASet(FALSE);
                SdrTextHorzAdjust eTHA = SDRTEXTHORZADJUST_CENTER;
                BOOL bTHASet(FALSE);

                switch( eTextAnchor )
                {
                    case mso_anchorTop:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = TRUE;
                    }
                    break;
                    case mso_anchorTopCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;

                    case mso_anchorMiddle:
                        bTVASet = TRUE;
                    break;
                    case mso_anchorMiddleCentered:
                    {
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;
                    case mso_anchorBottom:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = TRUE;
                    }
                    break;
                    case mso_anchorBottomCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;
    /*
                    case mso_anchorTopBaseline:
                    case mso_anchorBottomBaseline:
                    case mso_anchorTopCenteredBaseline:
                    case mso_anchorBottomCenteredBaseline:
                    break;
    */
                }
                // Einsetzen
                if ( bTVASet )
                    aSet.Put( SdrTextVertAdjustItem( eTVA ) );
                if ( bTHASet )
                    aSet.Put( SdrTextHorzAdjustItem( eTHA ) );
            }

            pTextObj->SetMergedItemSet(aSet);
            pTextObj->SetModel(pSdrModel);

            if (bVerticalText)
                pTextObj->SetVerticalWriting(sal_True);

            if (nTextRotationAngle)
            {
                long nMinWH = rTextRect.GetWidth() < rTextRect.GetHeight() ?
                    rTextRect.GetWidth() : rTextRect.GetHeight();
                nMinWH /= 2;
                Point aPivot(rTextRect.TopLeft());
                aPivot.X() += nMinWH;
                aPivot.Y() += nMinWH;
                double a = nTextRotationAngle * nPi180;
                pTextObj->NbcRotate(aPivot, nTextRotationAngle, sin(a), cos(a));
            }

            // rotate text with shape ?
            if ( mnFix16Angle )
            {
                double a = mnFix16Angle * nPi180;
                pTextObj->NbcRotate( rObjData.rBoundRect.Center(), mnFix16Angle,
                    sin( a ), cos( a ) );
            }

            if( !pObj )
            {
                pObj = pTextObj;
            }
            else
            {
                if( pTextObj != pObj )
                {
                    SdrObject* pGroup = new SdrObjGroup;
                    pGroup->GetSubList()->NbcInsertObject( pObj );
                    pGroup->GetSubList()->NbcInsertObject( pTextObj );
                    if (pOrgObj == pObj)
                        pOrgObj = pGroup;
                    else
                        pOrgObj = pObj;
                    pObj = pGroup;
                }
            }
        }
        else if( !pObj )
        {
            // simple rectangular objects are ignored by ImportObj()  :-(
            // this is OK for Draw but not for Calc and Writer
            // cause here these objects have a default border
            pObj = new SdrRectObj(rTextRect);
            pOrgObj = pObj;
            pObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, rObjData.eShapeType, rObjData.nSpFlags );

            const SfxPoolItem* pPoolItem=NULL;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     FALSE, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( String(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        //Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x20)
            pImpRec->bDrawHell = TRUE;
        else
            pImpRec->bDrawHell = FALSE;
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x02)
            pImpRec->bHidden = TRUE;
        pTextImpRec->bDrawHell  = pImpRec->bDrawHell;
        pTextImpRec->bHidden = pImpRec->bHidden;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );
        pTextImpRec->nNextShapeId=pImpRec->nNextShapeId;

        if ( nTextId )
        {
            pTextImpRec->aTextId.nTxBxS = (UINT16)( nTextId >> 16 );
            pTextImpRec->aTextId.nSequence = (UINT16)nTextId;
        }

        pTextImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 114935L ) / 635L;
        pTextImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635L;
        pTextImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 114935L ) / 635L;
        pTextImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635L;
        // 16.16 fraction times total image width or height, as appropriate.

        if (SeekToContent(DFF_Prop_pWrapPolygonVertices, rSt))
        {
            delete pTextImpRec->pWrapPolygon;
            sal_uInt16 nNumElemVert, nNumElemMemVert, nElemSizeVert;
            rSt >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                pTextImpRec->pWrapPolygon = new Polygon(nNumElemVert);
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX, nY;
                    if (nElemSizeVert == 8)
                        rSt >> nX >> nY;
                    else
                    {
                        sal_Int16 nSmallX, nSmallY;
                        rSt >> nSmallX >> nSmallY;
                        nX = nSmallX;
                        nY = nSmallY;
                    }
                    (*(pTextImpRec->pWrapPolygon))[i].X() = nX;
                    (*(pTextImpRec->pWrapPolygon))[i].Y() = nY;
                }
            }
        }

        pImpRec->nCropFromTop = GetPropertyValue(
                                    DFF_Prop_cropFromTop, 0 );
        pImpRec->nCropFromBottom = GetPropertyValue(
                                    DFF_Prop_cropFromBottom, 0 );
        pImpRec->nCropFromLeft = GetPropertyValue(
                                    DFF_Prop_cropFromLeft, 0 );
        pImpRec->nCropFromRight = GetPropertyValue(
                                    DFF_Prop_cropFromRight, 0 );

        UINT32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );
        pImpRec->eLineStyle = (nLineFlags & 8)
                            ? (MSO_LineStyle)GetPropertyValue(
                                                DFF_Prop_lineStyle,
                                                mso_lineSimple )
                            : (MSO_LineStyle)USHRT_MAX;
        pTextImpRec->eLineStyle = pImpRec->eLineStyle;

        if( pImpRec->nShapeId )
        {
            // Import-Record-Liste ergaenzen
            if( pOrgObj )
            {
                pImpRec->pObj = pOrgObj;
                rImportData.aRecords.Insert( pImpRec );
            }

            if( pTextObj && (pOrgObj != pTextObj) )
            {
                // Modify ShapeId (must be unique)
                pImpRec->nShapeId |= 0x8000000;
                pTextImpRec->pObj = pTextObj;
                rImportData.aRecords.Insert( pTextImpRec );
            }

            // Eintrag in Z-Order-Liste um Zeiger auf dieses Objekt ergaenzen
            /*Only store objects which are not deep inside the tree*/
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( ( (ULONG)pImpRec->aTextId.nTxBxS ) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }
        else
            delete pImpRec;
    }

    return pObj;
}

/***************************************************************************
#  Spezial FastSave - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_StyleCode( USHORT, const BYTE* pData, short nLen )
{
    if (nLen < 0)
    {
        bCpxStyle = false;
        return;
    }
    USHORT nColl = SVBT16ToShort(pData);
    if (nColl < nColls)
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[nColl] );
        bCpxStyle = true;
    }
}

// Read_Majority ist fuer Majority ( 103 ) und Majority50 ( 108 )
void SwWW8ImplReader::Read_Majority( USHORT, const BYTE* , short )
{
}

//-----------------------------------------
//            Stack
//-----------------------------------------
void SwWW8FltControlStack::NewAttr(const SwPosition& rPos,
    const SfxPoolItem& rAttr)
{
    ASSERT(RES_TXTATR_FIELD != rAttr.Which(), "probably don't want to put"
        "fields into the control stack");
    ASSERT(RES_FLTR_REDLINE != rAttr.Which(), "probably don't want to put"
        "redlines into the control stack");
    SwFltControlStack::NewAttr(rPos, rAttr);
}

void SwWW8FltControlStack::SetAttr(const SwPosition& rPos, USHORT nAttrId,
    bool bTstEnde, long nHand)
{
    //Doing a textbox, and using the control stack only as a temporary
    //collection point for properties which will are not to be set into
    //the real document
    if (rReader.pPlcxMan && rReader.pPlcxMan->GetDoingDrawTextBox())
    {
        USHORT nCnt = Count();
        for (USHORT i=0; i < nCnt; ++i)
        {
            SwFltStackEntry* pEntry = (*this)[i];
            if (nAttrId == pEntry->pAttr->Which())
            {
                DeleteAndDestroy(i--);
                --nCnt;
            }
        }
    }
    else //Normal case, set the attribute into the document
        SwFltControlStack::SetAttr(rPos, nAttrId, bTstEnde, nHand);
}

long GetListFirstLineIndent(const SwNumFmt &rFmt)
{
    SvxAdjust eAdj = rFmt.GetNumAdjust();
    long nReverseListIndented;
    if (eAdj == SVX_ADJUST_RIGHT)
        nReverseListIndented = -rFmt.GetCharTextDistance();
    else if (eAdj == SVX_ADJUST_CENTER)
        nReverseListIndented = rFmt.GetFirstLineOffset()/2;
    else
        nReverseListIndented = rFmt.GetFirstLineOffset();
    return nReverseListIndented;
}

long lcl_GetTrueMargin(const SvxLRSpaceItem &rLR, const SwNumFmt &rFmt,
    long &rFirstLinePos)
{
    long nBodyIndent = rLR.GetTxtLeft();
    long nFirstLineDiff = rLR.GetTxtFirstLineOfst();
    rFirstLinePos = nBodyIndent + nFirstLineDiff;

    long nPseudoListBodyIndent = rFmt.GetAbsLSpace();
    long nReverseListIndented = GetListFirstLineIndent(rFmt);
    long nExtraListIndent = nPseudoListBodyIndent + nReverseListIndented;

    return nExtraListIndent > 0 ? nExtraListIndent : 0;
}

void SyncIndentWithList(SvxLRSpaceItem &rLR, const SwNumFmt &rFmt)
{
    long nWantedFirstLinePos;
    long nExtraListIndent = lcl_GetTrueMargin(rLR, rFmt, nWantedFirstLinePos);
    rLR.SetTxtLeft(nWantedFirstLinePos - nExtraListIndent);
    rLR.SetTxtFirstLineOfst(0);
}

const SwNumFmt* SwWW8FltControlStack::GetNumFmtFromStack(const SwPosition &rPos,
    const SwTxtNode &rTxtNode)
{
    const SwNumFmt *pRet = 0;
    const SfxPoolItem *pItem = GetStackAttr(rPos, RES_FLTR_NUMRULE);
    if (pItem && rTxtNode.GetNum())
    {
        String sName(((SfxStringItem*)pItem)->GetValue());
        const SwNodeNum *pNum = rTxtNode.GetNum();
        if (pNum && NO_NUM > pNum->GetLevel())
        {
            const SwNumRule *pRule = pDoc->FindNumRulePtr(sName);
            BYTE nLvl = GetRealLevel(pNum->GetLevel());
            pRet = &(pRule->Get(nLvl));
        }
    }
    return pRet;
}

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry* pEntry)
{
    using sw::util::AdjustTabs;
    switch( pEntry->pAttr->Which() )
    {
        case RES_PARATR_TABSTOP:
            {
                /*
                Loop over the affected nodes and adjust the 0 based word style
                tabstops to writer style tabstops relative to the paragraph
                indent
                */
                SwPaM aRegion(rTmpPos);
                if (pEntry->MakeRegion(pDoc, aRegion, false))
                {
                    SvxTabStopItem aTabStops(*(SvxTabStopItem*)pEntry->pAttr);
                    ULONG nStart = aRegion.Start()->nNode.GetIndex();
                    ULONG nEnd = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwCntntNode *pNd =
                            pDoc->GetNodes()[nStart]->GetCntntNode();
                        if (!pNd)
                            continue;
                        const SvxLRSpaceItem &rLR = (const SvxLRSpaceItem&)
                            pNd->GetAttr(RES_LR_SPACE);
                        AdjustTabs(aTabStops, 0, rLR.GetTxtLeft());
                        pNd->SetAttr(aTabStops);
                    }
                }
            }
        break;
        case RES_LR_SPACE:
            {
                /*
                 Loop over the affect nodes and
                 a) convert the word style absolute indent to indent relative
                  to any numbering indent active on the nodes
                 b) adjust the writer style tabstops relative to the old
                  paragraph indent to be relative to the new paragraph indent
                */
                using namespace sw::util;
                SwPaM aRegion(rTmpPos);
                if (pEntry->MakeRegion(pDoc, aRegion, false))
                {
                    SvxLRSpaceItem aNewLR( *(SvxLRSpaceItem*)pEntry->pAttr );
                    ULONG nStart = aRegion.Start()->nNode.GetIndex();
                    ULONG nEnd   = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (!pNode || !pNode->IsTxtNode())
                            continue;

                        SwCntntNode* pNd = (SwCntntNode*)pNode;
                        SvxLRSpaceItem aOldLR = (const SvxLRSpaceItem&)
                                pNd->GetAttr(RES_LR_SPACE);

                        SwTxtNode *pTxtNode = (SwTxtNode*)pNode;
                        const SwNumFmt *pNum = 0;
                        pNum = GetNumFmtFromStack(*aRegion.GetPoint(),
                            *pTxtNode);
                        if (!pNum)
                            pNum = GetNumFmtFromTxtNode(*pTxtNode);

                        if (pNum)
                            SyncIndentWithList(aNewLR, *pNum);

                        if (aNewLR == aOldLR)
                            continue;

                        pNd->SetAttr(aNewLR);

                        SvxTabStopItem aTabs = (const SvxTabStopItem&)
                            pNd->GetAttr(RES_PARATR_TABSTOP);
                        if (AdjustTabs(aTabs, aOldLR.GetTxtLeft(),
                            aNewLR.GetTxtLeft()))
                        {
                            pNd->SetAttr(aTabs);
                        }
                    }
                }
            }
            break;
        case RES_TXTATR_FIELD:
            ASSERT(!this, "What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;
        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (pEntry->MakeRegion(pDoc, aRegion, false))
                {
                    SwFrmFmt *pFrm;
                    //If we have just one single inline graphic then
                    //don't insert a field for the single frame, set
                    //the frames hyperlink field attribute directly.
                    if ((pFrm = rReader.ContainsSingleInlineGraphic(aRegion)))
                    {
                        const SwFmtINetFmt *pAttr = (const SwFmtINetFmt *)
                            pEntry->pAttr;
                        SwFmtURL aURL;
                        aURL.SetURL(pAttr->GetValue(), false);
                        aURL.SetTargetFrameName(pAttr->GetTargetFrame());
                        pFrm->SetAttr(aURL);
                    }
                    else
                        pDoc->Insert(aRegion, *pEntry->pAttr);
                }
            }
            break;
        default:
            SwFltControlStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
    }
}

const SfxPoolItem* SwWW8FltControlStack::GetFmtAttr(const SwPosition& rPos,
    USHORT nWhich)
{
    const SfxPoolItem *pItem = GetStackAttr(rPos, nWhich);
    if (!pItem)
    {
        const SwCntntNode *pNd = pDoc->GetNodes()[ rPos.nNode ]->GetCntntNode();
        if (!pNd)
            pItem = &pDoc->GetAttrPool().GetDefaultItem(nWhich);
        else
        {
            /*
            If we're hunting for the indent on a paragraph and need to use the
            parent style indent, then return the indent in msword format, and
            not writer format, because that's the style that the filter works
            in (naturally)
            */
            if (nWhich == RES_LR_SPACE)
            {
                SfxItemState eState = SFX_ITEM_DEFAULT;
                if (const SwAttrSet *pSet = pNd->GetpSwAttrSet())
                    eState = pSet->GetItemState(RES_LR_SPACE, false);
                if (eState != SFX_ITEM_SET)
                    pItem = &(rReader.pCollA[rReader.nAktColl].maWordLR);
            }

            if (!pItem)
                pItem = &pNd->GetAttr(nWhich);
        }
    }
    return pItem;
}

const SfxPoolItem* SwWW8FltControlStack::GetStackAttr(const SwPosition& rPos,
    USHORT nWhich)
{
    SwNodeIndex aNode( rPos.nNode, -1 );
    USHORT nIdx = rPos.nContent.GetIndex();

    USHORT nSize = Count();
    while (nSize)
    {
        const SwFltStackEntry* pEntry = (*this)[ --nSize ];
        if (pEntry->pAttr->Which() == nWhich)
        {
            if ( (pEntry->bLocked) || (
                (pEntry->nMkNode <= aNode) && (pEntry->nPtNode >= aNode) &&
                (pEntry->nMkCntnt <= nIdx) && (pEntry->nPtCntnt >= nIdx) ) )
            {
                return (const SfxPoolItem*)pEntry->pAttr;
            }
        }
    }
    return 0;
}

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->nNode, -1);
    USHORT nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    USHORT nEndIdx = pEnd->nContent.GetIndex();

    //We don't support deleting content that is over one node, or removing a node.
    ASSERT(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (USHORT nSize = Count(); nSize > 0;)
    {
        SwFltStackEntry* pEntry = (*this)[--nSize];

        bool bEntryStartAfterSelStart =
            (pEntry->nMkNode == aStartNode && pEntry->nMkCntnt >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (pEntry->nMkNode == aEndNode && pEntry->nMkCntnt <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!pEntry->bLocked)
        {
            bEntryEndAfterSelStart =
                (pEntry->nPtNode == aStartNode && pEntry->nPtCntnt >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (pEntry->nPtNode == aEndNode && pEntry->nPtCntnt <= nEndIdx);
        }

        bool bTotallyContained = false;
        if (
             bEntryStartAfterSelStart && bEntryStartBeforeSelEnd &&
             bEntryEndAfterSelStart && bEntryEndBeforeSelEnd
           )
        {
           bTotallyContained = true;
        }

        if (bTotallyContained)
        {
            //after start, before end, delete
            DeleteAndDestroy(nSize);
            continue;
        }

        xub_StrLen nCntntDiff = nEndIdx - nStartIdx;

        //to be adjusted
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                //move start to new start
                pEntry->nMkNode = aStartNode;
                pEntry->nMkCntnt = nStartIdx;
            }
            else
                pEntry->nMkCntnt -= nCntntDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
            {
                pEntry->nPtNode = aStartNode;
                pEntry->nPtCntnt = nStartIdx;
            }
            else
                pEntry->nPtCntnt -= nCntntDiff;
        }

        //That's what locked is, end equal to start, and nPtCntnt is invalid
        if (pEntry->bLocked)
            pEntry->nPtNode = pEntry->nMkNode;
    }
}

bool SwWW8FltRefStack::IsFtnEdnBkmField(const SwFmtFld& rFmtFld, USHORT& rBkmNo)
{
    const SwField* pFld = rFmtFld.GetFld();
    USHORT nSubType;
    return (pFld && (RES_GETREFFLD == pFld->Which())
            && ((REF_FOOTNOTE == (nSubType = pFld->GetSubType())) ||
                (REF_ENDNOTE  == nSubType))
            && ((SwGetRefField*)pFld)->GetSetRefName().Len()
                // find Sequence No of corresponding Foot-/Endnote
            && (USHRT_MAX != (rBkmNo = pDoc->FindBookmark(
                ((SwGetRefField*)pFld)->GetSetRefName() ))));
}

void SwWW8FltRefStack::SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry* pEntry)
{
    switch( pEntry->pAttr->Which() )
    {
        /*
        Look up these in our lists of bookmarks that were changed to
        variables, and replace the ref field with a var field, otherwise
        do normal (?) strange stuff
        */
        case RES_TXTATR_FIELD:
        {
            SwNodeIndex aIdx(pEntry->nMkNode, 1);
            SwPaM aPaM(aIdx, pEntry->nMkCntnt);

            SwFmtFld& rFmtFld   = *(SwFmtFld*)pEntry->pAttr;
            SwField* pFld = rFmtFld.GetFld();

            // OD 2004-03-19 - <NOT> got lost from revision 1.128 to 1.129
            if (!RefToVar(pFld,pEntry))
            {
                USHORT nBkmNo;
                if( IsFtnEdnBkmField(rFmtFld, nBkmNo) )
                {
                    SwBookmark& rBkMrk = pDoc->GetBookmark( nBkmNo );

                    const SwPosition& rBkMrkPos = rBkMrk.GetPos();

                    SwTxtNode* pTxt = rBkMrkPos.nNode.GetNode().GetTxtNode();
                    if( pTxt && rBkMrkPos.nContent.GetIndex() )
                    {
                        SwTxtAttr* pFtn = pTxt->GetTxtAttr(
                            rBkMrkPos.nContent.GetIndex()-1, RES_TXTATR_FTN );
                        if( pFtn )
                        {
                            USHORT nRefNo = ((SwTxtFtn*)pFtn)->GetSeqRefNo();

                            ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                            if( pFtn->GetFtn().IsEndNote() )
                                ((SwGetRefField*)pFld)->SetSubType(REF_ENDNOTE);
                        }
                    }
                }
            }

            pDoc->Insert(aPaM, *pEntry->pAttr);
            MoveAttrs(*aPaM.GetPoint());
        }
        break;
        case RES_FLTR_TOX:
        case RES_FLTR_BOOKMARK:
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
        default:
            ASSERT(!this, "EndStck used with non field, not what we want");
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
    }
}

/*
 For styles we will do our tabstop arithmetic in word style and adjust them to
 writer style after all the styles have been finished and the dust settles as
 to what affects what.

 For explicit attributes we turn the adjusted writer tabstops back into 0 based
 word indexes and we'll turn them back into writer indexes when setting them
 into the document. If explicit left indent exist which affects them, then this
 is handled when the explict left indent is set into the document
*/
void SwWW8ImplReader::Read_Tab(USHORT , const BYTE* pData, short nLen)
{
    if (nLen < 0)
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_TABSTOP);
        return;
    }

    short i;
    const BYTE* pDel = pData + 1;                       // Del - Array
    BYTE nDel = pData[0];
    const BYTE* pIns = pData + 2*nDel + 2;          // Ins - Array
    BYTE nIns = pData[nDel*2+1];
    WW8_TBD* pTyp = (WW8_TBD*)(pData + 2*nDel + 2*nIns + 2);// Typ - Array

    SvxTabStopItem aAttr(0, 0, SVX_TAB_ADJUST_DEFAULT);

    const SwTxtFmtColl* pSty = 0;
    USHORT nTabBase;
    if (pAktColl)               // StyleDef
    {
        nTabBase = pCollA[nAktColl].nBase;
        if (nTabBase < nColls)              // Based On
            pSty = (const SwTxtFmtColl*)pCollA[nTabBase].pFmt;
    }
    else
    {                                       // Text
        nTabBase = nAktColl;
        pSty = (const SwTxtFmtColl*)pCollA[nAktColl].pFmt;
    }

    bool bFound = false;
    while (pSty && !bFound)
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP, false,
            &pTabs) == SFX_ITEM_SET;
        if( bFound )
            aAttr = *((const SvxTabStopItem*)pTabs);
        else
        {

            USHORT nOldTabBase = nTabBase;
            // If based on another
            if (nTabBase < nColls)
                nTabBase = pCollA[nTabBase].nBase;

            if (
                    nTabBase < nColls &&
                    nOldTabBase != nTabBase &&
                    nTabBase != ww::stiNil
               )
            {
                pSty = (const SwTxtFmtColl*)pCollA[nTabBase].pFmt;
            }
            else
                pSty = 0;                           // gib die Suche auf
        }
    }

    //Now turn back into 0 based word-style index, during style import
    //we're already in word based 0 index, we only move into crackpot
    //writer style when we're finished importing the styles. For
    //explicit tabstops we'll have to adjust on setting into the
    //document
    long nLeftPMgn(0);
    if (pAktColl)
        nLeftPMgn = 0;  //During style import we are always 0 based (word style)
    else if (nAktColl < nColls && pCollA[nAktColl].pFmt)
    {
        //otherwise we are writer x based, so turn back into 0 based word style
        const SvxLRSpaceItem &rLR = pCollA[nAktColl].pFmt->GetLRSpace();
        nLeftPMgn = rLR.GetTxtLeft();
    }

    sw::util::AdjustTabs(aAttr, nLeftPMgn, 0);

    SvxTabStop aTabStop;
    for (i=0; i < nDel; ++i)
    {
        USHORT nPos = aAttr.GetPos(SVBT16ToShort(pDel + i*2));
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }

    for (i=0; i < nIns; ++i)
    {
        short nPos = SVBT16ToShort(pIns + i*2);
        aTabStop.GetTabPos() = nPos;
        switch( SVBT8ToByte( pTyp[i].aBits1 ) & 0x7 )       // pTyp[i].jc
        {
            case 0:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
                break;
            case 1:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
                break;
            case 2:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
                break;
            case 3:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL;
                break;
            case 4:
                continue;                   // ignoriere Bar
        }

        switch( SVBT8ToByte( pTyp[i].aBits1 ) >> 3 & 0x7 )
        {
            case 0:
                aTabStop.GetFill() = ' ';
                break;
            case 1:
                aTabStop.GetFill() = '.';
                break;
            case 2:
                aTabStop.GetFill() = '-';
                break;
            case 3:
            case 4:
                aTabStop.GetFill() = '_';
                break;
        }

        USHORT nPos2 = aAttr.GetPos( nPos );
        if (nPos2 != SVX_TAB_NOTFOUND)
            aAttr.Remove(nPos2, 1); // sonst weigert sich das Insert()
        aAttr.Insert(aTabStop);
    }

    if (nIns || nDel)
        NewAttr(aAttr);
    else
    {
        using namespace sw::util;
        SvxTabStopItem aOrig = pSty ?
            ItemGet<SvxTabStopItem>(*pSty, RES_PARATR_TABSTOP) :
            DefaultItemGet<SvxTabStopItem>(rDoc, RES_PARATR_TABSTOP);
        NewAttr(aOrig);
    }
}

//-----------------------------------------
//              DOP
//-----------------------------------------

void SwWW8ImplReader::ImportDop()
{
    maTracer.EnterEnvironment(sw::log::eDocumentProperties);
    // correct the LastPrinted date in DocumentInfo
    if (rDoc.GetpInfo())
    {
        DateTime aLastPrinted(
            WW8ScannerBase::WW8DTTM2DateTime(pWDop->dttmLastPrint));
        SfxDocumentInfo aNeuDocInf(*rDoc.GetpInfo());
        SfxStamp aPrinted(aNeuDocInf.GetPrinted());
        if (aPrinted.GetTime() != aLastPrinted)
        {
            // check if WW8 date was set
            if (aLastPrinted == DateTime(Date(0), Time(0)))
            {
                // create "invalid" value for SfxStamp
                // (as seen in sfx2/DOSINF.HXX)
                aPrinted.SetTime(DateTime(Date(1, 1, 1601), Time(0, 0, 0)));
            }
            else
                aPrinted.SetTime( aLastPrinted );

            aNeuDocInf.SetPrinted( aPrinted );
            rDoc.SetInfo(aNeuDocInf);
        }
    }

    //
    // COMPATIBILITY FLAGS START
    //

    // Abstand zwischen zwei Absaetzen ist die SUMME von unterem
    // Abst. des ersten und oberem Abst. des zweiten
    rDoc.SetParaSpaceMax(pWDop->fDontUseHTMLAutoSpacing, true);
    maTracer.Log(sw::log::eDontUseHTMLAutoSpacing);
    // move tabs on alignment
    rDoc.SetTabCompat(true);
    maTracer.Log(sw::log::eTabStopDistance);
    // OD 14.10.2003 #i18732# - adjust default of option 'FollowTextFlow'
    rDoc.SetDefault( SwFmtFollowTextFlow( FALSE ) );

    // Import Default-Tabs
    long nDefTabSiz = pWDop->dxaTab;
    if( nDefTabSiz < 56 )
        nDefTabSiz = 709;

    // wir wollen genau einen DefaultTab
    SvxTabStopItem aNewTab( 1, USHORT(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT );
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

    rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

    if (!pWDop->fUsePrinterMetrics)
        maTracer.Log(sw::log::ePrinterMetrics);

    if (!pWDop->fNoLeading)
        maTracer.Log(sw::log::eExtraLeading);

    if ( pWDop->fUsePrinterMetrics )
        rDoc._SetUseVirtualDevice( com::sun::star::document::PrinterIndependentLayout::DISABLED );
    else
        rDoc._SetUseVirtualDevice( com::sun::star::document::PrinterIndependentLayout::HIGH_RESOLUTION );

    rDoc.SetAddFlyOffsets( true );
    rDoc.SetAddExtLeading(!pWDop->fNoLeading);

    // -> #111955#
    rDoc.SetOldNumbering( false );
    // <- #111955#

    rDoc.SetUseFormerLineSpacing( false );

    // OD, MMAHER 2004-03-01 #i25901#- set new compatibility option
    //      'Add paragraph and table spacing at bottom of table cells'
    rDoc.SetAddParaSpacingToTableCells( true );

    // OD 2004-03-17 #i11860# - set new compatibility option
    //      'Use former object positioning' to <FALSE>
    rDoc.SetUseFormerObjectPositioning( false );

    // OD 2004-05-10 #i27767# - set new compatibility option
    //      'Conder Wrapping mode when positioning object' to <TRUE>
    rDoc.SetConsiderWrapOnObjPos( true );

    // --> FME 2004-04-22 # #108724#, #i13832#, #i24135#
    rDoc.SetUseFormerTextWrapping( false );
    // <--

    //
    // COMPATIBILITY FLAGS START
    //

    if (!pWDop->fNoLeading)
        maTracer.Log(sw::log::eExtraLeading);

    //import magic doptypography information, if its there
    if (pWwFib->nFib > 105)
        ImportDopTypography(pWDop->doptypography);

    if (pWDop->fProtEnabled)
    {
        using namespace com::sun::star;

        uno::Reference<lang::XComponent> xModelComp(mpDocShell->GetModel(),
           uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDocProps(xModelComp,
           uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xInfo =
                xDocProps->getPropertySetInfo();
            sal_Bool bValue = false;
            if (xInfo.is() &&
                xInfo->hasPropertyByName(C2U("ApplyFormDesignMode")))
            {
                xDocProps->setPropertyValue(C2U("ApplyFormDesignMode"),
                    cppu::bool2any(bValue));
            }
        }
    }

    maTracer.LeaveEnvironment(sw::log::eDocumentProperties);
}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    using namespace com::sun::star;
    switch (rTypo.iLevelOfKinsoku)
    {
#if 0
        /*
        Do the defaults differ between Microsoft versions ?, do we do
        something about it if so ?
        */
        case 0:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
        case 1:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
#endif
        case 2: //custom
            {
                i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                    rTypo.rgxchLPunct);
                rDoc.SetForbiddenCharacters(rTypo.GetConvertedLang(),
                        aForbidden);
                //Obviously cannot set the standard level 1 for japanese, so
                //bail out now while we can.
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                    return;
            }
            break;
        default:
            break;
    }

    /*
    This MS hack means that level 2 of japanese is not in operation, so we put
    in what we know are the MS defaults, there is a complementary reverse
    hack in the writer. Its our default as well, but we can set it anyway
    as a flag for later.
    */
    if (!rTypo.reserved2)
    {
        i18n::ForbiddenCharacters aForbidden(rTypo.GetJapanNotBeginLevel1(),
            rTypo.GetJapanNotEndLevel1());
        rDoc.SetForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    rDoc.SetKernAsianPunctuation(rTypo.fKerningPunct);
    rDoc.SetCharCompressType(
        static_cast<SwCharCompressType>(rTypo.iJustification));
}

//-----------------------------------------
//      Fuss- und Endnoten
//-----------------------------------------

WW8ReaderSave::WW8ReaderSave(SwWW8ImplReader* pRdr ,WW8_CP nStartCp)
    : maTmpPos(*pRdr->pPaM->GetPoint()), mpOldStck(pRdr->pCtrlStck),
    mpOldAnchorStck(pRdr->pAnchorStck), mpOldRedlines(pRdr->mpRedlineStack),
    mpOldPlcxMan(pRdr->pPlcxMan), mpWFlyPara(pRdr->pWFlyPara),
    mpSFlyPara(pRdr->pSFlyPara), mpPreviousNumPaM(pRdr->pPreviousNumPaM),
    mpPrevNumRule(pRdr->pPrevNumRule), mpTableDesc(pRdr->pTableDesc),
    mnInTable(pRdr->nInTable), mnAktColl(pRdr->nAktColl),
    mbIgnoreText(pRdr->bIgnoreText), mbSymbol(pRdr->bSymbol),
    mbHdFtFtnEdn(pRdr->bHdFtFtnEdn), mbTxbxFlySection(pRdr->bTxbxFlySection),
    mbAnl(pRdr->bAnl), mbInHyperlink(pRdr->bInHyperlink),
    mbPgSecBreak(pRdr->bPgSecBreak), mbWasParaEnd(pRdr->bWasParaEnd),
    mbHasBorder(pRdr->bHasBorder),
    mcSymbol(pRdr->cSymbol),
    mbFirstPara(pRdr->bFirstPara)
{
    pRdr->bSymbol = false;
    pRdr->bHdFtFtnEdn = true;
    pRdr->bTxbxFlySection = pRdr->bAnl = pRdr->bPgSecBreak = pRdr->bWasParaEnd
        = pRdr->bHasBorder = false;
    pRdr->bFirstPara = true;
    pRdr->nInTable = 0;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pPreviousNumPaM = 0;
    pRdr->pPrevNumRule = 0;
    pRdr->pTableDesc = 0;
    pRdr->nAktColl = 0;


    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags,
        *pRdr);

    pRdr->mpRedlineStack = new wwRedlineStack(pRdr->rDoc);

    pRdr->pAnchorStck = new SwWW8FltAnchorStack(&pRdr->rDoc, pRdr->nFieldFlags);

    // rette die Attributverwaltung: dies ist noetig, da der neu anzulegende
    // PLCFx Manager natuerlich auf die gleichen FKPs zugreift, wie der alte
    // und deren Start-End-Positionen veraendert...
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->SaveAllPLCFx(maPLCFxSave);

    if (nStartCp != -1)
    {
        pRdr->pPlcxMan = new WW8PLCFMan(pRdr->pSBase,
            mpOldPlcxMan->GetManType(), nStartCp);
    }

    maOldApos.push_back(false);
    maOldApos.swap(pRdr->maApos);
    maOldFieldStack.swap(pRdr->maFieldStack);
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara = mpWFlyPara;
    pRdr->pSFlyPara = mpSFlyPara;
    pRdr->pPreviousNumPaM = mpPreviousNumPaM;
    pRdr->pPrevNumRule = mpPrevNumRule;
    pRdr->pTableDesc = mpTableDesc;
    pRdr->cSymbol = mcSymbol;
    pRdr->bSymbol = mbSymbol;
    pRdr->bIgnoreText = mbIgnoreText;
    pRdr->bHdFtFtnEdn = mbHdFtFtnEdn;
    pRdr->bTxbxFlySection = mbTxbxFlySection;
    pRdr->nInTable = mnInTable;
    pRdr->bAnl = mbAnl;
    pRdr->bInHyperlink = mbInHyperlink;
    pRdr->bWasParaEnd = mbWasParaEnd;
    pRdr->bPgSecBreak = mbPgSecBreak;
    pRdr->nAktColl = mnAktColl;
    pRdr->bHasBorder = mbHasBorder;
    pRdr->bFirstPara = mbFirstPara;

    // schliesse alle Attribute, da sonst Attribute
    // entstehen koennen, die aus dem Fly rausragen
    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = mpOldStck;

    pRdr->mpRedlineStack->closeall(*pRdr->pPaM->GetPoint());
    delete pRdr->mpRedlineStack;
    pRdr->mpRedlineStack = mpOldRedlines;

    pRdr->DeleteAnchorStk();
    pRdr->pAnchorStck = mpOldAnchorStck;

    *pRdr->pPaM->GetPoint() = maTmpPos;

    if (mpOldPlcxMan != pRdr->pPlcxMan)
    {
        delete pRdr->pPlcxMan;
        pRdr->pPlcxMan = mpOldPlcxMan;
    }
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->RestoreAllPLCFx(maPLCFxSave);
    pRdr->maApos.swap(maOldApos);
    pRdr->maFieldStack.swap(maOldFieldStack);
}

void SwWW8ImplReader::Read_HdFtFtnText( const SwNodeIndex* pSttIdx,
    long nStartCp, long nLen, short nType )
{
    // rettet Flags u.ae. u. setzt sie zurueck
    WW8ReaderSave aSave( this );

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;      //
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    // dann Text fuer Header, Footer o. Footnote einlesen

    ReadText( nStartCp, nLen, nType );              // Sepx dabei ignorieren
    aSave.Restore( this );
}

//Use authornames, if not available fall back to initials.
long SwWW8ImplReader::Read_And(WW8PLCFManResult* pRes)
{
    WW8PLCFx_SubDoc* pSD = pPlcxMan->GetAtn();
    if( !pSD )
        return 0;

    String sAuthor;
    if( bVer67 )
    {
        const WW67_ATRD* pDescri = (const WW67_ATRD*)pSD->GetData();
        const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if (pA)
            sAuthor = *pA;
        else
            sAuthor = String(pDescri->xstUsrInitl + 1, pDescri->xstUsrInitl[0],
                RTL_TEXTENCODING_MS_1252);
    }
    else
    {
        const WW8_ATRD* pDescri = (const WW8_ATRD*)pSD->GetData();

        const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if( pA )
            sAuthor = *pA;
        else
        {
            sal_uInt16 nLen = SVBT16ToShort(pDescri->xstUsrInitl[0]);
            for(sal_uInt16 nIdx = 1; nIdx <= nLen; ++nIdx)
                sAuthor += SVBT16ToShort(pDescri->xstUsrInitl[nIdx]);
        }
    }

    SwNodeIndex aNdIdx( rDoc.GetNodes().GetEndOfExtras() );
    aNdIdx = *rDoc.GetNodes().MakeTextSection(aNdIdx, SwNormalStartNode,
        rDoc.GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD, false));

    {
        SwPaM *pTempPaM = pPaM;
        SwPaM aPaM(aNdIdx);
        pPaM = &aPaM;
        Read_HdFtFtnText( &aNdIdx, pRes->nCp2OrIdx, pRes->nMemLen, MAN_AND );
        pPaM = pTempPaM;
    }

    // erzeuge das PostIt
    Date aDate;
    String sTxt;

    {   // Text aus den Nodes in den String uebertragen
        SwNodeIndex aIdx( aNdIdx, 1 ),
                    aEnd( *aNdIdx.GetNode().EndOfSectionNode() );
        SwTxtNode* pTxtNd;
        while( aIdx != aEnd )
        {
            if( 0 != ( pTxtNd = aIdx.GetNode().GetTxtNode() ))
            {
                if( sTxt.Len() )
                    sTxt += '\x0a';     // Zeilenumbruch
                sTxt += pTxtNd->GetExpandTxt();
            }
            aIdx++;
        }
        rDoc.DeleteSection( &aNdIdx.GetNode() );
    }

    rDoc.Insert( *pPaM, SwFmtFld( SwPostItField(
        (SwPostItFieldType*)rDoc.GetSysFldType( RES_POSTITFLD ), sAuthor, sTxt,
        aDate )));
    return 0;
}

void SwWW8ImplReader::Read_HdFtTextAsHackedFrame(long nStart, long nLen,
    SwFrmFmt &rHdFtFmt, sal_uInt16 nPageWidth)
{
    const SwNodeIndex* pSttIdx = rHdFtFmt.GetCntnt().GetCntntIdx();
    ASSERT(pSttIdx, "impossible");
    if (!pSttIdx)
        return;

    SwPosition aTmpPos(*pPaM->GetPoint());

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);

    SwFlyFrmFmt *pFrame = rDoc.MakeFlySection(FLY_AT_CNTNT, pPaM->GetPoint());

    pFrame->SetAttr(SwFmtFrmSize(ATT_MIN_SIZE, nPageWidth, MINLAY));
    pFrame->SetAttr(SwFmtSurround(SURROUND_THROUGHT));
    pFrame->SetAttr(SwFmtHoriOrient(0, HORI_RIGHT)); //iFOO
    MoveInsideFly(pFrame);

    const SwNodeIndex* pHackIdx = pFrame->GetCntnt().GetCntntIdx();

    Read_HdFtFtnText(pHackIdx, nStart, nLen - 1, MAN_HDFT);

    MoveOutsideFly(pFrame, aTmpPos);
}

void SwWW8ImplReader::Read_HdFtText(long nStart, long nLen, SwFrmFmt* pHdFtFmt)
{
    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition

    Read_HdFtFtnText(pSttIdx, nStart, nLen - 1, MAN_HDFT);

    *pPaM->GetPoint() = aTmpPos;
}

bool SwWW8ImplReader::HasOwnHeaderFooter(BYTE nWhichItems, BYTE grpfIhdt,
    int nSect)
{
    if (pHdFt)
    {
        WW8_CP start;
        long nLen;
        BYTE nNumber = 5;

        for( BYTE nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact(nNumber + (nSect+1)*6, start, nLen);
                    bOk = ( 2 <= nLen );
                }

                if (bOk)
                    return true;
            }
        }
    }
    return false;
}

void SwWW8ImplReader::Read_HdFt(bool bIsTitle, int nSect,
    const SwPageDesc *pPrev, const wwSection &rSection)
{
    BYTE nWhichItems = 0;
    SwPageDesc *pPD = 0;
    if (!bIsTitle)
    {
        nWhichItems =
            rSection.maSep.grpfIhdt & ~(WW8_HEADER_FIRST | WW8_FOOTER_FIRST);
        pPD = rSection.mpPage;
    }
    else
    {
        nWhichItems =
            rSection.maSep.grpfIhdt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
        pPD = rSection.mpTitlePage;
    }

    BYTE grpfIhdt = rSection.maSep.grpfIhdt;


    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        BYTE nNumber = 5;

        for( BYTE nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact(nNumber + (nSect+1)*6, start, nLen);
                    bOk = ( 2 <= nLen );
                }

                bool bUseLeft
                    = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? true: false;
                bool bFooter
                    = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? true: false;

                SwFrmFmt* pFmt = bUseLeft ? &pPD->GetLeft() : &pPD->GetMaster();

                SwFrmFmt* pHdFtFmt;
                if (bFooter)
                {
                    bIsFooter = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetFooter().GetFooterFmt())
                        pPD->GetMaster().SetAttr(SwFmtFooter(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetAttr(SwFmtFooter(true));
                    pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
                }
                else
                {
                    bIsHeader = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetHeader().GetHeaderFmt())
                        pPD->GetMaster().SetAttr(SwFmtHeader(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetAttr(SwFmtHeader(true));
                    pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
                }

                if (bOk)
                {
                    bool bHackRequired = false;
                    if (bIsHeader && rSection.IsFixedHeightHeader())
                        bHackRequired = true;
                    else if (bIsFooter && rSection.IsFixedHeightFooter())
                        bHackRequired = true;

                    if (bHackRequired)
                    {
                        Read_HdFtTextAsHackedFrame(start, nLen, *pHdFtFmt,
                            rSection.GetTextAreaWidth());
                    }
                    else
                        Read_HdFtText(start, nLen, pHdFtFmt);
                }
                else if (!bOk && pPrev)
                    CopyPageDescHdFt(pPrev, pPD, nI);

                bIsHeader = bIsFooter = false;
            }
        }
    }
    maTracer.LeaveEnvironment(sw::log::eDocumentProperties);
}

bool wwSectionManager::SectionIsProtected(const wwSection &rSection) const
{
    return (mrReader.pWDop->fProtEnabled && !rSection.IsNotProtected());
}

void wwSectionManager::SetHdFt(wwSection &rSection, int nSect,
    const wwSection *pPrevious)
{
    // Header / Footer nicht da
    if (!rSection.maSep.grpfIhdt)
        return;

    ASSERT(rSection.mpPage, "makes no sense to call with a main page");
    if (rSection.mpPage)
    {
        mrReader.Read_HdFt(false, nSect, pPrevious ? pPrevious->mpPage : 0,
                rSection);
    }

    if (rSection.mpTitlePage)
    {
        // 2 Pagedescs noetig: 1.Seite und folgende
        // 1. Seite einlesen
        mrReader.Read_HdFt(true, nSect, pPrevious ? pPrevious->mpTitlePage : 0,
            rSection);
    }

    // Kopf / Fuss - Index Updaten
    // Damit der Index auch spaeter noch stimmt
    if (mrReader.pHdFt)
        mrReader.pHdFt->UpdateIndex(rSection.maSep.grpfIhdt);

}

class AttribHere : public std::unary_function<const xub_StrLen*, bool>
{
private:
    xub_StrLen nPosition;
public:
    AttribHere(xub_StrLen nPos) : nPosition(nPos) {}
    bool operator()(const xub_StrLen *pPosition) const
    {
        return (*pPosition >= nPosition);
    }
};

void SwWW8ImplReader::AppendTxtNode(SwPosition& rPos)
{
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();

#if 0 //I'm giving up on this approach, see issue #i11873# & #i20871#

    //If a paragraph ends in tabs and that paragraph has a tab at or beyond
    //the right of the page then word has a bug where further use of tab will
    //not wrap the paragraph around to the next page. We will, so we will
    //remove these tabs ourselves.

    //To be fair this is only a partial solution for the most common case of
    //ending in redundant tabs. It is possible to have text after these flawed
    //tabs which in word will not line wrap to the next page, and this
    //solution will not fix that.
    const String &rStr = pTxt->GetTxt();
    xub_StrLen nLen=rStr.Len();


    //If we happen to end on a tab, something a well formed sentence shouldn't
    //do :-), then go into strip mode.
    if (nLen && rStr.GetChar(nLen-1) == '\x9')
    {
        std::vector<const xub_StrLen*> aLocs;
        GetNoninlineNodeAttribs(pTxt,aLocs);
        while (nLen)
        {
            if (rStr.GetChar(nLen-1) != '\x9')
                break;
            else
            {
                std::vector<const xub_StrLen*>::iterator aIter =
                    std::find_if(aLocs.begin(),aLocs.end(),AttribHere(nLen-1));
                if (aIter != aLocs.end())
                    break;
                else
                    --nLen;
            }
        }
        //We have some candidate tabs whose removal would not affect the
        //document logically. Let us see if this is one of the buggy
        //paragraphs that word would place all in one line, if so remove them
        //to match word's layout.
        if (xub_StrLen nDiff = rStr.Len() - nLen)
        {
            const SvxTabStopItem* pTabStop =
                (const SvxTabStopItem*)GetFmtAttr(RES_PARATR_TABSTOP);

            const SvxLRSpaceItem* pLR =
                (const SvxLRSpaceItem*)GetFmtAttr(RES_LR_SPACE);

            for (USHORT nI = pTabStop ? pTabStop->Count() : 0; nI--;)
            {
                //Give ourselves a leeway of 1 twip.
                if ((*pTabStop)[nI].GetTabPos() + pLR->GetTxtLeft() + 1
                    >= maSectionManager.GetPageWidth()
                    - maSectionManager.GetPageRight()
                    - maSectionManager.GetPageLeft())
                {
                    pPaM->SetMark();
                    pPaM->GetMark()->nContent-=nDiff;
                    pCtrlStck->Delete(*pPaM);
                    rDoc.Delete(*pPaM);     //this frobs the mark
                    pPaM->DeleteMark();
                    break;
                }
            }
        }
    }
#endif

    const SwNumRule* pRule = sw::util::GetNumRuleFromTxtNode(*pTxt);

    if (
         pRule && !pWDop->fDontUseHTMLAutoSpacing &&
         (bParaAutoBefore || bParaAutoAfter)
       )
    {
        // If after spacing is set to auto, set the after space to 0
        if (bParaAutoAfter)
            SetLowerSpacing(*pPaM, 0);

        // If the previous textnode had numbering and
        // and before spacing is set to auto, set before space to 0
        if(pPrevNumRule && bParaAutoBefore)
            SetUpperSpacing(*pPaM, 0);

        // If the previous numbering rule was different we need
        // to insert a space after the previous paragraph
        if((pRule != pPrevNumRule) && pPreviousNumPaM)
            SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));

        // cache current paragraph
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;

        pPreviousNumPaM = new SwPaM(*pPaM);
        pPrevNumRule = pRule;
    }
    else if(!pRule && pPreviousNumPaM)
    {
        // If the previous paragraph has numbering but the current one does not
        // we need to add a space after the previous paragraph
        SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));
        delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = 0;
    }
    else
    {
        // clear paragraph cache
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = pRule;
    }

    // If this is the first paragraph in the document and
    // Auto-spacing before paragraph is set,
    // set the upper spacing value to 0
    if(bParaAutoBefore && bFirstPara && !pWDop->fDontUseHTMLAutoSpacing)
        SetUpperSpacing(*pPaM, 0);

    bFirstPara = false;

    rDoc.AppendTxtNode(rPos);

    //We can flush all anchored graphics at the end of a paragraph.
    pAnchorStck->Flush();
}

bool SwWW8ImplReader::SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper )
{
        bool bRet = false;
        const SwPosition* pSpacingPos = rMyPam.GetPoint();

        const SvxULSpaceItem* pULSpaceItem = (const SvxULSpaceItem*)pCtrlStck->GetFmtAttr(*pSpacingPos, RES_UL_SPACE);

        if(pULSpaceItem != 0)
        {
            SvxULSpaceItem aUL(*pULSpaceItem);

            if(bIsUpper)
                aUL.SetUpper(nSpace);
            else
                aUL.SetLower(nSpace);

            xub_StrLen nEnd = pSpacingPos->nContent.GetIndex();
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), 0);
            pCtrlStck->NewAttr(*pSpacingPos, aUL);
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), nEnd);
            pCtrlStck->SetAttr(*pSpacingPos, RES_UL_SPACE);
            bRet = true;
        }
        return bRet;
}

bool SwWW8ImplReader::SetLowerSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, false);
}

bool SwWW8ImplReader::SetUpperSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, true);
}

USHORT SwWW8ImplReader::TabRowSprm(int nLevel) const
{
    if (bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

void SwWW8ImplReader::EndSpecial()
{
    // Frame / Table / Anl
    if (bAnl)
        StopAllAnl();                  // -> bAnl = false

    while(maApos.size() > 1)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
        if (maApos[nInTable] == true)
            StopApo();
    }

    if (maApos[0] == true)
        StopApo();

    ASSERT(!nInTable, "unclosed table!");
}

bool SwWW8ImplReader::ProcessSpecial(bool &rbReSync, WW8_CP nStartCp)
{
    // Frame / Table / Anl
    if (bInHyperlink)
        return false;

    rbReSync = false;

    ASSERT(nInTable >= 0,"nInTable < 0!");

    // TabRowEnd
    bool bTableRowEnd = (pPlcxMan->HasParaSprm(bVer67 ? 25 : 0x2417) != 0 );

// es muss leider fuer jeden Absatz zuerst nachgesehen werden,
// ob sich unter den sprms
// das sprm 29 (bzw. 0x261B) befindet, das ein APO einleitet.
// Alle weiteren sprms  beziehen sich dann naemlich auf das APO und nicht
// auf den normalen Text drumrum.
// Dasselbe gilt fuer eine Tabelle ( sprm 24 (bzw. 0x2416) )
// und Anls ( sprm 13 ).
// WW: Tabelle in APO geht ( Beide Anfaende treten gleichzeitig auf )
// WW: APO in Tabelle geht nicht
// d.h. Wenn eine Tabelle Inhalt eines Apo ist, dann muss der
// Apo-Anfang zuerst bearbeitet werden, damit die Tabelle im Apo steht
// und nicht umgekehrt. Am Ende muss dagegen zuerst das Tabellenende
// bearbeitet werden, da die Apo erst nach der Tabelle abgeschlossen
// werden darf ( sonst wird das Apo-Ende nie gefunden ).
// Dasselbe gilt fuer Fly / Anl, Tab / Anl, Fly / Tab / Anl.
//
// Wenn die Tabelle in einem Apo steht, fehlen im TabRowEnd-Bereich
// die Apo-Angaben. Damit hier die Apo nicht beendet wird, wird
// ProcessApo dann nicht aufgerufen.

// KHZ: When there is a table inside the Apo the Apo-flags are also
//      missing for the 2nd, 3rd... paragraphs of each cell.


//  1st look for in-table flag, for 2000+ there is a subtable flag to
//  be considered, the sprm 6649 gives the level of the table
    BYTE nCellLevel = 0;

    if (bVer67)
        nCellLevel = 0 != pPlcxMan->HasParaSprm(24);
    else
    {
        nCellLevel = 0 != pPlcxMan->HasParaSprm(0x2416);
        if (!nCellLevel)
            nCellLevel = 0 != pPlcxMan->HasParaSprm(0x244B);
    }

    WW8_TablePos *pTabPos=0;
    WW8_TablePos aTabPos;
    if (nCellLevel && !bVer67)
    {
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        rbReSync = true;
        WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp=nStartCp;

        if (const BYTE *pLevel = pPlcxMan->HasParaSprm(0x6649))
            nCellLevel = *pLevel;

        bool bHasRowEnd = SearchRowEnd(pPap, nMyStartCp, nCellLevel-1);

        //Bad Table, remain unchanged in level, e.g. #i19667#
        if (!bHasRowEnd)
            nCellLevel = nInTable;

        if (bHasRowEnd && ParseTabPos(&aTabPos,pPap))
            pTabPos = &aTabPos;

        pPlcxMan->GetPap()->Restore( aSave );
    }

//  then look if we are in an Apo

    ApoTestResults aApo = TestApo(nCellLevel, bTableRowEnd, pTabPos);

    //look to see if we are in a Table, but Table in foot/end note not allowed
    bool bStartTab = (nInTable < nCellLevel) && !bFtnEdn;

    bool bStopTab = bWasTabRowEnd && (nInTable > nCellLevel) && !bFtnEdn;

    bWasTabRowEnd = false;  // must be deactivated right here to prevent next
                            // WW8TabDesc::TableCellEnd() from making nonsense

    if (nInTable && !bTableRowEnd && !bStopTab && (nInTable == nCellLevel && aApo.HasStartStop()))
        bStopTab = bStartTab = true;    // Required to stop and start table

//  Dann auf Anl (Nummerierung) testen
//  und dann alle Ereignisse in der richtigen Reihenfolge bearbeiten

    if( bAnl && !bTableRowEnd )
    {
        const BYTE* pSprm13 = pPlcxMan->HasParaSprm( 13 );
        if( pSprm13 )
        {                                   // Noch Anl ?
            BYTE nT = GetNumType( *pSprm13 );
            if( ( nT != WW8_Pause && nT != nWwNumType ) // Anl-Wechsel
                || aApo.HasStartStop()                  // erzwungenes Anl-Ende
                || bStopTab || bStartTab )
            {
                StopAnlToRestart(nT);  // Anl-Restart ( = Wechsel ) ueber sprms
            }
            else
            {
                NextAnlLine( pSprm13 );                 // naechste Anl-Zeile
            }
        }
        else
        {                                           // Anl normal zuende
            StopAllAnl();                                  // Wirkliches Ende
        }
    }
    if (bStopTab)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
    }
    if (aApo.mbStopApo)
    {
        StopApo();
        maApos[nInTable] = false;
    }

    if (aApo.mbStartApo)
    {
        maApos[nInTable] = StartApo(aApo, pTabPos);
        // nach StartApo ist ein ReSync noetig ( eigentlich nur, falls die Apo
        // ueber eine FKP-Grenze geht
        rbReSync = true;
    }
    if (bStartTab)
    {
        if (bAnl)                           // Nummerierung ueber Zellengrenzen
            StopAllAnl();                   // fuehrt zu Absturz -> keine Anls
                                            // in Tabellen
        while (nInTable < nCellLevel)
        {
            nInTable += StartTable(nStartCp);
            maApos.push_back(false);
        }
        // nach StartTable ist ein ReSync noetig ( eigentlich nur, falls die
        // Tabelle ueber eine FKP-Grenze geht
        rbReSync = true;
    }
    return bTableRowEnd;
}

CharSet SwWW8ImplReader::GetCurrentCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    CharSet eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCharSets.empty())
            eSrcCharSet = maFontSrcCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && (nCharFmt != -1))
            eSrcCharSet = pCollA[nCharFmt].GetCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
            eSrcCharSet = pCollA[nAktColl].GetCharSet();
    }
    return eSrcCharSet;
}

/*
 #i9241#
 It appears that some documents that are in a baltic 8 bit encoding which has
 some undefined characters can have use made of those characters, in which
 case they default to CP1252. If not then its perhaps that the font encoding
 is only in use for 6/7 and for 8+ if we are in 8bit mode then the encoding
 is always 1252.
*/
sal_Unicode Custom8BitToUnicode(rtl_TextToUnicodeConverter hConverter,
    sal_Char cChar)
{
    const sal_uInt32 nFlags =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    sal_Unicode nConvChar;
    sal_uInt32 nInfo=0;
    sal_Size nSrcBytes=0;
    sal_Size nDestChars = rtl_convertTextToUnicode(hConverter, 0,
        &cChar, 1, &nConvChar, 1, nFlags, &nInfo, &nSrcBytes );

    if (nInfo & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE)
    {
        rtl_TextToUnicodeConverter hCP1252Converter =
            rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_MS_1252);
        nDestChars = rtl_convertTextToUnicode(hCP1252Converter, 0,
            &cChar, 1, &nConvChar, 1, nFlags, &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter(hCP1252Converter);
    }

    ASSERT(nDestChars == 1, "impossible to get more than 1 char");

    ASSERT(nInfo == 0, "A character conversion failed, gulp!");

    if (nDestChars == 1)
        return nConvChar;
    else
        return cChar;
}

// Returnwert: true for no Sonderzeichen
bool SwWW8ImplReader::ReadPlainChars(long& rPos, long nEnd, long nCpOfs)
{
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    WW8_FC nStreamPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    pStrm->Seek( nStreamPos );
#if 0
    // amount of characters to read == length to next attribute
    DBG_ASSERT(nEnd - rPos <= (STRING_MAXLEN-1),
        "String too long for stringclass!");
#endif
    xub_StrLen nLen;
    if (nEnd - rPos <= (STRING_MAXLEN-1))
        nLen = writer_cast<xub_StrLen>(nEnd - rPos);
    else
        nLen = STRING_MAXLEN-1;
    ASSERT(nLen, "String is 0");
    if (!nLen)
        return true;

    const CharSet eSrcCharSet = bVer67 ? GetCurrentCharSet() :
        RTL_TEXTENCODING_MS_1252;

    // (re)alloc UniString data
    String sPlainCharsBuf;

    sal_Unicode* pWork = sPlainCharsBuf.AllocBuffer( nLen );

    rtl_TextToUnicodeConverter hConverter = 0;
    if (!bIsUnicode || bVer67)
        hConverter = rtl_createTextToUnicodeConverter(eSrcCharSet);

    // read the stream data
    BYTE   nBCode;
    UINT16 nUCode;
    xub_StrLen nL2;

    for( nL2 = 0; nL2 < nLen; ++nL2, ++pWork )
    {
        if (bIsUnicode)
            *pStrm >> nUCode;   // unicode  --> read 2 bytes
        else
        {
            *pStrm >> nBCode;   // old code --> read 1 byte
            nUCode = nBCode;
        }

        if (pStrm->GetError())
        {
            rPos = LONG_MAX-10;     // -> eof or other error
            sPlainCharsBuf.ReleaseBufferAccess( 0 );
            return true;
        }

        if ((32 > nUCode) || (0xa0 == nUCode))
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            sPlainCharsBuf.ReleaseBufferAccess( nL2 );
            break;              // Sonderzeichen < 32, == 0xa0 gefunden
        }

        if (bIsUnicode)
        {
            if (!bVer67)
                *pWork = nUCode;
            else
            {
                if (nUCode >= 0x3000)       //0x8000 ?
                {
                    sal_Char aTest[2];
                    aTest[0] = (nUCode & 0xFF00) >> 8;
                    aTest[1] = (nUCode & 0x00FF);
                    String aTemp(aTest, 2, eSrcCharSet);
                    ASSERT(aTemp.Len() == 1, "so much for that theory");
                    *pWork = aTemp.GetChar(0);
                }
                else
                {
                    *pWork = Custom8BitToUnicode(hConverter, nUCode & 0x00FF);
                }
            }
        }
        else
            *pWork = Custom8BitToUnicode(hConverter, nBCode);
    }

    if (nL2)
    {
        AddTextToParagraph(sPlainCharsBuf);
        rPos += nL2;
        if (!maApos.back()) //a para end in apo doesn't count
            bWasParaEnd = false;            //kein CR
    }

    if (hConverter)
        rtl_destroyTextToUnicodeConverter(hConverter);
    return nL2 >= nLen;
}

bool SwWW8ImplReader::AddTextToParagraph(const String& rAddString)
{
    const SwTxtNode* pNd = pPaM->GetCntntNode()->GetTxtNode();
    if (rAddString.Len())
    {
        if ((pNd->GetTxt().Len() + rAddString.Len()) < STRING_MAXLEN -1)
        {
            rDoc.Insert (*pPaM, rAddString);
        }
        else
        {

            if (pNd->GetTxt().Len()< STRING_MAXLEN -1)
            {
                String sTempStr (rAddString,0,
                    STRING_MAXLEN - pNd->GetTxt().Len() -1);
                rDoc.Insert (*pPaM, sTempStr);
                sTempStr = rAddString.Copy(sTempStr.Len(),
                    rAddString.Len() - sTempStr.Len());
                AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert (*pPaM,sTempStr );
            }
            else
            {
                AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert (*pPaM, rAddString);
            }
        }
    }
    return true;
}

// Returnwert: true for para end
bool SwWW8ImplReader::ReadChars(long& rPos, long nNextAttr, long nTextEnd,
    long nCpOfs)
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if (bSymbol || bIgnoreText)
    {
        if( bSymbol )   // Spezialzeichen einfuegen
        {
            for(USHORT nCh = 0; nCh < nEnd - rPos; ++nCh)
                rDoc.Insert( *pPaM, cSymbol );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd;    // ignoriere bis Attributende
        return false;
    }

    while (true)
    {
        if (ReadPlainChars(rPos, nEnd, nCpOfs))
            return false;                   // Fertig

        bool bStartLine = ReadChar(rPos, nCpOfs);
        rPos++;
        if (bPgSecBreak || bStartLine || rPos == nEnd)  // CR oder Fertig
            return bStartLine;
    }
}

bool SwWW8ImplReader::HandlePageBreakChar()
{
    bool bParaEndAdded = false;
    //#i1909# section/page breaks should not occur in tables, word
    //itself ignores them in this case.
    if (!nInTable)
    {
        bPgSecBreak = true;
        pCtrlStck->KillUnlockedAttrs(*pPaM->GetPoint());
        /*
        #74468#
        If its a 0x0c without a paragraph end before it, act like a
        paragraph end, but nevertheless, numbering (and perhaps other
        similiar constructs) do not exist on the para.
        */
        if (!bWasParaEnd)
        {
            bParaEndAdded = true;
            if (0 >= pPaM->GetPoint()->nContent.GetIndex())
            {
                if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
                {
                    pTxtNode->SwCntntNode::SetAttr(
                        *GetDfltAttr(RES_PARATR_NUMRULE));
                    pTxtNode->UpdateNum(SwNodeNum(NO_NUMBERING));
                }
            }
        }
    }
    return bParaEndAdded;
}

bool SwWW8ImplReader::ReadChar(long nPosCp, long nCpOfs)
{
    bool bNewParaEnd = false;
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    pStrm->Seek( pSBase->WW8Cp2Fc(nCpOfs+nPosCp, &bIsUnicode) );

    BYTE   nBCode;
    UINT16 nWCharVal;
    if( bIsUnicode )
        *pStrm >> nWCharVal;    // unicode  --> read 2 bytes
    else
    {
        *pStrm  >>  nBCode;     // old code --> read 1 byte
        nWCharVal = nBCode;
    }

    sal_Char cInsert = '\x0';
    bool bRet = false;
    switch (nWCharVal)
    {
        case 0:
            {
                // Seitennummer
                SwPageNumberField aFld(
                    (SwPageNumberFieldType*)rDoc.GetSysFldType(
                    RES_PAGENUMBERFLD ), PG_RANDOM, SVX_NUM_ARABIC);
                rDoc.Insert(*pPaM, SwFmtFld(aFld));
            }
            break;
        case 0xe:
            //#108817# if there is only one column word treats a column
            //break like a pagebreak.
            if (maSectionManager.CurrentSectionColCount() < 2)
                bRet = HandlePageBreakChar();
            else if (!nInTable)
            {
                // Always insert a txtnode for a column break, e.g. ##
                AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert(*pPaM, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE));
            }
            break;
        case 0x7:
            bNewParaEnd = true;
            TabCellEnd();       // table cell end (Flags abfragen!)
            break;
        case 0xf:
            if( !bSpec )        // "Satellit"
                cInsert = '\xa4';
            break;
        case 0x14:
            if( !bSpec )        // "Para-Ende"-Zeichen
                cInsert = '\xb5';
            break;
        case 0x15:
            if( !bSpec )        // Juristenparagraph
                cInsert = '\xa7';
            break;
        case 0x9:
            cInsert = '\x9';    // Tab
            break;
        case 0xb:
            cInsert = '\xa';    // Hard NewLine
            break;
        case 0xc:
            bRet = HandlePageBreakChar();
            break;
        case 0x1e:
            rDoc.Insert( *pPaM, CHAR_HARDHYPHEN);   // Non-breaking hyphen
            break;
        case 0x1f:
            rDoc.Insert( *pPaM, CHAR_SOFTHYPHEN);   // Non-required hyphens
            break;
        case 0xa0:
            rDoc.Insert( *pPaM, CHAR_HARDBLANK);    // Non-breaking spaces
            break;
        case 0x1:
            /*
            Current thinking is that if bObj is set then we have a
            straightforward "traditional" ole object, otherwise we have a
            graphic preview of an associated ole2 object (or a simple
            graphic of course)
            */
            if (!IsInlineEscherHack())
            {
                SwFrmFmt *pResult = 0;
                if (bObj)
                    pResult = ImportOle();
                else if (bSpec)
                    pResult = ImportGraf();

                //#102160# If we have a bad 0x1 insert a space instead.
                if (!pResult)
                {
                    cInsert = ' ';
                    ASSERT(!bObj && !bEmbeddObj && !nObjLocFc,
                        "WW8: Please report this document, it may have a "
                        "missing graphic");
                }
                else
                {
                    // reset the flags.
                    bObj = bEmbeddObj = false;
                    nObjLocFc = 0;
                }
            }
            break;
        case 0x8:
            if( !bObj )
                Read_GrafLayer( nPosCp );
            break;
        case 0xd:
            bNewParaEnd = bRet = true;
            if (nInTable > 1)
            {
                /*
                #i9666#/#i23161#
                Yes complex, if there is an entry in the undocumented PLCF
                which I believe to be a record of cell and row boundaries
                see if the magic bit which I believe to mean cell end is
                set. I also think btw that the third byte of the 4 byte
                value is the level of the cell
                */
                WW8PLCFspecial* pTest = pPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    long nPos;
                    void *pData;
                    pTest->Get(nPos, pData);
                    sal_uInt32 nData = SVBT32ToLong(*(SVBT32*)pData);
                    if (nData & 0x2)    //Might be how it works
                    {
                        TabCellEnd();
                        bRet = false;
                    }
                }
            }
            break;              // line end
        case 0x5:               // Annotation reference
        case 0x13:
            break;
        case 0x2:
            if (!maFtnStack.empty())
                cInsert = 0x2;
            break;                  // Auto-Fussnoten-Nummer
#if OSL_DEBUG_LEVEL > 1
        default:
            {
                String sUnknown( '<' );
                sUnknown += String::CreateFromInt32( nWCharVal );
                sUnknown += '>';
                rDoc.Insert( *pPaM, sUnknown );
            }
#endif
            break;
    }

    if( '\x0' != cInsert )
    {
        String sInsert = ByteString::ConvertToUnicode(cInsert,
            RTL_TEXTENCODING_MS_1252 );
        AddTextToParagraph(sInsert);
    }
    if (!maApos.back()) //a para end in apo doesn't count
        bWasParaEnd = bNewParaEnd;
    return bRet;
}

void SwWW8ImplReader::ProcessAktCollChange(WW8PLCFManResult& rRes,
    bool* pStartAttr, bool bCallProcessSpecial)
{
    USHORT nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    // Invalid Style-Id
    if (nAktColl >= nColls || !pCollA[nAktColl].pFmt || !pCollA[nAktColl].bColl)
    {
        nAktColl = 0;
        bParaAutoBefore = false;
        bParaAutoAfter = false;
    }
    else
    {
        bParaAutoBefore = pCollA[nAktColl].bParaAutoBefore;
        bParaAutoAfter = pCollA[nAktColl].bParaAutoAfter;
    }

    bool bTabRowEnd = false;
    if( pStartAttr && bCallProcessSpecial && !bInHyperlink )
    {
        bool bReSync;
        // Frame / Table / Autonumbering List Level
        bTabRowEnd = ProcessSpecial(bReSync, rRes.nAktCp+pPlcxMan->GetCpOfs());
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); // hole Attribut-Pos neu
    }

    if (!bTabRowEnd && StyleExists(nAktColl))
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ]);
        ChkToggleAttr(pCollA[ nOldColl ].n81Flags, pCollA[ nAktColl ].n81Flags);
        ChkToggleBiDiAttr(pCollA[nOldColl].n81BiDiFlags,
            pCollA[nAktColl].n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr(long& rTxtPos, bool& rbStartLine)
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    bool bStartAttr = pPlcxMan->Get(&aRes); // hole Attribut-Pos
    aRes.nAktCp = rTxtPos;              // Akt. Cp-Pos

    if ((aRes.nFlags & MAN_MASK_NEW_SEP) && !bIgnoreText)   // neue Section
    {
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
        // PageDesc erzeugen und fuellen
        maSectionManager.CreateSep(rTxtPos, bPgSecBreak);
                                            // -> 0xc war ein Sectionbreak, aber
                                            // kein Pagebreak;
        bPgSecBreak = false;                // PageDesc erzeugen und fuellen
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    }

    // neuer Absatz ueber Plcx.Fkp.papx
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessAktCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !bIgnoreText );
        rbStartLine = false;
    }

    // position of last CP that's to be ignored
    long nSkipPos = -1;

    if( 0 < aRes.nSprmId )                      // leere Attrs ignorieren
    {
        if( ( eFTN > aRes.nSprmId ) || ( 0x0800 <= aRes.nSprmId ) )
        {
            if( bStartAttr )                            // WW-Attribute
            {
                if( aRes.nMemLen >= 0 )
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
            }
            else
                EndSprm( aRes.nSprmId );        // Attr ausschalten
        }
        else if( aRes.nSprmId < 0x800 ) // eigene Hilfs-Attribute
        {
            if (bStartAttr)
            {
                nSkipChars = ImportExtSprm(&aRes);
                if (
                    (aRes.nSprmId == eFTN) || (aRes.nSprmId == eEDN) ||
                    (aRes.nSprmId == eFLD) || (aRes.nSprmId == eAND)
                   )
                {
                    // Felder/Ftn-/End-Note hier ueberlesen
                    rTxtPos += nSkipChars;
                    nSkipPos = rTxtPos-1;
                }
            }
            else
                EndExtSprm( aRes.nSprmId );
        }
    }

    pStrm->Seek(pSBase->WW8Cp2Fc( pPlcxMan->GetCpOfs() + rTxtPos, &bIsUnicode));

    // Find next Attr position (and Skip attributes of field contents if needed)
    if (nSkipChars && !bIgnoreText)
        pCtrlStck->MarkAllAttrsOld();
    bool bOldIgnoreText = bIgnoreText;
    bIgnoreText = true;
    USHORT nOldColl = nAktColl;
    bool bDoPlcxManPlusPLus = true;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            (*pPlcxMan)++;
        nNext = pPlcxMan->Where();
        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = false;
            bIgnoreText = true;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, false);
    }

    return nNext;
}

void SwWW8ImplReader::ReadAttrs(long& rNext, long& rTxtPos, bool& rbStartLine)
{
    if( rTxtPos >= rNext )
    {           // Stehen Attribute an ?

        do
        {
            rNext = ReadTextAttr( rTxtPos, rbStartLine );
        }while( rTxtPos >= rNext );

    }
    else if ( rbStartLine )
    {
// keine Attribute, aber trotzdem neue Zeile
// wenn eine Zeile mit einem Seitenumbruch aufhoert und sich keine
// Absatzattribute / Absatzvorlagen aendern, ist das Zeilenende
// nicht im Plcx.Fkp.papx eingetragen, d.h. ( nFlags & MAN_MASK_NEW_PAP )
// ist false. Deshalb muss als Sonderbehandlung hier die Vorlage gesetzt
// werden.
        if (!bCpxStyle && nAktColl < nColls)
            SetTxtFmtCollAndListLevel(*pPaM, pCollA[nAktColl]);
        rbStartLine = false;
    }
}

// CloseAttrEnds zum Lesen nur der Attributenden am Ende eines Textes oder
// Textbereiches ( Kopfzeile, Fussnote, ...). Attributanfaenge, Felder
// werden ignoriert.
void SwWW8ImplReader::CloseAttrEnds()
{
    //If there are any unclosed sprms then copy them to
    //another stack and close the ones that must be closed
    std::stack<USHORT> aStack;
    pPlcxMan->TransferOpenSprms(aStack);

    while (!aStack.empty())
    {
        USHORT nSprmId = aStack.top();
        if ((0 < nSprmId) && (( eFTN > nSprmId) || (0x0800 <= nSprmId)))
            EndSprm(nSprmId);
        aStack.pop();
    }

    EndSpecial();
}

bool SwWW8ImplReader::ReadText(long nStartCp, long nTextLen, short nType)
{
    sw::log::Environment eContext = sw::log::eMainText;
    if (nType == MAN_MAINTEXT)
        eContext = sw::log::eMainText;
    else
        eContext = sw::log::eSubDoc;
    maTracer.EnterEnvironment(eContext);

    bool bJoined=false;

    bool bStartLine = true;
    short nCrCount = 0;
    short nDistance = 0;

    bWasParaEnd = false;
    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = false;
    bPgSecBreak = false;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); // Offset fuer Header/Footer, Footnote

    WW8_CP nNext = pPlcxMan->Where();
    SwTxtNode* pPreviousNode = 0;
    BYTE nDropLines = 0;
    SwCharFmt* pNewSwCharFmt = 0;
    const SwCharFmt* pFmt;
    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {

        ReadAttrs( nNext, l, bStartLine );// behandelt auch Section-Breaks
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = ReadChars(l, nNext, nStartCp+nTextLen, nCpOfs);

        // If the previous paragraph was a dropcap then do not
        // create a new txtnode and join the two paragraphs together

        if (bStartLine && !pPreviousNode) // Zeilenende
            AppendTxtNode(*pPaM->GetPoint());

        if (pPreviousNode && bStartLine)
        {
            SwTxtNode* pEndNd = pPaM->GetNode()->GetTxtNode();
            const xub_StrLen nDropCapLen = pPreviousNode->GetTxt().Len();

            // Need to reset the font size and text position for the dropcap
            {
                SwPaM aTmp(*pEndNd, 0, *pEndNd, nDropCapLen+1);
                pCtrlStck->Delete(aTmp);
            }

            // Get the default document dropcap which we can use as our template
            const SwFmtDrop* defaultDrop =
                (const SwFmtDrop*) GetFmtAttr(RES_PARATR_DROP);
            SwFmtDrop aDrop(*defaultDrop);

            aDrop.GetLines() = nDropLines;
            aDrop.GetDistance() = nDistance;
            aDrop.GetChars() = writer_cast<BYTE>(nDropCapLen);
            // Word has no concept of a "whole word dropcap"
            aDrop.GetWholeWord() = false;

            if(pFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pFmt));
            else if(pNewSwCharFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pNewSwCharFmt));

            SwPosition aStart(*pEndNd);
            pCtrlStck->NewAttr(aStart, aDrop);
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_DROP);
            pPreviousNode = 0;
        }
        else if (bDropCap)
        {
            // If we have found a dropcap store the textnode
            pPreviousNode = pPaM->GetNode()->GetTxtNode();

            const BYTE *pDCS;

            if (bVer67)
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(46);
            else
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(0x442C);

            if (pDCS)
                nDropLines = (*pDCS) >> 3;
            else    // There is no Drop Cap Specifier hence no dropcap
                pPreviousNode = 0;

            if (const BYTE *pDistance = pPlcxMan->GetPapPLCF()->HasSprm(0x842F))
                nDistance = SVBT16ToShort( pDistance );
            else
                nDistance = 0;

            const SwFmtCharFmt *pSwFmtCharFmt = 0;

            if(pAktItemSet)
                pSwFmtCharFmt = &(ItemGet<SwFmtCharFmt>(*pAktItemSet, RES_TXTATR_CHARFMT));

            if(pSwFmtCharFmt)
                pFmt = pSwFmtCharFmt->GetCharFmt();

            if(pAktItemSet && !pFmt)
            {
                String sPrefix(CREATE_CONST_ASC( "WW8Dropcap"));
                sPrefix += String::CreateFromInt32( nDropCap++ );
                pNewSwCharFmt = rDoc.MakeCharFmt(sPrefix, (SwCharFmt*)rDoc.GetDfltCharFmt());
                 pAktItemSet->ClearItem(RES_CHRATR_ESCAPEMENT);
                pNewSwCharFmt->SetAttr( *pAktItemSet );
            }

            delete pAktItemSet;
            pAktItemSet = 0;
            bDropCap=false;
        }

        if (bStartLine || bWasTabRowEnd)
        {
            // alle 64 CRs aufrufen not for Header u. ae.
            if ((nCrCount++ & 0x40) == 0 && nType == MAN_MAINTEXT)
            {
                nProgress = (USHORT)( l * 100 / nTextLen );
                ::SetProgressState(nProgress, mpDocShell); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if (bPgSecBreak)
        {
            // We need only to see if a section is ending at this cp,
            // the plcf will already be sitting on the correct location
            // if it is there.
            WW8PLCFxDesc aTemp;
            aTemp.nStartPos = aTemp.nEndPos = LONG_MAX;
            if (pPlcxMan->GetSepPLCF())
                pPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
//              AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert(*pPaM, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
                bPgSecBreak = false;
            }
        }
    }

    if (pPaM->GetPoint()->nContent.GetIndex())
        AppendTxtNode(*pPaM->GetPoint());

    if (!bInHyperlink)
        bJoined = JoinNode(*pPaM);

    CloseAttrEnds();

    delete pPlcxMan, pPlcxMan = 0;
    maTracer.LeaveEnvironment(eContext);
    return bJoined;
}

/***************************************************************************
#           class SwWW8ImplReader
#**************************************************************************/

SwWW8ImplReader::SwWW8ImplReader(BYTE nVersionPara, SvStorage* pStorage,
    SvStream* pSt, SwDoc& rD, bool bNewDoc)
    : mpDocShell(rD.GetDocShell()), maTracer(*(mpDocShell->GetMedium())),
    pStg(pStorage), pStrm(pSt), pTableStream(0), pDataStream(0), rDoc(rD),
    maSectionManager(*this), maInsertedTables(rD),
    maSectionNameGenerator(rD,CREATE_CONST_ASC("WW")),
    maGrfNameGenerator(bNewDoc,String('G')), maParaStyleMapper(rD),
    maCharStyleMapper(rD), pMSDffManager(0), mpAtnNames(0), pAuthorInfos(0),
    mbNewDoc(bNewDoc), nDropCap(0)
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    mpRedlineStack = 0;
    pRefStck = 0;
    pAnchorStck = 0;
    pFonts = 0;
    pSBase = 0;
    pPlcxMan = 0;
    pStyles = 0;
    pAktColl = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pCollA = 0;
    pDfltTxtFmtColl = 0;
    pStandardFmtColl = 0;
    pHdFt = 0;
    pWFlyPara = 0;
    pSFlyPara = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedApo = 0;
    pPreviousNumPaM = 0;
    pPrevNumRule = 0;
    nColls = nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    nInTable=0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText
               = bWasTabRowEnd = false;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bIsUnicode = bCpxStyle = bStyNormal =
                 bWWBugNormal  = false;

    bNoAttrImport = bPgChpLevel = bEmbeddObj = false;
    bAktAND_fNumberAcross = false;
    bNoLnNumYet = true;
    bInHyperlink = false;
    bWasParaEnd = false;
    bDropCap = false;
    bFirstPara = true;
    bParaAutoBefore = false;
    bParaAutoAfter = false;
    nProgress = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    pNumOlst = 0;
    pNode_FLY_AT_CNTNT = 0;
    pDrawModel = 0;
    pDrawPg = 0;
    nDrawTxbx = 0;
    pDrawEditEngine = 0;
    pWWZOrder = 0;
    pFormImpl = 0;
    mpChosenOutlineNumRule = 0;
    pNumFldType = 0;
    nFldNum = 0;

    nLFOPosition = USHRT_MAX;
    nListLevel = WW8ListManager::nMaxLevel;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;

    nPgChpDelim = nPgChpLevel = 0;

    maApos.push_back(false);
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        delete pStck;
    }
    else
    {
        ASSERT( !this, "WW-Stack bereits geloescht" );
    }
}

void wwSectionManager::SetSegmentToPageDesc(const wwSection &rSection,
    bool bTitlePage, bool bIgnoreCols)
{
    SwPageDesc &rPage = bTitlePage ? *rSection.mpTitlePage : *rSection.mpPage;

    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();

    if (mrReader.pWDop->fUseBackGroundInAllmodes && mrReader.pMSDffManager)
    {
        Rectangle aRect(0, 0, 100, 100); //A dummy, we don't care about the size
        SvxMSDffImportData aData(aRect);
        SdrObject* pObject = 0;
        if (mrReader.pMSDffManager->GetShape(0x401, pObject, aData))
        {
            SfxItemSet aSet(rFmt.GetAttrSet());
            mrReader.MatchSdrItemsIntoFlySet(pObject, aSet, mso_lineSimple,
                mso_sptRectangle, aRect);
            rFmt.SetAttr(aSet.Get(RES_BACKGROUND));
        }
    }
    wwULSpaceData aULData;
    GetPageULData(rSection, bTitlePage, aULData);
    SetPageULSpaceItems(rFmt, aULData, rSection);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    bool bSetBorder = false;
    switch (rSection.maSep.pgbApplyTo)
    {
        case 0:
        case 3:
            bSetBorder = true;
            break;
        case 1:
            bSetBorder = bTitlePage;
            break;
        case 2:
            bSetBorder = !bTitlePage;
            break;
    }
    if (bSetBorder)
        mrReader.SetPageBorder(rFmt, rSection);

    mrReader.SetDocumentGrid(rFmt, rSection);
}

void wwSectionManager::SetUseOn(wwSection &rSection)
{
    bool bEven = (rSection.maSep.grpfIhdt & (WW8_HEADER_EVEN|WW8_FOOTER_EVEN)) ?
        true : false;

    bool bMirror = mrReader.pWDop->fMirrorMargins ||
        mrReader.pWDop->doptypography.f2on1;

    UseOnPage eUseBase = bMirror ? PD_MIRROR : PD_ALL;
    UseOnPage eUse = eUseBase;
    if (!bEven)
        eUse = (UseOnPage)(eUse | PD_HEADERSHARE | PD_FOOTERSHARE);

    ASSERT(rSection.mpPage, "Makes no sense to call me with no pages to set");
    if (rSection.mpPage)
        rSection.mpPage->WriteUseOn(eUse);
    if (rSection.mpTitlePage)
    {
        rSection.mpTitlePage->WriteUseOn(
            (UseOnPage) (eUseBase | PD_HEADERSHARE | PD_FOOTERSHARE));
    }
}

//Set the page descriptor on this node, handle the different cases for a text
//node or a table
void GiveNodePageDesc(SwNodeIndex &rIdx, const SwFmtPageDesc &rPgDesc,
    SwDoc &rDoc)
{
    /*
    If its a table here, apply the pagebreak to the table
    properties, otherwise we add it to the para at this
    position
    */
    if (rIdx.GetNode().IsTableNode())
    {
        SwTable& rTable =
            rIdx.GetNode().GetTableNode()->GetTable();
        SwFrmFmt* pApply = rTable.GetFrmFmt();
        ASSERT(pApply, "impossible");
        if (pApply)
            pApply->SetAttr(rPgDesc);
    }
    else
    {
        SwPosition aPamStart(rIdx);
        aPamStart.nContent.Assign(
            rIdx.GetNode().GetCntntNode(), 0);
        SwPaM aPage(aPamStart);

        rDoc.Insert(aPage, rPgDesc);
    }
}

//Map a word section with to either one or two writer page descriptors
//depending on if the word section has a title page
SwFmtPageDesc wwSectionManager::SetSwFmtPageDesc(mySegIter &rIter,
    mySegIter &rStart, bool bIgnoreCols)
{
    SwFmtPageDesc aEmpty;
    if (rIter->HasTitlePage())
    {
        if (IsNewDoc() && rIter == rStart)
        {
            rIter->mpTitlePage =
                mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_FIRST);
        }
        else
        {
            USHORT nPos = mrReader.rDoc.MakePageDesc(
                ViewShell::GetShellRes()->GetPageDescName(mnDesc)
                , 0, false);
            rIter->mpTitlePage = &mrReader.rDoc._GetPageDesc(nPos);
        }
        ASSERT(rIter->mpTitlePage, "no page!");
        if (!rIter->mpTitlePage)
            return aEmpty;

        SetSegmentToPageDesc(*rIter, true, bIgnoreCols);
    }

    if (IsNewDoc() && rIter == rStart)
    {
        rIter->mpPage =
            mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    }
    else
    {
        USHORT nPos = mrReader.rDoc.MakePageDesc(
            ViewShell::GetShellRes()->GetPageDescName(mnDesc,
                false, rIter->HasTitlePage()),
                rIter->mpTitlePage, false);
        rIter->mpPage = &mrReader.rDoc._GetPageDesc(nPos);
    }
    ASSERT(rIter->mpPage, "no page!");
    if (!rIter->mpPage)
        return aEmpty;

    //Set page before hd/ft
    const wwSection *pPrevious = 0;
    if (rIter != rStart)
        pPrevious = &(*(rIter-1));
    SetHdFt(*rIter, std::distance(rStart, rIter), pPrevious);
    SetUseOn(*rIter);

    //Set hd/ft after set page
    if (rIter->mpTitlePage)
        SetSegmentToPageDesc(*rIter, true, bIgnoreCols);
    SetSegmentToPageDesc(*rIter, false, bIgnoreCols);

    SwFmtPageDesc aRet(rIter->HasTitlePage() ?
        rIter->mpTitlePage : rIter->mpPage);

    rIter->mpPage->SetFollow(rIter->mpPage);

    if (rIter->mpTitlePage)
        rIter->mpTitlePage->SetFollow(rIter->mpPage);

    if (rIter->PageRestartNo())
        aRet.SetNumOffset(rIter->PageStartAt());

    ++mnDesc;
    return aRet;
}

bool wwSectionManager::IsNewDoc() const
{
    return mrReader.mbNewDoc;
}

void wwSectionManager::InsertSegments()
{
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        mySegIter aNext = aIter+1;

        bool bInsertSection = aIter != aStart ? aIter->IsContinous() : false;
        bool bInsertPageDesc = !bInsertSection;
        bool bProtected = SectionIsProtected(*aIter);

        if (bInsertPageDesc)
        {
            /*
             If a cont section follows this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            bool bIgnoreCols = false;
            if ((aNext != aEnd && aNext->IsContinous() || bProtected))
            {
                bIgnoreCols = true;
                if ((aIter->NoCols() > 1) || bProtected)
                    bInsertSection = true;
            }

            SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, bIgnoreCols));
            if (!aDesc.GetPageDesc())
                continue;
            GiveNodePageDesc(aIter->maStart, aDesc, mrReader.rDoc);
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            //Start getting the bounds of this section
            SwPaM aSectPaM(*mrReader.pPaM);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            const SwTxtNode* pSttNd =
                mrReader.rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.rDoc.GetNodes().MakeTxtNode(aAnchor,
                    mrReader.rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);
            //End getting the bounds of this section, quite a job eh ?

            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            //The last section if continous is always unbalanced
            if (pRet)
            {
                //Set the columns to be UnBalanced if that compatability option
                //is set
                if (mrReader.pWDop->fNoColumnBalance)
                    pRet->SetAttr(SwFmtNoBalancedColumns(true));
                else
                {
                    //Otherwise set to unbalanced if the following section is
                    //not continuous, (which also means that the last section
                    //is unbalanced)
                    if (aNext == aEnd || !aNext->IsContinous())
                        pRet->SetAttr(SwFmtNoBalancedColumns(true));
                }
            }

            bool bHasOwnHdFt = false;
            /*
             #112027# #110379#
             In this nightmare scenario the continuous section has its own
             headers and footers so we will try and find a hard page break
             between here and the end of the section and put the headers and
             footers there.
            */
            if (!bInsertPageDesc)
            {
               bHasOwnHdFt =
                mrReader.HasOwnHeaderFooter(
                 aIter->maSep.grpfIhdt & ~(WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
                 aIter->maSep.grpfIhdt, std::distance(aStart, aIter)
                );
            }
            if (bHasOwnHdFt)
            {
                SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, true));
                if (aDesc.GetPageDesc())
                {
                    ULONG nStart = aSectPaM.Start()->nNode.GetIndex();
                    ULONG nEnd   = aSectPaM.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = mrReader.rDoc.GetNodes()[nStart];
                        if (!pNode)
                            continue;
                        if (sw::util::HasPageBreak(*pNode))
                        {
                            SwNodeIndex aIdx(*pNode);
                            GiveNodePageDesc(aIdx, aDesc, mrReader.rDoc);
                            break;
                        }
                    }
                }
            }
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPosition aPos(aIdx);
            SwPaM aTest(aPos);
            mrReader.rDoc.DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

void SwWW8ImplReader::StoreMacroCmds()
{
    if (pWwFib->lcbCmds)
    {
        maTracer.Log(sw::log::eContainsWordBasic);

        pTableStream->Seek(pWwFib->fcCmds);

        SvStorageRef xDestRoot(mpDocShell->GetStorage());
        SvStorageStreamRef xDestStream =
            xDestRoot->OpenStream(CREATE_CONST_ASC(SL::aMSMacroCmds));

        sal_uInt8 *pBuffer = new sal_uInt8[pWwFib->lcbCmds];
        pTableStream->Read(pBuffer, pWwFib->lcbCmds);
        xDestStream->Write(pBuffer, pWwFib->lcbCmds);
        delete[] pBuffer;
    }
}

ULONG SwWW8ImplReader::CoreLoad(WW8Glossary *pGloss, const SwPosition &rPos)
{
    ULONG nErrRet = 0;

    if (mbNewDoc && pStg && !pGloss)
        ReadDocInfo();

    pPaM = new SwPaM(rPos);

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    mpRedlineStack = new wwRedlineStack(rDoc);

    /*
        RefFldStck: Keeps track of bookmarks which may be inserted as
        variables intstead.
    */
    pRefStck = new SwWW8FltRefStack(&rDoc, nFieldFlags);

    pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

    sal_uInt16 nPageDescOffset = rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule(rDoc, mbNewDoc);

    USHORT eMode = REDLINE_SHOW_INSERT;

    mpSprmParser = new wwSprmParser(pWwFib->nVersion);

    // praktische Hilfsvariablen besetzen:
    bVer6  = (6 == pWwFib->nVersion);
    bVer7  = (7 == pWwFib->nVersion);
    bVer67 = bVer6 || bVer7;
    bVer8  = (8 == pWwFib->nVersion);

    eTextCharSet = WW8Fib::GetFIBCharset(pWwFib->chse);
    eStructCharSet = WW8Fib::GetFIBCharset(pWwFib->chseTables);

    bWWBugNormal = pWwFib->nProduct == 0xc03d;

    if (!mbNewDoc)
        aSttNdIdx = pPaM->GetPoint()->nNode;

    ::StartProgress(STR_STATSTR_W4WREAD, 0, 100, mpDocShell);

#ifdef DEBUGDUMP
    //experimental embedded ttf dumper
    if (pWwFib->lcbSttbttmbd && (7 < pWwFib->nVersion))
    {
        pTableStream->Seek(pWwFib->fcSttbttmbd);
        sal_uInt16 nZeros;
        *pTableStream >> nZeros;
        sal_uInt16 nNoEntries;
        *pTableStream >> nNoEntries;
        sal_uInt32 nUnknown1;
        *pTableStream >> nUnknown1;
        sal_uInt16 nUnknown2;
        *pTableStream >> nUnknown2;
        std::vector<sal_uInt32> aOffsets;
        for (sal_uInt16 nI = 0; nI < nNoEntries; ++nI)
        {
            sal_uInt32 nOffset;
            *pTableStream >> nOffset;
            aOffsets.push_back(nOffset);
            sal_uInt32 nUnknown3;
            *pTableStream >> nUnknown3;
            sal_uInt32 nUnknown4;
            *pTableStream >> nUnknown4;
        }
        typedef std::vector<sal_uInt32>::iterator myIter;
        myIter aEnd = aOffsets.end();
        myIter aIter = aOffsets.begin();
        while (aIter != aEnd)
        {
            sal_uInt32 nOffset = *aIter;
            sal_uInt32 nLen = STREAM_SEEK_TO_END;
            ++aIter;
            pStrm->Seek(nOffset);
            if (aIter != aEnd)
                nLen = *aIter - nOffset;
            SvStream *pDbg = sw::hack::CreateDebuggingStream(CREATE_CONST_ASC(".ttf.dump"));
            sw::hack::DumpStream(*pStrm, *pDbg, nLen);
            delete pDbg;
        }
    }
#endif

    // read Font Table
    pFonts = new WW8Fonts( *pTableStream, *pWwFib );

    // Document Properties
    pWDop = new WW8Dop( *pTableStream, pWwFib->nFib, pWwFib->fcDop,
        pWwFib->lcbDop );

    if (mbNewDoc)
        ImportDop();

    /*
        Import revisioning data: author names
    */
    if( pWwFib->lcbSttbfRMark )
    {
        ReadRevMarkAuthorStrTabl( *pTableStream,
                                    pWwFib->fcSttbfRMark,
                                    pWwFib->lcbSttbfRMark, rDoc );
    }

    // M.M. Initialize our String/ID map for Linked Sections
    std::vector<String> aLinkStrings;
    std::vector<ww::bytes> aStringIds;

    WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcSttbFnm,
        pWwFib->lcbSttbFnm, bVer67 ? 2 : 0, eStructCharSet,
        aLinkStrings, &aStringIds);

    for (int i=0; i < aLinkStrings.size(); ++i)
    {
        ww::bytes stringId = aStringIds[i];
        WW8_STRINGID *stringIdStruct = (WW8_STRINGID*)(&stringId[0]);
        aLinkStringMap[SVBT16ToShort(stringIdStruct->nStringId)] =
            aLinkStrings[i];
    }

    ::SetProgressState(nProgress, mpDocShell);    // Update

    pLstManager = new WW8ListManager( *pTableStream, *this );

    /*
        zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
            VOR dem Import der Listen !!
    */
    ::SetProgressState(nProgress, mpDocShell);    // Update
    pStyles = new WW8RStyle( *pWwFib, this );   // Styles
    pStyles->Import();

    /*
        zu guter Letzt: (siehe ebenfalls WW8PAR3.CXX)
        ===============
        alle Styles durchgehen und ggfs. zugehoeriges Listen-Format
        anhaengen NACH dem Import der Styles und NACH dem Import der
        Listen !!
    */
    ::SetProgressState(nProgress, mpDocShell);    // Update
    pStyles->PostProcessStyles();

    if (pCollA)
        SetOutLineStyles();

    pSBase = new WW8ScannerBase(pStrm,pTableStream,pDataStream,pWwFib);

    static const SvxExtNumType eNumTA[16] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC
    };

    if (pSBase->AreThereFootnotes())
    {
        static const SwFtnNum eNumA[4] =
        {
            FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC
        };

        SwFtnInfo aInfo;
        aInfo = rDoc.GetFtnInfo();      // Copy-Ctor privat

        aInfo.ePos = FTNPOS_PAGE;
        aInfo.eNum = eNumA[pWDop->rncFtn];
        aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcFtnRef]);
        if( pWDop->nFtn )
            aInfo.nFtnOffset = pWDop->nFtn - 1;
        rDoc.SetFtnInfo( aInfo );
    }
    if( pSBase->AreThereEndnotes() )
    {
        SwEndNoteInfo aInfo;
        aInfo = rDoc.GetEndNoteInfo();  // parallel zu Ftn

        // Ich kann nicht setzen, wann neu nummerieren...
        //  aInfo.eNum = eNumA[pWDop->pDop->rncEdn];
        aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcEdnRef]);
        if( pWDop->nEdn )
            aInfo.nFtnOffset = pWDop->nEdn - 1;
        rDoc.SetEndNoteInfo( aInfo );
    }

    if( pWwFib->lcbPlcfhdd )
        pHdFt = new WW8PLCF_HdFt( pTableStream, *pWwFib, *pWDop );

    if (!mbNewDoc)
    {
        // in ein Dokument einfuegen ?
        // Da immer ganze Zeile eingelesen werden, muessen
        // evtl. Zeilen eingefuegt / aufgebrochen werden
        //
        const SwPosition* pPos = pPaM->GetPoint();
        const SwTxtNode* pSttNd =
            rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();

        USHORT nCntPos = pPos->nContent.GetIndex();

        // EinfuegePos nicht in leerer Zeile
        if( nCntPos && pSttNd->GetTxt().Len() )
            rDoc.SplitNode( *pPos );            // neue Zeile erzeugen

        if( pSttNd->GetTxt().Len() )
        {   // EinfuegePos nicht am Ende der Zeile
            rDoc.SplitNode( *pPos );    // neue Zeile
            pPaM->Move( fnMoveBackward );   // gehe in leere Zeile
        }

        // verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
        ULONG nNd = pPos->nNode.GetIndex();
        bReadNoTbl = 0 != pSttNd->FindTableNode() ||
            ( nNd < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
            rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex()
            < nNd );

    }

    ::SetProgressState(nProgress, mpDocShell);    // Update

    // loop for each glossary entry and add dummy section node
    if (pGloss)
    {
        WW8PLCF aPlc(pTableStream, pWwFib->fcPlcfglsy, pWwFib->lcbPlcfglsy, 0);

        WW8_CP nStart, nEnd;
        void* pDummy;

        for (int i=0;i<pGloss->GetNoStrings();i++,aPlc++)
        {
            SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfContent());
            SwTxtFmtColl* pColl =
                rDoc.GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD,
                false);
            SwStartNode *pNode =
                rDoc.GetNodes().MakeTextSection(aIdx,
                SwNormalStartNode,pColl);
            pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
            pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(),0);
            aPlc.Get( nStart, nEnd, pDummy );
            ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
        }
    }
    else //ordinary case
    {
        ReadText(0, pWwFib->ccpText, MAN_MAINTEXT);
    }

    ::SetProgressState(nProgress, mpDocShell);    // Update

    if (pDrawPg && pMSDffManager && pMSDffManager->GetShapeOrders())
    {
        // Hilfsarray zum Verketten der (statt SdrTxtObj) eingefuegten
        // Rahmen
        SvxMSDffShapeTxBxSort aTxBxSort;

        // korrekte Z-Order der eingelesen Escher-Objekte sicherstellen
        USHORT nShapeCount = pMSDffManager->GetShapeOrders()->Count();

        for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
        {
            SvxMSDffShapeOrder *pOrder =
                pMSDffManager->GetShapeOrders()->GetObject(nShapeNum);
            // Pointer in neues Sort-Array einfuegen
            if (pOrder->nTxBxComp && pOrder->pFly)
                aTxBxSort.Insert(pOrder);
        }
        // zu verkettende Rahmen jetzt verketten
        USHORT nTxBxCount = aTxBxSort.Count();
        if( nTxBxCount )
        {
            SwFmtChain aChain;
            for (USHORT nTxBxNum=0; nTxBxNum < nTxBxCount; nTxBxNum++)
            {
                SvxMSDffShapeOrder *pOrder =
                    aTxBxSort.GetObject(nTxBxNum);

                // Fly-Frame-Formate initialisieren
                SwFlyFrmFmt* pFlyFmt     = pOrder->pFly;
                SwFlyFrmFmt* pNextFlyFmt = 0;
                SwFlyFrmFmt* pPrevFlyFmt = 0;
                // ggfs. Nachfolger ermitteln
                if( 1+nTxBxNum < nTxBxCount )
                {
                    SvxMSDffShapeOrder *pNextOrder =
                        aTxBxSort.GetObject(nTxBxNum+1);
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pNextOrder->nTxBxComp))
                        pNextFlyFmt = pNextOrder->pFly;
                }
                // ggfs. Vorgaenger ermitteln
                if( nTxBxNum )
                {
                    SvxMSDffShapeOrder *pPrevOrder =
                        aTxBxSort.GetObject(nTxBxNum-1);
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pPrevOrder->nTxBxComp))
                        pPrevFlyFmt = pPrevOrder->pFly;
                }
                // Falls Nachfolger oder Vorgaenger vorhanden,
                // die Verkettung am Fly-Frame-Format eintragen
                if (pNextFlyFmt || pPrevFlyFmt)
                {
                    aChain.SetNext( pNextFlyFmt );
                    aChain.SetPrev( pPrevFlyFmt );
                    pFlyFmt->SetAttr( aChain );
                }
            }

        }

    }

    if (mbNewDoc)
    {
        if( pWDop->fRevMarking )
            eMode |= REDLINE_ON;
        if( pWDop->fRMView )
            eMode |= REDLINE_SHOW_DELETE;
        if (pStg && !pGloss) /*meaningless for a glossary, cmc*/
        {
            const SvtFilterOptions* pVBAFlags = SvtFilterOptions::Get();
            maTracer.EnterEnvironment(sw::log::eMacros);
            SvxImportMSVBasic aVBasic(*mpDocShell, *pStg,
                            pVBAFlags->IsLoadWordBasicCode(),
                            pVBAFlags->IsLoadWordBasicStorage() );
            String s1(CREATE_CONST_ASC("Macros"));
            String s2(CREATE_CONST_ASC("VBA"));
            int nRet = aVBasic.Import( s1, s2 );
            if( 2 & nRet )
            {
                maTracer.Log(sw::log::eContainsVisualBasic);
                rDoc.SetContainsMSVBasic(true);
            }

            StoreMacroCmds();

            maTracer.LeaveEnvironment(sw::log::eMacros);
        }
    }

    maInsertedTables.DelAndMakeTblFrms();
    maSectionManager.InsertSegments();

    if (pCollA)
        delete[] pCollA;

    DELETEZ( pStyles );

    if( pFormImpl )
        DeleteFormImpl();
    GrafikDtor();
    DELETEZ( pMSDffManager );
    DELETEZ( pHdFt );
    DELETEZ( pLstManager );
    DELETEZ( pSBase );
    delete pWDop;
    DELETEZ( pFonts );
    delete mpAtnNames;
    DELETEZ( pAuthorInfos );
    delete mpSprmParser;
    ::EndProgress(mpDocShell);

    pDataStream = 0;
    pTableStream = 0;

    DeleteCtrlStk();
    mpRedlineStack->closeall(*pPaM->GetPoint());
    delete mpRedlineStack;
    DeleteAnchorStk();
    DeleteRefStk();

    UpdateFields();

    // delete the pam before the call for hide all redlines (Bug 73683)
    if (mbNewDoc)
        rDoc.SetRedlineMode( eMode );

    UpdatePageDescs(rDoc, nPageDescOffset);

    delete pPaM, pPaM = 0;
    return nErrRet;
}

ULONG SwWW8ImplReader::SetSubStreams(SvStorageStreamRef &rTableStream,
    SvStorageStreamRef &rDataStream)
{
    ULONG nErrRet = 0;
    // 6 stands for "6 OR 7",  7 stand for "ONLY 7"
    switch (pWwFib->nVersion)
    {
        case 6:
        case 7:
            pTableStream = pStrm;
            pDataStream = pStrm;
            break;
        case 8:
            if(!pStg)
            {
                ASSERT( pStg, "Version 8 muss immer einen Storage haben!" );
                nErrRet = ERR_SWG_READ_ERROR;
                break;
            }

            rTableStream = pStg->OpenStream(String::CreateFromAscii(
                pWwFib->fWhichTblStm ? SL::a1Table : SL::a0Table),
                STREAM_STD_READ);

            pTableStream = &rTableStream;
            pTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

            rDataStream = pStg->OpenStream(CREATE_CONST_ASC(SL::aData),
                STREAM_STD_READ | STREAM_NOCREATE );

            if (rDataStream.Is() && SVSTREAM_OK == rDataStream->GetError())
            {
                pDataStream = &rDataStream;
                pDataStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            }
            else
                pDataStream = pStrm;
            break;
        default:
            // Programm-Fehler!
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            nErrRet = ERR_SWG_READ_ERROR;
            break;
    }
    return nErrRet;
}

namespace
{
    utl::TempFile *MakeTemp(SvFileStream &rSt)
    {
        utl::TempFile *pT = new utl::TempFile;
        pT->EnableKillingFile();
        rSt.Open(pT->GetFileName(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE);
        return pT;
    }

#define WW_BLOCKSIZE 0x200

    void DecryptRC4(svx::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        rIn.Seek(STREAM_SEEK_TO_END);
        ULONG nLen = rIn.Tell();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (ULONG nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            ULONG nBS = (nLen - nI > WW_BLOCKSIZE) ? WW_BLOCKSIZE : nLen - nI;
            rIn.Read(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Decode(in, nBS, in, nBS);
            rOut.Write(in, nBS);
        }
    }

    void DecryptXOR(svx::MSCodec_XorWord95 &rCtx, SvStream &rIn, SvStream &rOut)
    {
        ULONG nSt = rIn.Tell();
        rIn.Seek(STREAM_SEEK_TO_END);
        ULONG nLen = rIn.Tell();
        rIn.Seek(nSt);

        rCtx.InitCipher();
        rCtx.Skip(nSt);

        sal_uInt8 in[0x4096];
        for (ULONG nI = nSt; nI < nLen; nI += 0x4096)
        {
            ULONG nBS = (nLen - nI > 0x4096 ) ? 0x4096 : nLen - nI;
            rIn.Read(in, nBS);
            rCtx.Decode(in, nBS);
            rOut.Write(in, nBS);
        }
    }

    //moan, copy and paste :-(
    String QueryPasswordForMedium(SfxMedium& rMedium)
    {
        String aPassw;

        using namespace com::sun::star;
        try
        {
            uno::Reference< task::XInteractionHandler > xHandler( rMedium.GetInteractionHandler() );
            if( xHandler.is() )
            {
                RequestDocumentPassword* pRequest = new RequestDocumentPassword(
                    task::PasswordRequestMode_PASSWORD_ENTER,
                    INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );
                uno::Reference< task::XInteractionRequest > xRequest( pRequest );

                xHandler->handle( xRequest );

                if( pRequest->isPassword() )
                    aPassw = pRequest->getPassword();
            }
        }
        catch( uno::Exception& )
        {
        }

        return aPassw;
    }

}

ULONG SwWW8ImplReader::LoadThroughDecryption(SwPaM& rPaM ,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;
    if (pGloss)
        pWwFib = pGloss->GetFib();
    else
        pWwFib = new WW8Fib(*pStrm, nWantedVersion);

    if (pWwFib->nFibError)
        nErrRet = ERR_SWG_READ_ERROR;

    SvStorageStreamRef xTableStream, xDataStream;

    if (!nErrRet)
        nErrRet = SetSubStreams(xTableStream, xDataStream);

    utl::TempFile *pTempMain = 0;
    utl::TempFile *pTempTable = 0;
    utl::TempFile *pTempData = 0;
    SvFileStream aDecryptMain;
    SvFileStream aDecryptTable;
    SvFileStream aDecryptData;

    bool bDecrypt = false;
    enum {RC4, XOR, Other} eAlgo = Other;
    if (pWwFib->fEncrypted && !nErrRet)
    {
        if (!pGloss)
        {
            bDecrypt = true;
            if (8 != pWwFib->nVersion)
                eAlgo = XOR;
            else
            {
                if (pWwFib->nKey != 0)
                    eAlgo = XOR;
                else
                {
                    pTableStream->Seek(0);
                    sal_uInt32 nEncType;
                    *pTableStream >> nEncType;
                    if (nEncType == 0x10001)
                        eAlgo = RC4;
                }
            }
        }
    }

    if (bDecrypt)
    {
        nErrRet = ERRCODE_SVX_WRONGPASS;
        switch (eAlgo)
        {
            default:
                nErrRet = ERRCODE_SVX_READ_FILTER_CRYPT;
                break;
            case XOR:
            {
                String sUniPassword =
                    QueryPasswordForMedium(*(mpDocShell->GetMedium()));

                ByteString sPassword(sUniPassword,
                    WW8Fib::GetFIBCharset(pWwFib->chseTables));

                xub_StrLen nLen = sPassword.Len();
                // DR: do not cut a wrong (too long) password
                if( nLen <= 15 )
                {
                    sal_uInt8 aPassword[16] = {0};

                    for (xub_StrLen nChar = 0; nChar < sPassword.Len(); ++nChar )
                        aPassword[nChar] = sPassword.GetChar(nChar);

                    svx::MSCodec_XorWord95 aCtx;
                    aCtx.InitKey(aPassword);
                    if (aCtx.VerifyKey(pWwFib->nKey, pWwFib->nHash))
                    {
                        nErrRet = 0;
                        pTempMain = MakeTemp(aDecryptMain);

                        pStrm->Seek(0);
                        size_t nUnencryptedHdr =
                            (8 == pWwFib->nVersion) ? 0x44 : 0x34;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        pStrm->Read(pIn, nUnencryptedHdr);
                        aDecryptMain.Write(pIn, nUnencryptedHdr);
                        delete pIn;

                        DecryptXOR(aCtx, *pStrm, aDecryptMain);

                        if (!pTableStream || pTableStream == pStrm)
                            pTableStream = &aDecryptMain;
                        else
                        {
                            pTempTable = MakeTemp(aDecryptTable);
                            DecryptXOR(aCtx, *pTableStream, aDecryptTable);
                            pTableStream = &aDecryptTable;
                        }

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptXOR(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }
                    }
                }
            }
            break;
            case RC4:
            {
                String sUniPassword =
                    QueryPasswordForMedium(*(mpDocShell->GetMedium()));

                xub_StrLen nLen = sUniPassword.Len();
                // DR: do not cut a wrong (too long) password
                if (nLen <= 15)
                {
                    sal_Unicode aPassword[16] = {0};
                    for (xub_StrLen nChar = 0; nChar < nLen; ++nChar )
                        aPassword[nChar] = sUniPassword.GetChar(nChar);

                    sal_uInt8 aDocId[ 16 ];
                    pTableStream->Read(aDocId, 16);
                    sal_uInt8 aSaltData[ 16 ];
                    pTableStream->Read(aSaltData, 16);
                    sal_uInt8 aSaltHash[ 16 ];
                    pTableStream->Read(aSaltHash, 16);

                    svx::MSCodec_Std97 aCtx;
                    aCtx.InitKey(aPassword, aDocId);
                    if (aCtx.VerifyKey(aSaltData, aSaltHash))
                    {
                        nErrRet = 0;

                        pTempTable = MakeTemp(aDecryptTable);
                        DecryptRC4(aCtx, *pTableStream, aDecryptTable);
                        pTableStream = &aDecryptTable;

                        pTempMain = MakeTemp(aDecryptMain);
                        DecryptRC4(aCtx, *pStrm, aDecryptMain);

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptRC4(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }
                    }
                }
            }
            break;
        }

        if (nErrRet == 0)
        {
            pStrm = &aDecryptMain;

            delete pWwFib;
            pWwFib = new WW8Fib(*pStrm, nWantedVersion);
            if (pWwFib->nFibError)
                nErrRet = ERR_SWG_READ_ERROR;
        }
    }

    if (!nErrRet)
        nErrRet = CoreLoad(pGloss, *rPaM.GetPoint());

    delete pTempMain;
    delete pTempTable;
    delete pTempData;

    if (!pGloss)
        delete pWwFib;
    return nErrRet;
}

class outlineeq : public std::unary_function<const SwTxtFmtColl*, bool>
{
private:
    BYTE mnNum;
public:
    outlineeq(BYTE nNum) : mnNum(nNum) {}
    bool operator()(const SwTxtFmtColl *pTest) const
    {
        return pTest->GetOutlineLevel() == mnNum;
    }
};

void SwWW8ImplReader::SetOutLineStyles()
{
    /*
    #i3674# & #101291# Load new document and insert document cases.
    */
    SwNumRule aOutlineRule(*rDoc.GetOutlineNumRule());
    mpChosenOutlineNumRule = &aOutlineRule;

    sw::ParaStyles aOutLined(sw::util::GetParaStyles(rDoc));
    sw::util::SortByOutline(aOutLined);

    typedef sw::ParaStyleIter myiter;
    /*
    If we are inserted into a document then don't clobber existing existing
    levels.
    */
    USHORT nFlagsStyleOutlLevel = 0;
    if (!mbNewDoc)
    {
        myiter aEnd = aOutLined.end();
        for (myiter aIter = aOutLined.begin(); aIter < aEnd; ++aIter)
        {
            if ((*aIter)->GetOutlineLevel() < MAXLEVEL)
                nFlagsStyleOutlLevel |= 1 << (*aIter)->GetOutlineLevel();
            else
                break;
        }
    }
    else
    {
        /*
        #111955#
        Only import *one* of the possible multiple outline numbering rules, so
        pick the one that affects most styles. If we're not importing a new
        document, we got to stick with what is already there.
        */
        std::map<SwNumRule *, int>aRuleMap;
        typedef std::map<SwNumRule *, int>::iterator myIter;
        for (USHORT nI = 0; nI < nColls; ++nI)
        {
            SwWW8StyInf& rSI = pCollA[ nI ];
            if (
                (MAXLEVEL > rSI.nOutlineLevel) && rSI.pOutlineNumrule &&
                rSI.pFmt
               )
            {
                myIter aIter = aRuleMap.find(rSI.pOutlineNumrule);
                if (aIter == aRuleMap.end())
                    aRuleMap[rSI.pOutlineNumrule] = 1;
                else
                    ++(aIter->second);
            }
        }

        int nMax = 0;
        myIter aEnd = aRuleMap.end();
        for (myIter aIter = aRuleMap.begin(); aIter != aEnd; ++aIter++)
        {
            if (aIter->second > nMax)
            {
                nMax = aIter->second;
                mpChosenOutlineNumRule = aIter->first;
            }
        }

        ASSERT(mpChosenOutlineNumRule, "Impossible");
        if (mpChosenOutlineNumRule)
            aOutlineRule = *mpChosenOutlineNumRule;

        if (mpChosenOutlineNumRule != &aOutlineRule)
        {
            myiter aEnd = aOutLined.end();
            for (myiter aIter = aOutLined.begin(); aIter < aEnd; ++aIter)
            {
                if ((*aIter)->GetOutlineLevel() < MAXLEVEL)
                    (*aIter)->SetOutlineLevel(NO_NUMBERING);
                else
                    break;
            }
        }
    }

    USHORT nOldFlags = nFlagsStyleOutlLevel;

    for (USHORT nI = 0; nI < nColls; ++nI)
    {
        SwWW8StyInf& rSI = pCollA[nI];

        if (rSI.IsOutlineNumbered())
        {
            USHORT nAktFlags = 1 << rSI.nOutlineLevel;
            if (
                 (nAktFlags & nFlagsStyleOutlLevel) ||
                 (rSI.pOutlineNumrule != mpChosenOutlineNumRule)
               )
            {
                /*
                If our spot is already taken by something we can't replace
                then don't insert and remove our outline level.
                */
                rSI.pFmt->SetAttr(
                        SwNumRuleItem( rSI.pOutlineNumrule->GetName() ) );
                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel(NO_NUMBERING);
            }
            else
            {
                /*
                If there is a style already set for this outline
                numbering level and its not a style set by us already
                then we can remove it outline numbering.
                (its one of the default headings in a new document
                so we can clobber it)
                Of course if we are being inserted into a document that
                already has some set we can't do this, thats covered by
                the list of level in nFlagsStyleOutlLevel to ignore.
                */
                outlineeq aCmp(rSI.nOutlineLevel);
                myiter aResult = std::find_if(aOutLined.begin(),
                    aOutLined.end(), aCmp);

                myiter aEnd = aOutLined.end();
                while (aResult != aEnd  && aCmp(*aResult))
                {
                    (*aResult)->SetOutlineLevel(NO_NUMBERING);
                    ++aResult;
                }

                /*
                #i1886#
                I believe that when a list is registered onto a winword
                style which is an outline numbering style (i.e.
                nOutlineLevel is set) that the style of numbering is for
                the level is indexed by the *list* level that was
                registered on that style, and not the outlinenumbering
                level, which is probably a logical sequencing, and not a
                physical mapping into the list style reged on that outline
                style.
                */
                BYTE nFromLevel = rSI.nListLevel;
                BYTE nToLevel = rSI.nOutlineLevel;
                const SwNumFmt& rRule=rSI.pOutlineNumrule->Get(nFromLevel);
                aOutlineRule.Set(nToLevel, rRule);
                // Set my outline level
                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel(nToLevel);
                // If there are more styles on this level ignore them
                nFlagsStyleOutlLevel |= nAktFlags;
            }
        }
    }
    if (nOldFlags != nFlagsStyleOutlLevel)
        rDoc.SetOutlineNumRule(aOutlineRule);
}

const String* SwWW8ImplReader::GetAnnotationAuthor(sal_uInt16 nIdx)
{
    if (!mpAtnNames && pWwFib->lcbGrpStAtnOwners)
    {
        // Authoren bestimmen: steht im TableStream
        mpAtnNames = new ::std::vector<String>;
        SvStream& rStrm = *pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( pWwFib->fcGrpStAtnOwners );

        long nRead = 0, nCount = pWwFib->lcbGrpStAtnOwners;
        while (nRead < nCount)
        {
            if( bVer67 )
            {
                mpAtnNames->push_back(WW8ReadPString(rStrm, false));
                nRead += mpAtnNames->rbegin()->Len() + 1;   // Laenge + BYTE Count
            }
            else
            {
                mpAtnNames->push_back(WW8Read_xstz(rStrm, 0, false));
                // UNICode: doppelte Laenge + USHORT Count
                nRead += mpAtnNames->rbegin()->Len() * 2 + 2;
            }
        }
        rStrm.Seek( nOldPos );
    }

    const String *pRet = 0;
    if (mpAtnNames && nIdx < mpAtnNames->size())
        pRet = &((*mpAtnNames)[nIdx]);
    return pRet;
}

ULONG SwWW8ImplReader::LoadDoc( SwPaM& rPaM,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    {
        static const sal_Char* aNames[ 12 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2"
        };
        sal_uInt32 aVal[ 12 ];
        SwFilterOptions aOpt( 12, aNames, aVal );

        nIniFlags = aVal[ 0 ];
        nIniFlags1= aVal[ 1 ];
        // schiebt Flys um x twips nach rechts o. links
        nIniFlyDx = aVal[ 3 ];
        nIniFlyDy = aVal[ 4 ];

        nFieldFlags = aVal[ 5 ];
        nFieldTagAlways[0] = aVal[ 6 ];
        nFieldTagAlways[1] = aVal[ 7 ];
        nFieldTagAlways[2] = aVal[ 8 ];
        nFieldTagBad[0] = aVal[ 9 ];
        nFieldTagBad[1] = aVal[ 10 ];
        nFieldTagBad[2] = aVal[ 11 ];
    }

    UINT16 nMagic;
    *pStrm >> nMagic;

    // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
    switch (nWantedVersion)
    {
        case 6:
        case 7:
            if ((0xa5dc != nMagic) && ((nMagic < 0xa697 ) || (nMagic > 0xa699)))
            {
                //JP 06.05.99: teste auf eigenen 97-Fake!
                if (pStg && 0xa5ec == nMagic)
                {
                    ULONG nCurPos = pStrm->Tell();
                    if (pStrm->Seek(nCurPos + 22))
                    {
                        UINT32 nfcMin;
                        *pStrm >> nfcMin;
                        if (0x300 != nfcMin)
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    pStrm->Seek( nCurPos );
                }
                else
                    nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
            }
            break;
        case 8:
            if (0xa5ec != nMagic)
                nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            break;
        default:
            nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            break;
    }

    if (!nErrRet)
        nErrRet = LoadThroughDecryption(rPaM ,pGloss);

    return nErrRet;
}

ULONG WW8Reader::Read(SwDoc &rDoc, SwPaM &rPam, const String & /* FileName */)
{
    USHORT nOldBuffSize = 32768;
    bool bNew = !bInsertMode;               // Neues Doc ( kein Einfuegen )


    SvStorageStreamRef refStrm;         // damit uns keiner den Stream klaut
    SvStream* pIn = pStrm;

    ULONG nRet = 0;
    BYTE nVersion = 8;

    String sFltName = GetFltName();
    if( sFltName.EqualsAscii( "WW6" ) )
    {
        if (pStrm)
            nVersion = 6;
        else
        {
            ASSERT(!this, "WinWord 95 Reader-Read ohne Stream");
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if( sFltName.EqualsAscii( "CWW6" ) )
            nVersion = 6;
        else if( sFltName.EqualsAscii( "CWW7" ) )
            nVersion = 7;

        if( pStg )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = &refStrm;
        }
        else
        {
            ASSERT(!this, "WinWord 95/97 Reader-Read ohne Storage");
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
        //              Kapitelnummer. Darum hier explizit abschalten
        //              weil das Default jetzt wieder auf AN ist.
        if (bNew)
        {
            Reader::SetNoOutlineNum( rDoc );
            // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
            Reader::ResetFrmFmts( rDoc );
        }
        SwWW8ImplReader* pRdr = new SwWW8ImplReader(nVersion, pStg, pIn, rDoc,
            bNew);
        nRet = pRdr->LoadDoc( rPam );
        delete pRdr;

        if( refStrm.Is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.Clear();
        }
    }
    return nRet;
}

int WW8Reader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

BOOL WW8Reader::HasGlossaries() const
{
    return true;
}

BOOL WW8Reader::ReadGlossaries(SwTextBlocks& rBlocks, BOOL bSaveRelFiles) const
{
    bool bRet=false;

    WW8Reader *pThis = const_cast<WW8Reader *>(this);

    USHORT nOldBuffSize = 32768;
    SvStorageStreamRef refStrm;
    if (!pThis->OpenMainStream(refStrm, nOldBuffSize))
    {
        WW8Glossary aGloss( refStrm, 8, pStg );
        bRet = aGloss.Load( rBlocks, bSaveRelFiles ? true : false);
    }
    return bRet ? true : false;
}

BOOL SwMSDffManager::GetOLEStorageName(long nOLEId, String& rStorageName,
    SvStorageRef& rSrcStorage, SvStorageRef& rDestStorage) const
{
    bool bRet = false;

    long nPictureId = 0;
    if (rReader.pStg)
    {
        // dann holen wir uns mal ueber den TextBox-PLCF die richtigen
        // Char Start-/End-Positionen. In dem Bereich sollte dann
        // das EinbettenFeld und die entsprechenden Sprms zu finden
        // sein. Wir brauchen hier aber nur das Sprm fuer die Picture Id
        long nOldPos = rReader.pStrm->Tell();
        {
            long nStartCp, nEndCp;
            rReader.GetTxbxTextSttEndCp(nStartCp, nEndCp,
                static_cast<sal_uInt16>((nOLEId >> 16) & 0xFFFF),
                static_cast<sal_uInt16>(nOLEId & 0xFFFF));

            WW8PLCFxSaveAll aSave;
            memset( &aSave, 0, sizeof( aSave ) );
            rReader.pPlcxMan->SaveAllPLCFx( aSave );

            nStartCp += rReader.nDrawCpO;
            nEndCp   += rReader.nDrawCpO;
            WW8PLCFx_Cp_FKP* pChp = rReader.pPlcxMan->GetChpPLCF();
            wwSprmParser aSprmParser(rReader.pWwFib->nVersion);
            while (nStartCp <= nEndCp && !nPictureId)
            {
                WW8PLCFxDesc aDesc;
                pChp->SeekPos( nStartCp );
                pChp->GetSprms( &aDesc );

                if (aDesc.nSprmsLen && aDesc.pMemPos)   // Attribut(e) vorhanden
                {
                    long nLen = aDesc.nSprmsLen;
                    const BYTE* pSprm = aDesc.pMemPos;

                    while (nLen >= 2 && !nPictureId)
                    {
                        USHORT nId = aSprmParser.GetSprmId(pSprm);
                        USHORT nSL = aSprmParser.GetSprmSize(nId, pSprm);

                        if( nLen < nSL )
                            break;              // nicht mehr genug Bytes uebrig

                        if( 0x6A03 == nId && 0 < nLen )
                        {
                            nPictureId = SVBT32ToLong(pSprm +
                                aSprmParser.DistanceToData(nId));
                            bRet = true;
                        }
                        pSprm += nSL;
                        nLen -= nSL;
                    }
                }
                nStartCp = aDesc.nEndPos;
            }

            rReader.pPlcxMan->RestoreAllPLCFx( aSave );
        }
        rReader.pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        rStorageName = '_';
        rStorageName += String::CreateFromInt32(nPictureId);
        rSrcStorage = rReader.pStg->OpenStorage(CREATE_CONST_ASC(
            SL::aObjectPool));
        if (!rReader.mpDocShell)
            bRet=false;
        else
            rDestStorage = rReader.mpDocShell->GetStorage();
    }
    return bRet;
}

BOOL SwMSDffManager::ShapeHasText(ULONG, ULONG) const
{
    // Zur Zeit des Einlesens einer einzelnen Box, die womoeglich Teil einer
    // Gruppe ist, liegen noch nicht genuegend Informationen vor, um
    // entscheiden zu koennen, ob wir sie nicht doch als Textfeld benoetigen.
    // Also vorsichtshalber mal alle umwandeln:
    return true;
}

bool SwWW8ImplReader::InEqualOrHigherApo(int nLvl) const
{
    if (nLvl)
        --nLvl;
    mycApoIter aIter = std::find(maApos.begin() + nLvl, maApos.end(), true);
    if (aIter != maApos.end())
        return true;
    else
        return false;
}

bool SwWW8ImplReader::InEqualApo(int nLvl) const
{
    //If we are in a table, see if an apo was inserted at the level below
    //the table.
    if (nLvl)
        --nLvl;
    return maApos[nLvl];
}

namespace sw
{
    namespace hack
    {
        Position::Position(const SwPosition &rPos)
            : maPtNode(rPos.nNode), mnPtCntnt(rPos.nContent.GetIndex())
        {
        }

        Position::Position(const Position &rPos)
            : maPtNode(rPos.maPtNode), mnPtCntnt(rPos.mnPtCntnt)
        {
        }

        Position::operator SwPosition() const
        {
            SwPosition aRet(maPtNode);
            aRet.nContent.Assign(maPtNode.GetNode().GetCntntNode(), mnPtCntnt);
            return aRet;
        }
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
