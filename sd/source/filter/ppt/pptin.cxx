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
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
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
#include <vcl/bitmapaccess.hxx>
#include "customshowlist.hxx"
#include "sddll.hxx"

#include "../../ui/inc/DrawDocShell.hxx"
#include "../../ui/inc/FrameView.hxx"
#include "../../ui/inc/optsitem.hxx"

#include <unotools/fltrcfg.hxx>
#include <sfx2/progress.hxx>
#include <editeng/editstat.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfac.hxx>
#define MAX_USER_MOVE       2

#include "pptinanimations.hxx"
#include "ppt97animations.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <comphelper/string.hxx>
#include <oox/ole/olehelper.hxx>

#include <boost/optional.hpp>

#include <cassert>
#include <memory>

using namespace ::com::sun::star;

SdPPTImport::SdPPTImport( SdDrawDocument* pDocument, SvStream& rDocStream, SotStorage& rStorage, SfxMedium& rMedium )
{

    sal_uInt32 nImportFlags = 0;

#ifdef DBG_UTIL
    PropRead* pSummaryInformation = new PropRead( rStorage, "\005SummaryInformation" );
    if ( pSummaryInformation->IsValid() )
    {
        pSummaryInformation->Read();
        sal_uInt8 aPropSetGUID[ 16 ] =
        {
            0xe0, 0x85, 0x9f, 0xf2, 0xf9, 0x4f, 0x68, 0x10, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9
        };
        Section* pSection = const_cast<Section*>(pSummaryInformation->GetSection( aPropSetGUID ));
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
    SvStream* pCurrentUserStream = rStorage.OpenSotStream( "Current User", StreamMode::STD_READ );
    if( pCurrentUserStream )
    {
        ReadPptCurrentUserAtom( *pCurrentUserStream, aParam.aCurrentUserAtom );
        delete pCurrentUserStream;
    }

    if( pDocument )
    {
        // iterate over all styles
        SdStyleSheetPool* pStyleSheetPool = pDocument->GetSdStyleSheetPool();
        std::shared_ptr<SfxStyleSheetIterator> aIter =
                std::make_shared<SfxStyleSheetIterator>(pStyleSheetPool, SfxStyleFamily::All);

        for (SfxStyleSheetBase *pSheet = aIter->First(); pSheet; pSheet = aIter->Next())
        {
            SfxItemSet& rSet = pSheet->GetItemSet();
            // if autokerning is set in style, override it, ppt has no autokerning
            if( rSet.GetItemState( EE_CHAR_PAIRKERNING, false ) == SfxItemState::SET )
                rSet.ClearItem( EE_CHAR_PAIRKERNING );
        }
    }

    pFilter.reset( new ImplSdPPTImport( pDocument, rStorage, rMedium, aParam ) );
}

bool SdPPTImport::Import()
{
    return pFilter->Import();
}

SdPPTImport::~SdPPTImport()
{
}

ImplSdPPTImport::ImplSdPPTImport( SdDrawDocument* pDocument, SotStorage& rStorage_, SfxMedium& rMedium, PowerPointImportParam& rParam )
    : SdrPowerPointImport(rParam, rMedium.GetBaseURL())
    , mrMed(rMedium)
    , mrStorage(rStorage_)
    , mbDocumentFound(false)
    , mnFilterOptions(0)
    , mpDoc(pDocument)
    , mePresChange(PRESCHANGE_MANUAL)
    , mnBackgroundLayerID(0)
    , mnBackgroundObjectsLayerID(0)
{
    if ( bOk )
    {
        mbDocumentFound = SeekToDocument( &maDocHd );                           // maDocHd = the latest DocumentHeader
        while ( SeekToRec( rStCtrl, PPT_PST_Document, nStreamLen, &maDocHd ) )
            mbDocumentFound = true;

        sal_uInt32 nDggContainerOfs = 0;

        if ( mbDocumentFound )
        {
            sal_uLong nPosMerk = rStCtrl.Tell();

            pStData = rStorage_.OpenSotStream( "Pictures", StreamMode::STD_READ );

            rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
            sal_uLong nDocLen = maDocHd.GetRecEndFilePos();
            DffRecordHeader aPPDGHd;
            if ( SeekToRec( rStCtrl, PPT_PST_PPDrawingGroup, nDocLen, &aPPDGHd ) )
            {
                sal_uLong nPPDGLen = aPPDGHd.GetRecEndFilePos();
                if ( SeekToRec( rStCtrl, DFF_msofbtDggContainer, nPPDGLen ) )
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

// Dtor
ImplSdPPTImport::~ImplSdPPTImport()
{
    delete pStData;
}

// Import
bool ImplSdPPTImport::Import()
{
    if ( !bOk )
        return false;

    pSdrModel->setLock(true);
    pSdrModel->EnableUndo(false);

    SdrOutliner& rOutl = mpDoc->GetDrawOutliner();
    EEControlBits nControlWord = rOutl.GetEditEngine().GetControlWord();
    nControlWord |=  EEControlBits::ULSPACESUMMATION;
    nControlWord &= ~EEControlBits::ULSPACEFIRSTPARA;
    ((EditEngine&)rOutl.GetEditEngine()).SetControlWord( nControlWord );

    SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
    mnBackgroundLayerID = rAdmin.GetLayerID( SD_RESSTR( STR_LAYER_BCKGRND ), false );
    mnBackgroundObjectsLayerID = rAdmin.GetLayerID( SD_RESSTR( STR_LAYER_BCKGRNDOBJ ), false );

    ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
    if ( pDocShell )
        SeekOle( pDocShell, mnFilterOptions );

    // hyperlinks
    PropRead* pDInfoSec2 = new PropRead( mrStorage, "\005DocumentSummaryInformation" );
    if ( pDInfoSec2->IsValid() )
    {
        PropItem aPropItem;

        sal_uInt32 nType, nPropSize, nPropCount;

        pDInfoSec2->Read();

        sal_uInt8 aPropSetGUID[ 16 ] =
        {
            0x02, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
        };
        Section* pSection = const_cast<Section*>(pDInfoSec2->GetSection( aPropSetGUID ));
        if ( pSection )
        {
            if ( pSection->GetProperty( PID_SLIDECOUNT, aPropItem ) )
            {
                aPropItem.ReadUInt32( nType );
                if ( ( nType == VT_I4 ) || ( nType == VT_UI4 ) )
                {
                    // examine PID_HEADINGPAIR to get the correct entry for PID_DOCPARTS
                    sal_uInt32 nSlideCount, nVecCount;
                    aPropItem.ReadUInt32( nSlideCount );
                    if ( nSlideCount && pSection->GetProperty( PID_HEADINGPAIR, aPropItem ) )
                    {
                        sal_uInt32  nSlideTitleIndex = 0, nSlideTitleCount = 0;

                        OUString aUString;

                        aPropItem.ReadUInt32( nType )
                                 .ReadUInt32( nVecCount );

                        if ( ( nType == ( VT_VARIANT | VT_VECTOR ) ) && ( nVecCount ^ 1 ) )
                        {
                            nVecCount >>= 1;
                            sal_uInt32 nEntryCount = 0;
                            for (sal_uInt32 i = 0; i < nVecCount; ++i)
                            {
                                if ( !aPropItem.Read( aUString, VT_EMPTY, false ) )
                                    break;
                                aPropItem.ReadUInt32( nType );
                                if ( ( nType != VT_I4 ) && ( nType != VT_UI4 ) )
                                    break;
                                sal_uInt32 nTemp(0);
                                aPropItem.ReadUInt32( nTemp );
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
                            aPropItem.ReadUInt32( nType )
                                     .ReadUInt32( nVecCount );

                            bool bVecOk = ( ( nVecCount >= (nSlideTitleIndex + nSlideTitleCount) )
                                    && ( nType == ( VT_LPSTR | VT_VECTOR ) ) );

                            if (bVecOk)
                            {
                                for (sal_uInt32 i = 0; i != nSlideTitleIndex; ++i)
                                {
                                    sal_uInt32 nTemp(0);
                                    aPropItem.ReadUInt32(nTemp);
                                    if (!aPropItem.good())
                                    {
                                        bVecOk = false;
                                        break;
                                    }
                                    auto nPos = aPropItem.Tell() + nTemp;
                                    if (nPos != aPropItem.Seek(nPos))
                                    {
                                        bVecOk = false;
                                        break;
                                    }
                                }
                            }
                            if (bVecOk)
                            {
                                for (sal_uInt32 i = 0; i < nSlideTitleCount; ++i)
                                {
                                    if (!aPropItem.Read(aUString, nType, false))
                                        break;

                                    OUString aString( aUString );
                                    if ( aString == "No Slide Title" )
                                        aString.clear();
                                    else
                                    {
                                        std::vector<OUString>::const_iterator pIter =
                                                std::find(maSlideNameList.begin(),maSlideNameList.end(),aString);

                                        if (pIter != maSlideNameList.end())
                                            aString.clear();
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
            pSection = const_cast<Section*>(pDInfoSec2->GetSection( aUserPropSetGUID ));
            if ( pSection )
            {
                Dictionary aDict;
                pSection->GetDictionary(aDict);
                if (!aDict.empty())
                {
                    Dictionary::const_iterator iter = aDict.find( OUString("_PID_HLINKS") );

                    if ( iter != aDict.end() )
                    {
                        if ( pSection->GetProperty( iter->second, aPropItem ) )
                        {
                            aPropItem.Seek( STREAM_SEEK_TO_BEGIN );
                            aPropItem.ReadUInt32( nType );
                            if ( nType == VT_BLOB )
                            {
                                aPropItem.ReadUInt32( nPropSize )
                                         .ReadUInt32( nPropCount );

                                if ( ! ( nPropCount % 6 ) )
                                {
                                    sal_uInt32 i;

                                    nPropCount /= 6;    // 6 properties per hyperlink

                                    SdHyperlinkEntry* pHyperlink = nullptr;
                                    for ( i = 0; i < nPropCount; i++ )
                                    {
                                        pHyperlink = new SdHyperlinkEntry;
                                        pHyperlink->nIndex = 0;
                                        aPropItem.ReadUInt32( nType );
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem.ReadInt32( pHyperlink->nPrivate1 )
                                                 .ReadUInt32( nType );
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem.ReadInt32( pHyperlink->nPrivate2 )
                                                 .ReadUInt32( nType );
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem.ReadInt32( pHyperlink->nPrivate3 )
                                                 .ReadUInt32( nType );
                                        if ( nType != VT_I4 )
                                            break;
                                        aPropItem.ReadInt32( pHyperlink->nInfo );
                                        if ( !aPropItem.Read( pHyperlink->aTarget ) )
                                            break;

                                        // Convert '\\' notation to 'smb://'
                                        INetURLObject aUrl( pHyperlink->aTarget, INetProtocol::File );
                                        pHyperlink->aTarget = aUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                                        if ( !aPropItem.Read( pHyperlink->aSubAdress ) )
                                            break;
                                        pHyperlink->nStartPos = -1;

                                        if ( !pHyperlink->aSubAdress.isEmpty() ) // get the converted subaddress
                                        {
                                            sal_uInt32 nPageNumber = 0;
                                            OUString aString( pHyperlink->aSubAdress );
                                            OString aStringAry[ 3 ];
                                            size_t nTokenCount = 0;
                                            sal_Int32 nPos = 0;
                                            do
                                            {
                                                aStringAry[nTokenCount] =
                                                    OUStringToOString(aString.getToken( 0, (sal_Unicode)',', nPos ), RTL_TEXTENCODING_UTF8);
                                            }
                                            while ( ++nTokenCount < SAL_N_ELEMENTS(aStringAry) && nPos >= 0 );

                                            bool bDocInternalSubAddress = false;

                                            // first pass, searching for a SlideId
                                            for( size_t nToken = 0; nToken < nTokenCount; ++nToken )
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
                                                                bDocInternalSubAddress = true;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            if ( !bDocInternalSubAddress )
                                            {   // second pass, searching for a SlideName
                                                for ( size_t nToken = 0; nToken < nTokenCount; ++nToken )
                                                {
                                                    OUString aToken(OStringToOUString(aStringAry[nToken], RTL_TEXTENCODING_UTF8));
                                                    std::vector<OUString>::const_iterator pIter =
                                                            std::find(maSlideNameList.begin(),maSlideNameList.end(),aToken);

                                                    if (pIter != maSlideNameList.end())
                                                    {
                                                        nPageNumber = pIter - maSlideNameList.begin();
                                                        bDocInternalSubAddress = true;
                                                    }
                                                }
                                            }
                                            if ( !bDocInternalSubAddress )
                                            {   // third pass, searching for a slide number
                                                for ( size_t nToken = 0; nToken < nTokenCount; ++nToken )
                                                {
                                                    if (comphelper::string::isdigitAsciiString(aStringAry[nToken]))
                                                    {
                                                        sal_Int32 nNumber = aStringAry[ nToken ].toInt32();
                                                        if ( ( nNumber & ~0xff ) == 0 )
                                                        {
                                                            nPageNumber = (sal_uInt32)nNumber - 1;
                                                            bDocInternalSubAddress = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                            // if a document internal sub address
                                            if ( bDocInternalSubAddress )
                                            {
                                                if ( nPageNumber < maSlideNameList.size() )
                                                    pHyperlink->aConvSubString = maSlideNameList[ nPageNumber ];
                                                if ( pHyperlink->aConvSubString.isEmpty() )
                                                {
                                                    pHyperlink->aConvSubString = OUString( SdResId( STR_PAGE ) ) + " " + ( mpDoc->CreatePageNumValue( (sal_uInt16)nPageNumber + 1 ) );
                                                }
                                            } else {
                                                // if sub address is given but not internal, use it as it is
                                                if ( pHyperlink->aConvSubString.isEmpty() )
                                                {
                                                    pHyperlink->aConvSubString = aString;
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
            for (SdHyperlinkEntry* pPtr : aHyperList)
            {
                DffRecordHeader aHyperE;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlink, nExObjHyperListLen, &aHyperE ) )
                    break;
                if ( !SeekToRec( rStCtrl, PPT_PST_ExHyperlinkAtom, nExObjHyperListLen ) )
                    break;
                rStCtrl.SeekRel( 8 );
                rStCtrl.ReadUInt32( pPtr->nIndex );
                if (!aHyperE.SeekToEndOfRecord(rStCtrl))
                    break;
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

    // create master pages:

    std::unique_ptr<SfxProgress> xStbMgr;
    if (!utl::ConfigManager::IsAvoidConfig())
    {
        xStbMgr.reset(new SfxProgress(pDocShell,
                        SD_RESSTR( STR_POWERPOINT_IMPORT),
                        m_pMasterPages->size() +
                        m_pSlidePages->size() + m_pNotePages->size()));
    }

    sal_uInt32 nImportedPages = 0;
    {
        sal_uInt16          nMasterAnz = GetPageCount( PPT_MASTERPAGE );

        for ( sal_uInt16 nMasterNum = 0; nMasterNum < nMasterAnz; nMasterNum++ )
        {
            SetPageNum( nMasterNum, PPT_MASTERPAGE );
            SdPage* pPage = static_cast<SdPage*>(MakeBlancPage( true ));
            if ( pPage )
            {
                bool bNotesMaster = (*GetPageList( eAktPageKind ) )[ nAktPageNum ].bNotesMaster;
                bool bStarDrawFiller = (*GetPageList( eAktPageKind ) )[ nAktPageNum ].bStarDrawFiller;

                PageKind ePgKind = ( bNotesMaster ) ? PageKind::Notes : PageKind::Standard;
                bool bHandout = (*GetPageList( eAktPageKind ) )[ nAktPageNum ].bHandoutMaster;
                if ( bHandout )
                    ePgKind = PageKind::Handout;

                pPage->SetPageKind( ePgKind );
                pSdrModel->InsertMasterPage( static_cast<SdrPage*>(pPage) );
                if ( bNotesMaster && bStarDrawFiller )
                    pPage->SetAutoLayout( AUTOLAYOUT_NOTES, true );
                if ( nMasterNum )
                {
                    boost::optional< sal_Int16 > oStartNumbering;
                    SfxStyleSheet* pSheet;
                    if ( nMasterNum == 1 )
                    {
                        // standardsheet
                        pSheet = static_cast<SfxStyleSheet*>(mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_STANDARD_STYLESHEET_NAME), SD_STYLE_FAMILY_GRAPHICS ));
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_Type::TextInShape, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_Type::TextInShape, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, nullptr );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                        }
                    }

                    // PSEUDO
                    pSheet = static_cast<SfxStyleSheet*>(mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_PSEUDOSHEET_BACKGROUNDOBJECTS), SD_STYLE_FAMILY_PSEUDO ));
                    if ( pSheet )
                    {
                        SfxItemSet& rItemSet = pSheet->GetItemSet();
                        PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_Type::TextInShape, 0 );
                        PPTPortionObj aPortion( *pPPTStyleSheet, TSS_Type::TextInShape, 0 );
                        aParagraph.AppendPortion( aPortion );
                        aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, nullptr );
                        aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                    }

                    // create layoutstylesheets, set layoutname and stylesheet
                    // (only on standard and not pages)

                    OUString aLayoutName( SD_RESSTR( STR_LAYOUT_DEFAULT_NAME ) );
                    if ( nMasterNum > 2 )
                    {
                        if ( ePgKind == PageKind::Standard )
                        {   // standard page: create new presentation layout
                            aLayoutName = SD_RESSTR( STR_LAYOUT_DEFAULT_TITLE_NAME );
                            aLayoutName += OUString::number( (sal_Int32)( ( nMasterNum + 1 ) / 2 - 1 ) );
                            static_cast<SdStyleSheetPool*>( mpDoc->GetStyleSheetPool() )->CreateLayoutStyleSheets( aLayoutName );
                        }
                        else    // note page: use presentation layout of standard page
                            aLayoutName = static_cast<SdPage*>( mpDoc->GetMasterPage( nMasterNum - 1 ) )->GetName();
                    }
                    pPage->SetName( aLayoutName );
                    aLayoutName += SD_LT_SEPARATOR;
                    aLayoutName += SD_RESSTR( STR_LAYOUT_OUTLINE );
                    pPage->SetLayoutName( aLayoutName );

                    // set stylesheets
                    if ( pPage->GetPageKind() == PageKind::Standard )
                    {
                        TSS_Type nTitleInstance = TSS_Type::PageTitle;
                        TSS_Type nOutlinerInstance = TSS_Type::Body;
                        const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
                        bool bSwapStyleSheet = pSlideLayout->eLayout == PptSlideLayout::TITLEMASTERSLIDE;
                        if ( bSwapStyleSheet )
                        {
                            nTitleInstance = TSS_Type::Title;
                            nOutlinerInstance = TSS_Type::Subtitle;
                        }

                        // titelstylesheet
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, nTitleInstance, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, nTitleInstance, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, nullptr );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                        }

                        // outlinerstylesheet
                        sal_uInt16 nLevel;
                        PPTParagraphObj* pParagraphs[ 9 ];
                        PPTParagraphObj* pPreviousPara = nullptr;

                        for ( nLevel = 0; nLevel < 9; nLevel++ )
                        {
                            OUString aName( pPage->GetLayoutName() );
                            aName += " ";
                            aName += OUString::number( nLevel + 1 );
                            SfxStyleSheet* pOutlineSheet = static_cast<SfxStyleSheet*>( mpDoc->GetStyleSheetPool()->Find( aName, SD_STYLE_FAMILY_MASTERPAGE ) );
                            DBG_ASSERT( pOutlineSheet, "Template for outline object not found" );
                            if ( pOutlineSheet )
                            {
                                pParagraphs[ nLevel ] = new PPTParagraphObj( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                SfxItemSet& rItemSet = pOutlineSheet->GetItemSet();
                                PPTPortionObj aPortion( *pPPTStyleSheet, nOutlinerInstance, nLevel );
                                pParagraphs[ nLevel ]->AppendPortion( aPortion );
                                pParagraphs[ nLevel ]->ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, pPreviousPara );
                                aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                                pPreviousPara = pParagraphs[ nLevel ];
                            }
                            else
                                pParagraphs[ nLevel ] = nullptr;
                        }
                        for ( nLevel = 0; nLevel < 9; delete pParagraphs[ nLevel++ ] ) ;

                        // subtitle stylesheet
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_Type::Subtitle, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_Type::Subtitle, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, nullptr );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                        }
                    }
                    else if ( ePgKind == PageKind::Notes )
                    {
                        pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_NOTES );
                        if ( pSheet )
                        {
                            SfxItemSet& rItemSet = pSheet->GetItemSet();
                            PPTParagraphObj aParagraph( *pPPTStyleSheet, TSS_Type::Notes, 0 );
                            PPTPortionObj aPortion( *pPPTStyleSheet, TSS_Type::Notes, 0 );
                            aParagraph.AppendPortion( aPortion );
                            aParagraph.ApplyTo( rItemSet, oStartNumbering, (SdrPowerPointImport&)*this, TSS_Type::Unknown, nullptr );
                            aPortion.ApplyTo( rItemSet, (SdrPowerPointImport&)*this, TSS_Type::Unknown );
                        }
                    }
                }
            }
        }
    }
    sal_uInt16 i;
    for (i = 0; i < mpDoc->GetMasterPageCount(); ++i)
    {
        SdPage *const pMPage(static_cast<SdPage*>(mpDoc->GetMasterPage(i)));
        if (pMPage == nullptr)
            break;
        SetPageNum( i, PPT_MASTERPAGE );

        // importing master page objects
        PptSlidePersistList* pList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pPersist = ( pList && ( nAktPageNum < pList->size() ) )
                                                    ? &(*pList)[ nAktPageNum ] : nullptr;
        if ( pPersist )
        {
            if ( pPersist->bStarDrawFiller && pPersist->bNotesMaster && ( nAktPageNum > 2 ) && ( ( nAktPageNum & 1 ) == 0 ) )
            {
                pSdrModel->DeleteMasterPage( nAktPageNum );
                SdrPage* pNotesClone = static_cast<SdPage*>(pSdrModel->GetMasterPage( 2 ))->Clone();
                pSdrModel->InsertMasterPage( pNotesClone, nAktPageNum );
                if ( pNotesClone )
                {
                    OUString aLayoutName( static_cast<SdPage*>(pSdrModel->GetMasterPage( nAktPageNum - 1 ))->GetLayoutName() );
                    static_cast<SdPage*>(pNotesClone)->SetPresentationLayout( aLayoutName, false, false );
                    static_cast<SdPage*>(pNotesClone)->SetLayoutName( aLayoutName );
                }
            }
            else if ( !pPersist->bStarDrawFiller )
            {
                PptSlidePersistEntry* pE = pPersist;
                while( ( pE->aSlideAtom.nFlags & 4 ) && pE->aSlideAtom.nMasterId )
                {
                    auto nOrigMasterId = pE->aSlideAtom.nMasterId;
                    sal_uInt16 nNextMaster = m_pMasterPages->FindPage(nOrigMasterId);
                    if ( nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                        break;
                    else
                        pE = &(*pList)[ nNextMaster ];
                    if (pE->aSlideAtom.nMasterId == nOrigMasterId)
                    {
                        SAL_WARN("filter.ms", "loop in atom chain");
                        break;
                    }
                }
                SdrObject* pObj = ImportPageBackgroundObject( *pMPage, pE->nBackgroundOffset );   // import background
                if ( pObj )
                    pMPage->NbcInsertObject( pObj );

                bool bNewAnimationsUsed = false;
                ProcessData aProcessData( (*pList)[ nAktPageNum ], SdPageCapsule(pMPage) );
                sal_uInt32 nFPosMerk = rStCtrl.Tell();
                DffRecordHeader aPageHd;
                if ( SeekToAktPage( &aPageHd ) )
                {
                    auto nEndRecPos = SanitizeEndPos(rStCtrl, aPageHd.GetRecEndFilePos());
                    while( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nEndRecPos ) )
                    {
                        DffRecordHeader aHd;
                        ReadDffRecordHeader( rStCtrl, aHd );
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
                                            auto nListEndRecPos = SanitizeEndPos(rStCtrl, aEscherObjListHd.GetRecEndFilePos());
                                            while( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nListEndRecPos ) )
                                            {
                                                DffRecordHeader aHd2;
                                                ReadDffRecordHeader( rStCtrl, aHd2 );
                                                if ( ( aHd2.nRecType == DFF_msofbtSpContainer ) || ( aHd2.nRecType == DFF_msofbtSpgrContainer ) )
                                                {
                                                    if ( nObjCount++ )      // skipping the first object
                                                    {
                                                        Rectangle aEmpty;
                                                        if (!aHd2.SeekToBegOfRecord(rStCtrl))
                                                            break;
                                                        SdrObject* pImpObj = ImportObj( rStCtrl, static_cast<void*>(&aProcessData), aEmpty, aEmpty );
                                                        if ( pImpObj )
                                                        {
                                                            pImpObj->SetLayer( mnBackgroundObjectsLayerID );
                                                            pMPage->NbcInsertObject( pImpObj );
                                                        }
                                                    }
                                                }
                                                if (!aHd2.SeekToEndOfRecord(rStCtrl))
                                                    break;
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
                                    auto nTagEndRecPos = SanitizeEndPos(rStCtrl, aProgTagHd.GetRecEndFilePos());
                                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nTagEndRecPos ) )
                                    {
                                        DffRecordHeader aProgTagContentHd;
                                        ReadDffRecordHeader( rStCtrl, aProgTagContentHd );
                                        switch( aProgTagContentHd.nRecType )
                                        {
                                            case DFF_msofbtAnimGroup :
                                            {
                                                css::uno::Reference< css::drawing::XDrawPage > xPage( pMPage->getUnoPage(), css::uno::UNO_QUERY );
                                                ppt::AnimationImporter aImporter( this, rStCtrl );
                                                bNewAnimationsUsed = aImporter.import( xPage, aProgTagContentHd ) > 0;
                                            }
                                            break;
                                        }
                                        if (!aProgTagContentHd.SeekToEndOfRecord(rStCtrl))
                                            break;
                                    }
                                }
                            }
                            break;
                        }
                        bool bSuccess = aHd.SeekToEndOfRecord(rStCtrl);
                        if (!bSuccess)
                        {
                            SAL_WARN("filter.ms", "Count not seek to end of record");
                            break;
                        }
                    }
                }
                rStCtrl.Seek( nFPosMerk );
                ImportPageEffect( pMPage, bNewAnimationsUsed );

                // background object
                pObj = pMPage->GetObj( 0 );
                if ( pObj && pObj->GetObjIdentifier() == OBJ_RECT )
                {
                    if ( pMPage->GetPageKind() == PageKind::Standard )
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
        if (xStbMgr)
            xStbMgr->SetState( nImportedPages++ );
    }

    // importing slide pages
    {
        sal_uInt32          nFPosMerk = rStCtrl.Tell();
        PptPageKind     ePageKind = eAktPageKind;
        sal_uInt16          nPageNum = nAktPageNum;

        SdPage* pHandoutPage = static_cast<SdPage*>(MakeBlancPage( false ));
        pHandoutPage->SetPageKind( PageKind::Handout );
        pSdrModel->InsertPage( pHandoutPage );

        sal_uInt16 nPageAnz = GetPageCount();
        if ( nPageAnz )
        {
            for ( sal_uInt16 nPage = 0; nPage < nPageAnz; nPage++ )
            {
                mePresChange = PRESCHANGE_SEMIAUTO;
                SetPageNum( nPage );
                SdPage* pPage = static_cast<SdPage*>(MakeBlancPage( false ));
                PptSlidePersistEntry* pMasterPersist = nullptr;
                if ( HasMasterPage( nPage ) )     // try to get the LayoutName from the masterpage
                {
                    sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                    pPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nMasterNum));
                    PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                    if ( pPageList && nMasterNum < pPageList->size() )
                        pMasterPersist = &(*pPageList)[ nMasterNum ];
                    pPage->SetLayoutName(static_cast<SdPage&>(pPage->TRG_GetMasterPage()).GetLayoutName());
                }
                pPage->SetPageKind( PageKind::Standard );
                pSdrModel->InsertPage( pPage );         // SJ: #i29625# because of form controls, the
                ImportPage( pPage, pMasterPersist );    //  page must be inserted before importing
                SetHeaderFooterPageSettings( pPage, pMasterPersist );
                // CWS preseng01: pPage->SetPageKind( PageKind::Standard );

                DffRecordHeader aPageHd;
                if ( SeekToAktPage( &aPageHd ) )
                {
                    bool bNewAnimationsUsed = false;

                    aPageHd.SeekToContent( rStCtrl );
                    auto nEndRecPos = SanitizeEndPos(rStCtrl, aPageHd.GetRecEndFilePos());
                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nEndRecPos ) )
                    {
                        DffRecordHeader aHd;
                        ReadDffRecordHeader( rStCtrl, aHd );
                        switch ( aHd.nRecType )
                        {
                            case PPT_PST_ProgTags :
                            {
                                DffRecordHeader aProgTagHd;
                                if ( SeekToContentOfProgTag( 10, rStCtrl, aPageHd, aProgTagHd ) )
                                {
                                    auto nHdEndRecPos = SanitizeEndPos(rStCtrl, aProgTagHd.GetRecEndFilePos());
                                    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nHdEndRecPos ) )
                                    {
                                        DffRecordHeader aProgTagContentHd;
                                        ReadDffRecordHeader( rStCtrl, aProgTagContentHd );
                                        switch( aProgTagContentHd.nRecType )
                                        {
                                            case DFF_msofbtAnimGroup :
                                            {
                                                css::uno::Reference< css::drawing::XDrawPage > xPage( pPage->getUnoPage(), css::uno::UNO_QUERY );
                                                ppt::AnimationImporter aImporter( this, rStCtrl );
                                                bNewAnimationsUsed = aImporter.import( xPage, aProgTagContentHd ) > 0;
                                            }
                                            break;

                                            case PPT_PST_HashCodeAtom :  // ???
                                            break;

                                            case PPT_PST_SlideTime10Atom :  // ??? don't know, this atom is always 8 bytes big
                                            break;                          // and is appearing in nearly every l10 progtag
                                        }
                                        if (!aProgTagContentHd.SeekToEndOfRecord(rStCtrl))
                                            break;
                                    }
                                }
                            }
                            break;

                            case PPT_PST_HeadersFooters :
                            case PPT_PST_PPDrawing :
                            default:
                            break;
                        }

                        if (!aHd.SeekToEndOfRecord(rStCtrl))
                            break;
                    }
                    ImportPageEffect( pPage, bNewAnimationsUsed );
                }

                // creating the corresponding note page
                eAktPageKind = PPT_NOTEPAGE;
                SdPage* pNotesPage = static_cast<SdPage*>(MakeBlancPage( false ));
                sal_uInt16 nNotesMasterNum = GetMasterPageIndex( nPage ) + 1;
                sal_uInt32 nNotesPageId = GetNotesPageId( nPage );
                if ( nNotesPageId )
                {
                    nImportedPages++;
                    sal_uInt16 nNotesPageIndex = m_pNotePages->FindPage( nNotesPageId );
                    if ( nNotesPageIndex == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                        nNotesPageIndex = 0;
                    SetPageNum( nNotesPageIndex, PPT_NOTEPAGE );
                    PptSlidePersistEntry* pMasterPersist2 = nullptr;
                    if ( HasMasterPage( nNotesPageIndex, PPT_NOTEPAGE ) ) // try to get the LayoutName from the masterpage
                    {
                        pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                        if ( pPageList && nNotesMasterNum < pPageList->size() )
                            pMasterPersist2 = &(*pPageList)[ nNotesMasterNum ];
                        pNotesPage->SetLayoutName( static_cast<SdPage&>(pNotesPage->TRG_GetMasterPage()).GetLayoutName() );
                    }
                    pNotesPage->SetPageKind( PageKind::Notes );
                    pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                    pSdrModel->InsertPage( pNotesPage );        // SJ: #i29625# because of form controls, the
                    ImportPage( pNotesPage, pMasterPersist2 );  // page must be inserted before importing
                    SetHeaderFooterPageSettings( pNotesPage, pMasterPersist2 );
                    pNotesPage->SetAutoLayout( AUTOLAYOUT_NOTES );
                }
                else
                {
                    pNotesPage->SetPageKind( PageKind::Notes );
                    pNotesPage->TRG_SetMasterPage(*pSdrModel->GetMasterPage(nNotesMasterNum));
                    pNotesPage->SetAutoLayout( AUTOLAYOUT_NOTES, true );
                    pSdrModel->InsertPage( pNotesPage );
                    SdrObject* pPageObj = pNotesPage->GetPresObj( PRESOBJ_PAGE );
                    if ( pPageObj )
                        static_cast<SdrPageObj*>(pPageObj)->SetReferencedPage(pSdrModel->GetPage(( nPage << 1 ) + 1));
                }

                if (xStbMgr)
                    xStbMgr->SetState( nImportedPages++ );
            }
        }
        else
        {
            // that can happen by document templates
            eAktPageKind = PPT_SLIDEPAGE;
            SdrPage* pPage = MakeBlancPage( false );
            pSdrModel->InsertPage( pPage );

            // #i37397#, trying to set the title master for the first page
            sal_uInt16 nMaster, nMasterCount = pSdrModel->GetMasterPageCount();
            SdPage* pFoundMaster = nullptr;
            for ( nMaster = 1; nMaster < nMasterCount; nMaster++ )
            {
                SdPage* pMaster = static_cast<SdPage*>( pSdrModel->GetMasterPage( nMaster ) );
                if ( pMaster->GetPageKind() == PageKind::Standard )
                {
                    SetPageNum( nMaster, PPT_MASTERPAGE );
                    if ( !pFoundMaster )
                        pFoundMaster = pMaster;
                    else if ( GetSlideLayoutAtom()->eLayout == PptSlideLayout::TITLEMASTERSLIDE )
                        pFoundMaster = pMaster;
                    if ( GetSlideLayoutAtom()->eLayout == PptSlideLayout::TITLEMASTERSLIDE )
                        break;
                }
            }
            if ( pFoundMaster )
            {
                static_cast<SdPage*>(pPage)->TRG_SetMasterPage( *pFoundMaster );
                static_cast<SdPage*>(pPage)->SetLayoutName( pFoundMaster->GetLayoutName() );
            }
            static_cast<SdPage*>(pPage)->SetAutoLayout( AUTOLAYOUT_TITLE, true, true );

            eAktPageKind = PPT_NOTEPAGE;
            SdrPage* pNPage = MakeBlancPage( false );
            pSdrModel->InsertPage( pNPage );
        }
        SetPageNum( nPageNum, ePageKind );
        rStCtrl.Seek( nFPosMerk );
    }

    // create handout and note pages
    bOk = mpDoc->CreateMissingNotesAndHandoutPages();
    if ( bOk )
    {
        for ( i = 0; i < mpDoc->GetSdPageCount( PageKind::Standard ); i++ )
        {

            // set AutoLayout
            SetPageNum( i );
            SdPage* pPage = mpDoc->GetSdPage( i, PageKind::Standard );
            AutoLayout eAutoLayout = AUTOLAYOUT_NONE;
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout )
            {
                switch ( pSlideLayout->eLayout )            // presentation layout for standard pages
                {
                    case PptSlideLayout::TITLEANDBODYSLIDE :
                    {
                        eAutoLayout = AUTOLAYOUT_ENUM;
                        PptPlaceholder nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        switch ( nID1 )
                        {
                            case PptPlaceholder::BODY :
                                eAutoLayout = AUTOLAYOUT_ENUM;
                            break;
                            case PptPlaceholder::TABLE :
                                eAutoLayout = AUTOLAYOUT_TAB;
                            break;
                            case PptPlaceholder::ORGANISZATIONCHART :
                                eAutoLayout = AUTOLAYOUT_ORG;
                            break;
                            case PptPlaceholder::GRAPH :
                                eAutoLayout = AUTOLAYOUT_CHART;
                            break;
                            case PptPlaceholder::OBJECT :
                                eAutoLayout = AUTOLAYOUT_OBJ;
                            break;
                            case PptPlaceholder::VERTICALTEXTBODY :
                                eAutoLayout = AUTOLAYOUT_TITLE_VERTICAL_OUTLINE;
                            break;
                            default: break;
                        }
                    }
                    break;

                    case PptSlideLayout::TWOCOLUMNSANDTITLE :
                    {
                        eAutoLayout = AUTOLAYOUT_2TEXT;
                        PptPlaceholder nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        PptPlaceholder nID2 = pSlideLayout->aPlaceholderId[ 2 ];
                        if ( nID1 == PptPlaceholder::BODY && nID2 == PptPlaceholder::GRAPH )
                            eAutoLayout = AUTOLAYOUT_TEXTCHART;
                        else if ( nID1 == PptPlaceholder::GRAPH && nID2 == PptPlaceholder::BODY )
                            eAutoLayout = AUTOLAYOUT_CHARTTEXT;
                        else if ( nID1 == PptPlaceholder::BODY && nID2 == PptPlaceholder::CLIPART )
                            eAutoLayout = AUTOLAYOUT_TEXTCLIP;
                        else if ( nID1 == PptPlaceholder::CLIPART && nID2 == PptPlaceholder::BODY )
                            eAutoLayout = AUTOLAYOUT_CLIPTEXT;
                        else if ( nID1 == PptPlaceholder::CLIPART && nID2 == PptPlaceholder::VERTICALTEXTBODY )
                            eAutoLayout = AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART;
                        else if ( ( nID1 == PptPlaceholder::BODY )
                            && ( ( nID2 == PptPlaceholder::OBJECT ) || ( nID2 == PptPlaceholder::MEDIACLIP ) ) )
                            eAutoLayout = AUTOLAYOUT_TEXTOBJ;
                        else if ( ( nID2 == PptPlaceholder::BODY )
                            && ( ( nID1 == PptPlaceholder::OBJECT ) || ( nID1 == PptPlaceholder::MEDIACLIP ) ) )
                            eAutoLayout = AUTOLAYOUT_OBJTEXT;
                        else if ( ( nID1 == PptPlaceholder::OBJECT ) && ( nID2 == PptPlaceholder::OBJECT  ) )
                            eAutoLayout = AUTOLAYOUT_OBJ;
                    }
                    break;

                    case PptSlideLayout::TWOROWSANDTITLE :
                    {
                        eAutoLayout = AUTOLAYOUT_2TEXT;
                        PptPlaceholder nID1 = pSlideLayout->aPlaceholderId[ 1 ];
                        PptPlaceholder nID2 = pSlideLayout->aPlaceholderId[ 2 ];
                        if ( nID1 == PptPlaceholder::BODY && nID2 == PptPlaceholder::OBJECT )
                            eAutoLayout = AUTOLAYOUT_TEXTOVEROBJ;
                        else if ( nID1 == PptPlaceholder::OBJECT && nID2 == PptPlaceholder::BODY )
                            eAutoLayout = AUTOLAYOUT_OBJOVERTEXT;
                    }
                    break;

                    case PptSlideLayout::TITLESLIDE :
                        eAutoLayout = AUTOLAYOUT_TITLE;
                    break;
                    case PptSlideLayout::ONLYTITLE :
                        eAutoLayout = AUTOLAYOUT_ONLY_TITLE;
                    break;
                    case PptSlideLayout::RIGHTCOLUMN2ROWS :
                        eAutoLayout = AUTOLAYOUT_TEXT2OBJ;
                    break;
                    case PptSlideLayout::LEFTCOLUMN2ROWS :
                        eAutoLayout = AUTOLAYOUT_2OBJTEXT;
                    break;
                    case PptSlideLayout::TOPROW2COLUMN :
                        eAutoLayout = AUTOLAYOUT_2OBJOVERTEXT;
                    break;
                    case PptSlideLayout::FOUROBJECTS :
                        eAutoLayout = AUTOLAYOUT_4OBJ;
                    break;
                    case PptSlideLayout::BIGOBJECT :
                        eAutoLayout = AUTOLAYOUT_OBJ;
                    break;
                    case PptSlideLayout::TITLERIGHTBODYLEFT :
                        eAutoLayout = AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE; // AUTOLAYOUT_ENUM;
                    break;
                    case PptSlideLayout::TITLERIGHT2BODIESLEFT :
                        eAutoLayout = AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART; // AUTOLAYOUT_TEXT2OBJ;
                    break;

                    case PptSlideLayout::BOTTOMROW2COLUMNS :
                    case PptSlideLayout::BLANCSLIDE :
                    case PptSlideLayout::MASTERSLIDE :           // layout of the standard and title master page
                    case PptSlideLayout::TITLEMASTERSLIDE :
                    case PptSlideLayout::MASTERNOTES :           // layout of the note master page
                    case PptSlideLayout::NOTESTITLEBODY :        // presentation layout for note pages
                    case PptSlideLayout::HANDOUTLAYOUT :         // presentation layout for handout
                        eAutoLayout = AUTOLAYOUT_NONE;
                    break;
                }
                if ( eAutoLayout != AUTOLAYOUT_NONE )
                    pPage->SetAutoLayout( eAutoLayout );
            }
        }

        // handout master page: auto layout
        SdPage* pHandoutMPage = mpDoc->GetMasterSdPage( 0, PageKind::Handout );
        pHandoutMPage->SetAutoLayout( AUTOLAYOUT_HANDOUT6, true, true );
    }

    sal_uInt32 nSlideCount = GetPageCount();
    for ( i = 0; ( i < nSlideCount) && ( i < maSlideNameList.size() ); i++ )
    {
        SdPage* pPage = mpDoc->GetSdPage( i, PageKind::Standard );
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
        mpDoc->SetSummationOfParagraphs();
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
                PageKind    ePageKind = PageKind::Standard;
                EditMode    eEditMode = EditMode::Page;

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
                        SAL_FALLTHROUGH;
                    case 2 :    // master
                    {
                        ePageKind = PageKind::Standard;
                        eEditMode = EditMode::MasterPage;
                    }
                    break;
                    case 5 :    // notes master
                        eEditMode = EditMode::MasterPage;
                        SAL_FALLTHROUGH;
                    case 3 :    // notes
                        ePageKind = PageKind::Notes;
                    break;
                    case 4 :    // handout
                        ePageKind = PageKind::Handout;
                    break;
                    default :
                    case 1 :    // normal
                    break;
                }
                pFrameView->SetPageKind( ePageKind );
                pFrameView->SetSelectedPage( nSelectedPage );
                pFrameView->SetViewShEditMode( eEditMode );
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
                                SdCustomShowList* pList = mpDoc->GetCustomShowList( true );
                                if ( pList )
                                {
                                    SdCustomShow* pSdCustomShow = new SdCustomShow;
                                    pSdCustomShow->SetName( aCuShow );
                                    sal_uInt32 nFound = 0;
                                    for ( sal_uInt32 nS = 0; nS < nSCount; nS++ )
                                    {
                                        sal_uInt32 nPageNumber;
                                        rStCtrl.ReadUInt32( nPageNumber );
                                        sal_uInt16 nPage = pPageList->FindPage( nPageNumber );
                                        if ( nPage != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                        {
                                            SdPage* pPage = mpDoc->GetSdPage( nPage, PageKind::Standard );
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
        // this is defaulted, maybe there is no SSDocInfoAtom
        OUStringBuffer aCustomShow;
        sal_uInt32  nFlags = 1;                 // Bit 0:   Auto advance
        sal_uInt16  nStartSlide = 0;

        // read the pres. configuration
        rStCtrl.Seek( maDocHd.GetRecBegFilePos() + 8 );
        if ( SeekToRec( rStCtrl, PPT_PST_SSDocInfoAtom, maDocHd.GetRecEndFilePos(), &aCustomShowHeader ) )
        {
            sal_uInt32  nPenColor = 0x1000000;
            sal_Int32   nRestartTime = 0x7fffffff;
            sal_Int16   nEndSlide = 0;
            rStCtrl.ReadUInt32( nPenColor )
                   .ReadInt32( nRestartTime )
                   .ReadUInt16( nStartSlide )
                   .ReadInt16( nEndSlide );

            sal_Unicode nChar;
            for ( sal_uInt32 i2 = 0; i2 < 32; i2++ )
            {
                rStCtrl.ReadUtf16( nChar );
                if ( nChar )
                    aCustomShow.append( nChar );
                else
                {
                    rStCtrl.SeekRel( ( 31 - i2 ) << 1 );
                    break;
                }
            }
            rStCtrl.ReadUInt32( nFlags );
        }
        // set the current custom show
        if ( !aCustomShow.isEmpty() )
        {
            SdCustomShowList* pList = mpDoc->GetCustomShowList();
            if ( pList )
            {
                SdCustomShow* pPtr = nullptr;
                OUString aCustomShowStr = aCustomShow.makeStringAndClear();
                for( pPtr = pList->First(); pPtr; pPtr = pList->Next() )
                {
                    if ( pPtr->GetName() == aCustomShowStr )
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
            SdPage* pPage = mpDoc->GetSdPage( nStartSlide - 1, PageKind::Standard );
            if ( pPage )
                rPresSettings.maPresPage = pPage->GetName();
        }
    }

    xStbMgr.reset();

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
    PptSlidePersistEntry& rSlidePersist = (*pList)[ nAktPageNum ];
    HeaderFooterEntry* pHFE = rSlidePersist.pHeaderFooterEntry;
    if ( pHFE )
    {
        for ( i = 0; i < 4; i++ )
        {
            bool bVisible = pHFE->IsToDisplay( i );
            if ( ( eAktPageKind == PPT_SLIDEPAGE )
                && ( rSlidePersist.aSlideAtom.aLayout.eLayout == PptSlideLayout::TITLESLIDE )
                    && ( aDocAtom.bTitlePlaceholdersOmitted  ) )
            {
                bVisible = false;
            }
            if ( bVisible && pMasterPersist )
            {
                sal_uInt32 nPosition = pHFE->NeedToImportInstance( i, rSlidePersist );
                if ( nPosition )
                {
                    Rectangle aEmpty;
                    bVisible = false;
                    rStCtrl.Seek( nPosition );
                    ProcessData aProcessData( rSlidePersist, SdPageCapsule(pPage) );
                    SdrObject* pObj = ImportObj( rStCtrl, static_cast<void*>(&aProcessData), aEmpty, aEmpty );
                    if ( pObj )
                        pPage->NbcInsertObject( pObj, 0 );
                }
            }
            OUString aPlaceHolderString = pHFE->pPlaceholder[ i ];

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

// Import of pages
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

void ImplSdPPTImport::ImportPageEffect( SdPage* pPage, const bool bNewAnimationsUsed )
{
    sal_uLong nFilePosMerk = rStCtrl.Tell();

    // set PageKind at page (up to now only PageKind::Standard or PageKind::Notes)
    if ( pPage->GetPageKind() == PageKind::Standard )
    {
        PptSlidePersistList* pPersistList = GetPageList( eAktPageKind );
        PptSlidePersistEntry* pActualSlidePersist = ( pPersistList && ( nAktPageNum < pPersistList->size() ) )
                                                        ? &(*pPersistList)[ nAktPageNum ] : nullptr;

        if ( pActualSlidePersist && ( eAktPageKind == PPT_SLIDEPAGE ) )
        {
            if ( ! ( pActualSlidePersist->aSlideAtom.nFlags & 1 ) ) // do not follow master objects ?
            {
                if(pPage->TRG_HasMasterPage())
                {
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(mnBackgroundObjectsLayerID, false);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                }
            }
        }
        DffRecordHeader aPageRecHd;
        if ( SeekToAktPage( &aPageRecHd ) )
        {
            sal_uLong nPageRecEnd = SanitizeEndPos(rStCtrl, aPageRecHd.GetRecEndFilePos());

            bool bTryTwice = ( eAktPageKind == PPT_SLIDEPAGE );
            bool bSSSlideInfoAtom = false;
            while ( true )
            {
                while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPageRecEnd ) )
                {
                    DffRecordHeader aHd;
                    ReadDffRecordHeader( rStCtrl, aHd );
                    switch ( aHd.nRecType )
                    {
                        case PPT_PST_SSSlideInfoAtom:
                        {
                            bSSSlideInfoAtom = true;
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
                                rStCtrl.ReadInt32( nSlideTime )           // time to show (in Ticks)
                                       .ReadInt32( nSoundRef )            // Index of SoundCollection
                                       .ReadSChar( nDirection )           // direction of fade effect
                                       .ReadSChar( nTransitionType )      // fade effect
                                       .ReadInt16( nBuildFlags )          // Buildflags (s.u.)
                                       .ReadSChar( nSpeed )               // speed (slow, medium, fast)
                                       .ReadSChar( nByteDummy ).ReadSChar( nByteDummy ).ReadSChar( nByteDummy );

                                switch ( nTransitionType )
                                {
                                    case PPT_TRANSITION_TYPE_BLINDS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_VERTICAL_STRIPES );        // fade vertical
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_HORIZONTAL_STRIPES );      // fade horizontal
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_CHECKER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_HORIZONTAL_CHECKERBOARD ); // fade vertical with offset ??
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_VERTICAL_CHECKERBOARD );   // fade horizontal with offset ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_COVER :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_RIGHT );         // overlay from right
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_BOTTOM );        // overlay from bottom
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_LEFT );          // overlay from left
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_TOP );           // overlay from top
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_LOWERRIGHT );    // overlay from bottom right ??
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_LOWERLEFT );     // overlay from bottom left ??
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_UPPERRIGHT );    // overlay from top right
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_MOVE_FROM_UPPERLEFT );     // overlay from top left ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_NONE :
                                    {
                                        if ( nBuildFlags )
                                        {
                                            if ( nDirection == 0 )
                                                pPage->SetFadeEffect( css::presentation::FadeEffect_NONE );                // direct
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
                                        pPage->SetFadeEffect(css::presentation::FadeEffect_DISSOLVE);                      // dissolve
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM_BARS :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_HORIZONTAL_LINES );        // horizontal lines
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_VERTICAL_LINES );          // vertical lines
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_SPLIT :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_OPEN_VERTICAL );           // open horizontal ??
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_CLOSE_VERTICAL );          // close horizontal ??
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_OPEN_HORIZONTAL );         // open vertical ??
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_CLOSE_HORIZONTAL );        // close vertical ??
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_STRIPS :
                                    {
                                        if ( nDirection == 4 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_LOWERRIGHT );    // diagonal to top left
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_LOWERLEFT );     // diagonal to top right
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_UPPERRIGHT );    // diagonal to bottom left
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_UPPERLEFT );     // diagonal to bottom right
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_PULL :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_LEFT );         // uncover to left
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_TOP );          // uncover to top
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_RIGHT );        // uncover to right
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_BOTTOM );       // uncover to bottom
                                        else if ( nDirection == 4 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_UPPERLEFT );    // uncover to top left
                                        else if ( nDirection == 5 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT );   // uncover to top right
                                        else if ( nDirection == 6 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_LOWERLEFT );    // uncover to bottom left
                                        else if ( nDirection == 7 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT );   // uncover to bottom right
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_WIPE :
                                    {
                                        if ( nDirection == 0 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_RIGHT );         // roll from right
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_BOTTOM );        // roll from bottom
                                        else if ( nDirection == 2 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_LEFT );          // roll from left
                                        else if ( nDirection == 3 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_TOP );           // roll from top
                                    }
                                    break;
                                    case PPT_TRANSITION_TYPE_RANDOM :
                                        pPage->SetFadeEffect( css::presentation::FadeEffect_RANDOM );                      // automatic
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
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_FROM_CENTER );        // fade from center
                                        else if ( nDirection == 1 )
                                            pPage->SetFadeEffect( css::presentation::FadeEffect_FADE_TO_CENTER );          // fade from the outside
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
                                    pPage->SetExcluded( true );             // don't show slide
                                if ( nBuildFlags & 16 )
                                {   // slide with sound effect
                                    pPage->SetSound( true );
                                    OUString aSoundFile( ReadSound( nSoundRef ) );
                                    pPage->SetSoundFile( aSoundFile );
                                }
                                if ( nBuildFlags & ( 1 << 6 ) )     // Loop until next sound
                                    pPage->SetLoopSound( true );
                                if ( nBuildFlags & ( 1 << 8 ) )     // Stop the previous sound
                                    pPage->SetStopSound( true );
                                break;
                            }
                        }
                    }
                    if (!aHd.SeekToEndOfRecord(rStCtrl))
                        break;
                }
                if ( bTryTwice && !bSSSlideInfoAtom )
                {
                    bTryTwice = false;
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                        if ( pPageList && ( nMasterNum < pPageList->size() ) )
                        {
                            assert( !pPageList->is_null( nMasterNum ) );
                            const PptSlidePersistEntry& rE = (*pPageList)[ nMasterNum ];
                            sal_uInt32 nOfs = rE.aPersistAtom.nReserved;
                            if ( nOfs )
                            {
                                rStCtrl.Seek( nOfs );
                                nPageRecEnd = nOfs + 16;
                                continue;
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
        SdrObjListIter aSdrIter( *pPage, SdrIterMode::Flat );
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
            Ppt97AnimationPtr pPpt97Animation = (*aIter).second;
            if( pPpt97Animation.get() )
                pPpt97Animation->createAndSetCustomAnimationEffect( (*aIter).first );
        }
    }
    rStCtrl.Seek( nFilePosMerk );
}

// import of sounds

// Not only the sounds are imported as string, they are also inserted to
// the gallery if they are not already there.
OUString ImplSdPPTImport::ReadSound(sal_uInt32 nSoundRef) const
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
            bool bRefStrValid = false;
            bool bDone = false;

            while( !bDone && SeekToRec( rStCtrl, PPT_PST_Sound, nDataLen, &aSoundRecHd ) )
            {
                sal_uInt32 nStrLen = aSoundRecHd.GetRecEndFilePos();
                OUString aRefStr;
                sal_uInt32 nPosMerk2 = rStCtrl.Tell();
                if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen, nullptr, 2 ) )
                {
                    if ( ReadString( aRefStr ) )
                        bRefStrValid = true;
                }
                if ( bRefStrValid )
                {
                    if ( OUString::number(nSoundRef) == aRefStr )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        if ( SeekToRec( rStCtrl, PPT_PST_CString, nStrLen ) )
                        {
                            ReadString( aRetval );
                            bDone = true;
                        }
                    }
                }
                if ( bDone )
                {
                    // Check if this sound file already exists.
                    // If not, it is exported to our local sound directory.
                    bool    bSoundExists = false;
                    ::std::vector< OUString > aSoundList;

                    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, aSoundList );
                    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, aSoundList );

                    for( size_t n = 0; ( n < aSoundList.size() ) && !bSoundExists; ++n )
                    {
                        INetURLObject   aURL( aSoundList[ n ] );

                        if( aURL.GetName() == aRetval )
                        {
                            aRetval = aSoundList[ n ];
                            bSoundExists = true;
                        }
                    }

                    aSoundList.clear();

                    if ( !bSoundExists )
                    {
                        rStCtrl.Seek( nPosMerk2 );
                        DffRecordHeader aSoundDataRecHd;
                        if ( SeekToRec( rStCtrl, PPT_PST_SoundData, nStrLen, &aSoundDataRecHd ) )
                        {
                            OUString aGalleryDir( SvtPathOptions().GetGalleryPath() );
                            sal_Int32 nTokenCount = comphelper::string::getTokenCount(aGalleryDir, ';');
                            INetURLObject aGalleryUserSound( aGalleryDir.getToken( nTokenCount - 1, ';' ) );

                            aGalleryUserSound.Append( aRetval );
                            sal_uInt32 nSoundDataLen = aSoundDataRecHd.nRecLen;
                            std::unique_ptr<sal_uInt8[]> pBuf( new sal_uInt8[ nSoundDataLen ] );

                            rStCtrl.ReadBytes(pBuf.get(), nSoundDataLen);
                            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aGalleryUserSound.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC );

                            if( pOStm )
                            {
                                pOStm->WriteBytes(pBuf.get(), nSoundDataLen);

                                if( pOStm->GetError() == ERRCODE_NONE )
                                {
                                    GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aGalleryUserSound.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                                    aRetval = aGalleryUserSound.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                                }

                                delete pOStm;
                            }
                        }
                    }
                }
                if ( !bDone )
                {
                    if (!aSoundRecHd.SeekToEndOfRecord(rStCtrl))
                        break;
                }
            }
        }
    }
    rStCtrl.Seek( nPosMerk );
    return aRetval;
}

// media object import, the return value is the url to the media object
OUString ImplSdPPTImport::ReadMedia( sal_uInt32 nMediaRef ) const
{
    OUString aRetVal;
    DffRecordHeader* pHd( const_cast<ImplSdPPTImport*>(this)->aDocRecManager.GetRecordHeader( PPT_PST_ExObjList ) );
    if ( pHd )
    {
        pHd->SeekToContent( rStCtrl );
        auto nEndRecPos = SanitizeEndPos(rStCtrl, pHd->GetRecEndFilePos());
        while ( ( rStCtrl.Tell() < nEndRecPos ) && aRetVal.isEmpty() )
        {
            DffRecordHeader aHdMovie;
            ReadDffRecordHeader( rStCtrl, aHdMovie );
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
                            rStCtrl.ReadUInt32( nRef );
                            if ( nRef == nMediaRef )
                            {
                                aExVideoHd.SeekToContent( rStCtrl );
                                auto nHdEndRecPos = SanitizeEndPos(rStCtrl, aExVideoHd.GetRecEndFilePos());
                                while (rStCtrl.Tell() < nHdEndRecPos)
                                {
                                    DffRecordHeader aHd;
                                    ReadDffRecordHeader( rStCtrl, aHd );
                                    switch( aHd.nRecType )
                                    {
                                        case PPT_PST_CString :
                                        {
                                            aHd.SeekToBegOfRecord( rStCtrl );
                                            OUString aStr;
                                            if ( ReadString( aStr ) )
                                            {
                                                if( osl::FileBase::getFileURLFromSystemPath( aStr, aRetVal )
                                                    == osl::FileBase::E_None )
                                                {
                                                    aRetVal = INetURLObject( aRetVal ).GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
                                                }else{
                                                    aRetVal = aStr;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    if (!aHd.SeekToEndOfRecord(rStCtrl))
                                        break;
                                }
                                break;
                            }
                        }
                    }
                }
                break;
            }
            if (!aHdMovie.SeekToEndOfRecord(rStCtrl))
                break;
        }
    }
    return aRetVal;
}

// import of objects
void ImplSdPPTImport::FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, const OUString& aMacroName )
{
    // set local information into pInfo
    if( pIAtom->nSoundRef )
    {
        pInfo->SetBookmark( ReadSound( pIAtom->nSoundRef ) );   // path to sound file in MS DOS notation
        pInfo->meClickAction = css::presentation::ClickAction_SOUND;           // RunProgramAction
    }

    switch ( pIAtom->nAction )
    {

        case 0x02 :                                         // RunProgramAction
        {
            pInfo->meClickAction = css::presentation::ClickAction_PROGRAM;
            pInfo->SetBookmark( aMacroName );                   // program name in aBookmark
        }
        break;
        case 0x03 :                                         // JumpAction
        {
            switch( pIAtom->nJump )
            {
                case 0x01 :
                    pInfo->meClickAction = css::presentation::ClickAction_NEXTPAGE;        // Next slide
                break;
                case 0x02 :
                    pInfo->meClickAction = css::presentation::ClickAction_PREVPAGE;        // Previous slide
                break;
                case 0x03 :
                    pInfo->meClickAction = css::presentation::ClickAction_FIRSTPAGE;       // First slide
                break;
                case 0x04 :
                    pInfo->meClickAction = css::presentation::ClickAction_LASTPAGE;        // last Slide
                break;
                case 0x05 :
                    pInfo->meClickAction = css::presentation::ClickAction_PREVPAGE;        // Last slide viewed
                break;
                case 0x06 :
                    pInfo->meClickAction = css::presentation::ClickAction_STOPPRESENTATION; // End show
                break;
                default :
                    pInfo->meClickAction = css::presentation::ClickAction_NONE;            // 0x00: no action, else unknown
                break;
            }
        }
        break;
        case 0x04 :
        {
            SdHyperlinkEntry* pPtr = nullptr;
            for (SdHyperlinkEntry* pEntry : aHyperList) {
                if ( pEntry->nIndex == pIAtom->nExHyperlinkId ) {
                    pPtr = pEntry;
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
                                OUString aBaseURL = pDocShell->GetMedium()->GetBaseURL();
                                OUString aBookmarkURL( pInfo->GetBookmark() );
                                INetURLObject aURL( pPtr->aTarget );
                                if( INetProtocol::NotValid == aURL.GetProtocol()
                                    && (osl::FileBase::getFileURLFromSystemPath(
                                            pPtr->aTarget, aBookmarkURL)
                                        != osl::FileBase::E_None) )
                                    aBookmarkURL.clear();
                                if( aBookmarkURL.isEmpty() )
                                    aBookmarkURL = URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), pPtr->aTarget, URIHelper::GetMaybeFileHdl() );
                                pInfo->SetBookmark( aBookmarkURL );
                                pInfo->meClickAction = css::presentation::ClickAction_PROGRAM;
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
                            pInfo->meClickAction = css::presentation::ClickAction_BOOKMARK;
                            pInfo->SetBookmark( pPtr->aConvSubString );
                        }
                    }
                    break;
                }
            }
        }
        break;
        case 0x05 :                     // OLEAction ( OLEVerb to use, 0==first, 1==second, .. )
        case 0x06 :                     // MediaAction
        case 0x07 :                     // CustomShowAction
        default :                       // 0x00: no action, else unknown action
        break;
    }
}

SdrObject* ImplSdPPTImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pObj, SdPageCapsule pPageCapsule,
                                        SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SdPage * pPage = static_cast<SdPage *>(pPageCapsule.page);
    SfxStyleSheet*  pStyleSheetAry[ 9 ];
    SdrTextObj*     pText = pObj;
    SdrObject*      pRet = pText;

    ppStyleSheetAry = nullptr;

    PresObjKind ePresKind = PRESOBJ_NONE;
    PptOEPlaceholderAtom* pPlaceHolder = pTextObj->GetOEPlaceHolderAtom();
    OUString aPresentationText;
    if ( pPlaceHolder )
    {
        switch( pPlaceHolder->nPlaceholderId )
        {
            case PptPlaceholder::MASTERNOTESSLIDEIMAGE :
            case PptPlaceholder::MASTERCENTEREDTITLE :
            case PptPlaceholder::MASTERTITLE :
            {
                ePresKind = PRESOBJ_TITLE;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PptPlaceholder::MASTERBODY :
            {
                ePresKind = PRESOBJ_OUTLINE;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PptPlaceholder::MASTERSUBTITLE :
            {
                ePresKind = PRESOBJ_TEXT;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PptPlaceholder::MASTERNOTESBODYIMAGE :
            {
                ePresKind = PRESOBJ_NOTES;
                aPresentationText = pPage->GetPresObjText( ePresKind );
            }
            break;
            case PptPlaceholder::MASTERDATE :           ePresKind = PRESOBJ_DATETIME;   break;
            case PptPlaceholder::MASTERSLIDENUMBER :    ePresKind = PRESOBJ_SLIDENUMBER;break;
            case PptPlaceholder::MASTERFOOTER :         ePresKind = PRESOBJ_FOOTER;     break;
            case PptPlaceholder::MASTERHEADER :         ePresKind = PRESOBJ_HEADER;     break;
            default: break;
        }
    }
    switch ( pTextObj->GetDestinationInstance() )
    {
        case TSS_Type::PageTitle :
        case TSS_Type::Title :
        {
            pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
            if ( pSheet )
                static_cast<SdrAttrObj*>(pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, true );
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
        }
        break;
        case TSS_Type::Subtitle :
        {
            pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
            if ( pSheet )
                static_cast<SdrAttrObj*>(pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, true );
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for subtitleobject (SJ)" );
        }
        break;
        case TSS_Type::Body :
        case TSS_Type::HalfBody :
        case TSS_Type::QuarterBody :
        {
            for ( sal_uInt16 nLevel = 9; nLevel; nLevel-- )
            {
                OUString aName( pPage->GetLayoutName() );
                aName += " ";
                aName += OUString::number( nLevel );
                pSheet = static_cast<SfxStyleSheet*>(mpDoc->GetStyleSheetPool()->Find( aName, SD_STYLE_FAMILY_MASTERPAGE ));
                if ( pSheet )
                    pText->StartListening( *pSheet );
                pStyleSheetAry[ nLevel - 1 ] = pSheet;
            }
            DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for outlinerobject (SJ)" );
            if ( pSheet )
                static_cast<SdrAttrObj*>(pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, true );
            ppStyleSheetAry = &pStyleSheetAry[ 0 ];
        }
        break;
        case TSS_Type::Notes :
        {
            if ( pPlaceHolder && ( ( pPlaceHolder->nPlaceholderId == PptPlaceholder::NOTESSLIDEIMAGE )
                || ( pPlaceHolder->nPlaceholderId == PptPlaceholder::MASTERNOTESSLIDEIMAGE ) ) )
            {
                pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
                if ( pSheet )
                    static_cast<SdrAttrObj*>(pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, true );
                DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for titleobject (SJ)" );
            }
            else
            {
                pSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_NOTES );
                DBG_ASSERT( pSheet, "ImplSdPPTImport::ApplyTextObj -> could not get stylesheet for notesobj (SJ)" );
                if ( pSheet )
                    static_cast<SdrAttrObj*>(pText)->SdrAttrObj::NbcSetStyleSheet( pSheet, true );
            }
        }
        break;
        case TSS_Type::Unused :
        case TSS_Type::TextInShape :
        {
            switch( ePresKind )
            {
                case PRESOBJ_DATETIME :
                case PRESOBJ_SLIDENUMBER :
                case PRESOBJ_FOOTER :
                case PRESOBJ_HEADER :
                    pSheet = static_cast<SfxStyleSheet*>(mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_PSEUDOSHEET_BACKGROUNDOBJECTS), SD_STYLE_FAMILY_PSEUDO ));
                break;
                default :
                    pSheet = static_cast<SfxStyleSheet*>(mpDoc->GetStyleSheetPool()->Find(SD_RESSTR(STR_STANDARD_STYLESHEET_NAME), SD_STYLE_FAMILY_GRAPHICS ));
            }
        }
        break;
        default: break;
    }

    pText = static_cast<SdrTextObj*>(SdrPowerPointImport::ApplyTextObj( pTextObj, pText, pPageCapsule, pSheet, ppStyleSheetAry ));

    if ( pPlaceHolder && pPlaceHolder->nPlaceholderId != PptPlaceholder::NONE )
    {
        if ( eAktPageKind == PPT_MASTERPAGE )
        {
            bool bCreatePlaceHolder = ( pTextObj->GetInstance() != TSS_Type::Unused );
            bool bIsHeaderFooter = ( ePresKind == PRESOBJ_HEADER) || (ePresKind == PRESOBJ_FOOTER)
                                        || (ePresKind == PRESOBJ_DATETIME) || (ePresKind == PRESOBJ_SLIDENUMBER);
            if ( bCreatePlaceHolder && ( pTextObj->GetInstance() == TSS_Type::TextInShape ) )
                bCreatePlaceHolder = bIsHeaderFooter;
            if ( bCreatePlaceHolder )
            {
                if ( !bIsHeaderFooter )
                {
                    pText->SetNotVisibleAsMaster( true );
                    pText->SetEmptyPresObj( true );
                }
                pText->SetUserCall( pPage );
                pPage->InsertPresObj( pText, ePresKind );
                SdrOutliner* pOutl = nullptr;
                if ( pTextObj->GetInstance() == TSS_Type::Notes )
                    pOutl = GetDrawOutliner( pText );
                if ( !aPresentationText.isEmpty() )
                    pPage->SetObjText( pText, pOutl, ePresKind, aPresentationText );

                if ( pPage->GetPageKind() != PageKind::Notes && pPage->GetPageKind() != PageKind::Handout)
                {
                    SfxStyleSheet* pSheet2( pPage->GetStyleSheetForPresObj( ePresKind ) );
                    if ( pSheet2 )
                    {
                        SfxItemSet& rItemSet = pSheet2->GetItemSet();
                        rItemSet.Put( static_cast<const SdrMetricItem&>(pText->GetMergedItem( SDRATTR_TEXT_LEFTDIST ) ) );
                        rItemSet.Put( static_cast<const SdrMetricItem&>(pText->GetMergedItem( SDRATTR_TEXT_RIGHTDIST )) );
                        rItemSet.Put( static_cast<const SdrMetricItem&>(pText->GetMergedItem( SDRATTR_TEXT_UPPERDIST )) );
                        rItemSet.Put( static_cast<const SdrMetricItem&>(pText->GetMergedItem( SDRATTR_TEXT_LOWERDIST )) );
                        rItemSet.Put( static_cast<const SdrTextVertAdjustItem&>(pText->GetMergedItem( SDRATTR_TEXT_VERTADJUST )) );
                        rItemSet.Put( static_cast<const SdrTextHorzAdjustItem&>(pText->GetMergedItem( SDRATTR_TEXT_HORZADJUST )) );
                        if (  pTextObj->GetInstance() ==  TSS_Type::Title
                            || pTextObj->GetInstance() == TSS_Type::Subtitle)
                        {
                            rItemSet.Put( pText->GetMergedItemSet() );
                        }
                    }
                }

                SfxItemSet aTempAttr( mpDoc->GetPool() );
                SdrMetricItem aMinHeight( makeSdrTextMinFrameHeightItem(pText->GetLogicRect().GetSize().Height()) );
                aTempAttr.Put( aMinHeight );
                SdrOnOffItem aAutoGrowHeight( makeSdrTextAutoGrowHeightItem(false) );
                aTempAttr.Put( aAutoGrowHeight );
                pText->SetMergedItemSet(aTempAttr);
            }
            else
            {
                pRet = nullptr;
            }
        }
        else
        {
            const PptSlideLayoutAtom* pSlideLayout = GetSlideLayoutAtom();
            if ( pSlideLayout || ( eAktPageKind == PPT_NOTEPAGE ) )
            {
                sal_uInt32 nPlacementId = pPlaceHolder->nPlacementId;
                PptPlaceholder nPlaceholderId = pPlaceHolder->nPlaceholderId;

                if ( eAktPageKind == PPT_SLIDEPAGE )
                {
                    PresObjKind ePresObjKind = PRESOBJ_NONE;
                    bool    bEmptyPresObj = true;
                    bool    bVertical = false;
                    if ( ( pTextObj->GetShapeType() == mso_sptRectangle ) || ( pTextObj->GetShapeType() == mso_sptTextBox ) )
                    {
                        //if a placeholder with some custom attribute,the pTextObj will keep those attr,whose text size is zero,
                        //so sdPage should renew a PresObj to process placeholder.
                        bEmptyPresObj = ( pTextObj->Count() == 0 ) || ( pTextObj->Count() == 1 && pTextObj->First()->GetTextSize() == 0 );
                        switch ( nPlaceholderId )
                        {
                            case PptPlaceholder::NOTESBODY :            ePresObjKind = PRESOBJ_NOTES;   break;
                            case PptPlaceholder::VERTICALTEXTTITLE :
                                bVertical = true;
                                SAL_FALLTHROUGH;
                            case PptPlaceholder::TITLE :                ePresObjKind = PRESOBJ_TITLE;   break;
                            case PptPlaceholder::VERTICALTEXTBODY :
                                bVertical = true;
                                SAL_FALLTHROUGH;
                            case PptPlaceholder::BODY :                 ePresObjKind = PRESOBJ_OUTLINE; break;
                            case PptPlaceholder::CENTEREDTITLE :        ePresObjKind = PRESOBJ_TITLE;   break;
                            case PptPlaceholder::SUBTITLE :             ePresObjKind = PRESOBJ_TEXT;    break;      // PRESOBJ_OUTLINE

                            default :
                            {
                                if ( pTextObj->Count() == 0 )
                                {
                                    switch ( nPlaceholderId )
                                    {
                                        case PptPlaceholder::MEDIACLIP :
                                        case PptPlaceholder::OBJECT : ePresObjKind = PRESOBJ_OBJECT; break;
                                        case PptPlaceholder::GRAPH : ePresObjKind = PRESOBJ_CHART; break;
                                        case PptPlaceholder::TABLE : ePresObjKind = PRESOBJ_TABLE; break;
                                        case PptPlaceholder::CLIPART : ePresObjKind = PRESOBJ_GRAPHIC; break;
                                        case PptPlaceholder::ORGANISZATIONCHART : ePresObjKind = PRESOBJ_ORGCHART; break;
                                        default: break;
                                    }
                                }
                            };
                        }
                    }
                    else if ( pTextObj->GetShapeType() == mso_sptPictureFrame )
                    {
                        if ( !pTextObj->Count() && dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr )
                        {
                            bEmptyPresObj = false;
                            switch ( nPlaceholderId )
                            {
                                case PptPlaceholder::MEDIACLIP :
                                case PptPlaceholder::OBJECT : ePresObjKind = PRESOBJ_OBJECT; break;
                                case PptPlaceholder::GRAPH : ePresObjKind = PRESOBJ_CHART; break;
                                case PptPlaceholder::TABLE : ePresObjKind = PRESOBJ_CALC; break;
                                case PptPlaceholder::CLIPART : ePresObjKind = PRESOBJ_GRAPHIC; break;
                                case PptPlaceholder::ORGANISZATIONCHART : ePresObjKind = PRESOBJ_ORGCHART; break;
                                default: break;
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
                            SdrObject* pPresObj = pPage->CreatePresObj( ePresObjKind, bVertical, pText->GetLogicRect(), true );
                            pPresObj->SetUserCall( pPage );

                            SfxItemSet aSet( pSdrModel->GetItemPool() );
                            ApplyAttributes( rStCtrl, aSet );
                            pPresObj->SetLogicRect(pText->GetLogicRect());
                            ApplyTextAnchorAttributes( *pTextObj, aSet );
                            //set custom font attribute of the placeholder
                            if ( pTextObj->Count() == 1 )
                            {
                                PPTParagraphObj* pPara = pTextObj->First();
                                if ( pPara && pPara->GetTextSize() == 0 )
                                {
                                    if ( PPTPortionObj * pPor = pPara->First() )
                                    {
                                        pPor->ApplyTo(aSet, (SdrPowerPointImport&)*this, pTextObj->GetDestinationInstance());
                                    }
                                }
                            }
                            pPresObj->SetMergedItemSet(aSet);

                            if ((eAktPageKind != PPT_NOTEPAGE) && (nPlacementId != 0xffffffff) && pPage->TRG_HasMasterPage())
                            {
                                SdrObject* pTitleObj = static_cast<SdPage&>(pPage->TRG_GetMasterPage()).GetPresObj( PRESOBJ_TITLE );
                                SdrObject* pOutlineObj = static_cast<SdPage&>(pPage->TRG_GetMasterPage()).GetPresObj( PRESOBJ_OUTLINE );

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
                                            pPresObj->SetUserCall( nullptr );
                                    }
                                    break;

                                    case 1:
                                    {
                                        if ( pSlideLayout->eLayout == PptSlideLayout::TITLEANDBODYSLIDE )
                                        {   // position in outline area
                                            if ( aLogicRect != aOutlineRect )
                                                pPresObj->SetUserCall( nullptr );
                                        }
                                        else if ( pSlideLayout->eLayout == PptSlideLayout::TWOCOLUMNSANDTITLE )
                                        {   // position in outline area left
                                            if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                    (double)aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                    (double)aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall(nullptr);
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PptSlideLayout::TWOROWSANDTITLE )
                                        {   // position in outline area top
                                            if (std::abs(aLogicRect.Left()  - aOutlineRect.Left())  > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( nullptr );
                                            }
                                        }
                                        else if (std::abs(aLogicRect.Left() - aOutlineRect.Left()) > MAX_USER_MOVE ||
                                                 std::abs(aLogicRect.Top()  - aOutlineRect.Top())  > MAX_USER_MOVE)
                                        {   // position in outline area top left
                                            pPresObj->SetUserCall( nullptr );
                                        }
                                    }
                                    break;

                                    case 2:
                                    {
                                        if ( pSlideLayout->eLayout == PptSlideLayout::TWOCOLUMNSANDTITLE )
                                        {   // position in outline area right
                                            if (std::abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Top()    - aOutlineRect.Top())    > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                (double)aLogicSize.Width()  / aOutlineSize.Width()   < 0.48          ||
                                                (double)aLogicSize.Width()  / aOutlineSize.Width()   > 0.5)
                                            {
                                                pPresObj->SetUserCall( nullptr );
                                            }
                                        }
                                        else if ( pSlideLayout->eLayout == PptSlideLayout::TWOROWSANDTITLE )
                                        {   // position in outline area bottom
                                            if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE ||
                                                std::abs(aLogicRect.Right()  - aOutlineRect.Right())  > MAX_USER_MOVE)
                                            {
                                                pPresObj->SetUserCall( nullptr );
                                            }
                                        }
                                        else if (std::abs(aLogicRect.Right() - aOutlineRect.Right()) > MAX_USER_MOVE ||
                                                 std::abs(aLogicRect.Top()   - aOutlineRect.Top())   > MAX_USER_MOVE)
                                        {   // position in outline area top right
                                            pPresObj->SetUserCall(nullptr);
                                        }
                                    }
                                    break;

                                    case 3:
                                    {   // position in outline area bottom left
                                        if (std::abs(aLogicRect.Left()   - aOutlineRect.Left())   > MAX_USER_MOVE ||
                                            std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
                                        {
                                            pPresObj->SetUserCall( nullptr );
                                        }
                                    }
                                    break;

                                    case 4:
                                    {   // position in outline area bottom right
                                        if (std::abs(aLogicRect.Right() - aOutlineRect.Right())   > MAX_USER_MOVE ||
                                            std::abs(aLogicRect.Bottom() - aOutlineRect.Bottom()) > MAX_USER_MOVE)
                                        {
                                            pObj->SetUserCall( nullptr );
                                        }
                                    }
                                    break;
                                }
                            }
                            pRet = nullptr;    // return zero cause this obj was already inserted by CreatePresObj
                        }
                    }
                    else if ( !pTextObj->Count() )
                        pRet = nullptr;
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
        if (dynamic_cast<const SdrPageObj*>(pObj))
        {
            const ProcessData* pProcessData=static_cast<const ProcessData*>(pData);
            if( pProcessData->pPage.page )
                static_cast<SdPage *>(pProcessData->pPage.page)->InsertPresObj(
                    pObj, PRESOBJ_PAGE );
        }

        DffRecordHeader aMasterShapeHd;

        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            bool bInhabitanceChecked = false;
            bool bAnimationInfoFound = false;

            DffRecordHeader& rHdClientData = *maShapeRecords.Current();
            while( true )
            {
                sal_uInt32 nClientDataLen = SanitizeEndPos(rSt, rHdClientData.GetRecEndFilePos());
                DffRecordHeader aHd;
                do
                {
                    ReadDffRecordHeader( rSt, aHd );
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
                                            dynamic_cast< SdrObjGroup *>( pObj ) ==  nullptr &&
                                            pAnimation->HasAnimateAssociatedShape() )
                                        {
                                            const SfxItemSet& rObjItemSet = pObj->GetMergedItemSet();

                                            drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(rObjItemSet.Get(XATTR_FILLSTYLE)).GetValue();
                                            drawing::LineStyle eLineStyle = static_cast<const XLineStyleItem&>(rObjItemSet.Get(XATTR_LINESTYLE)).GetValue();

                                            if ( ( eFillStyle == drawing::FillStyle_NONE ) && ( eLineStyle == drawing::LineStyle_NONE ) )
                                                bDontAnimateInvisibleShape = true;
                                        }
                                    }
                                    if( bDontAnimateInvisibleShape )
                                        pAnimation->SetAnimateAssociatedShape(false);

                                    //maybe some actions necessary to ensure that animations on master pages are played before animations on normal pages
                                    //maybe todo in future: bool bIsEffectOnMasterPage = !bInhabitanceChecked;?

                                    maAnimations[pObj] = pAnimation;

                                    bAnimationInfoFound = true;
                                }
                            }
                        }
                        break;
                        case PPT_PST_InteractiveInfo:
                        {
                            sal_uInt32 nFilePosMerk2 = rSt.Tell();
                            OUString aMacroName;

                            if(SeekToRec( rSt, PPT_PST_CString, nHdRecEnd ) )
                                ReadString(aMacroName);

                            rSt.Seek( nFilePosMerk2 );
                            DffRecordHeader aHdInteractiveInfoAtom;
                            if ( SeekToRec( rSt, PPT_PST_InteractiveInfoAtom, nHdRecEnd, &aHdInteractiveInfoAtom ) )
                            {
                                PptInteractiveInfoAtom aInteractiveInfoAtom;
                                ReadPptInteractiveInfoAtom( rSt, aInteractiveInfoAtom );

                                // interactive object
                                SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj, true);

                                FillSdAnimationInfo( pInfo, &aInteractiveInfoAtom, aMacroName );
                                if ( aInteractiveInfoAtom.nAction == 6 ) // Sj -> media action
                                {
                                    rHdClientData.SeekToContent( rStCtrl );
                                    DffRecordHeader aObjRefAtomHd;
                                    if ( SeekToRec( rSt, PPT_PST_ExObjRefAtom, nHdRecEnd, &aObjRefAtomHd ) )
                                    {
                                        sal_uInt32 nRef;
                                        rSt.ReadUInt32( nRef );
                                        OUString aMediaURL( ReadMedia( nRef ) );
                                        if ( aMediaURL.isEmpty() )
                                            aMediaURL = ReadSound( nRef );
                                        if ( !aMediaURL.isEmpty() )
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

                                            SdrObject::Free( pObj );
                                            pObj = pMediaObj;  // SJ: hoping that pObj is not inserted in any list
                                            pMediaObj->setURL( aMediaURL, ""/*TODO?*/ );
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    if (!aHd.SeekToEndOfRecord(rSt))
                        break;
                }
                while( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nClientDataLen ) );

                if ( bInhabitanceChecked || bAnimationInfoFound )
                    break;
                bInhabitanceChecked = true;
                if ( ! ( IsProperty( DFF_Prop_hspMaster ) && SeekToShape( rSt, pData, GetPropertyValue( DFF_Prop_hspMaster, 0 ) ) ) )
                    break;
                ReadDffRecordHeader( rSt, aMasterShapeHd );
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
ImplSdPPTImport::ReadFormControl( tools::SvRef<SotStorage>& rSrc1, css::uno::Reference< css::form::XFormComponent > & rFormComp ) const
{
    uno::Reference< frame::XModel > xModel;
    if (  mpDoc->GetDocSh() )
    {
        xModel = mpDoc->GetDocSh()->GetModel();
        oox::ole::MSConvertOCXControls aCtrlImporter( xModel );
        return aCtrlImporter.ReadOCXStorage( rSrc1, rFormComp );
    }
    return false;
}

// exported function
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL ImportPPT(
        SdDrawDocument* pDocument, SvStream& rDocStream, SotStorage& rStorage, SfxMedium& rMedium )
{
    std::unique_ptr<SdPPTImport> pImport( new SdPPTImport( pDocument, rDocStream, rStorage, rMedium ));
    bool bRet = pImport->Import();
    return bRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL TestImportPPT(const OUString &rURL)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    tools::SvRef<SotStorage> xStorage(new SotStorage(aFileStream));
    if (xStorage->GetError())
        return false;

    tools::SvRef<SotStorageStream> xDocStream(xStorage->OpenSotStream( "PowerPoint Document", StreamMode::STD_READ));
    if ( !xDocStream.is() )
        return false;

    SdDLL::Init();

    SfxMedium aSrcMed(rURL, StreamMode::STD_READ);

    xDocStream->SetVersion(xStorage->GetVersion());
    xDocStream->SetCryptMaskKey(xStorage->GetKey());

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();

    bool bRet = ImportPPT(pDoc, *xDocStream, *xStorage, aSrcMed);

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
