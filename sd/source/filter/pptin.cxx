/*************************************************************************
 *
 *  $RCSfile: pptin.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-12 10:16:23 $
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

#include <svx/numitem.hxx>

#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDFPPT_HXX
#include <svx/svdfppt.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDLAYER_HXX //autogen
#include <svx/svdlayer.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_COLOR                    EE_CHAR_COLOR
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SVX__XGRADIENT_HXX
#include <svx/xgrad.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX
#include <svx/xflgrit.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

#ifndef _SVX_BULITEM_HXX
#include <svx/bulitem.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif

#ifndef _SVX_LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif

#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif

#pragma hdrstop

#include "glob.hrc"
#include "pptin.hxx"
#include "sdoutl.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "sdresid.hxx"

#ifndef _SD_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef _SD_ANMINFO_HXX
#include "anminfo.hxx"
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx
#endif

#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_SVDOPAGE
#include <svx/svdopage.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _PROPREAD_HXX_
#include <propread.hxx>
#endif
#ifndef _SD_CUSSHOW_HXX
#include <cusshow.hxx>
#endif

#ifndef _SFXSTBMGR_HXX
#include <sfx2/stbmgr.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef MAC
#include "../ui/inc/docshell.hxx"
#else
#include "docshell.hxx"
#endif
#ifndef _SD_FRMVIEW_HXX
#include <../ui/inc/frmview.hxx>
#endif

#include <offmgr/app.hxx>
#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif
#include <svtools/pathoptions.hxx>
#include <sfx2/docfac.hxx>
#define MAX_USER_MOVE       2

//////////////////////////////////////////////////////////////////////////
//
// Ctor
//
//////////////////////////////////////////////////////////////////////////

SdPPTImport::SdPPTImport( SdDrawDocument* pDocument, SvStream& rDocStream, SvStorage& rStorage_, SfxMedium& rMedium ) :
    SdrPowerPointImport     ( rDocStream ),
    rMed                    ( rMedium ),
    rStorage                ( rStorage_ ),
    nFilterOptions          ( 0 )
{
    pDoc = pDocument;
    if ( bOk )
    {
        bDocumentFound = SeekToDocument( &aDocHd );                             // aDocHd = the latest DocumentHeader
        while ( SeekToRec( rStCtrl, PPT_PST_Document, nStreamLen, &aDocHd ) )
            bDocumentFound = TRUE;

        UINT32 nDggContainerOfs = 0;

        if ( bDocumentFound )
        {
            ULONG nPosMerk = rStCtrl.Tell();

            pStData = rStorage_.OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Pictures" ) ), STREAM_STD_READ );

            rStCtrl.Seek( aDocHd.GetRecBegFilePos() + 8 );
            ULONG nDocLen = aDocHd.GetRecEndFilePos();
            DffRecordHeader aPPDGHd;
            if ( SeekToRec( rStCtrl, PPT_PST_PPDrawingGroup, nDocLen, &aPPDGHd ) )
            {
                ULONG nPPDGLen = aPPDGHd.GetRecEndFilePos();
                if ( SeekToRec( rStCtrl, DFF_msofbtDggContainer, nPPDGLen, NULL ) )
                    nDggContainerOfs = rStCtrl.Tell();
            }
            rStCtrl.Seek( nPosMerk );
        }
        sal_uInt32 nSvxMSDffOLEConvFlags = 0;
        OfficeApplication* pApplication = OFF_APP();
        if ( pApplication )
        {
            OfaFilterOptions* pBasOpt = pApplication->GetFilterOptions();
            if ( pBasOpt )
            {
                if ( pBasOpt->IsLoadPPointBasicCode() )
                    nFilterOptions |= 1;
                if ( pBasOpt->IsMathType2Math() )
                    nSvxMSDffOLEConvFlags |= OLE_MATHTYPE_2_STARMATH;
                if ( pBasOpt->IsWinWord2Writer() )
                    nSvxMSDffOLEConvFlags |= OLE_WINWORD_2_STARWRITER;
                if ( pBasOpt->IsExcel2Calc() )
                    nSvxMSDffOLEConvFlags |= OLE_EXCEL_2_STARCALC;
            }
        }
        InitSvxMSDffManager( nDggContainerOfs, pStData, nSvxMSDffOLEConvFlags );
        SetSvxMSDffSettings( SVXMSDFF_SETTINGS_CROP_BITMAPS | 2 );              // SVXMSDFF_SETTINGS_IMPORT_PPT
        SetModel( pDoc, 576 );
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Dtor
//
//////////////////////////////////////////////////////////////////////////

SdPPTImport::~SdPPTImport()
{
    for ( void* pPtr = aSlideNameList.First(); pPtr; pPtr = aSlideNameList.Next() )
        delete (String*)pPtr;
    delete pStData;
}

//////////////////////////////////////////////////////////////////////////
//
// Import
//
//////////////////////////////////////////////////////////////////////////

BOOL SdPPTImport::Import()
{
    if ( !bOk )
        return FALSE;

    SetStarDraw();
    SdrOutliner& rOutl = pDoc->GetDrawOutliner();
    sal_uInt32 nControlWord = rOutl.GetEditEngine().GetControlWord();
    nControlWord |=  EE_CNTRL_ULSPACESUMMATION;
    nControlWord &=~ EE_CNTRL_ULSPACEFIRSTPARA;
    ((EditEngine&)rOutl.GetEditEngine()).SetControlWord( nControlWord );

    SdrLayerAdmin& rAdmin = pDoc->GetLayerAdmin();
    nBackgroundLayerID = rAdmin.GetLayerID( String( SdResId( STR_LAYER_BCKGRND )), FALSE );
    nBackgroundObjectsLayerID = rAdmin.GetLayerID( String( SdResId( STR_LAYER_BCKGRNDOBJ )), FALSE );

    SdDrawDocShell* pDocShell = pDoc->GetDocSh();
    if ( pDocShell )
        SeekOle( pDocShell, nFilterOptions );

    // hyperlinks
    PropRead* pDInfoSec2 = new PropRead( rStorage, String( RTL_CONSTASCII_USTRINGPARAM( "\005DocumentSummaryInformation" ) ) );
    if ( pDInfoSec2->IsValid() )
    {
        PropItem aPropItem;

        UINT32 nType, nPropSize, nPropCount;

        pDInfoSec2->Read();

        BYTE aPropSetGUID[ 16 ] =
        {
            0x02, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
        };
        Section* pSection = (Section*)pDInfoSec2->GetSection( aPropSetGUID );
        if ( pSection )
        {
            if ( pSection->GetProperty( PID_SLIDECOUNT, aPropItem ) )
            {
                aPropItem >> nType;
                if ( ( nType == VT_I4 ) || ( nType == VT_UI4 ) )
                {
                    // examine PID_HEADINGPAIR to get the correct entry for PID_DOCPARTS
                    UINT32 nSlideCount, nVecCount;
                    aPropItem >> nSlideCount;
                    if ( nSlideCount && pSection->GetProperty( PID_HEADINGPAIR, aPropItem ) )
                    {
                        UINT32  nSlideTitleIndex, nSlideTitleCount = 0;
                        UINT32  nFontIndex, nFontCount = 0;
                        UINT32  nDesignTemplateIndex, nDesignTemplateCount = 0;
                        UINT32  i, nTemp, nEntryCount = 0;

                        String aUString;

                        aPropItem >> nType
                                  >> nVecCount;

                        if ( ( nType == ( VT_VARIANT | VT_VECTOR ) ) && ( nVecCount ^ 1 ) )
                        {
                            nVecCount >>= 1;

                            for ( i = 0; i < nVecCount; i++ )
                            {
                                if ( !aPropItem.Read( aUString, VT_EMPTY, FALSE ) )
                                    break;
                                aPropItem >> nType;
                                if ( ( nType != VT_I4 ) && ( nType != VT_UI4 ) )
                                    break;
                                aPropItem >> nTemp;
                                if ( aUString.EqualsAscii("Slide Titles") || aUString.EqualsAscii("Folientitel") )
                                {
                                    nSlideTitleCount = nTemp;
                                    nSlideTitleIndex = nEntryCount;
                                }
                                else if ( aUString.EqualsAscii("Fonts Used") )
                                {
                                    nFontCount = nTemp;
                                    nFontIndex = nEntryCount;
                                }
                                else if ( aUString.EqualsAscii("Design Template") )
                                {
                                    nDesignTemplateCount = nTemp;
                                    nDesignTemplateIndex = nEntryCount;
                                }
                                nEntryCount += nTemp;
                            }
                        }
                        if ( ( nSlideCount == nSlideTitleCount ) && pSection->GetProperty( PID_DOCPARTS, aPropItem ) )
                        {
                            aPropItem >> nType
                                      >> nVecCount;

                            if ( ( nVecCount >= ( nSlideTitleIndex + nSlideTitleCount ) )
                                    && ( nType == ( VT_LPSTR | VT_VECTOR ) ) )
                            {
                                for ( i = 0; i != nSlideTitleIndex; i++ )
                                {
                                    aPropItem >> nTemp;
                                    aPropItem.SeekRel( nTemp );
                                }
                                for ( i = 0; i < nSlideTitleCount; i++ )
                                {
                                    if ( !aPropItem.Read( aUString, nType, FALSE ) )
                                        break;
                                    String* pString = new String( aUString );
                                    if ( pString->EqualsAscii( "No Slide Title" ))
                                        *pString = String();
                                    else
                                    {
                                        void* pPtr;
                                        for ( pPtr = aSlideNameList.First(); pPtr; pPtr = aSlideNameList.Next() )
                                        {
                                            if ( *((String*)pPtr ) == *pString )
                                            {
                                                *pString = String();
                                                break;
                                            }
                                        }
                                    }
                                    aSlideNameList.Insert( pString, LIST_APPEND );
                                }
                            }
                        }
                    }
                }
            }

            BYTE aUserPropSetGUID[ 16 ] =
            {
                0x05, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
            };
            pSection = (Section*)pDInfoSec2->GetSection( aUserPropSetGUID );
            if ( pSection )
            {
                Dictionary aDict;
                if ( pSection->GetDictionary( aDict ) )
                {
                    UINT32 nPropId = aDict.GetProperty( rtl::OUString::createFromAscii("_PID_HLINKS" ));
                    if ( nPropId )
                    {
                        if ( pSection->GetProperty( nPropId, aPropItem ) )
                        {
                            aPropItem.Seek( STREAM_SEEK_TO_BEGIN );
                            aPropItem >> nType;
                            if ( nType == VT_BLOB )
                            {
                                aPropItem >> nPropSize
                                          >> nPropCount;

                                if ( ! ( nPropCount % 6 ) )
                                {
                                    nPropCount /= 6;    // 6 propertys a hyperlink

                                    SdHyperlinkEntry* pHyperlink;
                                    for ( UINT32 i = 0; i < nPropCount; i++ )
                                    {
                                        pHyperlink = new SdHyperlinkEntry;
                                        pHyperlink->nIndex = 0;
                                        aPropItem >> nType;
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem >> pHyperlink->nPrivate1
                                                  >> nType;
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem >> pHyperlink->nPrivate2
                                                  >> nType;
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem >> pHyperlink->nPrivate3
                                                  >> nType;
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem >> pHyperlink->nInfo;
                                        if ( !aPropItem.Read( pHyperlink->aTarget, VT_EMPTY ) )
                                            break;
                                        if ( !aPropItem.Read( pHyperlink->aSubAdress, VT_EMPTY ) )
                                            break;
                                        pHyperlink->nStartPos = pHyperlink->nEndPos = -1;
                                        aHyperList.Insert( pHyperlink, LIST_APPEND );
                                    }
                                    if ( i != nPropCount )
                                        delete pHyperlink;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    delete pDInfoSec2;

    if ( bDocumentFound )
    {
        rStCtrl.Seek( aDocHd.GetRecBegFilePos() + 8 );
        // HyperList lesen / Indexe der einzelnen Eintraege setzen
        DffRecordHeader aHyperHd;
        if ( SeekToRec( rStCtrl, PPT_PST_ExObjList, aDocHd.GetRecEndFilePos(), &aHyperHd ) )
        {
            UINT32 nExObjHyperListLen = aHyperHd.GetRecEndFilePos();
            for ( void* pPtr = aHyperList.First(); pPtr; pPtr = aHyperList.Next() )
            {
                DffRecordHeader aHyperE;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlink, nExObjHyperListLen, &aHyperE ) )
                    break;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlinkAtom, nExObjHyperListLen, NULL, 0 ) )
                    break;
                rStCtrl.SeekRel( 8 );
                rStCtrl >> ((SdHyperlinkEntry*)pPtr)->nIndex;
                aHyperE.SeekToEndOfRecord( rStCtrl );
            }
        }
    }
    ///////////////////////////////////////////////////////////
    // create master pages:
    ///////////////////////////////////////////////////////////
    SfxProgress* pStbMgr = new SfxProgress( pDocShell, String( SdResId( STR_POWERPOINT_IMPORT ) ),
            pMasterPages->Count() + pSlidePages->Count() + pNotePages->Count() );

    UINT32 nImportedPages = 0;
    {
        PptPageKind     ePageKind = eAktPageKind;
        UINT16          nPageNum = nAktPageNum;
        UINT16          nMasterAnz = GetPageCount( PPT_MASTERPAGE );

        for ( USHORT nMasterNum = 0; nMasterNum < nMasterAnz; nMasterNum++ )
        {
            SetPageNum( nMasterNum, PPT_MASTERPAGE );
            SdPage* pPage = (SdPage*)MakeBlancPage( TRUE );
            if ( pPage )
            {
                BOOL bNotesMaster = (*GetPageList( eAktPageKind ) )[ nAktPageNum ]->bNotesMaster;
                BOOL bStarDrawFiller = (*GetPageList( eAktPageKind ) )[ nAktPageNum ]->bStarDrawFiller;

                PageKind ePgKind = ( bNotesMaster ) ? PK_NOTES : PK_STANDARD;
                pPage->SetPageKind( ePgKind );
                pSdrModel->InsertMasterPage( (SdrPage*)pPage );
                if ( bNotesMaster && bStarDrawFiller )
                    ((SdPage*)pPage)->SetAutoLayout( AUTOLAYOUT_NOTES, TRUE );
                if ( nMasterNum )
                {
                    SfxStyleSheet* pSheet;
                    if ( nMasterNum == 1 )
                    {
                        ///////////////////
                        // standardsheet //
                        ///////////////////
                        pSheet = (SfxStyleSheet*)pDoc->GetStyleSheetPool()->Find( String(SdResId( STR_STANDARD_STYLESHEET_NAME )), SFX_STYLE_FAMILY_PARA );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                    }

                    ///////////////////////////////////////////////////////////
                    // create layoutstylesheets, set layoutname and stylesheet
                    // (nur auf Standard- und Notizseiten)
                    ///////////////////////////////////////////////////////////
                    const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
                    String aLayoutName( SdResId( STR_LAYOUT_DEFAULT_NAME ) );
                    if ( nMasterNum > 2 )
                    {
                        if ( ePgKind == PK_STANDARD )
                        {
                            // Standardseite: Neues Praesentationslayout erzeugen
                            if ( pSlideLayout && pSlideLayout->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE )
                                aLayoutName = String( SdResId( STR_LAYOUT_DEFAULT_TITLE_NAME ) );
                            else
                                aLayoutName += String::CreateFromInt32( (sal_Int32)( ( nMasterNum + 1 ) / 2 - 1 ) );
                            ( (SdStyleSheetPool*)pDoc->GetStyleSheetPool() )->CreateLayoutStyleSheets( aLayoutName );
                        }
                        else    // Notizseite: Praesentationslayout von der Standardseite verwenden
                            aLayoutName = ( (SdPage*)pDoc->GetMasterPage( nMasterNum - 1 ) )->GetName();
                    }
                    pPage->SetName( aLayoutName );
                    aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
                    aLayoutName += String( SdResId( STR_LAYOUT_OUTLINE ) );
                    pPage->SetLayoutName( aLayoutName );

                    /////////////////////
                    // set stylesheets //
                    /////////////////////
                    if ( pPage->GetPageKind() == PK_STANDARD )
                    {
                        UINT32 nTitleInstance = TSS_TYPE_PAGETITLE;
                        UINT32 nOutlinerInstance = TSS_TYPE_BODY;
//                      BOOL bSwapStyleSheet = pSlideLayout->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE;
//                      if ( bSwapStyleSheet )
//                      {
//                          nTitleInstance = TSS_TYPE_TITLE;
//                          nOutlinerInstance = TSS_TYPE_SUBTITLE;
//                      }
                        /////////////////////
                        // titelstylesheet //
                        /////////////////////
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, nTitleInstance, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, nTitleInstance, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                        ////////////////////////
                        // outlinerstylesheet //
                        ////////////////////////
                        UINT16 nLevel;
                        PPTParagraphObj* pParagraphs[ 9 ];
                        PPTParagraphObj* pPreviousPara = NULL;

                        for ( nLevel = 0; nLevel < 9; nLevel++ )
                        {
                            String aName( pPage->GetLayoutName() );
                            aName.Append( (sal_Unicode)( ' ' ) );
                            aName.Append( String::CreateFromInt32( nLevel + 1 ) );
                            SfxStyleSheet* pSheet = (SfxStyleSheet*)pDoc->GetStyleSheetPool()->Find( aName, SD_LT_FAMILY );
                            DBG_ASSERT( pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden" );
                            if ( pSheet )
                            {
                                pParagraphs[ nLevel ] = new PPTParagraphObj( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                SfxItemSet& rItemSet = pSheet->GetItemSet();
                                PPTPortionObj aPortion( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                pParagraphs[ nLevel ]->AppendPortion( aPortion );
                                pParagraphs[ nLevel ]->ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff, pPreviousPara );
                                aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                                pPreviousPara = pParagraphs[ nLevel ];
                            }
                            else
                                pParagraphs[ nLevel ] = NULL;
                        }
                        for ( nLevel = 0; nLevel < 9; delete pParagraphs[ nLevel++ ] );
                        /////////////////////////
                        // subtitle stylesheet //
                        /////////////////////////
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_TYPE_SUBTITLE, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_TYPE_SUBTITLE, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                    }
                    else if ( ePgKind == PK_NOTES )
                    {
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_NOTES );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_TYPE_NOTES, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_TYPE_NOTES, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                    }
                }
            }
        }
    }
    SdPage* pMPage;
    sal_uInt16 i;
    for ( i = 1; i < pDoc->GetMasterPageCount() && ( pMPage = (SdPage*)pDoc->GetMasterPage( i ) ); i++ )
    {
        SetPageNum( i, PPT_MASTERPAGE );
        const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
        /////////////////////////////////////////////
        // importing master page objects           //
        /////////////////////////////////////////////
        PptSlidePersistList* pList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pPersist = ( pList && ( nAktPageNum < pList->Count() ) )
                                                    ? (*pList)[ nAktPageNum ] : NULL;

        if ( pPersist && ( pPersist->bStarDrawFiller == FALSE ) )
        {
            SdrObject* pObj = ImportPageBackgroundObject( *pMPage, pPersist->nBackgroundOffset, TRUE ); // import background
            if ( pObj )
                pMPage->NbcInsertObject( pObj );

            ProcessData aProcessData( *(*pList)[ nAktPageNum ], (SdPage*)pMPage );

            // und nun die Page nach Objekten abklappern
            ULONG nObjAnz = GetObjCount();
            for ( ULONG nObjNum = 0; nObjNum < nObjAnz; nObjNum++ )
            {
                SdrObject* pObj = NULL;
                ULONG nFPosMerk = rStCtrl.Tell();
                if ( SeekToObj( nObjNum ) )
                    pObj = ImportObj( rStCtrl, (void*)&aProcessData, NULL );
                rStCtrl.Seek( nFPosMerk );
                if ( pObj )
                {
                    pObj->SetLayer( nBackgroundObjectsLayerID );
                    pMPage->NbcInsertObject( pObj );
                }
            }
            ImportPageEffect( (SdPage*)pMPage );
            if( pStbMgr )
                pStbMgr->SetState( nImportedPages++ );
        }
        ///////////////////////
        // background object //
        ///////////////////////
        SdrObject* pObj = pMPage->GetObj( 0 );
        if ( pObj && pObj->GetObjIdentifier() == OBJ_RECT )
        {
            if ( pMPage->GetPageKind() == PK_STANDARD )
            {
                // Hintergrundobjekt gefunden (erstes Objekt der MasterPage)
                pObj->SetEmptyPresObj( TRUE );
                pObj->SetUserCall( pMPage );
                pObj->SetLayer( nBackgroundLayerID );

                // Schatten am ersten Objekt (Hintergrundobjekt) entfernen (#57918#)
                SfxItemSet aTempAttr( pDoc->GetPool() );
                aTempAttr.Put( pObj->GetItemSet() );

                BOOL bShadowIsOn = ( (SdrShadowItem&)( aTempAttr.Get( SDRATTR_SHADOW ) ) ).GetValue();
                if( bShadowIsOn )
                {
                    aTempAttr.Put( SdrShadowItem( FALSE ) );
                    pObj->SetItemSet( aTempAttr );
                }
                SfxStyleSheet* pSheet = pMPage->GetStyleSheetForPresObj( PRESOBJ_BACKGROUND );
                if ( pSheet )
                {   // StyleSheet fuellen und dem Objekt zuweisen
                    pSheet->GetItemSet().ClearItem();
                    pSheet->GetItemSet().Put( pObj->GetItemSet() );
                    pObj->SetStyleSheet( pSheet, TRUE );
                    SfxItemSet aSet( pDoc->GetPool() );
                    pObj->SetItemSet( aSet );
                }
                pMPage->GetPresObjList()->Insert( pObj, LIST_APPEND );
            }
        }
    }
    ////////////////////////////////////
    // importing slide pages          //
    ////////////////////////////////////
    {
        UINT32          nFPosMerk = rStCtrl.Tell();
        PptPageKind     ePageKind = eAktPageKind;
        UINT16          nPageNum = nAktPageNum;

        eAktPageKind = PPT_NOTEPAGE;                // fuer das richtige Seitenformat
        SdrPage* pPage = MakeBlancPage( FALSE );
        pSdrModel->InsertPage( pPage );

        USHORT nPageAnz = GetPageCount( PPT_SLIDEPAGE );
        if ( nPageAnz )
        {
            for ( USHORT nPageNum = 0; nPageNum < nPageAnz; nPageNum++ )
            {
                ePresChange = PRESCHANGE_SEMIAUTO;
                SetPageNum( nPageNum, PPT_SLIDEPAGE );
                SdPage* pPage = (SdPage*)ImportPage();
                if ( pPage )
                {
                    pPage->SetPageKind( PK_STANDARD );
                    ImportPageEffect( (SdPage*)pPage );
                    pSdrModel->InsertPage( pPage );
                    if ( HasNotesPage( nAktPageNum, eAktPageKind ) )
                    {
                        USHORT nNotesPageNum = GetNotesPageIndex( nAktPageNum, eAktPageKind );
                        SetPageNum( nNotesPageNum, PPT_NOTEPAGE );
                        SdPage* pPage = (SdPage*)ImportPage();
                        nImportedPages++;
                        if ( pPage )
                        {
                            pPage->SetPageKind( PK_NOTES );
                            USHORT nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                            pPage->InsertMasterPage( nMasterNum );
                            pPage->SetAutoLayout( AUTOLAYOUT_NOTES, FALSE );
                            pSdrModel->InsertPage( pPage );
                        }
                    }
                    else
                    {
                        eAktPageKind = PPT_NOTEPAGE; // fuer das richtige Seitenformat
                        SdPage* pPage = (SdPage*)MakeBlancPage( FALSE );
                        pPage->SetPageKind( PK_NOTES );
                        USHORT nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        pPage->InsertMasterPage( nMasterNum );
                        pPage->SetAutoLayout( AUTOLAYOUT_NOTES, TRUE );
                        pSdrModel->InsertPage( pPage );
                        SdrObject* pPageObj = pPage->GetPresObj( PRESOBJ_PAGE, 1 );
                        if ( pPageObj )
                            ((SdrPageObj*)pPageObj)->SetPageNum( ( nPageNum << 1 ) + 1 );
                    }
                }
                if( pStbMgr )
                    pStbMgr->SetState( nImportedPages++ );
            }
        }
        else
        {
            // Das kann bei Dokumentvorlagen vorkommen
            eAktPageKind = PPT_SLIDEPAGE;
            SdrPage* pPage = MakeBlancPage( FALSE );
            pSdrModel->InsertPage( pPage );

            eAktPageKind = PPT_NOTEPAGE;
            SdrPage* pNPage = MakeBlancPage( FALSE );
            pSdrModel->InsertPage( pNPage );
        }
        SetPageNum( nPageNum, ePageKind );
        rStCtrl.Seek( nFPosMerk );
    }
    ///////////////////////////////////////////////////////////////////
    // Handzettel und Notiz-Seiten erzeugen                          //
    ///////////////////////////////////////////////////////////////////
    bOk = pDoc->CreateMissingNotesAndHandoutPages();
    if ( bOk )
    {
        for ( i = 0; i < pDoc->GetSdPageCount( PK_STANDARD ); i++ )
        {
            ////////////////////
            // set AutoLayout //
            ////////////////////
            SetPageNum( i, PPT_SLIDEPAGE );
            SdPage* pPage = pDoc->GetSdPage( i, PK_STANDARD );
            pPage->SetLayoutName( ( (SdPage*)pPage->GetMasterPage( 0 ) )->GetLayoutName() );
            AutoLayout eAutoLayout = AUTOLAYOUT_NONE;
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout )
            {
                switch ( pSlideLayout->eLayout )            // Praesentationslayouts fuer Standard-Seiten
                {
                    case PPT_LAYOUT_TITLEANDBODYSLIDE :
                    {
                        eAutoLayout = AUTOLAYOUT_ENUM;
                        USHORT nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        switch ( nID1 )
                        {
                            case PPT_PLACEHOLDER_BODY :
                                eAutoLayout = AUTOLAYOUT_ENUM;
                            break;
                            case PPT_PLACEHOLDER_TABLE :
                                eAutoLayout = AUTOLAYOUT_TAB;
                            break;
                            case PPT_PLACEHOLDER_ORGANISZATIONCHART :
                                eAutoLayout = AUTOLAYOUT_ORG;
                            break;
                            case PPT_PLACEHOLDER_GRAPH :
                                eAutoLayout = AUTOLAYOUT_CHART;
                            break;
                            case PPT_PLACEHOLDER_OBJECT :
                                eAutoLayout = AUTOLAYOUT_OBJ;
                            break;
                            case PPT_PLACEHOLDER_VERTICALTEXTBODY :
                                eAutoLayout = AUTOLAYOUT_TITLE_VERTICAL_OUTLINE;
                            break;
                        }
                    }
                    break;

                    case PPT_LAYOUT_2COLUMNSANDTITLE :
                    {
                        eAutoLayout = AUTOLAYOUT_2TEXT;
                        USHORT nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        USHORT nID2 = pSlideLayout->aPlaceholderId[ 2 ];
                        if ( nID1 == PPT_PLACEHOLDER_BODY && nID2 == PPT_PLACEHOLDER_GRAPH )
                            eAutoLayout = AUTOLAYOUT_TEXTCHART;
                        else if ( nID1 == PPT_PLACEHOLDER_GRAPH && nID2 == PPT_PLACEHOLDER_BODY )
                            eAutoLayout = AUTOLAYOUT_CHARTTEXT;
                        else if ( nID1 == PPT_PLACEHOLDER_BODY && nID2 == PPT_PLACEHOLDER_CLIPART )
                            eAutoLayout = AUTOLAYOUT_TEXTCLIP;
                        else if ( nID1 == PPT_PLACEHOLDER_CLIPART && nID2 == PPT_PLACEHOLDER_BODY )
                            eAutoLayout = AUTOLAYOUT_CLIPTEXT;
                        else if ( nID1 == PPT_PLACEHOLDER_CLIPART && nID2 == PPT_PLACEHOLDER_VERTICALTEXTBODY )
                            eAutoLayout = AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART;
                        else if ( ( nID1 == PPT_PLACEHOLDER_BODY )
                            && ( ( nID2 == PPT_PLACEHOLDER_OBJECT ) || ( nID2 == PPT_PLACEHOLDER_MEDIACLIP ) ) )
                            eAutoLayout = AUTOLAYOUT_TEXTOBJ;
                        else if ( ( nID2 == PPT_PLACEHOLDER_BODY )
                            && ( ( nID1 == PPT_PLACEHOLDER_OBJECT ) || ( nID1 == PPT_PLACEHOLDER_MEDIACLIP ) ) )
                            eAutoLayout = AUTOLAYOUT_OBJTEXT;
                    }
                    break;

                    case PPT_LAYOUT_2ROWSANDTITLE :
                    {
                        eAutoLayout = AUTOLAYOUT_2TEXT;
                        USHORT nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        USHORT nID2 = pSlideLayout->aPlaceholderId[ 2 ];
                        if ( nID1 == PPT_PLACEHOLDER_BODY && nID2 == PPT_PLACEHOLDER_OBJECT )
                            eAutoLayout = AUTOLAYOUT_TEXTOVEROBJ;
                        else if ( nID1 == PPT_PLACEHOLDER_OBJECT && nID2 == PPT_PLACEHOLDER_BODY )
                            eAutoLayout = AUTOLAYOUT_OBJOVERTEXT;
                    }
                    break;

                    case PPT_LAYOUT_TITLESLIDE :
                        eAutoLayout = AUTOLAYOUT_TITLE;
                    break;
                    case PPT_LAYOUT_ONLYTITLE :
                        eAutoLayout = AUTOLAYOUT_ONLY_TITLE;
                    break;
                    case PPT_LAYOUT_RIGHTCOLUMN2ROWS :
                        eAutoLayout = AUTOLAYOUT_TEXT2OBJ;
                    break;
                    case PPT_LAYOUT_LEFTCOLUMN2ROWS :
                        eAutoLayout = AUTOLAYOUT_2OBJTEXT;
                    break;
                    case PPT_LAYOUT_TOPROW2COLUMN :
                        eAutoLayout = AUTOLAYOUT_2OBJOVERTEXT;
                    break;
                    case PPT_LAYOUT_4OBJECTS :
                        eAutoLayout = AUTOLAYOUT_4OBJ;
                    break;
                    case PPT_LAYOUT_BIGOBJECT :
                        eAutoLayout = AUTOLAYOUT_OBJ;
                    break;
                    case PPT_LAYOUT_TITLERIGHTBODYLEFT :
                        eAutoLayout = AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE; // AUTOLAYOUT_ENUM;
                    break;
                    case PPT_LAYOUT_TITLERIGHT2BODIESLEFT :
                        eAutoLayout = AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART; // AUTOLAYOUT_TEXT2OBJ;
                    break;

                    case PPT_LAYOUT_BOTTOMROW2COLUMNS :
                    case PPT_LAYOUT_BLANCSLIDE :
                    case PPT_LAYOUT_MASTERSLIDE :           // Layout der Standard- und Titel-MasterPage
                    case PPT_LAYOUT_TITLEMASTERSLIDE :
                    case PPT_LAYOUT_MASTERNOTES :           // Layout der Notizen-MasterPage
                    case PPT_LAYOUT_NOTESTITLEBODY :        // Praesentationslayout fuer Notiz-Seiten
                    case PPT_LAYOUT_HANDOUTLAYOUT :         // Praesentationslayout fuer Handzettelseiten
                        eAutoLayout = AUTOLAYOUT_NONE;
                    break;
                }
                if ( eAutoLayout != AUTOLAYOUT_NONE )
                    pPage->SetAutoLayout( eAutoLayout, FALSE );
            }
        }
        //////////////////////////////////////////////////////////////
        // Handzettel-MasterPage: Autolayout setzen                 //
        //////////////////////////////////////////////////////////////
        SdPage* pHandoutMPage = pDoc->GetMasterSdPage( 0, PK_HANDOUT );
        pHandoutMPage->SetAutoLayout( AUTOLAYOUT_HANDOUT4, TRUE );
    }

    /////////////////////////////////
    // create missing headerfooter //
    /////////////////////////////////
    HeaderFooterMaster* pM;
    for ( pM = (HeaderFooterMaster*)aHFMasterList.First(); pM; pM = (HeaderFooterMaster*)aHFMasterList.Next() )
    {
        HeaderFooterEntry *pSlave, *pMaster = (HeaderFooterEntry*)pM->aHeaderFooterEntryList.First();
        if ( pMaster )
        {
            HeaderFooterMaster& rM = pMaster->rMaster;
            SdrObject* pObj;
            for ( UINT32 nInstanceCount = pMaster->nInstanceCount; nInstanceCount; )
            {
                UINT32 nInstance = pMaster->pInstanceOrder[ --nInstanceCount ];
                if ( nInstance < 4 )
                {
                    if ( rM.nDirtyInstance & ( 1 << nInstance ) )
                    {
                        while ( ( pSlave = (HeaderFooterEntry*)pM->aHeaderFooterEntryList.Next() ) )
                        {
                            if ( pSlave->IsToDisplay( nInstance ) )
                            {   // this headerfooter is to display
                                pObj = pSlave->pSdrObject[ nInstance ];
                                if ( pObj )
                                    pSlave->pSdrObject[ nInstance ] = NULL;
                                else
                                {   // we have to use the default from the masterpage
                                    pObj = pMaster->pSdrObject[ nInstance ];
                                    if ( pObj )
                                        pObj = pObj->Clone();
                                }
                                if ( pObj )
                                    pSlave->pSdPage->NbcInsertObject( pObj, 0 );
                            }
                        }
                        pM->aHeaderFooterEntryList.Seek( (UINT32)0 );
                    }
                    else
                    {
                        pObj = pMaster->pSdrObject[ nInstance ];
                        if ( pObj && pMaster->IsToDisplay( nInstance ) )
                        {
                            pMaster->pSdPage->NbcInsertObject( pObj, pMaster->pPageIndex[ nInstance ] );
                            pMaster->pSdrObject[ nInstance ] = NULL;
                        }
                    }
                }
            }
        }
    }

    UINT32 nSlideCount = GetPageCount();
    for ( i = 0; ( i < nSlideCount) && ( i < aSlideNameList.Count() ); i++ )
    {
        SdPage* pPage = pDoc->GetSdPage( i, PK_STANDARD );
        String* pName = (String*)aSlideNameList.GetObject( i );
        if ( pPage && pName )
        {
            if ( pName->Len() )
                pPage->SetName( *pName );
            else
                *pName = pPage->GetName();
        }
    }
    if ( bDocumentFound )
    {
        if ( pDocShell )
        {
            FrameView* pFrameView = pDoc->GetFrameView( 0 );
            if ( !pFrameView )
            {
                List* pFrameViewList = pDoc->GetFrameViewList();
                if ( pFrameViewList )
                {
                    pFrameView = new FrameView( pDoc );
                    if ( pFrameView )
                        pFrameViewList->Insert( pFrameView );
                }
            }
            if ( pFrameView )
            {
                sal_uInt16  nSelectedPage = 0;
                PageKind    ePageKind = PK_STANDARD;
                EditMode    eEditMode = EM_PAGE;

                switch ( aUserEditAtom.eLastViewType )
                {
                    case 7 :    // outliner view
                    {
                        SfxItemSet* pSet = rMed.GetItemSet();
                        if ( pSet )
                            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 3 ) );
                    }
                    break;
                    case 8 :    // slide sorter
                    {
                        SfxItemSet* pSet = rMed.GetItemSet();
                        if ( pSet )
                            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 2 ) );
                    }
                    break;
                    case 10 :   // titlemaster
                        nSelectedPage = 1;
                    case 2 :    // master
                    {
                        ePageKind = PK_STANDARD;
                        eEditMode = EM_MASTERPAGE;
                    }
                    break;
                    case 5 :    // notes master
                        eEditMode = EM_MASTERPAGE;
                    case 3 :    // notes
                        ePageKind = PK_NOTES;
                    break;
                    case 4 :    // handout
                        ePageKind = PK_HANDOUT;
                    break;
                    default :
                    case 1 :    // normal
                    break;
                }
                pFrameView->SetPageKind( ePageKind );
                pFrameView->SetSelectedPage( nSelectedPage );
                pFrameView->SetViewShEditMode( eEditMode, ePageKind );
            }
        }
        DffRecordHeader aCustomShowHeader;
        // custom show einlesen und setzen
        rStCtrl.Seek( aDocHd.GetRecBegFilePos() + 8 );
        if ( SeekToRec( rStCtrl, PPT_PST_NamedShows, aDocHd.GetRecEndFilePos(), &aCustomShowHeader ) )
        {
            DffRecordHeader aCuHeader;
            while( SeekToRec( rStCtrl, PPT_PST_NamedShow, aCustomShowHeader.GetRecEndFilePos(), &aCuHeader ) )
            {
                DffRecordHeader aContent;
                if ( SeekToRec( rStCtrl, PPT_PST_CString, aCuHeader.GetRecEndFilePos(), &aContent ) )
                {
                    String aCuShow;
                    aContent.SeekToBegOfRecord( rStCtrl );
                    if ( ReadString( aCuShow ) )
                    {
                        if ( SeekToRec( rStCtrl, PPT_PST_NamedShowSlides, aCuHeader.GetRecEndFilePos(), &aContent ) )
                        {
                            PptSlidePersistList* pPageList = GetPageList( PPT_SLIDEPAGE );
                            UINT32 nSCount = aContent.nRecLen >> 2;
                            if ( pPageList && nSCount )
                            {
                                List* pList = pDoc->GetCustomShowList( TRUE );
                                if ( pList )
                                {
                                    SdCustomShow* pSdCustomShow = new SdCustomShow( pDoc );
                                    if ( pSdCustomShow )
                                    {
                                        pSdCustomShow->SetName( aCuShow );
                                        UINT32 nFound = 0;
                                        for ( UINT32 i = 0; i < nSCount; i++ )
                                        {
                                            UINT32 nPageNumber;
                                            rStCtrl >> nPageNumber;
                                            USHORT nPage = pPageList->FindPage( nPageNumber );
                                            if ( nPage != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                            {
                                                SdPage* pPage = pDoc->GetSdPage( nPage, PK_STANDARD );
                                                if ( pPage )
                                                {
                                                    pSdCustomShow->Insert( pPage, LIST_APPEND );
                                                    nFound++;
                                                }
                                            }
                                        }
                                        if ( nFound )
                                            pList->Insert( pSdCustomShow, LIST_APPEND );
                                        else
                                            delete pSdCustomShow;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // this is defaulted, maybe there is no SSDocInfoAtom
        String      aCustomShow;
        sal_uInt32  nFlags = 0;                 // Bit 0:   Auto advance
        sal_uInt32  nPenColor = 0x1000000;
        sal_Int32   nRestartTime = 0x7fffffff;
        sal_uInt16  nStartSlide = 0;
        sal_Int16   nEndSlide = 0;

        // read the pres. configuration
        rStCtrl.Seek( aDocHd.GetRecBegFilePos() + 8 );
        if ( SeekToRec( rStCtrl, PPT_PST_SSDocInfoAtom, aDocHd.GetRecEndFilePos(), &aCustomShowHeader ) )
        {
            rStCtrl >> nPenColor
                    >> nRestartTime
                    >> nStartSlide
                    >> nEndSlide;

            sal_Unicode nChar;
            for ( UINT32 i = 0; i < 32; i++ )
            {
                rStCtrl >> nChar;
                if ( nChar )
                    aCustomShow.Append( nChar );
                else
                {
                    rStCtrl.SeekRel( ( 31 - i ) << 1 );
                    break;
                }
            }
            rStCtrl >> nFlags;
        }
        // set the current custom show
        if ( aCustomShow.Len() )
        {
            List* pList = pDoc->GetCustomShowList( FALSE );
            if ( pList )
            {
                for ( void* pPtr = pList->First(); pPtr; pPtr = pList->Next() )
                {
                    if ( ((SdCustomShow*)pPtr)->GetName() == aCustomShow )
                        break;
                }
                if ( !pPtr )
                    pList->First();
            }
        }
        pDoc->SetPresManual( ( nFlags & 1 ) == 0 );
        pDoc->SetAnimationAllowed( ( nFlags & 2 ) == 0 );
        pDoc->SetPresAll( ( nFlags & 4 ) == 0 );
        pDoc->SetCustomShow( ( nFlags & 8 ) != 0 );
        pDoc->SetPresEndless( ( nFlags & 0x80 ) != 0 );
        pDoc->SetPresFullScreen( ( nFlags & 0x10 ) == 0 );
//      pDoc->SetPresPause( );
//      pDoc->SetPresShowLogo( );
        if ( nStartSlide && ( nStartSlide <= aSlideNameList.Count() ) )
        {
            String aPresPage( *(String*)aSlideNameList.GetObject( nStartSlide - 1 ) );
            pDoc->SetPresPage( aPresPage );
        }
    }

    delete pStbMgr;
    // DocInfo lesen und an der ObjectShell setzen
    SfxDocumentInfo* pNewDocInfo = new SfxDocumentInfo();
    pNewDocInfo->LoadPropertySet( &rStorage );
    pNewDocInfo->SetTemplateName( String() );
    SfxDocumentInfo& rOldInfo = pDoc->GetObjectShell()->GetDocInfo();
    rOldInfo = *pNewDocInfo;
    delete( pNewDocInfo );
    return bOk;
}

//////////////////////////////////////////////////////////////////////////
//
// Import von Seiten
//
//////////////////////////////////////////////////////////////////////////

void SdPPTImport::ImportPageEffect( SdPage* pPage )
{
    ULONG nFilePosMerk = rStCtrl.Tell();

    // PageKind an der Seite setzen (bisher nur PK_STANDARD oder PK_NOTES)
    if ( pPage->GetPageKind() == PK_STANDARD )
    {
        PptSlidePersistList* pPersistList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pActualSlidePersist = ( pPersistList && ( nAktPageNum < pPersistList->Count() ) )
                                                        ? (*pPersistList)[ nAktPageNum ] : NULL;

        if ( pActualSlidePersist && ( eAktPageKind == PPT_SLIDEPAGE ) )
        {
            if ( ! ( pActualSlidePersist->aSlideAtom.nFlags & 1 ) ) // do not follow master objects ?
            {
                SetOfByte aVisibleLayers = pPage->GetMasterPageVisibleLayers( 0 );
                aVisibleLayers.Set( nBackgroundObjectsLayerID, FALSE );
                pPage->SetMasterPageVisibleLayers( aVisibleLayers, 0 );
            }
        }
        DffRecordHeader aPageRecHd;
        if ( pPage && SeekToAktPage( &aPageRecHd ) )
        {
            ULONG nPageRecEnd = aPageRecHd.GetRecEndFilePos();

            BOOL bTryTwice = ( eAktPageKind == PPT_SLIDEPAGE );
            BOOL bSSSlideInfoAtom = FALSE;
            while ( TRUE )
            {
                while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPageRecEnd ) )
                {
                    DffRecordHeader aHd;
                    rStCtrl >> aHd;
                    switch ( aHd.nRecType )
                    {
                        case PPT_PST_SSSlideInfoAtom:
                        {
                            bSSSlideInfoAtom = TRUE;
                            if ( eAktPageKind == PPT_MASTERPAGE )
                            {
                                if ( pActualSlidePersist )
                                    pActualSlidePersist->aPersistAtom.nReserved = aHd.GetRecBegFilePos();
                            }
                            else
                            {
                                BYTE    nDirection, nTransitionType, nBuildFlags, nByteDummy, nSpeed;
                                INT32   nSlideTime, nSoundRef;
                                rStCtrl >> nSlideTime           // Standzeit (in Ticks)
                                        >> nSoundRef            // Index in SoundCollection
                                        >> nDirection           // Richtung des Ueberblendeffekts
                                        >> nTransitionType      // Ueberblendeffekt
                                        >> nBuildFlags          // Buildflags (s.u.)
                                        >> nByteDummy
                                        >> nSpeed               // Geschwindigkeit (langsam, mittel, schnell)
                                        >> nByteDummy >> nByteDummy >> nByteDummy;

                                switch ( nTransitionType )
                                {
                                    case PPT_TRANSITION_TYPE_BLINDS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_STRIPES );// Vertikal blenden
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_STRIPES );// Horizontal blenden
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_CHECKER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_CHECKERBOARD );// Vertikal versetzt einblenden ??
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_CHECKERBOARD );// Horizontal versetzt einblenden ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_COVER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_RIGHT );     // Von rechts ueberdecken
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_BOTTOM );    // Von unten ueberdecken
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LEFT );      // Von links ueberdecken
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_TOP );       // Von oben ueberdecken
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERRIGHT );// Von rechts unten ueberdecken ??
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERLEFT ); // Von links unten ueberdecken ??
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERRIGHT );// Von rechts oben ueberdecken
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERLEFT ); // Von links oben ueberdecken ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_NONE :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_NONE );                // Direkt
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_NONE );                // Direkt ueber Schwarz
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_DISSOLVE :
                                        pPage->SetFadeEffect(::com::sun::star::presentation::FadeEffect_DISSOLVE);                  // Aufloesen
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM_BARS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_LINES );    // Horizontale Linien
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_LINES );      // Vertikale Linien
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_SPLIT :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_OPEN_VERTICAL );   // Horizontal oeffnen
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_CLOSE_VERTICAL );  // Horizontal schliessen
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_OPEN_HORIZONTAL ); // Vertikal oeffnen
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_CLOSE_HORIZONTAL );// Vertikal schliessen
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_STRIPS :
                                    {
                                        if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERRIGHT );// Diagonal nach links oben
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERLEFT ); // Diagonal nach rechts oben
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERRIGHT );// Diagonal nach links unten
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERLEFT ); // Diagonal nach rechts unten
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_PULL :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_RIGHT );        // Nach links aufdecken
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_BOTTOM );       // Nach oben aufdecken
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LEFT );         // Nach rechts aufdecken
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_TOP );          // Nach unten aufdecken
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERLEFT );    // Nach links oben aufdecken
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT );   // Nach rechts oben aufdecken
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERLEFT );    // Nach links unten aufdecken
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT );   // Nach rechts unten aufdecken
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_WIPE :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_RIGHT ); // Von rechts rollen
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_BOTTOM );// Von unten rollen
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LEFT );  // Von links rollen
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_TOP );   // Von oben rollen
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM :
                                        pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_RANDOM );              // Automatisch
                                    break;

                                    // the following effects does not match 100%
                                    case PPT_TRANSITION_TYPE_FADE :
                                        pPage->SetFadeEffect(::com::sun::star::presentation::FadeEffect_NONE);                  // Ueber Schwarz blenden ??
                                    break;
                                    case PPT_TRANSITION_TYPE_ZOOM :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_CENTER );// Von innen einblenden
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_TO_CENTER );  // Von aussen einblenden
                                    }
                                    break;
                                }

                                if ( nSpeed == 0 )
                                    pPage->SetFadeSpeed( FADE_SPEED_SLOW );     // langsam
                                else if ( nSpeed == 1 )
                                    pPage->SetFadeSpeed( FADE_SPEED_MEDIUM );   // mittel
                                else if ( nSpeed == 2 )
                                    pPage->SetFadeSpeed( FADE_SPEED_FAST );     // schnell

                                if ( nBuildFlags & 1 )
                                    pPage->SetPresChange( ePresChange );    // Diawechsel bei Klick auf Hintergrund
                                else
                                {   // Standzeit (in Ticks)
                                    pPage->SetPresChange( PRESCHANGE_AUTO );
                                    pPage->SetTime( nSlideTime / 1000 );
                                }
                                if ( nBuildFlags & 4 )
                                    pPage->SetExcluded( TRUE );                 // Dia nicht anzeigen
                                if ( nBuildFlags & 16 )
                                {   // Dia mit Soundeffekt
                                    pPage->SetSound( TRUE );
                                    String aSoundFile( ReadSound( nSoundRef ) );
                                    pPage->SetSoundFile( aSoundFile );
                                }
        //                      if ( nBuildFlags & ( 1 << 6 ) )     Loop until next sound: wird nicht unterstuetzt
        //                      if ( nBuildFlags & ( 1 << 8 ) )     Stop the previous sound: wird nicht unterstuetzt
                                break;
                            }
                        }
                    }
                    aHd.SeekToEndOfRecord( rStCtrl );
                }
                if ( bTryTwice && ( bSSSlideInfoAtom == FALSE ) )
                {
                    bTryTwice = FALSE;
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        USHORT nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                        if ( pPageList && ( nMasterNum < pPageList->Count() ) )
                        {
                            PptSlidePersistEntry* pE = (*pPageList)[ nMasterNum ];
                            if ( pE )
                            {
                                UINT32 nOfs = pE->aPersistAtom.nReserved;
                                if ( nOfs )
                                {
                                    rStCtrl.Seek( nOfs );
                                    nPageRecEnd = nOfs + 16;
                                    continue;
                                }
                            }
                        }

                    }
                }
                break;
            }
        }
    }


    // Animationsobjekte der Page in der Reihenfolge abstimmen
    SdrObjListIter aIter( *pPage, IM_FLAT );
    sal_Int32   i;
    List    aAnimInfo;
    while ( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        const SdAnimationInfo *pInfo = pDoc->GetAnimationInfo( pObj );
        if ( pInfo )
        {
            for ( i = aAnimInfo.Count() - 1; i >= 0; i-- )
            {
                if ( ( (const SdAnimationInfo*)aAnimInfo.GetObject( i ) )->nPresOrder <= pInfo->nPresOrder )
                    break;
            }
            aAnimInfo.Insert( (void*)pInfo, i + 1 );
        }
    }
    for ( i = 0; (sal_uInt32)i < aAnimInfo.Count(); i++ )
        ( (SdAnimationInfo*)aAnimInfo.GetObject( i ) )->nPresOrder = i + 1;

    rStCtrl.Seek( nFilePosMerk );
}

//////////////////////////////////////////////////////////////////////////
//
// Import von Sounds
//
// Die Sounds werden nicht nur als String importiert sondern auch
// in die Gallery einefuegt, falls dort noch nicht vorhanden.
//
///////////////////////////////////////////////////////////////////////////

String SdPPTImport::ReadSound(UINT32 nSoundRef) const
{
    String aRetval;
    UINT32 nPosMerk = rStCtrl.Tell();
    DffRecordHeader aDocHd;
    if ( SeekToDocument( &aDocHd ) )
    {
        UINT32 nSoundLen = aDocHd.GetRecEndFilePos();
        DffRecordHeader aSoundBlockRecHd;
        if( SeekToRec( rStCtrl, PPT_PST_SoundCollection, nSoundLen, &aSoundBlockRecHd ) )
        {
            UINT32 nDataLen = aSoundBlockRecHd.GetRecEndFilePos();
            DffRecordHeader aSoundRecHd;
            BOOL bRefStrValid = FALSE;
            BOOL bDone = FALSE;

            while( !bDone && SeekToRec( rStCtrl, PPT_PST_Sound, nDataLen, &aSoundRecHd ) )
            {
                UINT32 nStrLen = aSoundRecHd.GetRecEndFilePos();
                String aRefStr;
                UINT32 nPosMerk2 = rStCtrl.Tell();
                if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen, NULL, 2 ) )
                {
                    if ( ReadString( aRefStr ) )
                        bRefStrValid = TRUE;
                }
                if ( bRefStrValid )
                {
                    if ( UniString::CreateFromInt32( nSoundRef ) == aRefStr )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen, NULL, 0 ) )
                        {
                            ReadString( aRetval );
                            bDone = TRUE;
                        }
                    }
                }
                if ( bDone )
                {
                    // ueberpruefen, ob diese Sound-Datei schon
                    // existiert. Wenn nicht, exportiere diese
                    // in unser lokales Sound-Verzeichnis.
                    BOOL    bSoundExists = FALSE;
                    List*   pSoundList = new List();

                    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, *pSoundList );
                    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, *pSoundList );

                    for( ULONG n = 0; ( n < pSoundList->Count() ) && !bSoundExists; n++ )
                    {
                        INetURLObject   aURL( *(String*)pSoundList->GetObject( n ) );
                        String          aSoundName( aURL.GetName() );

                        if( aSoundName == aRetval )
                        {
                            aRetval = *(String*)pSoundList->GetObject( n );
                            bSoundExists = TRUE;
                        }
                    }

                    for ( void* pPtr = pSoundList->First(); pPtr; pPtr = pSoundList->Next() )
                        delete (String*)pPtr;

                    delete pSoundList;

                    if ( !bSoundExists )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        DffRecordHeader aSoundDataRecHd;
                        if ( SeekToRec( rStCtrl, PPT_PST_SoundData, nStrLen, &aSoundDataRecHd, 0 ) )
                        {
                            String          aGalleryDir( SvtPathOptions().GetGalleryPath() );
                            INetURLObject   aGalleryUserSound( aGalleryDir.GetToken( aGalleryDir.GetTokenCount( ';' ) - 1 ) );

                            aGalleryUserSound.Append( aRetval );
                            UINT32 nSoundLen = aSoundDataRecHd.nRecLen;
                            UINT8* pBuf = new UINT8[ nSoundLen ];

                            rStCtrl.Read( pBuf, nSoundLen );
                            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aGalleryUserSound.GetMainURL(), STREAM_WRITE | STREAM_TRUNC );

                            if( pOStm )
                            {
                                pOStm->Write( pBuf, nSoundLen );

                                if( pOStm->GetError() == ERRCODE_NONE )
                                {
                                    GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aGalleryUserSound.GetMainURL() );
                                    aRetval = aGalleryUserSound.GetMainURL();
                                }

                                delete pOStm;
                            }

                            delete[] pBuf;
                        }
                    }
                }
                if ( !bDone )
                    aSoundRecHd.SeekToEndOfRecord( rStCtrl );
            }
        }
    }
    rStCtrl.Seek( nPosMerk );
    return aRetval;
}

//////////////////////////////////////////////////////////////////////////
//
// Import von Objekten
//
//////////////////////////////////////////////////////////////////////////

// Hilfsklasse zum Einlesen der PPT AnimationInfoAtom
struct PptAnimationInfoAtom
{
    UINT32          nDimColor;
    UINT32          nFlags;         // 0x0004: Zeit statt Mausklick
    UINT32          nSoundRef;
    INT32           nDelayTime;     // 1/1000stel Sekunden
    UINT16          nOrderID;
    UINT16          nSlideCount;
    UINT8           nBuildType;
    UINT8           nFlyMethod;
    UINT8           nFlyDirection;
    UINT8           nAfterEffect;
    UINT8           nSubEffect;
    UINT8           nOLEVerb;

    // unknown, da Gesamtgroesse 28 ist
    UINT8           nUnknown1;
    UINT8           nUnknown2;

public:
    ::com::sun::star::presentation::AnimationEffect GetAnimationEffect(::com::sun::star::presentation::AnimationSpeed& rSpeed);
    friend SvStream& operator>>(SvStream& rIn, PptAnimationInfoAtom& rAtom);
};

SvStream& operator>>(SvStream& rIn, PptAnimationInfoAtom& rAtom)
{
    rIn >> rAtom.nDimColor;
    rIn >> rAtom.nFlags;
    rIn >> rAtom.nSoundRef;
    rIn >> rAtom.nDelayTime;
    rIn >> rAtom.nOrderID;
    rIn >> rAtom.nSlideCount;
    rIn >> rAtom.nBuildType;
    rIn >> rAtom.nFlyMethod;
    rIn >> rAtom.nFlyDirection;
    rIn >> rAtom.nAfterEffect;
    rIn >> rAtom.nSubEffect;
    rIn >> rAtom.nOLEVerb;
    rIn >> rAtom.nUnknown1;
    rIn >> rAtom.nUnknown2;
    return rIn;
}

::com::sun::star::presentation::AnimationEffect PptAnimationInfoAtom::GetAnimationEffect( ::com::sun::star::presentation::AnimationSpeed& rSpeed )
{
    ::com::sun::star::presentation::AnimationEffect eRetval( ::com::sun::star::presentation::AnimationEffect_APPEAR );
    if ( !nBuildType )
        eRetval = ::com::sun::star::presentation::AnimationEffect_NONE;
    else
    {
        switch ( nFlyMethod )
        {
            case 0x0:
                eRetval = ::com::sun::star::presentation::AnimationEffect_APPEAR;                       // Erscheinen
            break;
            case 0x01:
                eRetval = ::com::sun::star::presentation::AnimationEffect_RANDOM;                       // Random
            break;
            case 0x02:                                                  // Blinds Effekt
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRIPES;     // Horizontal
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRIPES;   // Vertical
                    break;
                }
            }
            break;
            case 0x03:                                                  // (hor/ver) versetzt Einblenden
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD;  // vertikal
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_CHECKERBOARD;    // horizontal
                    break;
                }
            }
            break;
            case 0x05:
                eRetval = ::com::sun::star::presentation::AnimationEffect_DISSOLVE;                     // Aufloesen
            break;
            case 0x08:                                                  // (hor/ver) Linien
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_LINES;     // horizontal
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_LINES;       // vertikal
                    break;
                }
            }
            break;
            case 0x09:                                                  // Diagonal
            {
                switch ( nFlyDirection )
                {
                    case 0x4:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERRIGHT; // nach links oben
                    break;
                    case 0x5:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERLEFT;  // nach rechts oben
                    break;
                    case 0x6:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERRIGHT; // nach links unten
                    break;
                    case 0x7:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERLEFT;  // nach rechts unten
                    break;
                }
            }
            break;
            case 0x0a:                                                  // rollen
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_RIGHT;      // von rechts
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_BOTTOM;     // von unten
                    break;
                    case 0x2:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LEFT;       // von links
                    break;
                    case 0x3:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_TOP;        // von oben
                    break;
                }
            }
            break;
            case 0x0b:                                                  // (von aussen/innen) Einblenden
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER;     // von innen
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;       // von aussen
                    break;
                }
            }
            break;
            case 0x0c:                                                  // Text-Effekt
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT;
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP;
                    break;
                    case 0x2:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT;
                    break;
                    case 0x3:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM;
                    break;
                    case 0x4:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERLEFT;  // von oben links
                    break;
                    case 0x5:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT; // von oben rechts
                    break;
                    case 0x6:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERLEFT; // von unten links
                    break;
                    case 0x7:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT; // von unten rechts
                    break;
                    case 0x8:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT; // verkürzt von links
                    break;
                    case 0x9:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM;// verkürzt von unten
                    break;
                    case 0xa:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT;// verkürzt von rechts
                    break;
                    case 0xb:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_TOP;  // verkürzt von oben
                    break;
                    case 0xc:
                    {
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT;   // langsam von links
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    }
                    break;
                    case 0xd:
                    {
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP;    // langsam von oben
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    }
                    break;
                    case 0xe:
                    {
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT;  // langsam von rechts
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    }
                    break;
                    case 0xf:
                    {
                        eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM; // langsam von unten
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    }
                    break;
                    case 0x10:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER; // Vergroessern
                    break;
                    case 0x11:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER; // etwas Vergroessern
                    break;
                    case 0x12:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;   // Verkleinern
                    break;
                    case 0x13:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;   // etwas Verkleinern
                    break;
                    case 0x14:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER; // Vergroessern von Bildschirmmitte
                    break;
                    case 0x15:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;   // Verkleinern von unterem Bildschirm
                    break;
                    case 0x16:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRETCH; // quer dehnen
                    break;
                    case 0x17:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LEFT;    // von links dehnen
                    break;
                    case 0x18:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_TOP;     // von oben dehnen
                    break;
                    case 0x19:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_RIGHT;   // von rechts dehnen
                    break;
                    case 0x1a:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_BOTTOM;  // von unten dehnen
                    break;
                    case 0x1b:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_ROTATE;    // Rotieren
                    break;
                    case 0x1c:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_LEFT;       // Spirale
                    break;
                }
            }
            break;
            case 0x0d:                                              // Schliessen/Oeffnen
            {
                switch ( nFlyDirection )
                {
                    case 0x0:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_OPEN_VERTICAL;    // hor oeffnen
                    break;
                    case 0x1:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_CLOSE_VERTICAL;   // hor schliessen
                    break;
                    case 0x2:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_OPEN_HORIZONTAL;  // vert oeffnen
                    break;
                    case 0x3:
                        eRetval = ::com::sun::star::presentation::AnimationEffect_CLOSE_HORIZONTAL; // vert schliessen
                    break;
                }
            }
            break;
            case 0x0e:                                              // Blinken, danach unsichtbar
            {
                if( !nAfterEffect )
                    nAfterEffect = 3;
                switch ( nFlyDirection )
                {
                    case 0x0:
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_FAST;               // schnell
                    break;
                    case 0x1:
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_MEDIUM;             // mittel
                    break;
                    case 0x2:
                        rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;               // langsam
                    break;
                }
            }
            break;
        }
    }
    return eRetval;
}

void SdPPTImport::FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, String aMacroName )
{
    // Lokale Informationen in pInfo eintragen
    if( pIAtom->nSoundRef )
    {
        pInfo->aBookmark = ReadSound( pIAtom->nSoundRef );  // Pfad zum Soundfile in MSDOS-Notation
        pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_SOUND;            // RunProgramAction
    }
//  if ( nFlags & 0x01 )    // koennen wir nicht ( beim Anklicken markieren )
    switch ( pIAtom->nAction )
    {
//      case 0x01 :                                         // MacroAction
//      {
//          pInfo->eClickAction = ::com::sun::star::presentation::::com::sun::star::presentation::ClickAction_MACRO;
//          // aMacro liegt in folgender Form vor:
//          // "Macroname.Modulname.Libname.Dokumentname" oder
//          // "Macroname.Modulname.Libname.Applikationsname"
//          pInfo->aBookmark = aMacroName;
//      }
//      break;
        case 0x02 :                                         // RunProgramAction
        {
            pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_PROGRAM;
            pInfo->aBookmark = aMacroName;                  // Programmname in aBookmark
        }
        break;
        case 0x03 :                                         // JumpAction
        {
            switch( pIAtom->nJump )
            {
                case 0x01 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_NEXTPAGE;         // Next slide
                break;
                case 0x02 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_PREVPAGE;         // Previous slide
                break;
                case 0x03 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_FIRSTPAGE;        // First slide
                break;
                case 0x04 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_LASTPAGE;         // last Slide
                break;
                case 0x05 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_PREVPAGE;         // Last slide viewed
                break;
                case 0x06 :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_STOPPRESENTATION; // End show
                break;
                default :
                    pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_NONE;             // 0x00: no action, else unknown
                break;
            }
        }
        break;
        case 0x04 :
        {
            SdHyperlinkEntry* pPtr;
            for ( pPtr = (SdHyperlinkEntry*)aHyperList.First(); pPtr; pPtr = (SdHyperlinkEntry*)aHyperList.Next() )
            {
                if ( pPtr->nIndex == pIAtom->nExHyperlinkId )
                    break;
            }
            if ( pPtr )
            {
                switch( pIAtom->nHyperlinkType )
                {
                    case 9:
                    case 10:
                    break;
                    case 8:                                         // hyperlink : URL
                    {
                        if ( pPtr->aTarget.Len() )
                        {
                            pInfo->aBookmark = String( pPtr->aTarget );
                            pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_DOCUMENT;
                        }
                    }
                    break;
                    case 7:                                         // hyperlink auf eine Seite
                    {
                        if ( pPtr->aSubAdress.Len() )
                        {
                            UINT32 nPageNumber = 0;
                            String aString( pPtr->aSubAdress );
                            String aStringAry[ 3 ];

                            USHORT nTokenCount = aString.GetTokenCount( ',' );
                            if ( nTokenCount > 3 )
                                nTokenCount = 3;
                            USHORT nIndex, nLtT = 0;
                            for ( USHORT i = 0; i < nTokenCount; i++ )
                            {
                                nIndex = aString.Search( sal_Unicode(','), nLtT );
                                if ( nIndex == STRING_NOTFOUND )
                                {
                                    if ( nLtT < aString.Len() )
                                        aStringAry[ i ] = UniString( aString, nLtT, aString.Len() - nLtT );
                                    break;
                                }
                                if ( nLtT < nIndex )
                                    aStringAry[ i ] = UniString( aString, nLtT, nIndex - nLtT );
                                nLtT = nIndex + 1;
                            }
                            BOOL bPageByName = FALSE;
                            for ( INT32 j = nTokenCount - 1; !bPageByName && ( j >= 0 ); j-- )
                            {
                                const String* pString = &aStringAry[ i ];
                                for ( void* pPtr = aSlideNameList.First(); pPtr; pPtr = aSlideNameList.Next() )
                                {
                                    if ( *(String*)pPtr == *pString )
                                    {
                                        nPageNumber = aSlideNameList.GetCurPos() + 1;
                                        bPageByName = TRUE;
                                        break;
                                    }
                                }
                            }
                            if ( !bPageByName )
                            {
                                for ( i = 0; i < nTokenCount; i++ )
                                {
                                    if ( ByteString( aStringAry[ i ], gsl_getSystemTextEncoding() ).IsNumericAscii() )
                                    {
                                        for ( sal_uInt16 j = 0; j < aStringAry[ i ].Len(); j++ )
                                        {
                                            nPageNumber *= 10;
                                            nPageNumber += aString.GetChar( j ) - '0';
                                        }
                                        if ( nPageNumber &~ 0xff )      // then we assume that this is the pageID,
                                        {
                                            PptSlidePersistList* pPageList = GetPageList( PPT_SLIDEPAGE );
                                            if ( pPageList )
                                            {
                                                USHORT nPage = pPageList->FindPage( nPageNumber );
                                                if ( nPage != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                                    nPageNumber = nPage + 1;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                            if ( nPageNumber && ( nPageNumber > aSlideNameList.Count() ) )
                            {
                                SdPage* pPage = pDoc->GetSdPage( 0, PK_STANDARD );
                                if ( pPage )
                                {
                                    String aName( pPage->GetName() );
                                    USHORT nLen = aName.Len();
                                    while( --nLen && ( ( aName.GetChar( nLen ) >= '0' ) && ( aName.GetChar( nLen ) <= '9' ) ) )
                                        aName.Erase( nLen );
                                    if ( aName.Len() )
                                    {
                                        pInfo->aBookmark = aName;
                                        pInfo->aBookmark += String::CreateFromInt32( (sal_Int32)nPageNumber );  // sal_uInt32 -> sal_Int32 !
                                        pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_BOOKMARK;
                                    }
                                }
                            }
                            else
                            {
                                pInfo->aBookmark = *(String*)aSlideNameList.GetObject( nPageNumber - 1 );
                                pInfo->eClickAction = ::com::sun::star::presentation::ClickAction_BOOKMARK;
                            }
                        }
                    }
                    break;
                }
            }
        }
        break;
        case 0x05 :                     // OLEAction ( OLEVerb to use, 0==first, 1==secnd, .. )
        case 0x06 :                     // MediaAction
        case 0x07 :                     // CustomShowAction
        default :                       // 0x00: no action, else unknown action
        break;
    }
}

void SdPPTImport::FillSdAnimationInfo( SdAnimationInfo* pInfo, PptAnimationInfoAtom* pAnim )
{
    // Lokale Informationen in pInfo eintragen
    // Praesentationsreihenfolge; diese wird
    // eventuell (falls alle gleich sind) noch in
    // ImportPage() korrigiert

    pInfo->aBlueScreen = Color( (BYTE)1, (BYTE)2, (BYTE)3 );        // ppt does not have transparent objects, but we
                                                                    // have to set a color. ( this color is rarely used )
    pInfo->nPresOrder = pAnim->nOrderID;
    pInfo->nVerb = pAnim->nOLEVerb;                                 // fuer OLE-Objekt
    pInfo->eSpeed = ::com::sun::star::presentation::AnimationSpeed_MEDIUM;                          // Geschwindigkeit der Animation
    pInfo->eEffect = pAnim->GetAnimationEffect( pInfo->eSpeed );    // Animationseffekt
    pInfo->bDimPrevious = FALSE;                                    // Objekt abblenden
    pInfo->aDimColor = Color(COL_WHITE);                            // Default-Ausblendfarbe auf weiss

    if( pAnim->nSoundRef && ( pAnim->nFlags & 0x0010 ) )            // Sound
    {
        pInfo->bSoundOn = TRUE;                                     // Sound ist an
        pInfo->aSoundFile = ReadSound( pAnim->nSoundRef );          // Pfad zum Soundfile in MSDOS-Notation
    }
    else
        pInfo->bSoundOn = FALSE;                                    // Sound ist aus

    if( pAnim->nFlags & 0x0040 )
        pInfo->bPlayFull = FALSE;                                   // Sound des vorherigen Objektes abbrechen
    else
        pInfo->bPlayFull = TRUE;

//  if ( nFlags & 4 )   // mouse over effect after nDelayTime ( not supported )
    if ( pInfo->eEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
    {
        switch ( pAnim->nAfterEffect )
        {
            case 1 :                                        // zur Farbe in nDimColor ausblenden
            {
                pInfo->bDimPrevious = TRUE;                 // Objekt abblenden
                pInfo->bDimHide = FALSE;                    // verstecken statt abblenden
                pInfo->aDimColor = pAnim->nDimColor;        // zum Abblenden des Objekts
            }
            break;
            case 2 :                                        // Beim naechsten Mausklick ausblenden
            {
                ePresChange = PRESCHANGE_MANUAL;
                pInfo->bDimHide = TRUE;                     // verstecken statt abblenden
            }
            break;
            case 3 :                                        // Nach Animation ausblenden
                pInfo->bDimHide = TRUE;                     // verstecken statt abblenden
            break;
            default :
                pInfo->bDimHide = FALSE;                    // verstecken statt abblenden
            break;
        }
    }
    if ( pAnim->nBuildType > 1 )                                // texteffect active ( paragraph grouping level )
    {
        pInfo->eTextEffect = pInfo->eEffect;
        if ( ! ( pAnim->nFlags & 0x4000 ) )
        {   // Verknuepfte Form animieren aus
//          switch ( pAnim->nSubEffect )
//          {
//              case 0 :                                        // Text einfuehren - Paragraphweise
//              case 1 :                                        // Text einfuehren - Wortweise
//              case 2 :                                        // Text einfuehren - Zeichenweise
//          }
            if ( pInfo->eTextEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
                pInfo->eEffect = ::com::sun::star::presentation::AnimationEffect_APPEAR;
        }
    }
    else
        pInfo->eTextEffect = ::com::sun::star::presentation::AnimationEffect_NONE;

//  if ( nFlags & 1 )                                   // Koennen wir nicht: In umgekehrter Reihenfolge an
}

SdrObject* SdPPTImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pObj, SdPage* pPage,
                                        SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SfxStyleSheet*  pStyleSheetAry[ 9 ];
    SdrTextObj*     pText = pObj;
    SdrObject*      pRet = pText;

    ppStyleSheetAry = NULL;
    PresObjKind ePresKind = PRESOBJ_NONE;
    PptOEPlaceholderAtom* pPlaceHolder = pTextObj->GetOEPlaceHolderAtom();

    switch ( pTextObj->GetInstance() )
    {
        case TSS_TYPE_PAGETITLE :
        case TSS_TYPE_TITLE :
        {
            ePresKind = PRESOBJ_TITLE;
            pSheet = pPage->GetStyleSheetForPresObj( ePresKind );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, TRUE );
            DBG_ASSERT( pSheet, "SdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
        }
        break;
        case TSS_TYPE_SUBTITLE :
        {
            ePresKind = PRESOBJ_TEXT;
            pSheet = pPage->GetStyleSheetForPresObj( ePresKind );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, TRUE );
            DBG_ASSERT( pSheet, "SdPPTImport::ApplyTextObj -> could not get stylesheet for subtitleobject (SJ)" );
        }
        break;
        case TSS_TYPE_BODY :
        case TSS_TYPE_HALFBODY :
        case TSS_TYPE_QUARTERBODY :
        {
            ePresKind = PRESOBJ_OUTLINE;
            for ( UINT16 nLevel = 9; nLevel; nLevel-- )
            {
                String aName( pPage->GetLayoutName() );
                aName.Append( (sal_Unicode)( ' ' ) );
                aName.Append( String::CreateFromInt32( nLevel ) );
                pSheet = (SfxStyleSheet*)pDoc->GetStyleSheetPool()->Find( aName, SD_LT_FAMILY );
                if ( pSheet )
                    pText->StartListening( *pSheet );
                pStyleSheetAry[ nLevel - 1 ] = pSheet;
            }
            DBG_ASSERT( pSheet, "SdPPTImport::ApplyTextObj -> could not get stylesheet for outlinerobject (SJ)" );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, TRUE );
            ppStyleSheetAry = &pStyleSheetAry[ 0 ];
        }
        break;
        case TSS_TYPE_NOTES :
        {
            ePresKind = PRESOBJ_NOTES;
            if ( pPlaceHolder && ( ( pPlaceHolder->nPlaceholderId == PPT_PLACEHOLDER_NOTESSLIDEIMAGE )
                || ( pPlaceHolder->nPlaceholderId == PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE ) ) )
            {
                ePresKind = PRESOBJ_TITLE;
                pSheet = pPage->GetStyleSheetForPresObj( ePresKind );
                if ( pSheet )
                    ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, TRUE );
                DBG_ASSERT( pSheet, "SdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
            }
            else
            {
                pSheet = pPage->GetStyleSheetForPresObj( ePresKind );
                DBG_ASSERT( pSheet, "SdPPTImport::ApplyTextObj -> could not get stylesheet for notesobj (SJ)" );
                if ( pSheet )
                    ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, TRUE );
            }
        }
        break;
        case TSS_TYPE_UNUSED :
        case TSS_TYPE_TEXT_IN_SHAPE :
            pSheet = (SfxStyleSheet*)pDoc->GetStyleSheetPool()->Find( String(SdResId( STR_STANDARD_STYLESHEET_NAME )), SFX_STYLE_FAMILY_PARA );
        break;
    }
    pText = (SdrTextObj*)SdrPowerPointImport::ApplyTextObj( pTextObj, pText, pPage, pSheet, ppStyleSheetAry );
    if ( pPlaceHolder && pPlaceHolder->nPlaceholderId )
    {
        if ( eAktPageKind == PPT_MASTERPAGE )
        {
            if ( ePresKind != PRESOBJ_NONE )
            {
                if ( pTextObj->GetInstance() != TSS_TYPE_SUBTITLE )
                {
                    pText->SetNotVisibleAsMaster( TRUE );
                    pText->SetEmptyPresObj( TRUE );
    //              if ( pPlaceHolder->nPlaceholderId == PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE )
    //                  ePresKind = PRESOBJ_TITLE;
                    String aString( pPage->GetPresObjText( ePresKind ) );
                    pText->SetUserCall( pPage );
                    pPage->GetPresObjList()->Insert( pText, LIST_APPEND );
                    SdrOutliner* pOutl = NULL;
                    if ( pTextObj->GetInstance() == TSS_TYPE_NOTES )
                        pOutl = GetDrawOutliner( pText );
                    pPage->SetObjText( (SdrTextObj*)pText, pOutl, ePresKind, aString);
                    pText->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresKind ), TRUE );
                    SfxItemSet aTempAttr( pDoc->GetPool() );
                    SdrTextMinFrameHeightItem aMinHeight( pText->GetLogicRect().GetSize().Height() );
                    aTempAttr.Put( aMinHeight );
                    SdrTextAutoGrowHeightItem aAutoGrowHeight( FALSE );
                    aTempAttr.Put( aAutoGrowHeight );
                    pText->SetItemSet(aTempAttr);
                }
                else
                {
                    pRet = NULL;
                }
            }
        }
        else
        {
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout || ( eAktPageKind == PPT_NOTEPAGE ) )
            {
                INT16 nPlaceholderId = pPlaceHolder->nPlaceholderId;
                UINT16 i = 0;
                if ( eAktPageKind == PPT_SLIDEPAGE )
                {
                    for ( ; i < 8; i++ )
                    {
                        if ( pSlideLayout->aPlaceholderId[ i ] == nPlaceholderId )
                            break;
                    }
                }
                if ( i < 8 )
                {
                    PresObjKind ePresObjKind = PRESOBJ_NONE;
                    BOOL        bEmptyPresObj = TRUE;
                    if ( ( pTextObj->GetShapeType() == mso_sptRectangle ) || ( pTextObj->GetShapeType() == mso_sptTextBox ) )
                    {
                        if ( pTextObj->Count() )
                            bEmptyPresObj = FALSE;
                        switch ( nPlaceholderId )
                        {
                            case PPT_PLACEHOLDER_NOTESBODY :    ePresObjKind = PRESOBJ_NOTES;   break;
                            case PPT_PLACEHOLDER_TITLE :        ePresObjKind = PRESOBJ_TITLE;   break;
                            case PPT_PLACEHOLDER_BODY :         ePresObjKind = PRESOBJ_OUTLINE; break;
                            case PPT_PLACEHOLDER_CENTEREDTITLE :ePresObjKind = PRESOBJ_TITLE;   break;
                            case PPT_PLACEHOLDER_SUBTITLE :     ePresObjKind = PRESOBJ_TEXT;    break;      // PRESOBJ_OUTLINE

                            default :
                            {
                                if ( !pTextObj->Count() )
                                {
                                    switch ( nPlaceholderId )
                                    {
                                        case PPT_PLACEHOLDER_MEDIACLIP :
                                        case PPT_PLACEHOLDER_OBJECT : ePresObjKind = PRESOBJ_OBJECT; break;
                                        case PPT_PLACEHOLDER_GRAPH : ePresObjKind = PRESOBJ_CHART; break;
                                        case PPT_PLACEHOLDER_TABLE : ePresObjKind = PRESOBJ_TABLE; break;
                                        case PPT_PLACEHOLDER_CLIPART : ePresObjKind = PRESOBJ_GRAPHIC; break;
                                        case PPT_PLACEHOLDER_ORGANISZATIONCHART : ePresObjKind = PRESOBJ_ORGCHART; break;
                                    }
                                }
                            };
                        }
                    }
                    else if ( pTextObj->GetShapeType() == mso_sptPictureFrame )
                    {
                        if ( !pTextObj->Count() && pObj->ISA( SdrGrafObj ) )
                        {
                            bEmptyPresObj = FALSE;
                            switch ( nPlaceholderId )
                            {
                                case PPT_PLACEHOLDER_MEDIACLIP :
                                case PPT_PLACEHOLDER_OBJECT : ePresObjKind = PRESOBJ_OBJECT; break;
                                case PPT_PLACEHOLDER_GRAPH : ePresObjKind = PRESOBJ_CHART; break;
                                case PPT_PLACEHOLDER_TABLE : ePresObjKind = PRESOBJ_TABLE; break;
                                case PPT_PLACEHOLDER_CLIPART : ePresObjKind = PRESOBJ_GRAPHIC; break;
                                case PPT_PLACEHOLDER_ORGANISZATIONCHART : ePresObjKind = PRESOBJ_ORGCHART; break;
                            }
                        }
                    }
                    if ( ePresObjKind != PRESOBJ_NONE )
                    {
                        if ( !bEmptyPresObj )
                        {
                            List* pPresObjList = pPage->GetPresObjList();
                            pPresObjList->Insert( pRet );
                        }
                        else
                        {
                            SdrObject* pPresObj = pPage->CreatePresObj( ePresObjKind, FALSE, pText->GetLogicRect(), TRUE );
                            pPresObj->SetUserCall( pPage );

                            SfxItemSet aSet( pSdrModel->GetItemPool() );
                            ApplyAttributes( rStCtrl, aSet, pPresObj );
                            pPresObj->SetItemSet(aSet);

                            if ( ( eAktPageKind != PPT_NOTEPAGE ) && ( pSlideLayout->aPlacementId[ i ] != -1 ) )
                            {
                                SdrObject* pTitleObj = ((SdPage*)pPage->GetMasterPage(0))->GetPresObj( PRESOBJ_TITLE );
                                SdrObject* pOutlineObj = ((SdPage*)pPage->GetMasterPage(0))->GetPresObj( PRESOBJ_OUTLINE );

                                Rectangle aTitleRect;
                                Rectangle aOutlineRect;
                                Size      aOutlineSize;

                                if ( pTitleObj )
                                    aTitleRect = pTitleObj->GetLogicRect();
                                if ( pOutlineObj )
                                {
                                    aOutlineRect = pOutlineObj->GetLogicRect();
                                    aOutlineSize = aOutlineRect.GetSize();
                                }
                                Rectangle aLogicRect( pPresObj->GetLogicRect() );
                                Size      aLogicSize( aLogicRect.GetSize() );

                                switch ( pSlideLayout->aPlacementId[ i ] )
                                {
                                    case 0 :            // Lage im Titelbereich
                                    {
                                        if ( aLogicRect != aTitleRect )
                                            pPresObj->SetUserCall( NULL );
                                    }
                                    break;

                                    case 1:
                                    {
                                        if ( pSlideLayout->eLayout == PPT_LAYOUT_TITLEANDBODYSLIDE )
                                        {   // Lage im Outlinebereich
                                            if ( aLogicRect != aOutlineRect )
                                                pPresObj->SetUserCall( NULL );
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2COLUMNSANDTITLE )
                                        {   // Lage im Outlinebereich links
                                            if (Abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall(NULL);
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2ROWSANDTITLE )
                                        {   // Lage im Outlinebereich oben
                                            if (Abs(aLogicRect.Left()  - aOutlineRect.Left())  > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if (Abs(aLogicRect.Left() - aOutlineRect.Left()) > MAX_USER_MOVE ||
                                                 Abs(aLogicRect.Top()  - aOutlineRect.Top())  > MAX_USER_MOVE)
                                        {   // Lage im Outlinebereich links oben
                                            pPresObj->SetUserCall( NULL );
                                        }
                                    }
                                    break;

                                    case 2:
                                    {
                                        if ( pSlideLayout->eLayout == PPT_LAYOUT_2COLUMNSANDTITLE )
                                        {   // Lage im Outlinebereich rechts
                                            if (Abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2ROWSANDTITLE )
                                        {   // Lage im Outlinebereich unten
                                            if (Abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                Abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if (Abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE ||
                                                 Abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE)
                                        {   // Lage im Outlinebereich rechts oben
                                            pPresObj->SetUserCall(NULL);
                                        }
                                    }
                                    break;

                                    case 3:
                                    {   // Lage im Outlinebereich links unten
                                        if (Abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                            Abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
                                        {
                                            pPresObj->SetUserCall( NULL );
                                        }
                                    }
                                    break;

                                    case 4:
                                    {   // Lage im Outlinebereich rechts unten
                                        if (Abs(aLogicRect.Right() - aOutlineRect.Right())   > MAX_USER_MOVE ||
                                            Abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
                                        {
                                            pObj->SetUserCall( NULL );
                                        }
                                    }
                                    break;
                                }
                            }
                            pRet = NULL;    // return zero cause this obj was already inserted by CreatePresObj
                        }
                    }
                    else if ( !pTextObj->Count() )
                        pRet = NULL;
                }
            }
        }
    }
    if ( pRet != pText )
        delete (SdrObject*)pText;
    return pRet;
}

SdrObject* SdPPTImport::ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pRet )
{
    SdrObject* pObj = SdrPowerPointImport::ProcessObj( rSt, rObjData, pData, rTextRect, pRet );
    // Animationseffekte des Objektes lesen
    if ( pObj )
    {
        BOOL bInhabitanceChecked = FALSE;
        BOOL bAnimationInfoFound = FALSE;
        DffRecordHeader aMasterShapeHd;

        PptSlidePersistEntry& rEntry  = ((ProcessData*)pData)->rPersistEntry;
        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            DffRecordHeader& rHdClientData = *maShapeRecords.Current();
            while( TRUE )
            {
                UINT32 nClientDataLen = rHdClientData.GetRecEndFilePos();
                DffRecordHeader aHd;
                do
                {
                    rSt >> aHd;
                    UINT32 nHdRecEnd = aHd.GetRecEndFilePos();
                    switch ( aHd.nRecType )
                    {
                        case PPT_PST_AnimationInfo :
                        {
                            DffRecordHeader aHdAnimInfoAtom;
                            if ( SeekToRec( rSt, PPT_PST_AnimationInfoAtom, nHdRecEnd, &aHdAnimInfoAtom ) )
                            {
                                // Daten holen
                                PptAnimationInfoAtom aAnimationInfo;
                                rSt >> aAnimationInfo;

                                // Farbe aufloesen nach RGB
                                aAnimationInfo.nDimColor = MSO_CLR_ToColor(aAnimationInfo.nDimColor).GetColor();

                                SdrObject* pEffObj = pObj;
                                if ( ( !rObjData.nCalledByGroup ) && pObj->ISA( SdrObjGroup ) )
                                {
                                    if ( aAnimationInfo.nBuildType > 1 )    // ( ( aAnimationInfo.nFlags & 0x4000 ) == 0 )
                                    {   // if texteffect is used, we will split this groupobject later
                                        SdrObjList* pObjectList = ((SdrObjGroup*)pObj)->GetSubList();
                                        if ( pObjectList )
                                        {
                                            if ( pObjectList->GetObjCount() == 2 )
                                            {
                                                pEffObj = pObjectList->GetObj( 1 );
                                                ((ProcessData*)pData)->nGroupingFlags = 1;
                                            }
                                        }
                                    }
                                }
                                // Objekt ist animiert
                                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo( pEffObj );
                                if( !pInfo )
                                    pInfo = new SdAnimationInfo( pDoc );
                                ( (SdPPTImport*) this )->FillSdAnimationInfo( pInfo, &aAnimationInfo );

                                if ( pInfo->eEffect == ::com::sun::star::presentation::AnimationEffect_NONE )
                                    delete pInfo;
                                else
                                {   // transparency color: ppt does not support one, so a not used color is to set
                                    // ( #71012# on badly configured systems only 16 system colors are available, so
                                    // get the standard palette by temporarly create a 4Bit depth Bitmap );
                                    Color aBlueScreen( 0x00, 0xff, 0xff );
                                    Bitmap aBitmap( Size( 1, 1 ), 4 );
                                    BitmapReadAccess* pAcc = aBitmap.AcquireReadAccess();
                                    if ( pAcc )
                                    {
                                        UINT16 i;
                                        BitmapColor aForegroundColor( pAcc->GetBestMatchingColor( MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ) ) ) );
                                        BitmapColor aBackgroundColor( pAcc->GetBestMatchingColor( MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ) ) ) );
                                        BitmapColor aLineColor( pAcc->GetBestMatchingColor( MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineColor, 0 ) ) ) );
                                        BitmapColor aLineBackColor( pAcc->GetBestMatchingColor( MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineBackColor, 0 ) ) ) );

                                        for ( i = 5; i < 9; i++ )
                                        {
                                            if ( pAcc->GetPaletteColor( i ) == aForegroundColor )
                                                continue;
                                            if ( pAcc->GetPaletteColor( i ) == aBackgroundColor )
                                                continue;
                                            if ( pAcc->GetPaletteColor( i ) == aLineColor )
                                                continue;
                                            if ( pAcc->GetPaletteColor( i ) != aLineBackColor )
                                                break;
                                        }
                                        aBlueScreen = pAcc->GetPaletteColor( i );
                                        aBitmap.ReleaseAccess( pAcc );
                                    }
                                    if ( !bInhabitanceChecked )             // be sure that the master effects comes
                                        pInfo->nPresOrder |= 0x80000000;    // first by setting the highest presorder bit

                                    pInfo->aBlueScreen = aBlueScreen;
                                    pEffObj->InsertUserData( pInfo );
                                }
                                bAnimationInfoFound = TRUE;
                            }
                        }
                        break;
                        case PPT_PST_InteractiveInfo:
                        {
                            UINT32 nFilePosMerk2 = rSt.Tell();
                            String aMacroName;

                            if(SeekToRec( rSt, PPT_PST_CString, nHdRecEnd, NULL, 0 ) )
                                ReadString(aMacroName);

                            rSt.Seek( nFilePosMerk2 );
                            DffRecordHeader aHdInteractiveInfoAtom;
                            if ( SeekToRec( rSt, PPT_PST_InteractiveInfoAtom, nHdRecEnd, &aHdInteractiveInfoAtom ) )
                            {
                                PptInteractiveInfoAtom aInteractiveInfoAtom;
                                rSt >> aInteractiveInfoAtom;

                                // interactive object
                                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
                                if( !pInfo )
                                {
                                    pInfo = new SdAnimationInfo( pDoc );
                                    pObj->InsertUserData( pInfo );
                                }
                                ( (SdPPTImport*) this )->FillSdAnimationInfo( pInfo, &aInteractiveInfoAtom, aMacroName );
                            }
                            break;
                        }
                    }
                    aHd.SeekToEndOfRecord( rSt );
                }
                while( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nClientDataLen ) );

                if ( bInhabitanceChecked || bAnimationInfoFound )
                    break;
                bInhabitanceChecked = TRUE;
                if ( ! ( IsProperty( DFF_Prop_hspMaster ) && SeekToShape( rSt, pData, GetPropertyValue( DFF_Prop_hspMaster ) ) ) )
                    break;
                rSt >> aMasterShapeHd;
                if ( !SeekToRec( rSt, DFF_msofbtClientData, aMasterShapeHd.GetRecEndFilePos(), &aMasterShapeHd ) )
                    break;
                aMasterShapeHd.SeekToContent( rSt );
                rHdClientData = aMasterShapeHd;
            }
        }
    }
    return pObj;
}


