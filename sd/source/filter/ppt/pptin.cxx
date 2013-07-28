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


#include <editeng/numitem.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/urihelper.hxx>
#include <svx/svxids.hrc>
#include <filter/msfilter/svdfppt.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdlayer.hxx>
#include <vcl/msgbox.hxx>
#include <svl/style.hxx>
#include <svx/xflclit.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/colritem.hxx>
#include <svl/whiter.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlnclit.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/tstpitem.hxx>

#include <sfx2/docinf.hxx>

#include "glob.hrc"
#include "pptin.hxx"
#include "Outliner.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "stlpool.hxx"
#include "anminfo.hxx"
#include <svx/gallery.hxx>
#include <tools/urlobj.hxx>
#include <svl/itempool.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdogrp.hxx>
#include "propread.hxx"
#include <cusshow.hxx>
#include <vcl/bmpacc.hxx>
#include "customshowlist.hxx"

#include "../../ui/inc/DrawDocShell.hxx"
#include "../../ui/inc/FrameView.hxx"
#include "../../ui/inc/optsitem.hxx"

#include <unotools/fltrcfg.hxx>
#include <sfx2/progress.hxx>
#include <unotools/localfilehelper.hxx>
#include <editeng/editstat.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfac.hxx>
#define MAX_USER_MOVE       2

#include "pptinanimations.hxx"
#include "ppt97animations.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <comphelper/string.hxx>
#include <oox/ole/olehelper.hxx>

using namespace ::com::sun::star;

SdPPTImport::SdPPTImport( SdDrawDocument* pDocument, SvStream& rDocStream, SvStorage& rStorage, SfxMedium& rMedium )
{

    sal_uInt32 nImportFlags = 0;

#ifdef DBG_UTIL
    PropRead* pSummaryInformation = new PropRead( rStorage, String( "\005SummaryInformation"  ) );
    if ( pSummaryInformation->IsValid() )
    {
        pSummaryInformation->Read();
        sal_uInt8 aPropSetGUID[ 16 ] =
        {
            0xe0, 0x85, 0x9f, 0xf2, 0xf9, 0x4f, 0x68, 0x10, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9
        };
        Section* pSection = (Section*)pSummaryInformation->GetSection( aPropSetGUID );
        if ( pSection )
        {
            PropItem aPropItem;
            if ( pSection->GetProperty( PID_COMMENTS, aPropItem ) )
            {
                OUString aComment;
                aPropItem.Read( aComment );
                if ( aComment.indexOf( "Applixware" ) >= 0 )
                {
                    nImportFlags |= PPT_IMPORTFLAGS_NO_TEXT_ASSERT;
                }
            }
        }
    }
    delete pSummaryInformation;
#endif

    PowerPointImportParam aParam( rDocStream, nImportFlags );
    SvStream* pCurrentUserStream = rStorage.OpenSotStream( OUString( "Current User" ), STREAM_STD_READ );
    if( pCurrentUserStream )
    {
        *pCurrentUserStream >> aParam.aCurrentUserAtom;
        delete pCurrentUserStream;
    }

    if( pDocument )
    {
        // iterate over all styles
        SdStyleSheetPool* pStyleSheetPool = pDocument->GetSdStyleSheetPool();

        sal_uInt32 nStyles = pStyleSheetPool ? pStyleSheetPool->GetStyles().size() : 0;
        for (sal_uInt32 nStyle = 0; nStyle < nStyles; nStyle++)
        {
            SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>( pStyleSheetPool->GetStyles()[nStyle].get() );
            SfxItemSet& rSet = pSheet->GetItemSet();

            // if autokerning is set in style, override it, ppt has no autokerning
            if( rSet.GetItemState( EE_CHAR_PAIRKERNING, sal_False ) == SFX_ITEM_SET )
                rSet.ClearItem( EE_CHAR_PAIRKERNING );
        }
    }

    pFilter = new ImplSdPPTImport( pDocument, rStorage, rMedium, aParam );
}

sal_Bool SdPPTImport::Import()
{
    return pFilter->Import();
}

SdPPTImport::~SdPPTImport()
{
    delete pFilter;
}

ImplSdPPTImport::ImplSdPPTImport( SdDrawDocument* pDocument, SvStorage& rStorage_, SfxMedium& rMedium, PowerPointImportParam& rParam )
:   SdrPowerPointImport     ( rParam, rMedium.GetBaseURL() )
,   mrMed                   ( rMedium )
,   mrStorage               ( rStorage_ )
,   mbDocumentFound         ( sal_False )
,   mnFilterOptions         ( 0 )
{
    mpDoc = pDocument;
    if ( bOk )
    {
        mbDocumentFound = SeekToDocument( &maDocHd );                           // maDocHd = the latest DocumentHeader
        while ( SeekToRec( rStCtrl, PPT_PST_Document, nStreamLen, &maDocHd ) )
            mbDocumentFound = sal_True;

        sal_uInt32 nDggContainerOfs = 0;

        if ( mbDocumentFound )
        {
            sal_uLong nPosMerk = rStCtrl.Tell();

            pStData = rStorage_.OpenSotStream( OUString( "Pictures" ), STREAM_STD_READ );

            rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
            sal_uLong nDocLen = maDocHd.GetRecEndFilePos();
            DffRecordHeader aPPDGHd;
            if ( SeekToRec( rStCtrl, PPT_PST_PPDrawingGroup, nDocLen, &aPPDGHd ) )
            {
                sal_uLong nPPDGLen = aPPDGHd.GetRecEndFilePos();
                if ( SeekToRec( rStCtrl, DFF_msofbtDggContainer, nPPDGLen, NULL ) )
                    nDggContainerOfs = rStCtrl.Tell();
            }
            rStCtrl.Seek( nPosMerk );
        }
        sal_uInt32 nSvxMSDffOLEConvFlags2 = 0;

        const SvtFilterOptions& rBasOpt = SvtFilterOptions::Get();
        if ( rBasOpt.IsLoadPPointBasicCode() )
            mnFilterOptions |= 1;
        if ( rBasOpt.IsMathType2Math() )
            nSvxMSDffOLEConvFlags2 |= OLE_MATHTYPE_2_STARMATH;
        if ( rBasOpt.IsWinWord2Writer() )
            nSvxMSDffOLEConvFlags2 |= OLE_WINWORD_2_STARWRITER;
        if ( rBasOpt.IsExcel2Calc() )
            nSvxMSDffOLEConvFlags2 |= OLE_EXCEL_2_STARCALC;
        if ( rBasOpt.IsPowerPoint2Impress() )
            nSvxMSDffOLEConvFlags2 |= OLE_POWERPOINT_2_STARIMPRESS;

        InitSvxMSDffManager( nDggContainerOfs, pStData, nSvxMSDffOLEConvFlags2 );
        SetSvxMSDffSettings( SVXMSDFF_SETTINGS_CROP_BITMAPS
            | SVXMSDFF_SETTINGS_IMPORT_PPT );
        SetModel( mpDoc, 576 );
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Dtor
//
//////////////////////////////////////////////////////////////////////////

ImplSdPPTImport::~ImplSdPPTImport()
{
    delete pStData;
}

//////////////////////////////////////////////////////////////////////////
//
// Import
//
//////////////////////////////////////////////////////////////////////////

sal_Bool ImplSdPPTImport::Import()
{
    if ( !bOk )
        return sal_False;

    pSdrModel->setLock(true);
    pSdrModel->EnableUndo(false);

    SdrOutliner& rOutl = mpDoc->GetDrawOutliner();
    sal_uInt32 nControlWord = rOutl.GetEditEngine().GetControlWord();
    nControlWord |=  EE_CNTRL_ULSPACESUMMATION;
    nControlWord &=~ EE_CNTRL_ULSPACEFIRSTPARA;
    ((EditEngine&)rOutl.GetEditEngine()).SetControlWord( nControlWord );

    SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
    mnBackgroundLayerID = rAdmin.GetLayerID( String( SdResId( STR_LAYER_BCKGRND )), sal_False );
    mnBackgroundObjectsLayerID = rAdmin.GetLayerID( String( SdResId( STR_LAYER_BCKGRNDOBJ )), sal_False );

    ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
    if ( pDocShell )
        SeekOle( pDocShell, mnFilterOptions );

    // hyperlinks
    PropRead* pDInfoSec2 = new PropRead( mrStorage, OUString( "\005DocumentSummaryInformation" ) );
    if ( pDInfoSec2->IsValid() )
    {
        PropItem aPropItem;

        sal_uInt32 nType, nPropSize, nPropCount;

        pDInfoSec2->Read();

        sal_uInt8 aPropSetGUID[ 16 ] =
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
                    sal_uInt32 nSlideCount, nVecCount;
                    aPropItem >> nSlideCount;
                    if ( nSlideCount && pSection->GetProperty( PID_HEADINGPAIR, aPropItem ) )
                    {
                        sal_uInt32  nSlideTitleIndex = 0, nSlideTitleCount = 0;
                        sal_uInt32  i, nTemp;

                        OUString aUString;

                        aPropItem >> nType
                                  >> nVecCount;

                        if ( ( nType == ( VT_VARIANT | VT_VECTOR ) ) && ( nVecCount ^ 1 ) )
                        {
                            nVecCount >>= 1;
                            sal_uInt32 nEntryCount = 0;
                            for ( i = 0; i < nVecCount; i++ )
                            {
                                if ( !aPropItem.Read( aUString, VT_EMPTY, sal_False ) )
                                    break;
                                aPropItem >> nType;
                                if ( ( nType != VT_I4 ) && ( nType != VT_UI4 ) )
                                    break;
                                aPropItem >> nTemp;
                                if ( aUString == "Slide Titles" || aUString == "Folientitel" )
                                {
                                    nSlideTitleCount = nTemp;
                                    nSlideTitleIndex = nEntryCount;
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
                                    if ( !aPropItem.Read( aUString, nType, sal_False ) )
                                        break;

                                    OUString aString( aUString );
                                    if ( aString == "No Slide Title" )
                                        aString = OUString();
                                    else
                                    {
                                        std::vector<OUString>::const_iterator pIter =
                                                std::find(maSlideNameList.begin(),maSlideNameList.end(),aString);

                                        if (pIter != maSlideNameList.end())
                                            aString = OUString();
                                    }
                                    maSlideNameList.push_back( aString );
                                }
                            }
                        }
                    }
                }
            }

            sal_uInt8 aUserPropSetGUID[ 16 ] =
            {
                0x05, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
            };
            pSection = (Section*)pDInfoSec2->GetSection( aUserPropSetGUID );
            if ( pSection )
            {
                Dictionary aDict;
                if ( pSection->GetDictionary( aDict ) )
                {
                    Dictionary::const_iterator iter = aDict.find( OUString("_PID_HLINKS") );

                    if ( iter != aDict.end() )
                    {
                        if ( pSection->GetProperty( iter->second, aPropItem ) )
                        {
                            aPropItem.Seek( STREAM_SEEK_TO_BEGIN );
                            aPropItem >> nType;
                            if ( nType == VT_BLOB )
                            {
                                aPropItem >> nPropSize
                                          >> nPropCount;

                                if ( ! ( nPropCount % 6 ) )
                                {
                                    sal_uInt32 i;

                                    nPropCount /= 6;    // 6 propertys a hyperlink

                                    SdHyperlinkEntry* pHyperlink = 0;
                                    for ( i = 0; i < nPropCount; i++ )
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

                                        // Convert '\\' notation to 'smb://'
                                        INetURLObject aUrl( pHyperlink->aTarget, INET_PROT_FILE );
                                        pHyperlink->aTarget = aUrl.GetMainURL( INetURLObject::NO_DECODE );

                                        if ( !aPropItem.Read( pHyperlink->aSubAdress, VT_EMPTY ) )
                                            break;
                                        pHyperlink->nStartPos = pHyperlink->nEndPos = -1;

                                        if ( !pHyperlink->aSubAdress.isEmpty() ) // get the converted subadress
                                        {
                                            sal_uInt32 nPageNumber = 0;
                                            OUString aString( pHyperlink->aSubAdress );
                                            OString aStringAry[ 3 ];
                                            sal_uInt16 nTokenCount = comphelper::string::getTokenCount(aString, ',');
                                            if ( nTokenCount > 3 )
                                                nTokenCount = 3;
                                            sal_uInt16 nToken;
                                            for( nToken = 0; nToken < nTokenCount; nToken++ )
                                                aStringAry[nToken] = OUStringToOString(aString.getToken( nToken, (sal_Unicode)',' ), RTL_TEXTENCODING_UTF8);

                                            sal_Bool bSucceeded = sal_False;

                                            // first pass, searching for a SlideId
                                            for( nToken = 0; nToken < nTokenCount; nToken++ )
                                            {
                                                if (comphelper::string::isdigitAsciiString(aStringAry[nToken]))
                                                {
                                                    sal_Int32 nNumber = aStringAry[ nToken ].toInt32();
                                                    if ( nNumber & ~0xff )
                                                    {
                                                        PptSlidePersistList* pPageList = GetPageList( PPT_SLIDEPAGE );
                                                        if ( pPageList )
                                                        {
                                                            sal_uInt16 nPage = pPageList->FindPage( nNumber );
                                                            if ( nPage != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                                            {
                                                                nPageNumber = nPage;
                                                                bSucceeded = sal_True;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            if ( !bSucceeded )
                                            {   // second pass, searching for a SlideName
                                                for ( nToken = 0; nToken < nTokenCount; nToken++ )
                                                {
                                                    OUString aToken( aString.getToken( nToken, (sal_Unicode)',' ) );
                                                    std::vector<OUString>::const_iterator pIter =
                                                            std::find(maSlideNameList.begin(),maSlideNameList.end(),aToken);

                                                    if (pIter != maSlideNameList.end())
                                                    {
                                                        nPageNumber = pIter - maSlideNameList.begin();
                                                        bSucceeded = sal_True;
                                                    }
                                                }
                                            }
                                            if ( !bSucceeded )
                                            {   // third pass, searching for a slide number
                                                for ( nToken = 0; nToken < nTokenCount; nToken++ )
                                                {
                                                    if (comphelper::string::isdigitAsciiString(aStringAry[nToken]))
                                                    {
                                                        sal_Int32 nNumber = aStringAry[ nToken ].toInt32();
                                                        if ( ( nNumber & ~0xff ) == 0 )
                                                        {
                                                            nPageNumber = (sal_uInt32)nNumber - 1;
                                                            bSucceeded = sal_True;
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                            if ( bSucceeded )
                                            {
                                                if ( nPageNumber < maSlideNameList.size() )
                                                    pHyperlink->aConvSubString = maSlideNameList[ nPageNumber ];
                                                if ( pHyperlink->aConvSubString.isEmpty() )
                                                {
                                                    pHyperlink->aConvSubString = OUString( SdResId( STR_PAGE ) ) + " " + ( mpDoc->CreatePageNumValue( (sal_uInt16)nPageNumber + 1 ) );
                                                }
                                            }
                                        }
                                        aHyperList.push_back( pHyperlink );
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

    if ( mbDocumentFound )
    {
        rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
        // read hyperlist / set indices of the entries
        DffRecordHeader aHyperHd;
        if ( SeekToRec( rStCtrl, PPT_PST_ExObjList, maDocHd.GetRecEndFilePos(), &aHyperHd ) )
        {
            sal_uInt32 nExObjHyperListLen = aHyperHd.GetRecEndFilePos();
            for ( size_t i = 0, n = aHyperList.size(); i < n; ++i )
            {
                SdHyperlinkEntry* pPtr = aHyperList[ i ];
                DffRecordHeader aHyperE;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlink, nExObjHyperListLen, &aHyperE ) )
                    break;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlinkAtom, nExObjHyperListLen, NULL, 0 ) )
                    break;
                rStCtrl.SeekRel( 8 );
                rStCtrl >> pPtr->nIndex;
                aHyperE.SeekToEndOfRecord( rStCtrl );
            }
        }
    }

    Size aVisAreaSize;
    switch ( aUserEditAtom.eLastViewType )
    {
        case 5 :    // notes master
        case 3 :    // notes
            aVisAreaSize = aDocAtom.GetNotesPageSize();
        break;
        default :
            aVisAreaSize = aDocAtom.GetSlidesPageSize();
    }
    Scale( aVisAreaSize );
    pDocShell->SetVisArea( Rectangle( Point(), aVisAreaSize ) );

    ///////////////////////////////////////////////////////////
    // create master pages:
    ///////////////////////////////////////////////////////////
    SfxProgress* pStbMgr = new SfxProgress( pDocShell, String( SdResId( STR_POWERPOINT_IMPORT ) ),
            pMasterPages->size() + pSlidePages->size() + pNotePages->size() );

    sal_uInt32 nImportedPages = 0;
    {
        sal_uInt16          nMasterAnz = GetPageCount( PPT_MASTERPAGE );

        for ( sal_uInt16 nMasterNum = 0; nMasterNum < nMasterAnz; nMasterNum++ )
        {
            SetPageNum( nMasterNum, PPT_MASTERPAGE );
            SdPage* pPage = (SdPage*)MakeBlancPage( sal_True );
            if ( pPage )
            {
                sal_Bool bNotesMaster = (*GetPageList( eAktPageKind ) )[ nAktPageNum ]->bNotesMaster;
                sal_Bool bStarDrawFiller = (*GetPageList( eAktPageKind ) )[ nAktPageNum ]->bStarDrawFiller;

                PageKind ePgKind = ( bNotesMaster ) ? PK_NOTES : PK_STANDARD;
                sal_Bool bHandout = (*GetPageList( eAktPageKind ) )[ nAktPageNum ]->bHandoutMaster;
                if ( bHandout )
                    ePgKind = PK_HANDOUT;

                pPage->SetPageKind( ePgKind );
                pSdrModel->InsertMasterPage( (SdrPage*)pPage );
                if ( bNotesMaster && bStarDrawFiller )
                    ((SdPage*)pPage)->SetAutoLayout( AUTOLAYOUT_NOTES, sal_True );
                if ( nMasterNum )
                {
                    boost::optional< sal_Int16 > oStartNumbering;
                    SfxStyleSheet* pSheet;
                    if ( nMasterNum == 1 )
                    {
                        ///////////////////
                        // standardsheet //
                        ///////////////////
                        pSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_STANDARD_STYLESHEET_NAME), SD_STYLE_FAMILY_GRAPHICS );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                    }

                    // PSEUDO
                    pSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_PSEUDOSHEET_BACKGROUNDOBJECTS), SD_STYLE_FAMILY_PSEUDO );
                    if ( pSheet )
                    {
                        SfxItemSet& rItemSet = pSheet->GetItemSet();
                        PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                        PPTPortionObj aPortion( *pPPTStyleSheet, TSS_TYPE_TEXT_IN_SHAPE, 0 );
                        aParagraph.AppendPortion( aPortion );
                        aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                        aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                    }

                    ///////////////////////////////////////////////////////////
                    // create layoutstylesheets, set layoutname and stylesheet
                    // (only on standard and not pages)
                    ///////////////////////////////////////////////////////////
                    String aLayoutName( SdResId( STR_LAYOUT_DEFAULT_NAME ) );
                    if ( nMasterNum > 2 )
                    {
                        if ( ePgKind == PK_STANDARD )
                        {   // standard page: create new presentation layout
                            aLayoutName = String( SdResId( STR_LAYOUT_DEFAULT_TITLE_NAME ) );
                            aLayoutName += OUString::number( (sal_Int32)( ( nMasterNum + 1 ) / 2 - 1 ) );
                            ( (SdStyleSheetPool*)mpDoc->GetStyleSheetPool() )->CreateLayoutStyleSheets( aLayoutName );
                        }
                        else    // note page: use presentation layout of standard page
                            aLayoutName = ( (SdPage*)mpDoc->GetMasterPage( nMasterNum - 1 ) )->GetName();
                    }
                    pPage->SetName( aLayoutName );
                    aLayoutName.AppendAscii( SD_LT_SEPARATOR );
                    aLayoutName += String( SdResId( STR_LAYOUT_OUTLINE ) );
                    pPage->SetLayoutName( aLayoutName );

                    /////////////////////
                    // set stylesheets //
                    /////////////////////
                    if ( pPage->GetPageKind() == PK_STANDARD )
                    {
                        sal_uInt32 nTitleInstance = TSS_TYPE_PAGETITLE;
                        sal_uInt32 nOutlinerInstance = TSS_TYPE_BODY;
                        const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
                        sal_Bool bSwapStyleSheet = pSlideLayout->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE;
                        if ( bSwapStyleSheet )
                        {
                            nTitleInstance = TSS_TYPE_TITLE;
                            nOutlinerInstance = TSS_TYPE_SUBTITLE;
                        }
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
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                        ////////////////////////
                        // outlinerstylesheet //
                        ////////////////////////
                        sal_uInt16 nLevel;
                        PPTParagraphObj* pParagraphs[ 9 ];
                        PPTParagraphObj* pPreviousPara = NULL;

                        for ( nLevel = 0; nLevel < 9; nLevel++ )
                        {
                            OUString aName( pPage->GetLayoutName() );
                            aName += " ";
                            aName += OUString::number( nLevel + 1 );
                            SfxStyleSheet* pOutlineSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find( aName, SD_STYLE_FAMILY_MASTERPAGE );
                            DBG_ASSERT( pOutlineSheet, "Template for outline object not found" );
                            if ( pOutlineSheet )
                            {
                                pParagraphs[ nLevel ] = new PPTParagraphObj( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                SfxItemSet& rItemSet = pOutlineSheet->GetItemSet();
                                PPTPortionObj aPortion( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                pParagraphs[ nLevel ]->AppendPortion( aPortion );
                                pParagraphs[ nLevel ]->ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, pPreviousPara );
                                aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                                pPreviousPara = pParagraphs[ nLevel ];
                            }
                            else
                                pParagraphs[ nLevel ] = NULL;
                        }
                        for ( nLevel = 0; nLevel < 9; delete pParagraphs[ nLevel++ ] ) ;
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
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
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
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, 0xffffffff, NULL );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, 0xffffffff );
                        }
                    }
                }
            }
        }
    }
    SdPage* pMPage;
    sal_uInt16 i;
    for ( i = 0; i < mpDoc->GetMasterPageCount() && ( (pMPage = (SdPage*)mpDoc->GetMasterPage( i )) != 0 ); i++ )
    {
        SetPageNum( i, PPT_MASTERPAGE );
        /////////////////////////////////////////////
        // importing master page objects           //
        /////////////////////////////////////////////
        PptSlidePersistList* pList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pPersist = ( pList && ( nAktPageNum < pList->size() ) )
                                                    ? (*pList)[ nAktPageNum ] : NULL;
        if ( pPersist )
        {
            if ( pPersist->bStarDrawFiller && pPersist->bNotesMaster && ( nAktPageNum > 2 ) && ( ( nAktPageNum & 1 ) == 0 ) )
            {
                pSdrModel->DeleteMasterPage( nAktPageNum );
                SdrPage* pNotesClone = ((SdPage*)pSdrModel->GetMasterPage( 2 ))->Clone();
                pSdrModel->InsertMasterPage( pNotesClone, nAktPageNum );
                if ( pNotesClone )
                {
                    String aLayoutName( ((SdPage*)pSdrModel->GetMasterPage( nAktPageNum - 1 ))->GetLayoutName() );
                    ((SdPage*)pNotesClone)->SetPresentationLayout( aLayoutName, sal_False, sal_False, sal_False );
                    ((SdPage*)pNotesClone)->SetLayoutName( aLayoutName );
                }
            }
            else if ( pPersist->bStarDrawFiller == sal_False )
            {
                PptSlidePersistEntry* pE = pPersist;
                while( ( pE->aSlideAtom.nFlags & 4 ) && pE->aSlideAtom.nMasterId )
                {
                    sal_uInt16 nNextMaster = pMasterPages->FindPage( pE->aSlideAtom.nMasterId );
                    if ( nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                        break;
                    else
                        pE = (*pList)[ nNextMaster ];
                }
                SdrObject* pObj = ImportPageBackgroundObject( *pMPage, pE->nBackgroundOffset, sal_True );   // import background
                if ( pObj )
                    pMPage->NbcInsertObject( pObj );

                sal_Bool bNewAnimationsUsed = sal_False;
                ProcessData aProcessData( *(*pList)[ nAktPageNum ], (SdPage*)pMPage );
                sal_uInt32 nFPosMerk = rStCtrl.Tell();
                DffRecordHeader aPageHd;
                if ( SeekToAktPage( &aPageHd ) )
                {
                    while( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPageHd.GetRecEndFilePos() ) )
                    {
                        DffRecordHeader aHd;
                         rStCtrl >> aHd;
                        switch( aHd.nRecType )
                        {
                            case PPT_PST_PPDrawing :
                            {
                                aHd.SeekToBegOfRecord( rStCtrl );
                                DffRecordHeader aPPDrawHd;
                                if ( SeekToRec( rStCtrl, PPT_PST_PPDrawing, aHd.GetRecEndFilePos(), &aPPDrawHd ) )
                                {
                                    sal_uInt32 nPPDrawEnd = aPPDrawHd.GetRecEndFilePos();
                                    DffRecordHeader aEscherF002Hd;
                                    if ( SeekToRec( rStCtrl, DFF_msofbtDgContainer, nPPDrawEnd, &aEscherF002Hd ) )
                                    {
                                        sal_uInt32 nEscherF002End = aEscherF002Hd.GetRecEndFilePos();
                                        DffRecordHeader aEscherObjListHd;
                                        if ( SeekToRec( rStCtrl, DFF_msofbtSpgrContainer, nEscherF002End, &aEscherObjListHd ) )
                                        {
                                            sal_uInt32 nObjCount = 0;
                                            while( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aEscherObjListHd.GetRecEndFilePos() ) )
                                            {
                                                DffRecordHeader aHd2;
                                                rStCtrl >> aHd2;
                                                if ( ( aHd2.nRecType == DFF_msofbtSpContainer ) || ( aHd2.nRecType == DFF_msofbtSpgrContainer ) )
                                                {
                                                    if ( nObjCount++ )      // skipping the first object
                                                    {
                                                        Rectangle aEmpty;
                                                        aHd2.SeekToBegOfRecord( rStCtrl );
                                                        SdrObject* pImpObj = ImportObj( rStCtrl, (void*)&aProcessData, aEmpty, aEmpty );
                                                        if ( pImpObj )
                                                        {
                                                            pImpObj->SetLayer( mnBackgroundObjectsLayerID );
                                                            pMPage->NbcInsertObject( pImpObj );
                                                        }
                                                    }
                                                }
                                                aHd2.SeekToEndOfRecord( rStCtrl );
                                            }
                                        }
                                    }
                                }
                            }
                            break;

                            case PPT_PST_ProgTags :
                            {
                                DffRecordHeader aProgTagHd;
                                if ( SeekToContentOfProgTag( 10, rStCtrl, aPageHd, aProgTagHd ) )
                                {
                                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aProgTagHd.GetRecEndFilePos() ) )
                                    {
                                        DffRecordHeader aProgTagContentHd;
                                        rStCtrl >> aProgTagContentHd;
                                        switch( aProgTagContentHd.nRecType )
                                        {
                                            case DFF_msofbtAnimGroup :
                                            {
                                                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xPage( pMPage->getUnoPage(), ::com::sun::star::uno::UNO_QUERY );
                                                ppt::AnimationImporter aImporter( this, rStCtrl );
                                                bNewAnimationsUsed = aImporter.import( xPage, aProgTagContentHd ) > 0;
                                            }
                                            break;
                                        }
                                        aProgTagContentHd.SeekToEndOfRecord( rStCtrl );
                                    }
                                }
                            }
                            break;
                        }
                        aHd.SeekToEndOfRecord( rStCtrl );
                    }
                }
                rStCtrl.Seek( nFPosMerk );
                ImportPageEffect( (SdPage*)pMPage, bNewAnimationsUsed );

                ///////////////////////
                // background object //
                ///////////////////////
                pObj = pMPage->GetObj( 0 );
                if ( pObj && pObj->GetObjIdentifier() == OBJ_RECT )
                {
                    if ( pMPage->GetPageKind() == PK_STANDARD )
                    {
                        // transform data from imported background object to new form
                        // and delete the object. It was used as container to transport
                        // the attributes of the MasterPage background fill
                        SfxStyleSheet* pSheet = pMPage->GetStyleSheetForMasterPageBackground();

                        if(pSheet)
                        {
                            // if we have a StyleSheet (for Masterpages), set attributes there and use it
                            pSheet->GetItemSet().ClearItem();
                            pSheet->GetItemSet().Put(pObj->GetMergedItemSet());
                            pMPage->getSdrPageProperties().ClearItem();
                            pMPage->getSdrPageProperties().SetStyleSheet(pSheet);
                        }
                        else
                        {
                            // without StyleSheet, set attributes directly. This
                            // should not be done at all and is an error (will be asserted by SdrPage)
                            pMPage->getSdrPageProperties().ClearItem();
                            pMPage->getSdrPageProperties().PutItemSet(pObj->GetMergedItemSet());
                        }

                        pMPage->RemoveObject(pObj->GetOrdNum());
                        SdrObject::Free(pObj);
                    }
                }
            }
        }
        if( pStbMgr )
            pStbMgr->SetState( nImportedPages++ );
    }
    ////////////////////////////////////
    // importing slide pages          //
    ////////////////////////////////////
    {
        sal_uInt32          nFPosMerk = rStCtrl.Tell();
        PptPageKind     ePageKind = eAktPageKind;
        sal_uInt16          nPageNum = nAktPageNum;

        SdPage* pHandoutPage = (SdPage*)MakeBlancPage( sal_False );
        pHandoutPage->SetPageKind( PK_HANDOUT );
        pSdrModel->InsertPage( pHandoutPage );

        sal_uInt16 nPageAnz = GetPageCount( PPT_SLIDEPAGE );
        if ( nPageAnz )
        {
            for ( sal_uInt16 nPage = 0; nPage < nPageAnz; nPage++ )
            {
                sal_Bool bNewAnimationsUsed = sal_False;

                mePresChange = PRESCHANGE_SEMIAUTO;
                SetPageNum( nPage, PPT_SLIDEPAGE );
                SdPage* pPage = (SdPage*)MakeBlancPage( sal_False );
                PptSlidePersistEntry* pMasterPersist = NULL;
                if ( HasMasterPage( nPage, PPT_SLIDEPAGE ) )     // try to get the LayoutName from the masterpage
                {
                    sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                    pPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nMasterNum));
                    PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                    if ( pPageList && nMasterNum < pPageList->size() )
                        pMasterPersist = (*pPageList)[ nMasterNum ];
                    pPage->SetLayoutName(((SdPage&)pPage->TRG_GetMasterPage()).GetLayoutName());
                }
                pPage->SetPageKind( PK_STANDARD );
                pSdrModel->InsertPage( pPage );         // SJ: #i29625# because of form controls, the
                ImportPage( pPage, pMasterPersist );    //  page must be inserted before importing
                SetHeaderFooterPageSettings( pPage, pMasterPersist );
                // CWS preseng01: pPage->SetPageKind( PK_STANDARD );

                DffRecordHeader aPageHd;
                if ( SeekToAktPage( &aPageHd ) )
                {
                    aPageHd.SeekToContent( rStCtrl );
                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPageHd.GetRecEndFilePos() ) )
                    {
                        DffRecordHeader aHd;
                        rStCtrl >> aHd;
                        switch ( aHd.nRecType )
                        {
                            case PPT_PST_ProgTags :
                            {
                                DffRecordHeader aProgTagHd;
                                if ( SeekToContentOfProgTag( 10, rStCtrl, aPageHd, aProgTagHd ) )
                                {
                                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aProgTagHd.GetRecEndFilePos() ) )
                                    {
                                        DffRecordHeader aProgTagContentHd;
                                        rStCtrl >> aProgTagContentHd;
                                        switch( aProgTagContentHd.nRecType )
                                        {
                                            case DFF_msofbtAnimGroup :
                                            {
                                                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xPage( pPage->getUnoPage(), ::com::sun::star::uno::UNO_QUERY );
                                                ppt::AnimationImporter aImporter( this, rStCtrl );
                                                bNewAnimationsUsed = aImporter.import( xPage, aProgTagContentHd ) > 0;
                                            }
                                            break;

                                            case PPT_PST_NewlyAddedAtomByXP11008 :  // ???
                                            break;

                                            case PPT_PST_NewlyAddedAtomByXP12011 :  // ??? don't know, this atom is always 8 bytes big
                                            break;                                  // and is appearing in nearly every l10 progtag
                                        }
                                        aProgTagContentHd.SeekToEndOfRecord( rStCtrl );
                                    }
                                }
                            }
                            break;

                            case PPT_PST_HeadersFooters :
                            case PPT_PST_PPDrawing :
                            default:
                            break;
                        }

                        aHd.SeekToEndOfRecord( rStCtrl );
                    }
                    ImportPageEffect( (SdPage*)pPage, bNewAnimationsUsed );
                }

                // creating the corresponding note page
                eAktPageKind = PPT_NOTEPAGE;
                SdPage* pNotesPage = (SdPage*)MakeBlancPage( sal_False );
                sal_uInt16 nNotesMasterNum = GetMasterPageIndex( nPage, PPT_SLIDEPAGE ) + 1;
                sal_uInt32 nNotesPageId = GetNotesPageId( nPage );
                if ( nNotesPageId )
                {
                    nImportedPages++;
                    sal_uInt16 nNotesPageIndex = pNotePages->FindPage( nNotesPageId );
                    if ( nNotesPageIndex == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                        nNotesPageIndex = 0;
                    SetPageNum( nNotesPageIndex, PPT_NOTEPAGE );
                    PptSlidePersistEntry* pMasterPersist2 = NULL;
                    if ( HasMasterPage( nNotesPageIndex, PPT_NOTEPAGE ) ) // try to get the LayoutName from the masterpage
                    {
                        pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                        if ( pPageList && nNotesMasterNum < pPageList->size() )
                            pMasterPersist2 = (*pPageList)[ nNotesMasterNum ];
                        pNotesPage->SetLayoutName( ((SdPage&)pNotesPage->TRG_GetMasterPage()).GetLayoutName() );
                    }
                    pNotesPage->SetPageKind( PK_NOTES );
                    pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                    pSdrModel->InsertPage( pNotesPage );        // SJ: #i29625# because of form controls, the
                    ImportPage( pNotesPage, pMasterPersist2 );  // page must be inserted before importing
                    SetHeaderFooterPageSettings( pNotesPage, pMasterPersist2 );
                    pNotesPage->SetAutoLayout( AUTOLAYOUT_NOTES, sal_False );
                }
                else
                {
                    pNotesPage->SetPageKind( PK_NOTES );
                    pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                    pNotesPage->SetAutoLayout( AUTOLAYOUT_NOTES, sal_True );
                    pSdrModel->InsertPage( pNotesPage );
                    SdrObject* pPageObj = pNotesPage->GetPresObj( PRESOBJ_PAGE, 1 );
                    if ( pPageObj )
                        ((SdrPageObj*)pPageObj)->SetReferencedPage(pSdrModel->GetPage(( nPage << 1 ) + 1));
                }

                if( pStbMgr )
                    pStbMgr->SetState( nImportedPages++ );
            }
        }
        else
        {
            // that can happen by document templates
            eAktPageKind = PPT_SLIDEPAGE;
            SdrPage* pPage = MakeBlancPage( sal_False );
            pSdrModel->InsertPage( pPage );

            // #i37397#, trying to set the title master for the first page
            sal_uInt16 nMaster, nMasterCount = pSdrModel->GetMasterPageCount();
            SdPage* pFoundMaster = NULL;
            for ( nMaster = 1; nMaster < nMasterCount; nMaster++ )
            {
                SdPage* pMaster = static_cast<SdPage*>( pSdrModel->GetMasterPage( nMaster ) );
                if ( pMaster->GetPageKind() == PK_STANDARD )
                {
                    SetPageNum( nMaster, PPT_MASTERPAGE );
                    if ( !pFoundMaster )
                        pFoundMaster = pMaster;
                    else if ( GetSlideLayoutAtom()->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE )
                        pFoundMaster = pMaster;
                    if ( GetSlideLayoutAtom()->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE )
                        break;
                }
            }
            if ( pFoundMaster )
            {
                ((SdPage*)pPage)->TRG_SetMasterPage( *((SdPage*)pFoundMaster) );
                ((SdPage*)pPage)->SetLayoutName( ((SdPage*)pFoundMaster)->GetLayoutName() );
            }
            ((SdPage*)pPage)->SetAutoLayout( AUTOLAYOUT_TITLE, sal_True, sal_True );

            eAktPageKind = PPT_NOTEPAGE;
            SdrPage* pNPage = MakeBlancPage( sal_False );
            pSdrModel->InsertPage( pNPage );
        }
        SetPageNum( nPageNum, ePageKind );
        rStCtrl.Seek( nFPosMerk );
    }
    ///////////////////////////////////////////////////////////////////
    // create handout and note pages                                 //
    ///////////////////////////////////////////////////////////////////
    bOk = mpDoc->CreateMissingNotesAndHandoutPages();
    if ( bOk )
    {
        for ( i = 0; i < mpDoc->GetSdPageCount( PK_STANDARD ); i++ )
        {
            ////////////////////
            // set AutoLayout //
            ////////////////////
            SetPageNum( i, PPT_SLIDEPAGE );
            SdPage* pPage = mpDoc->GetSdPage( i, PK_STANDARD );
            AutoLayout eAutoLayout = AUTOLAYOUT_NONE;
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout )
            {
                switch ( pSlideLayout->eLayout )            // presentation layout for standard pages
                {
                    case PPT_LAYOUT_TITLEANDBODYSLIDE :
                    {
                        eAutoLayout = AUTOLAYOUT_ENUM;
                        sal_uInt16 nID1 = pSlideLayout->aPlaceholderId[ 1 ];
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
                        sal_uInt16 nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        sal_uInt16 nID2 = pSlideLayout->aPlaceholderId[ 2 ];
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
                        else if ( ( nID1 == PPT_PLACEHOLDER_OBJECT ) && ( nID2 == PPT_PLACEHOLDER_OBJECT  ) )
                            eAutoLayout = AUTOLAYOUT_OBJ;
                    }
                    break;

                    case PPT_LAYOUT_2ROWSANDTITLE :
                    {
                        eAutoLayout = AUTOLAYOUT_2TEXT;
                        sal_uInt16 nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        sal_uInt16 nID2 = pSlideLayout->aPlaceholderId[ 2 ];
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
                    case PPT_LAYOUT_MASTERSLIDE :           // layout of the standard and title master page
                    case PPT_LAYOUT_TITLEMASTERSLIDE :
                    case PPT_LAYOUT_MASTERNOTES :           // layout of the note master page
                    case PPT_LAYOUT_NOTESTITLEBODY :        // presentation layout for note pages
                    case PPT_LAYOUT_HANDOUTLAYOUT :         // presentation layout for handout
                        eAutoLayout = AUTOLAYOUT_NONE;
                    break;
                }
                if ( eAutoLayout != AUTOLAYOUT_NONE )
                    pPage->SetAutoLayout( eAutoLayout, sal_False );
            }
        }
        //////////////////////////////////////////////////////////////
        // handout master page: auto layout                         //
        //////////////////////////////////////////////////////////////
        SdPage* pHandoutMPage = mpDoc->GetMasterSdPage( 0, PK_HANDOUT );
        pHandoutMPage->SetAutoLayout( AUTOLAYOUT_HANDOUT6, sal_True, sal_True );
    }

    sal_uInt32 nSlideCount = GetPageCount();
    for ( i = 0; ( i < nSlideCount) && ( i < maSlideNameList.size() ); i++ )
    {
        SdPage* pPage = mpDoc->GetSdPage( i, PK_STANDARD );
        OUString &aName = maSlideNameList[ i ];
        if ( pPage )
        {
            if ( !aName.isEmpty() )
                pPage->SetName( aName );
            else
                aName = pPage->GetName();
        }
    }
    if ( mbDocumentFound )
    {
        mpDoc->SetSummationOfParagraphs( sal_True );
        if ( pDocShell )
        {
            ::sd::FrameView* pFrameView = mpDoc->GetFrameView( 0 );
            if ( !pFrameView )
            {
                std::vector<sd::FrameView*> &rViews = mpDoc->GetFrameViewList();
                pFrameView = new ::sd::FrameView( mpDoc );
                rViews.push_back( pFrameView );
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
                        SfxItemSet* pSet = mrMed.GetItemSet();
                        if ( pSet )
                            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 3 ) );
                    }
                    break;
                    case 8 :    // slide sorter
                    {
                        SfxItemSet* pSet = mrMed.GetItemSet();
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
        // read and set custom show
        rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
        if ( SeekToRec( rStCtrl, PPT_PST_NamedShows, maDocHd.GetRecEndFilePos(), &aCustomShowHeader ) )
        {
            DffRecordHeader aCuHeader;
            while( SeekToRec( rStCtrl, PPT_PST_NamedShow, aCustomShowHeader.GetRecEndFilePos(), &aCuHeader ) )
            {
                DffRecordHeader aContent;
                if ( SeekToRec( rStCtrl, PPT_PST_CString, aCuHeader.GetRecEndFilePos(), &aContent ) )
                {
                    OUString aCuShow;
                    aContent.SeekToBegOfRecord( rStCtrl );
                    if ( ReadString( aCuShow ) )
                    {
                        if ( SeekToRec( rStCtrl, PPT_PST_NamedShowSlides, aCuHeader.GetRecEndFilePos(), &aContent ) )
                        {
                            PptSlidePersistList* pPageList = GetPageList( PPT_SLIDEPAGE );
                            sal_uInt32 nSCount = aContent.nRecLen >> 2;
                            if ( pPageList && nSCount )
                            {
                                SdCustomShowList* pList = mpDoc->GetCustomShowList( sal_True );
                                if ( pList )
                                {
                                    SdCustomShow* pSdCustomShow = new SdCustomShow( mpDoc );
                                    if ( pSdCustomShow )
                                    {
                                        pSdCustomShow->SetName( aCuShow );
                                        sal_uInt32 nFound = 0;
                                        for ( sal_uInt32 nS = 0; nS < nSCount; nS++ )
                                        {
                                            sal_uInt32 nPageNumber;
                                            rStCtrl >> nPageNumber;
                                            sal_uInt16 nPage = pPageList->FindPage( nPageNumber );
                                            if ( nPage != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                            {
                                                SdPage* pPage = mpDoc->GetSdPage( nPage, PK_STANDARD );
                                                if ( pPage )
                                                {
                                                    pSdCustomShow->PagesVector().push_back( pPage );
                                                    nFound++;
                                                }
                                            }
                                        }
                                        if ( nFound )
                                            pList->push_back( pSdCustomShow );
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
        sal_uInt32  nFlags = 1;                 // Bit 0:   Auto advance
        sal_uInt16  nStartSlide = 0;

        // read the pres. configuration
        rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
        if ( SeekToRec( rStCtrl, PPT_PST_SSDocInfoAtom, maDocHd.GetRecEndFilePos(), &aCustomShowHeader ) )
        {
            sal_uInt32  nPenColor = 0x1000000;
            sal_Int32   nRestartTime = 0x7fffffff;
            sal_Int16   nEndSlide = 0;
            rStCtrl >> nPenColor
                    >> nRestartTime
                    >> nStartSlide
                    >> nEndSlide;

            sal_Unicode nChar;
            for ( sal_uInt32 i2 = 0; i2 < 32; i2++ )
            {
                rStCtrl >> nChar;
                if ( nChar )
                    aCustomShow.Append( nChar );
                else
                {
                    rStCtrl.SeekRel( ( 31 - i2 ) << 1 );
                    break;
                }
            }
            rStCtrl >> nFlags;
        }
        // set the current custom show
        if ( aCustomShow.Len() )
        {
            SdCustomShowList* pList = mpDoc->GetCustomShowList( sal_False );
            if ( pList )
            {
                SdCustomShow* pPtr = NULL;
                for( pPtr = pList->First(); pPtr; pPtr = pList->Next() )
                {
                    if ( pPtr->GetName() == aCustomShow )
                        break;
                }
                if ( !pPtr )
                    pList->First();
            }
        }
        sd::PresentationSettings& rPresSettings = mpDoc->getPresentationSettings();

        rPresSettings.mbManual = ( nFlags & 1 ) == 0;
        rPresSettings.mbAnimationAllowed = ( nFlags & 2 ) == 0;
        rPresSettings.mbAll = ( nFlags & 4 ) == 0;
        rPresSettings.mbCustomShow = ( nFlags & 8 ) != 0;
        rPresSettings.mbEndless = ( nFlags & 0x80 ) != 0;
        rPresSettings.mbFullScreen = ( nFlags & 0x10 ) == 0;

        if ( nStartSlide && ( nStartSlide <= GetPageCount() ) )
        {
            SdPage* pPage = mpDoc->GetSdPage( nStartSlide - 1, PK_STANDARD );
            if ( pPage )
                rPresSettings.maPresPage = pPage->GetName();
        }
    }

    delete pStbMgr;

    // read DocumentProperties
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mpDoc->GetObjectShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps
        = xDPS->getDocumentProperties();
    sfx2::LoadOlePropertySet(xDocProps, &mrStorage);
    xDocProps->setTemplateName(OUString());

    pSdrModel->setLock(false);
    pSdrModel->EnableUndo(true);
    return bOk;
}

void ImplSdPPTImport::SetHeaderFooterPageSettings( SdPage* pPage, const PptSlidePersistEntry* pMasterPersist )
{
    sal_uInt32 i;
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( ( !pList ) || ( pList->size() <= nAktPageNum ) )
        return;
    PptSlidePersistEntry& rSlidePersist = *(*pList)[ nAktPageNum ];
    HeaderFooterEntry* pHFE = rSlidePersist.pHeaderFooterEntry;
    if ( pHFE )
    {
        for ( i = 0; i < 4; i++ )
        {
            bool bVisible = pHFE->IsToDisplay( i );
            if ( ( eAktPageKind == PPT_SLIDEPAGE )
                && ( rSlidePersist.aSlideAtom.aLayout.eLayout == PPT_LAYOUT_TITLESLIDE )
                    && ( aDocAtom.bTitlePlaceholdersOmitted == sal_True ) )
            {
                bVisible = sal_False;
            }
            if ( bVisible && pMasterPersist )
            {
                sal_uInt32 nPosition = pHFE->NeedToImportInstance( i, rSlidePersist );
                if ( nPosition )
                {
                    Rectangle aEmpty;
                    bVisible = sal_False;
                    rStCtrl.Seek( nPosition );
                    ProcessData aProcessData( rSlidePersist, (SdPage*)pPage );
                    SdrObject* pObj = ImportObj( rStCtrl, (void*)&aProcessData, aEmpty, aEmpty );
                    if ( pObj )
                        pPage->NbcInsertObject( pObj, 0 );
                }
            }
            String aPlaceHolderString = pHFE->pPlaceholder[ i ];

            sd::HeaderFooterSettings rHeaderFooterSettings( pPage->getHeaderFooterSettings() );
            switch( i )
            {
                case 0 :
                {
                    rHeaderFooterSettings.mbDateTimeVisible = bVisible;
                    rHeaderFooterSettings.mbDateTimeIsFixed = ( pHFE->nAtom & 0x20000 ) == 0;
                    rHeaderFooterSettings.maDateTimeText = aPlaceHolderString;
                    SvxDateFormat eDateFormat;
                    SvxTimeFormat eTimeFormat;
                    PPTFieldEntry::GetDateTime( pHFE->nAtom & 0xff, eDateFormat, eTimeFormat );
                    rHeaderFooterSettings.meDateTimeFormat = eDateFormat | ( eTimeFormat << 4 );
                }
                break;
                case 1 :
                {
                    rHeaderFooterSettings.mbHeaderVisible = bVisible;
                    rHeaderFooterSettings.maHeaderText = aPlaceHolderString;
                }
                break;
                case 2 :
                {
                    rHeaderFooterSettings.mbFooterVisible = bVisible;
                    rHeaderFooterSettings.maFooterText = aPlaceHolderString;
                }
                break;
                case 3 :
                {
                    rHeaderFooterSettings.mbSlideNumberVisible = bVisible;
                }
                break;
            }
            pPage->setHeaderFooterSettings( rHeaderFooterSettings );
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Import of pages
//
//////////////////////////////////////////////////////////////////////////

struct Ppt97AnimationStlSortHelper
{
    bool operator()( const std::pair< SdrObject*, Ppt97AnimationPtr >& p1, const std::pair< SdrObject*, Ppt97AnimationPtr >& p2 );
};

bool Ppt97AnimationStlSortHelper::operator()( const std::pair< SdrObject*, Ppt97AnimationPtr >& p1, const std::pair< SdrObject*, Ppt97AnimationPtr >& p2 )
{
    if( !p1.second.get() || !p2.second.get() )
        return true;
    if( *p1.second < *p2.second )
        return true;
    if( *p1.second > *p2.second )
        return false;
    if( p1.first->GetOrdNum() < p2.first->GetOrdNum() )
        return true;
    return false;
}

void ImplSdPPTImport::ImportPageEffect( SdPage* pPage, const sal_Bool bNewAnimationsUsed )
{
    sal_uLong nFilePosMerk = rStCtrl.Tell();

    // set PageKind at page (up to now only PK_STANDARD or PK_NOTES)
    if ( pPage->GetPageKind() == PK_STANDARD )
    {
        PptSlidePersistList* pPersistList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pActualSlidePersist = ( pPersistList && ( nAktPageNum < pPersistList->size() ) )
                                                        ? (*pPersistList)[ nAktPageNum ] : NULL;

        if ( pActualSlidePersist && ( eAktPageKind == PPT_SLIDEPAGE ) )
        {
            if ( ! ( pActualSlidePersist->aSlideAtom.nFlags & 1 ) ) // do not follow master objects ?
            {
                if(pPage->TRG_HasMasterPage())
                {
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(mnBackgroundObjectsLayerID, sal_False);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                }
            }
        }
        DffRecordHeader aPageRecHd;
        if ( pPage && SeekToAktPage( &aPageRecHd ) )
        {
            sal_uLong nPageRecEnd = aPageRecHd.GetRecEndFilePos();

            sal_Bool bTryTwice = ( eAktPageKind == PPT_SLIDEPAGE );
            sal_Bool bSSSlideInfoAtom = sal_False;
            while ( true )
            {
                while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPageRecEnd ) )
                {
                    DffRecordHeader aHd;
                    rStCtrl >> aHd;
                    switch ( aHd.nRecType )
                    {
                        case PPT_PST_SSSlideInfoAtom:
                        {
                            bSSSlideInfoAtom = sal_True;
                            if ( eAktPageKind == PPT_MASTERPAGE )
                            {
                                if ( pActualSlidePersist )
                                    pActualSlidePersist->aPersistAtom.nReserved = aHd.GetRecBegFilePos();
                            }
                            else
                            {
                                sal_Int8    nDirection, nTransitionType, nByteDummy, nSpeed;
                                sal_Int16   nBuildFlags;
                                sal_Int32   nSlideTime, nSoundRef;
                                rStCtrl >> nSlideTime           // time to show (in Ticks)
                                        >> nSoundRef            // Index of SoundCollection
                                        >> nDirection           // direction of fade effect
                                        >> nTransitionType      // fade effect
                                        >> nBuildFlags          // Buildflags (s.u.)
                                        >> nSpeed               // speed (slow, medium, fast)
                                        >> nByteDummy >> nByteDummy >> nByteDummy;

                                switch ( nTransitionType )
                                {
                                    case PPT_TRANSITION_TYPE_BLINDS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_STRIPES );        // fade vertical
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_STRIPES );      // fade horizontal
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_CHECKER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_CHECKERBOARD ); // fade vertical with offset ??
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_CHECKERBOARD );   // fade horizontal with offset ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_COVER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_RIGHT );         // overlay from right
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_BOTTOM );        // overlay from bottom
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LEFT );          // overlay from left
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_TOP );           // overlay from top
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERRIGHT );    // overlay from bottom right ??
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERLEFT );     // overlay from bottom left ??
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERRIGHT );    // overlay from top right
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERLEFT );     // overlay from top left ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_NONE :
                                    {
                                        if ( nBuildFlags )
                                        {
                                            if ( nDirection == 0 )
                                                pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_NONE );                // direct
                                            else if ( nDirection == 1 )
                                            {
                                                pPage->setTransitionType( animations::TransitionType::BARWIPE );
                                                pPage->setTransitionSubtype( animations::TransitionSubType::FADEOVERCOLOR );
                                                pPage->setTransitionFadeColor( 0 );
                                            }
                                        }
                                        else
                                            pPage->setTransitionType( 0 );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_DISSOLVE :
                                        pPage->SetFadeEffect(::com::sun::star::presentation::FadeEffect_DISSOLVE);                      // dissolve
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM_BARS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_HORIZONTAL_LINES );        // horizontal lines
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_VERTICAL_LINES );          // vertical lines
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_SPLIT :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_OPEN_VERTICAL );           // open horizontal ??
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_CLOSE_VERTICAL );          // close horizontal ??
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_OPEN_HORIZONTAL );         // open vertical ??
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_CLOSE_HORIZONTAL );        // close vertical ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_STRIPS :
                                    {
                                        if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERRIGHT );    // diagonal to top left
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERLEFT );     // diagonal to top right
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERRIGHT );    // diagonal to bottom left
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERLEFT );     // diagonal to bottom right
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_PULL :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LEFT );         // uncover to left
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_TOP );          // uncover to top
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_RIGHT );        // uncover to right
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_BOTTOM );       // uncover to bottom
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERLEFT );    // uncover to top left
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT );   // uncover to top right
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERLEFT );    // uncover to bottom left
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT );   // uncover to bottom right
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_WIPE :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_RIGHT );         // roll from right
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_BOTTOM );        // roll from bottom
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_LEFT );          // roll from left
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_TOP );           // roll from top
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM :
                                        pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_RANDOM );                      // automatic
                                    break;
                                    case PPT_TRANSITION_TYPE_FADE :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::FADE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::FADEOVERCOLOR );
                                        pPage->setTransitionFadeColor( 0 );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_ZOOM :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_FROM_CENTER );        // fade from center
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( ::com::sun::star::presentation::FadeEffect_FADE_TO_CENTER );          // fade from the outside
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_DIAMOND :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::IRISWIPE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::DIAMOND );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_PLUS :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::FOURBOXWIPE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::CORNERSOUT );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_CIRCLE :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::ELLIPSEWIPE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::CIRCLE );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_WEDGE :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::FANWIPE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::CENTERTOP );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_WHEEL :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::PINWHEELWIPE );
                                        sal_Int16 nSubType;
                                        switch( nDirection )
                                        {
                                            default:
                                            case 1 : nSubType = animations::TransitionSubType::ONEBLADE; break;
                                            case 2 : nSubType = animations::TransitionSubType::TWOBLADEVERTICAL; break;
                                            case 3 : nSubType = animations::TransitionSubType::THREEBLADE; break;
                                            case 4 : nSubType = animations::TransitionSubType::FOURBLADE; break;
                                            case 8 : nSubType = animations::TransitionSubType::EIGHTBLADE; break;
                                        }
                                        pPage->setTransitionSubtype( nSubType );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_PUSH :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::PUSHWIPE );
                                        sal_Int16 nSubType;
                                        switch( nDirection )
                                        {
                                            default:
                                            case 0 : nSubType = animations::TransitionSubType::FROMRIGHT; break;
                                            case 1 : nSubType = animations::TransitionSubType::FROMBOTTOM; break;
                                            case 2 : nSubType = animations::TransitionSubType::FROMLEFT; break;
                                            case 3 : nSubType = animations::TransitionSubType::FROMTOP; break;
                                        }
                                        pPage->setTransitionSubtype( nSubType );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_COMB :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::PUSHWIPE );
                                        pPage->setTransitionSubtype( nDirection ? animations::TransitionSubType::COMBVERTICAL : animations::TransitionSubType::COMBHORIZONTAL );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_NEWSFLASH :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::ZOOM );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::ROTATEIN );
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_SMOOTHFADE :
                                    {
                                        pPage->setTransitionType( animations::TransitionType::FADE );
                                        pPage->setTransitionSubtype( animations::TransitionSubType::CROSSFADE );
                                    }
                                    break;
                                }

                                if ( nSpeed == 0 )
                                    pPage->setTransitionDuration( 3.0 );    // slow
                                else if ( nSpeed == 1 )
                                    pPage->setTransitionDuration( 2.0 );    // medium
                                else if ( nSpeed == 2 )
                                    pPage->setTransitionDuration( 1.0 );    // fast

                                if ( nBuildFlags & 0x400 )                      // slidechange by time
                                {   // time to show (in Ticks)
                                    pPage->SetPresChange( PRESCHANGE_AUTO );
                                    pPage->SetTime( nSlideTime / 1000.0 );
                                }
                                else
                                    pPage->SetPresChange( mePresChange );

                                if ( nBuildFlags & 4 )
                                    pPage->SetExcluded( sal_True );             // don't show slide
                                if ( nBuildFlags & 16 )
                                {   // slide with sound effect
                                    pPage->SetSound( sal_True );
                                    String aSoundFile( ReadSound( nSoundRef ) );
                                    pPage->SetSoundFile( aSoundFile );
                                }
                                if ( nBuildFlags & ( 1 << 6 ) )     // Loop until next sound
                                    pPage->SetLoopSound( sal_True );
                                if ( nBuildFlags & ( 1 << 8 ) )     // Stop the previous sound
                                    pPage->SetStopSound( sal_True );
                                break;
                            }
                        }
                    }
                    aHd.SeekToEndOfRecord( rStCtrl );
                }
                if ( bTryTwice && ( bSSSlideInfoAtom == sal_False ) )
                {
                    bTryTwice = sal_False;
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                        if ( pPageList && ( nMasterNum < pPageList->size() ) )
                        {
                            PptSlidePersistEntry* pE = (*pPageList)[ nMasterNum ];
                            if ( pE )
                            {
                                sal_uInt32 nOfs = pE->aPersistAtom.nReserved;
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

    if ( !bNewAnimationsUsed )
    {
        tAnimationVector aAnimationsOnThisPage;

        // add effects from page in correct order
        SdrObjListIter aSdrIter( *pPage, IM_FLAT );
        while ( aSdrIter.IsMore() )
        {
            SdrObject* pObj = aSdrIter.Next();
            tAnimationMap::iterator aFound = maAnimations.find( pObj );
            if( aFound != maAnimations.end() )
            {
                std::pair< SdrObject*, Ppt97AnimationPtr > aPair( (*aFound).first, (*aFound).second );
                aAnimationsOnThisPage.push_back( aPair );
            }
        }

        Ppt97AnimationStlSortHelper aSortHelper;
        std::sort( aAnimationsOnThisPage.begin(), aAnimationsOnThisPage.end(), aSortHelper );

        tAnimationVector::iterator aIter( aAnimationsOnThisPage.begin() );
        const tAnimationVector::iterator aEnd( aAnimationsOnThisPage.end() );

        for( ;aIter != aEnd; ++aIter )
        {
            Ppt97AnimationPtr pPpt97Animation = (*aIter).second;;
            if( pPpt97Animation.get() )
                pPpt97Animation->createAndSetCustomAnimationEffect( (*aIter).first );
        }
    }
    rStCtrl.Seek( nFilePosMerk );
}

//////////////////////////////////////////////////////////////////////////
//
// import of sounds
//
// Not only the sounds are imported as string, they are also inserted to
// the gallery if they are not already there.
//
///////////////////////////////////////////////////////////////////////////

String ImplSdPPTImport::ReadSound(sal_uInt32 nSoundRef) const
{
    OUString aRetval;
    sal_uInt32 nPosMerk = rStCtrl.Tell();
    DffRecordHeader aDocHd;
    if ( SeekToDocument( &aDocHd ) )
    {
        sal_uInt32 nSoundLen = aDocHd.GetRecEndFilePos();
        DffRecordHeader aSoundBlockRecHd;
        if( SeekToRec( rStCtrl, PPT_PST_SoundCollection, nSoundLen, &aSoundBlockRecHd ) )
        {
            sal_uInt32 nDataLen = aSoundBlockRecHd.GetRecEndFilePos();
            DffRecordHeader aSoundRecHd;
            sal_Bool bRefStrValid = sal_False;
            sal_Bool bDone = sal_False;

            while( !bDone && SeekToRec( rStCtrl, PPT_PST_Sound, nDataLen, &aSoundRecHd ) )
            {
                sal_uInt32 nStrLen = aSoundRecHd.GetRecEndFilePos();
                OUString aRefStr;
                sal_uInt32 nPosMerk2 = rStCtrl.Tell();
                if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen, NULL, 2 ) )
                {
                    if ( ReadString( aRefStr ) )
                        bRefStrValid = sal_True;
                }
                if ( bRefStrValid )
                {
                    if ( OUString::number(nSoundRef) == aRefStr )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen, NULL, 0 ) )
                        {
                            ReadString( aRetval );
                            bDone = sal_True;
                        }
                    }
                }
                if ( bDone )
                {
                    // Check if this sound file already exists.
                    // If not, it is exported to our local sound directory.
                    sal_Bool    bSoundExists = sal_False;
                    ::std::vector< String > aSoundList;

                    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, aSoundList );
                    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, aSoundList );

                    for( size_t n = 0; ( n < aSoundList.size() ) && !bSoundExists; ++n )
                    {
                        INetURLObject   aURL( aSoundList[ n ] );

                        if( aURL.GetName() == aRetval )
                        {
                            aRetval = aSoundList[ n ];
                            bSoundExists = sal_True;
                        }
                    }

                    aSoundList.clear();

                    if ( !bSoundExists )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        DffRecordHeader aSoundDataRecHd;
                        if ( SeekToRec( rStCtrl, PPT_PST_SoundData, nStrLen, &aSoundDataRecHd, 0 ) )
                        {
                            String          aGalleryDir( SvtPathOptions().GetGalleryPath() );
                            INetURLObject   aGalleryUserSound( aGalleryDir.GetToken( comphelper::string::getTokenCount(aGalleryDir, ';') - 1 ) );

                            aGalleryUserSound.Append( aRetval );
                            sal_uInt32 nSoundDataLen = aSoundDataRecHd.nRecLen;
                            sal_uInt8* pBuf = new sal_uInt8[ nSoundDataLen ];

                            rStCtrl.Read( pBuf, nSoundDataLen );
                            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aGalleryUserSound.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

                            if( pOStm )
                            {
                                pOStm->Write( pBuf, nSoundDataLen );

                                if( pOStm->GetError() == ERRCODE_NONE )
                                {
                                    GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aGalleryUserSound.GetMainURL( INetURLObject::NO_DECODE ) );
                                    aRetval = aGalleryUserSound.GetMainURL( INetURLObject::NO_DECODE );
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
// media object import, the return value is the url to the media object
//
//////////////////////////////////////////////////////////////////////////

String ImplSdPPTImport::ReadMedia( sal_uInt32 nMediaRef ) const
{
    OUString aRetVal;
    DffRecordHeader* pHd( const_cast<ImplSdPPTImport*>(this)->aDocRecManager.GetRecordHeader( PPT_PST_ExObjList, SEEK_FROM_BEGINNING ) );
    if ( pHd )
    {
        pHd->SeekToContent( rStCtrl );
        while ( ( rStCtrl.Tell() < pHd->GetRecEndFilePos() ) && aRetVal.isEmpty() )
        {
            DffRecordHeader aHdMovie;
            rStCtrl >> aHdMovie;
            switch( aHdMovie.nRecType )
            {
                case PPT_PST_ExAviMovie :
                case PPT_PST_ExMCIMovie :
                {
                    DffRecordHeader aExVideoHd;
                    if ( SeekToRec( rStCtrl, PPT_PST_ExVideo, aHdMovie.GetRecEndFilePos(), &aExVideoHd ) )
                    {
                        DffRecordHeader aExMediaAtomHd;
                        if ( SeekToRec( rStCtrl, PPT_PST_ExMediaAtom, aExVideoHd.GetRecEndFilePos(), &aExMediaAtomHd ) )
                        {
                            sal_uInt32 nRef;
                            rStCtrl >> nRef;
                            if ( nRef == nMediaRef )
                            {
                                aExVideoHd.SeekToContent( rStCtrl );
                                while( rStCtrl.Tell() < aExVideoHd.GetRecEndFilePos() )
                                {
                                    DffRecordHeader aHd;
                                    rStCtrl >> aHd;
                                    switch( aHd.nRecType )
                                    {
                                        case PPT_PST_CString :
                                        {
                                            aHd.SeekToBegOfRecord( rStCtrl );
                                            OUString aStr;
                                            if ( ReadString( aStr ) )
                                            {
                                                if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aStr, aRetVal ) )
                                                {
                                                    aRetVal = INetURLObject( aRetVal ).GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
                                                }else{
                                                    aRetVal = aStr;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    aHd.SeekToEndOfRecord( rStCtrl );
                                }
                                break;
                            }
                        }
                    }
                }
                break;
            }
            aHdMovie.SeekToEndOfRecord( rStCtrl );
        }
    }
    return aRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// import of objects
//
//////////////////////////////////////////////////////////////////////////

void ImplSdPPTImport::FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, String aMacroName )
{
    // set local information into pInfo
    if( pIAtom->nSoundRef )
    {
        pInfo->SetBookmark( ReadSound( pIAtom->nSoundRef ) );   // path to sound file in MS DOS notation
        pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_SOUND;           // RunProgramAction
    }

    switch ( pIAtom->nAction )
    {

        case 0x02 :                                         // RunProgramAction
        {
            pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_PROGRAM;
            pInfo->SetBookmark( aMacroName );                   // program name in aBookmark
        }
        break;
        case 0x03 :                                         // JumpAction
        {
            switch( pIAtom->nJump )
            {
                case 0x01 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_NEXTPAGE;        // Next slide
                break;
                case 0x02 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_PREVPAGE;        // Previous slide
                break;
                case 0x03 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_FIRSTPAGE;       // First slide
                break;
                case 0x04 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_LASTPAGE;        // last Slide
                break;
                case 0x05 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_PREVPAGE;        // Last slide viewed
                break;
                case 0x06 :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_STOPPRESENTATION; // End show
                break;
                default :
                    pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_NONE;            // 0x00: no action, else unknown
                break;
            }
        }
        break;
        case 0x04 :
        {
            SdHyperlinkEntry* pPtr = NULL;
            for ( size_t i = 0, n = aHyperList.size(); i < n; ++i ) {
                if ( aHyperList[ i ]->nIndex == pIAtom->nExHyperlinkId ) {
                    pPtr = aHyperList[ i ];
                    break;
                }
            }
            if ( pPtr )
            {
                switch( pIAtom->nHyperlinkType )
                {
                    case 9:
                    case 8:                                         // hyperlink : URL
                    {
                        if ( !pPtr->aTarget.isEmpty() )
                        {
                            ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
                            if ( pDocShell )
                            {
                                String aBaseURL = pDocShell->GetMedium()->GetBaseURL();
                                OUString aBookmarkURL( pInfo->GetBookmark() );
                                INetURLObject aURL( pPtr->aTarget );
                                if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
                                    utl::LocalFileHelper::ConvertSystemPathToURL( pPtr->aTarget, aBaseURL, aBookmarkURL );
                                if( aBookmarkURL.isEmpty() )
                                    aBookmarkURL = URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), pPtr->aTarget, URIHelper::GetMaybeFileHdl(), true );
                                pInfo->SetBookmark( aBookmarkURL );
                                pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_PROGRAM;
                            }
                        }
                    }
                    break;

                    case 10:
                    break;

                    case 7:                                         // hyperlink to a page
                    {
                        if ( !pPtr->aConvSubString.isEmpty() )
                        {
                            pInfo->meClickAction = ::com::sun::star::presentation::ClickAction_BOOKMARK;
                            pInfo->SetBookmark( pPtr->aConvSubString );
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

SdrObject* ImplSdPPTImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pObj, SdPage* pPage,
                                        SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SfxStyleSheet*  pStyleSheetAry[ 9 ];
    SdrTextObj*     pText = pObj;
    SdrObject*      pRet = pText;

    ppStyleSheetAry = NULL;

    PresObjKind ePresKind = PRESOBJ_NONE;
    PptOEPlaceholderAtom* pPlaceHolder = pTextObj->GetOEPlaceHolderAtom();
    String aPresentationText;
    if ( pPlaceHolder )
    {
        switch( pPlaceHolder->nPlaceholderId )
        {
            case PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE :
            case PPT_PLACEHOLDER_MASTERCENTEREDTITLE :
            case PPT_PLACEHOLDER_MASTERTITLE :
            {
                ePresKind = PRESOBJ_TITLE;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PPT_PLACEHOLDER_MASTERBODY :
            {
                ePresKind = PRESOBJ_OUTLINE;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PPT_PLACEHOLDER_MASTERSUBTITLE :
            {
                ePresKind = PRESOBJ_TEXT;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PPT_PLACEHOLDER_MASTERNOTESBODYIMAGE :
            {
                ePresKind = PRESOBJ_NOTES;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PPT_PLACEHOLDER_MASTERDATE :           ePresKind = PRESOBJ_DATETIME;   break;
            case PPT_PLACEHOLDER_MASTERSLIDENUMBER :    ePresKind = PRESOBJ_SLIDENUMBER;break;
            case PPT_PLACEHOLDER_MASTERFOOTER :         ePresKind = PRESOBJ_FOOTER;     break;
            case PPT_PLACEHOLDER_MASTERHEADER :         ePresKind = PRESOBJ_HEADER;     break;
        }
    }
    switch ( pTextObj->GetDestinationInstance() )
    {
        case TSS_TYPE_PAGETITLE :
        case TSS_TYPE_TITLE :
        {
            pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, sal_True );
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
        }
        break;
        case TSS_TYPE_SUBTITLE :
        {
            pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, sal_True );
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for subtitleobject (SJ)" );
        }
        break;
        case TSS_TYPE_BODY :
        case TSS_TYPE_HALFBODY :
        case TSS_TYPE_QUARTERBODY :
        {
            for ( sal_uInt16 nLevel = 9; nLevel; nLevel-- )
            {
                OUString aName( pPage->GetLayoutName() );
                aName += " ";
                aName += OUString::number( nLevel );
                pSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find( aName, SD_STYLE_FAMILY_MASTERPAGE );
                if ( pSheet )
                    pText->StartListening( *pSheet );
                pStyleSheetAry[ nLevel - 1 ] = pSheet;
            }
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for outlinerobject (SJ)" );
            if ( pSheet )
                ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, sal_True );
            ppStyleSheetAry = &pStyleSheetAry[ 0 ];
        }
        break;
        case TSS_TYPE_NOTES :
        {
            if ( pPlaceHolder && ( ( pPlaceHolder->nPlaceholderId == PPT_PLACEHOLDER_NOTESSLIDEIMAGE )
                || ( pPlaceHolder->nPlaceholderId == PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE ) ) )
            {
                pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
                if ( pSheet )
                    ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, sal_True );
                DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
            }
            else
            {
                pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_NOTES );
                DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for notesobj (SJ)" );
                if ( pSheet )
                    ((SdrAttrObj*)pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, sal_True );
            }
        }
        break;
        case TSS_TYPE_UNUSED :
        case TSS_TYPE_TEXT_IN_SHAPE :
        {
            switch( ePresKind )
            {
                case PRESOBJ_DATETIME :
                case PRESOBJ_SLIDENUMBER :
                case PRESOBJ_FOOTER :
                case PRESOBJ_HEADER :
                    pSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_PSEUDOSHEET_BACKGROUNDOBJECTS), SD_STYLE_FAMILY_PSEUDO );
                break;
                default :
                    pSheet = (SfxStyleSheet*)mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_STANDARD_STYLESHEET_NAME), SD_STYLE_FAMILY_GRAPHICS );
            }
        }
        break;
    }
    pText = (SdrTextObj*)SdrPowerPointImport::ApplyTextObj( pTextObj, pText, pPage, pSheet, ppStyleSheetAry );
    if ( pPlaceHolder && pPlaceHolder->nPlaceholderId )
    {
        if ( eAktPageKind == PPT_MASTERPAGE )
        {
            sal_Bool bCreatePlaceHolder = ( pTextObj->GetInstance() != TSS_TYPE_UNUSED );
            sal_Bool bIsHeaderFooter = ( ePresKind == PRESOBJ_HEADER) || (ePresKind == PRESOBJ_FOOTER)
                                        || (ePresKind == PRESOBJ_DATETIME) || (ePresKind == PRESOBJ_SLIDENUMBER);
            if ( bCreatePlaceHolder && ( pTextObj->GetInstance() == TSS_TYPE_TEXT_IN_SHAPE ) )
                bCreatePlaceHolder = bIsHeaderFooter;
            if ( bCreatePlaceHolder )
            {
                if ( !bIsHeaderFooter )
                {
                    pText->SetNotVisibleAsMaster( sal_True );
                    pText->SetEmptyPresObj( sal_True );
                }
                pText->SetUserCall( pPage );
                pPage->InsertPresObj( pText, ePresKind );
                SdrOutliner* pOutl = NULL;
                if ( pTextObj->GetInstance() == TSS_TYPE_NOTES )
                    pOutl = GetDrawOutliner( pText );
                if ( aPresentationText.Len() )
                    pPage->SetObjText( (SdrTextObj*)pText, pOutl, ePresKind, aPresentationText );

                if ( pPage->GetPageKind() != PK_NOTES && pPage->GetPageKind() != PK_HANDOUT)
                {
                    SfxStyleSheet* pSheet2( pPage->GetStyleSheetForPresObj( ePresKind ) );
                    if ( pSheet2 )
                    {
                        SfxItemSet& rItemSet = pSheet2->GetItemSet();
                        rItemSet.Put( (SdrTextLeftDistItem&)pText->GetMergedItem( SDRATTR_TEXT_LEFTDIST ) );
                        rItemSet.Put( (SdrTextRightDistItem&)pText->GetMergedItem( SDRATTR_TEXT_RIGHTDIST ) );
                        rItemSet.Put( (SdrTextUpperDistItem&)pText->GetMergedItem( SDRATTR_TEXT_UPPERDIST ) );
                        rItemSet.Put( (SdrTextLowerDistItem&)pText->GetMergedItem( SDRATTR_TEXT_LOWERDIST ) );
                        rItemSet.Put( (SdrTextVertAdjustItem&)pText->GetMergedItem( SDRATTR_TEXT_VERTADJUST ) );
                        rItemSet.Put( (SdrTextHorzAdjustItem&)pText->GetMergedItem( SDRATTR_TEXT_HORZADJUST ) );
                        if (  pTextObj->GetInstance() ==  TSS_TYPE_TITLE
                            || pTextObj->GetInstance() == TSS_TYPE_SUBTITLE)
                        {
                            rItemSet.Put( pText->GetMergedItemSet() );
                        }
                    }
                    pText->NbcSetStyleSheet( pSheet2, sal_False );
                }

                SfxItemSet aTempAttr( mpDoc->GetPool() );
                SdrTextMinFrameHeightItem aMinHeight( pText->GetLogicRect().GetSize().Height() );
                aTempAttr.Put( aMinHeight );
                SdrTextAutoGrowHeightItem aAutoGrowHeight( sal_False );
                aTempAttr.Put( aAutoGrowHeight );
                pText->SetMergedItemSet(aTempAttr);
            }
            else
            {
                pRet = NULL;
            }
        }
        else
        {
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout || ( eAktPageKind == PPT_NOTEPAGE ) )
            {
                sal_uInt32 nPlacementId = pPlaceHolder->nPlacementId;
                sal_Int16 nPlaceholderId = pPlaceHolder->nPlaceholderId;

                if ( eAktPageKind == PPT_SLIDEPAGE )
                {
                    PresObjKind ePresObjKind = PRESOBJ_NONE;
                    sal_Bool    bEmptyPresObj = sal_True;
                    sal_Bool    bVertical = sal_False;
                    if ( ( pTextObj->GetShapeType() == mso_sptRectangle ) || ( pTextObj->GetShapeType() == mso_sptTextBox ) )
                    {
                        //if a placeholder with some custom attribute,the pTextObj will keep those attr,whose text size is zero,
                        //so sdPage should renew a PresObj to process placeholder.
                        bEmptyPresObj = ( pTextObj->Count() == 0 ) || ( pTextObj->Count() == 1 && pTextObj->First()->GetTextSize() == 0 );
                        switch ( nPlaceholderId )
                        {
                            case PPT_PLACEHOLDER_NOTESBODY :            ePresObjKind = PRESOBJ_NOTES;   break;
                            case PPT_PLACEHOLDER_VERTICALTEXTTITLE :
                                bVertical = sal_True;   // PASSTHROUGH !!!
                            case PPT_PLACEHOLDER_TITLE :                ePresObjKind = PRESOBJ_TITLE;   break;
                            case PPT_PLACEHOLDER_VERTICALTEXTBODY :
                                bVertical = sal_True;   // PASSTHROUGH !!!
                            case PPT_PLACEHOLDER_BODY :                 ePresObjKind = PRESOBJ_OUTLINE; break;
                            case PPT_PLACEHOLDER_CENTEREDTITLE :        ePresObjKind = PRESOBJ_TITLE;   break;
                            case PPT_PLACEHOLDER_SUBTITLE :             ePresObjKind = PRESOBJ_TEXT;    break;      // PRESOBJ_OUTLINE

                            default :
                            {
                                if ( pTextObj->Count() == 0 )
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
                            bEmptyPresObj = sal_False;
                            switch ( nPlaceholderId )
                            {
                                case PPT_PLACEHOLDER_MEDIACLIP :
                                case PPT_PLACEHOLDER_OBJECT : ePresObjKind = PRESOBJ_OBJECT; break;
                                case PPT_PLACEHOLDER_GRAPH : ePresObjKind = PRESOBJ_CHART; break;
                                case PPT_PLACEHOLDER_TABLE : ePresObjKind = PRESOBJ_CALC; break;
                                case PPT_PLACEHOLDER_CLIPART : ePresObjKind = PRESOBJ_GRAPHIC; break;
                                case PPT_PLACEHOLDER_ORGANISZATIONCHART : ePresObjKind = PRESOBJ_ORGCHART; break;
                            }
                        }
                    }
                    if ( ePresObjKind != PRESOBJ_NONE )
                    {
                        if ( !bEmptyPresObj )
                        {
                            pPage->InsertPresObj( pRet, ePresObjKind );
                        }
                        else
                        {
                            SdrObject* pPresObj = pPage->CreatePresObj( ePresObjKind, bVertical, pText->GetLogicRect(), sal_True );
                            pPresObj->SetUserCall( pPage );

                            SfxItemSet aSet( pSdrModel->GetItemPool() );
                            ApplyAttributes( rStCtrl, aSet );
                            pPresObj->SetLogicRect(pText->GetLogicRect());
                            ApplyTextAnchorAttributes( *pTextObj, aSet );
                            //set custom font attribute of the placeholder
                            if ( pTextObj->Count() == 1 )
                            {
                                PPTParagraphObj* pPara = pTextObj->First();
                                PPTPortionObj* pPor = NULL;
                                if ( pPara && pPara->GetTextSize() == 0 && (pPor = pPara->First()))
                                {
                                    pPor->ApplyTo(aSet, (SdrPowerPointImport&)*this, pTextObj->GetDestinationInstance());
                                }
                            }
                            pPresObj->SetMergedItemSet(aSet);

                            if ( ( eAktPageKind != PPT_NOTEPAGE ) && ( nPlacementId != 0xffffffff ) )
                            {
                                SdrObject* pTitleObj = ((SdPage&)pPage->TRG_GetMasterPage()).GetPresObj( PRESOBJ_TITLE );
                                SdrObject* pOutlineObj = ((SdPage&)pPage->TRG_GetMasterPage()).GetPresObj( PRESOBJ_OUTLINE );

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

                                switch ( nPlacementId )
                                {
                                    case 0 :            // position in title area
                                    {
                                        if ( aLogicRect != aTitleRect )
                                            pPresObj->SetUserCall( NULL );
                                    }
                                    break;

                                    case 1:
                                    {
                                        if ( pSlideLayout->eLayout == PPT_LAYOUT_TITLEANDBODYSLIDE )
                                        {   // position in outline area
                                            if ( aLogicRect != aOutlineRect )
                                                pPresObj->SetUserCall( NULL );
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2COLUMNSANDTITLE )
                                        {   // position in outline area left
                                            if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall(NULL);
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2ROWSANDTITLE )
                                        {   // position in outline area top
                                            if (std::abs(aLogicRect.Left()  - aOutlineRect.Left())  > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if (std::abs(aLogicRect.Left() - aOutlineRect.Left()) > MAX_USER_MOVE ||
                                                 std::abs(aLogicRect.Top()  - aOutlineRect.Top())  > MAX_USER_MOVE)
                                        {   // position in outline area top left
                                            pPresObj->SetUserCall( NULL );
                                        }
                                    }
                                    break;

                                    case 2:
                                    {
                                        if ( pSlideLayout->eLayout == PPT_LAYOUT_2COLUMNSANDTITLE )
                                        {   // position in outline area right
                                            if (std::abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                    aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PPT_LAYOUT_2ROWSANDTITLE )
                                        {   // position in outline area bottom
                                            if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( NULL );
                                            }
                                        }
                                        else if (std::abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE ||
                                                 std::abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE)
                                        {   // position in outline area top right
                                            pPresObj->SetUserCall(NULL);
                                        }
                                    }
                                    break;

                                    case 3:
                                    {   // position in outline area bottom left
                                        if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                            std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
                                        {
                                            pPresObj->SetUserCall( NULL );
                                        }
                                    }
                                    break;

                                    case 4:
                                    {   // position in outline area bottom right
                                        if (std::abs(aLogicRect.Right() - aOutlineRect.Right())   > MAX_USER_MOVE ||
                                            std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
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
    {
        SdrObject* pFree( pText );
        SdrObject::Free( pFree );
    }
    return pRet;
}

SdrObject* ImplSdPPTImport::ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pRet )
{
    SdrObject* pObj = SdrPowerPointImport::ProcessObj( rSt, rObjData, pData, rTextRect, pRet );

    // read animation effect of object
    if ( pObj )
    {
        // further setup placeholder objects
        if( pObj->ISA(SdrPageObj) && pData )
        {
            const ProcessData* pProcessData=(const ProcessData*)pData;
            if( pProcessData->pPage )
                pProcessData->pPage->InsertPresObj( pObj, PRESOBJ_PAGE );
        }

        sal_Bool bInhabitanceChecked = sal_False;
        sal_Bool bAnimationInfoFound = sal_False;
        DffRecordHeader aMasterShapeHd;

        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            DffRecordHeader& rHdClientData = *maShapeRecords.Current();
            while( true )
            {
                sal_uInt32 nClientDataLen = rHdClientData.GetRecEndFilePos();
                DffRecordHeader aHd;
                do
                {
                    rSt >> aHd;
                    sal_uInt32 nHdRecEnd = aHd.GetRecEndFilePos();
                    switch ( aHd.nRecType )
                    {
                        case PPT_PST_AnimationInfo :
                        {
                            DffRecordHeader aHdAnimInfoAtom;
                            if ( SeekToRec( rSt, PPT_PST_AnimationInfoAtom, nHdRecEnd, &aHdAnimInfoAtom ) )
                            {
                                // read data from stream
                                Ppt97AnimationPtr pAnimation( new Ppt97Animation( rSt ) );
                                // store animation information
                                if( pAnimation->HasEffect() )
                                {
                                    // translate color to RGB
                                    pAnimation->SetDimColor( MSO_CLR_ToColor(pAnimation->GetDimColor()).GetColor() );
                                    // translate sound bits to file url
                                    if( pAnimation->HasSoundEffect() )
                                        pAnimation->SetSoundFileUrl( ReadSound( pAnimation->GetSoundRef() ) );

                                    bool bDontAnimateInvisibleShape = false;
                                    {
                                        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObj);

                                        if( pTextObj && pTextObj->HasText() &&
                                            !pObj->ISA( SdrObjGroup ) &&
                                            pAnimation->HasAnimateAssociatedShape() )
                                        {
                                            const SfxItemSet& rObjItemSet = pObj->GetMergedItemSet();

                                            XFillStyle eFillStyle = ((XFillStyleItem&)(rObjItemSet.Get(XATTR_FILLSTYLE))).GetValue();
                                            XLineStyle eLineStyle = ((XLineStyleItem&)(rObjItemSet.Get(XATTR_LINESTYLE))).GetValue();

                                            if ( ( eFillStyle == XFILL_NONE ) && ( eLineStyle == XLINE_NONE ) )
                                                bDontAnimateInvisibleShape = true;
                                        }
                                    }
                                    if( bDontAnimateInvisibleShape )
                                        pAnimation->SetAnimateAssociatedShape(false);

                                    //maybe some actions necessary to ensure that animations on master pages are played before animations on normal pages
                                    ///mabe todo in future: bool bIsEffectOnMasterPage = !bInhabitanceChecked;?

                                    maAnimations[pObj] = pAnimation;

                                    bAnimationInfoFound = sal_True;
                                }
                            }
                        }
                        break;
                        case PPT_PST_InteractiveInfo:
                        {
                            sal_uInt32 nFilePosMerk2 = rSt.Tell();
                            OUString aMacroName;

                            if(SeekToRec( rSt, PPT_PST_CString, nHdRecEnd, NULL, 0 ) )
                                ReadString(aMacroName);

                            rSt.Seek( nFilePosMerk2 );
                            DffRecordHeader aHdInteractiveInfoAtom;
                            if ( SeekToRec( rSt, PPT_PST_InteractiveInfoAtom, nHdRecEnd, &aHdInteractiveInfoAtom ) )
                            {
                                PptInteractiveInfoAtom aInteractiveInfoAtom;
                                rSt >> aInteractiveInfoAtom;

                                // interactive object
                                SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj, true);

                                ( (ImplSdPPTImport*) this )->FillSdAnimationInfo( pInfo, &aInteractiveInfoAtom, aMacroName );
                                if ( aInteractiveInfoAtom.nAction == 6 ) // Sj -> media action
                                {
                                    rHdClientData.SeekToContent( rStCtrl );
                                    DffRecordHeader aObjRefAtomHd;
                                    if ( SeekToRec( rSt, PPT_PST_ExObjRefAtom, nHdRecEnd, &aObjRefAtomHd ) )
                                    {
                                        sal_uInt32 nRef;
                                        rSt >> nRef;
                                        String aMediaURL( ReadMedia( nRef ) );
                                        if ( !aMediaURL.Len() )
                                            aMediaURL = ReadSound( nRef );
                                        if ( aMediaURL.Len() )
                                        {
                                            SdrMediaObj* pMediaObj = new SdrMediaObj( pObj->GetSnapRect() );
                                            pMediaObj->SetModel( pObj->GetModel() );
                                            pMediaObj->SetMergedItemSet( pObj->GetMergedItemSet() );

                                            //--remove object from maAnimations list and add the new object instead
                                            Ppt97AnimationPtr pAnimation;
                                            {
                                                tAnimationMap::iterator aFound = maAnimations.find( pObj );
                                                if( aFound != maAnimations.end() )
                                                {
                                                    pAnimation = (*aFound).second;
                                                    maAnimations.erase(aFound);
                                                }
                                                maAnimations[pMediaObj] = pAnimation;
                                            }

                                            SdrObject::Free( pObj ), pObj = pMediaObj;  // SJ: hoping that pObj is not inserted in any list
                                            pMediaObj->setURL( aMediaURL );
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    aHd.SeekToEndOfRecord( rSt );
                }
                while( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nClientDataLen ) );

                if ( bInhabitanceChecked || bAnimationInfoFound )
                    break;
                bInhabitanceChecked = sal_True;
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


bool
ImplSdPPTImport::ReadFormControl( SotStorageRef& rSrc1, com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rFormComp ) const
{
    uno::Reference< frame::XModel > xModel;
    if (  mpDoc->GetDocSh() )
    {
        xModel = mpDoc->GetDocSh()->GetModel();
        oox::ole::MSConvertOCXControls mCtrlImporter( xModel );
        return mCtrlImporter.ReadOCXStorage( rSrc1, rFormComp );
    }
    return false;
}

// ---------------------
// - exported function -
// ---------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL ImportPPT(
        SdDrawDocument* pDocument, SvStream& rDocStream, SvStorage& rStorage, SfxMedium& rMedium )
{
    sal_Bool bRet = sal_False;

    SdPPTImport* pImport = new SdPPTImport( pDocument, rDocStream, rStorage, rMedium );
    bRet = pImport->Import();

    delete pImport;

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
