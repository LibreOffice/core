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

#include "eppt.hxx"
#include "epptdef.hxx"
#include "pptexanimations.hxx"
#include <o3tl/any.hxx>
#include <tools/globname.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <comphelper/sequence.hxx>
#include <tools/zcodec.hxx>
#include <filter/msfilter/classids.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/docinf.hxx>
#include <oox/export/utils.hxx>
#include <oox/ole/olehelper.hxx>
#include <memory>

class SfxObjectShell;
    // complete SfxObjectShell for SaveVBA under -fsanitize=function

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::beans::XPropertySet;

//============================ PPTWriter ==================================

PPTWriter::PPTWriter( tools::SvRef<SotStorage> const & rSvStorage,
            css::uno::Reference< css::frame::XModel > const & rXModel,
            css::uno::Reference< css::task::XStatusIndicator > const & rXStatInd,
            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags ) :
    PPTWriterBase           ( rXModel, rXStatInd ),
    mnCnvrtFlags            ( nCnvrtFlags ),
    mbStatus                ( false ),
    mnStatMaxValue          ( 0 ),
    mnLatestStatValue       ( 0 ),
    mnTextStyle( 0 ),
    mbFontIndependentLineSpacing( false ),
    mnTextSize( 0 ),
    mrStg                   ( rSvStorage ),
    mnVBAOleOfs             ( 0 ),
    mpVBA                   ( pVBA ),
    mnExEmbed               ( 0 ),
    mpExEmbed               ( new SvMemoryStream ),
    mnPagesWritten          ( 0 ),
    mnTxId                  ( 0x7a2f64 ),
    mnDiaMode               ( 0 ),
    mnShapeMasterTitle      ( 0 ),
    mnShapeMasterBody       ( 0 )
{
}

void PPTWriter::exportPPTPre( const std::vector< css::beans::PropertyValue >& rMediaData )
{
    if ( !mrStg.is() )
        return;

    if ( mXStatusIndicator.is() )
    {
        mbStatusIndicator = true;
        mnStatMaxValue = ( mnPages + mnMasterPages ) * 5;
        mXStatusIndicator->start( "PowerPoint Export", mnStatMaxValue + ( mnStatMaxValue >> 3 ) );
    }

    SvGlobalName aGName(MSO_PPT8_CLASSID);
    mrStg->SetClass( aGName, SotClipboardFormatId::NONE, "MS PowerPoint 97" );

    if ( !ImplCreateCurrentUserStream() )
        return;

    mpStrm.reset( mrStg->OpenSotStream( "PowerPoint Document" ) );
    if ( !mpStrm )
        return;

    if ( !mpPicStrm )
        mpPicStrm.reset( mrStg->OpenSotStream( "Pictures" ) );

    auto aIter = std::find_if(rMediaData.begin(), rMediaData.end(),
        [](const css::beans::PropertyValue& rProp) { return rProp.Name == "BaseURI"; });
    if (aIter != rMediaData.end())
        (*aIter).Value >>= maBaseURI;
    mpPptEscherEx.reset( new PptEscherEx( *mpStrm, maBaseURI ) );
}

void PPTWriter::exportPPTPost( )
{
    if ( !ImplCloseDocument() )
        return;

    if ( mbStatusIndicator )
    {
        mXStatusIndicator->setText( "PowerPoint Export" );
        sal_uInt32 nValue = mnStatMaxValue + ( mnStatMaxValue >> 3 );
        if ( nValue > mnLatestStatValue )
        {
            mXStatusIndicator->setValue( nValue );
            mnLatestStatValue = nValue;
        }
    }

    ImplWriteOLE();

    ImplWriteVBA();

    ImplWriteAtomEnding();

    ImplCreateDocumentSummaryInformation();

    mbStatus = true;
};

static void ImplExportComments( const uno::Reference< drawing::XDrawPage >& xPage, SvMemoryStream& rBinaryTagData10Atom );

void PPTWriter::ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                bool bHasBackground, Reference< XPropertySet > const & aXBackgroundPropSet )
{
    Any aAny;

    const PHLayout& rLayout = GetLayout( mXPagePropSet );
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Slide | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Slide );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    mpStrm->WriteInt32( static_cast<sal_Int32>(rLayout.nLayout) );
    mpStrm->WriteBytes(rLayout.nPlaceHolder, 8);    // placeholderIDs (8 parts)
    mpStrm->WriteUInt32( nMasterNum | 0x80000000 )  // master ID (equals 0x80000000 on a master page)
           .WriteUInt32( nPageNum + 0x100 )         // notes ID (equals null if no notes are present)
           .WriteUInt16( nMode )
           .WriteUInt16( 0 );                       // padword

    mnDiaMode = 0;
    bool bVisible = true;
    css::presentation::FadeEffect eFe = css::presentation::FadeEffect_NONE;

    if ( GetPropertyValue( aAny, mXPagePropSet, "Visible" ) )
        aAny >>= bVisible;
    if ( GetPropertyValue( aAny, mXPagePropSet, "Change" ) )
    {
        switch ( *o3tl::doAccess<sal_Int32>(aAny) )
        {
            case 1 :        // automatic
                mnDiaMode++;
                [[fallthrough]];
            case 2 :        // semi-automatic
                mnDiaMode++;
                break;
            default :
            case 0 :        // manual
            break;
        }
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, "Effect" ) )
        aAny >>= eFe;

    sal_uInt32  nSoundRef = 0;
    bool    bIsSound = false;
    bool    bStopSound = false;
    bool    bLoopSound = false;

    if ( GetPropertyValue( aAny, mXPagePropSet, "Sound" ) )
    {
        OUString aSoundURL;
        if ( aAny >>= aSoundURL )
        {
            nSoundRef = maSoundCollection.GetId( aSoundURL );
            bIsSound = true;
        }
        else
            aAny >>= bStopSound;
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, "LoopSound" ) )
        aAny >>= bLoopSound;

    bool bNeedsSSSlideInfoAtom = !bVisible
                                || ( mnDiaMode == 2 )
                                || bIsSound
                                || bStopSound
                                || ( eFe != css::presentation::FadeEffect_NONE );
    if ( bNeedsSSSlideInfoAtom )
    {
        sal_uInt8   nDirection = 0;
        sal_uInt8   nTransitionType = 0;
        sal_uInt16  nBuildFlags = 1;        // advance by mouseclick
        sal_Int32   nSlideTime = 0;         // still has to !!!
        sal_uInt8   nSpeed = 1;

        if ( GetPropertyValue( aAny, mXPagePropSet, "TransitionDuration" ) )
        {
            css::presentation::AnimationSpeed aAs;
            double fTransitionDuration = -1.0;
            aAny >>= fTransitionDuration;

            if (fTransitionDuration >= 0)
            {
                if (fTransitionDuration <= 0.5)
                {
                    aAs = css::presentation::AnimationSpeed::AnimationSpeed_FAST;
                }
                else if (fTransitionDuration >= 1.0)
                {
                    aAs = css::presentation::AnimationSpeed::AnimationSpeed_SLOW;
                }
                else
                {
                    aAs = css::presentation::AnimationSpeed::AnimationSpeed_MEDIUM;
                }
            }
            else
                aAs = css::presentation::AnimationSpeed::AnimationSpeed_MEDIUM;

            nSpeed = static_cast<sal_uInt8>(aAs);
        }
        sal_Int16 nTT = 0;
        if ( GetPropertyValue( aAny, mXPagePropSet, "TransitionType" )
            && ( aAny >>= nTT ) )
        {
            sal_Int16 nTST = 0;
            if ( GetPropertyValue( aAny, mXPagePropSet, "TransitionSubtype" )
                && ( aAny >>= nTST ) )
                nTransitionType = GetTransition( nTT, nTST, eFe, 0, nDirection );

        }
        if ( !nTransitionType )
            nTransitionType = GetTransition( eFe, nDirection );
        if ( mnDiaMode == 2 )                                   // automatic ?
            nBuildFlags |= 0x400;
        if ( !bVisible )
            nBuildFlags |= 4;
        if ( bIsSound )
            nBuildFlags |= 16;
        if ( bLoopSound )
            nBuildFlags |= 64;
        if ( bStopSound )
            nBuildFlags |= 256;

        if ( GetPropertyValue( aAny, mXPagePropSet, "Duration" ) )// duration of this slide
            nSlideTime = *o3tl::doAccess<sal_Int32>(aAny) << 10;        // in ticks

        mpPptEscherEx->AddAtom( 16, EPP_SSSlideInfoAtom );
        mpStrm->WriteInt32( nSlideTime )       // standtime in ticks
               .WriteUInt32( nSoundRef )
               .WriteUChar( nDirection )
               .WriteUChar( nTransitionType )
               .WriteUInt16( nBuildFlags )
               .WriteUChar( nSpeed )
               .WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 );
    }

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(nullptr,nullptr);
    ImplWritePage( rLayout, aSolverContainer, NORMAL, false, nPageNum );    // the shapes of the pages are created in the PPT document
    mpPptEscherEx->LeaveGroup();

    if ( bHasBackground )
        ImplWriteBackground( aXBackgroundPropSet );
    else
    {
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle,
                                 ShapeFlag::Background | ShapeFlag::HaveShapeProperty );
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
        aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
        aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                // if true, this is the background shape
        aPropOpt.Commit( *mpStrm );
        mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
    }

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x99cc00 ).WriteUInt32( 0xcc3333 ).WriteUInt32( 0xffcccc ).WriteUInt32( 0xb2b2b2 );

    SvMemoryStream aBinaryTagData10Atom;
    ImplExportComments( mXDrawPage, aBinaryTagData10Atom );
    SvMemoryStream amsofbtAnimGroup;
    ppt::AnimationExporter aExporter( aSolverContainer, maSoundCollection );
    aExporter.doexport( mXDrawPage, amsofbtAnimGroup );
    sal_uInt32 nmsofbtAnimGroupSize = amsofbtAnimGroup.Tell();
    if ( nmsofbtAnimGroupSize )
    {
        {
            EscherExAtom aMagic2( aBinaryTagData10Atom, 0x2eeb );
            aBinaryTagData10Atom.WriteUInt32( 0x01c45df9 )
                                .WriteUInt32( 0xe1471b30 );
        }
        {
            EscherExAtom aMagic( aBinaryTagData10Atom, 0x2b00 );
            aBinaryTagData10Atom.WriteUInt32( 0 );
        }
        aBinaryTagData10Atom.WriteBytes(amsofbtAnimGroup.GetData(), amsofbtAnimGroup.Tell());
        {
            EscherExContainer aMagic2( aBinaryTagData10Atom, 0x2b02 );
        }
    }
    if ( aBinaryTagData10Atom.Tell() )
    {
        EscherExContainer aProgTags     ( *mpStrm, EPP_ProgTags );
        EscherExContainer aProgBinaryTag( *mpStrm, EPP_ProgBinaryTag );
        {
            EscherExAtom aCString( *mpStrm, EPP_CString );
            mpStrm->WriteUInt32( 0x5f005f )
                   .WriteUInt32( 0x50005f )
                   .WriteUInt32( 0x540050 )
                   .WriteUInt16( 0x31 )
                   .WriteUInt16( 0x30 );
        }
        {
            EscherExAtom aBinaryTagData( *mpStrm, EPP_BinaryTagData );
            mpStrm->WriteBytes(aBinaryTagData10Atom.GetData(), aBinaryTagData10Atom.Tell());
        }
    }
    mpPptEscherEx->CloseContainer();    // EPP_Slide
}

void PPTWriter::ImplWriteSlideMaster( sal_uInt32 nPageNum, Reference< XPropertySet > const & aXBackgroundPropSet )
{
    if (!aXBackgroundPropSet)
        return;
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainMaster | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_MainMaster );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    mpStrm->WriteInt32( static_cast<sal_Int32>(EppLayout::TITLEANDBODYSLIDE) )  // slide layout -> title and body slide
           .WriteUChar( 1 ).WriteUChar( 2 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 )     // placeholderID
           .WriteUInt32( 0 )        // master ID (equals null at a master page)
           .WriteUInt32( 0 )        // notes ID (equals null if no notes are present)
           .WriteUInt16( 0 )        // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
           .WriteUInt16( 0 );       // padword

    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x99cc00 ).WriteUInt32( 0xcc3333 ).WriteUInt32( 0xffcccc ).WriteUInt32( 0xb2b2b2 );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xff0000 ).WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x00ffff ).WriteUInt32( 0x0099ff ).WriteUInt32( 0xffff00 ).WriteUInt32( 0x0000ff ).WriteUInt32( 0x969696 );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xccffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x336666 ).WriteUInt32( 0x008080 ).WriteUInt32( 0x339933 ).WriteUInt32( 0x000080 ).WriteUInt32( 0xcc3300 ).WriteUInt32( 0x66ccff );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x333333 ).WriteUInt32( 0x000000 ).WriteUInt32( 0xdddddd ).WriteUInt32( 0x808080 ).WriteUInt32( 0x4d4d4d ).WriteUInt32( 0xeaeaea );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x66ccff ).WriteUInt32( 0xff0000 ).WriteUInt32( 0xcc00cc ).WriteUInt32( 0xc0c0c0 );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0xc0c0c0 ).WriteUInt32( 0xff6600 ).WriteUInt32( 0x0000ff ).WriteUInt32( 0x009900 );
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0xff9933 ).WriteUInt32( 0xccff99 ).WriteUInt32( 0xcc00cc ).WriteUInt32( 0xb2b2b2 );

    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        // the auto color is dependent to the page background,so we have to set a page that is in the right context
        if ( nInstance == EPP_TEXTTYPE_Notes )
            (void)GetPageByIndex(0, NOTICE);
        else
            (void)GetPageByIndex(0, MASTER);

        mpPptEscherEx->BeginAtom();

        bool bSimpleText = false;

        mpStrm->WriteUInt16( 5 );                           // paragraph count

        for ( sal_uInt16 nLev = 0; nLev < 5; nLev++ )
        {
            if ( nInstance >= EPP_TEXTTYPE_CenterBody )
            {
                bSimpleText = true;
                mpStrm->WriteUInt16( nLev );
            }
            mpStyleSheet->mpParaSheet[ nInstance ]->Write( *mpStrm, nLev, bSimpleText, mXPagePropSet );
            mpStyleSheet->mpCharSheet[ nInstance ]->Write( *mpStrm, nLev, bSimpleText, mXPagePropSet );
        }
        mpPptEscherEx->EndAtom( EPP_TxMasterStyleAtom, 0, nInstance );
    }
    GetPageByIndex( nPageNum, MASTER );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );

    mpPptEscherEx->EnterGroup(nullptr,nullptr);
    ImplWritePage( GetLayout( 0 ), aSolverContainer, MASTER, true );    // the shapes of the pages are created in the PPT document
    mpPptEscherEx->LeaveGroup();

    ImplWriteBackground( aXBackgroundPropSet );

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x99cc00 ).WriteUInt32( 0xcc3333 ).WriteUInt32( 0xffcccc ).WriteUInt32( 0xb2b2b2 );

    if ( aBuExMasterStream.Tell() )
    {
        ImplProgTagContainer( mpStrm.get(), &aBuExMasterStream );
    }
    mpPptEscherEx->CloseContainer();    // EPP_MainMaster
};

PPTWriter::~PPTWriter()
{
    mpExEmbed.reset();
    mpPptEscherEx.reset();
    mpCurUserStrm.reset();
    mpPicStrm.reset();
    mpStrm.reset();
    maStyleSheetList.clear();
    maExOleObj.clear();
    if ( mbStatusIndicator )
        mXStatusIndicator->end();
}

bool PPTWriter::ImplCreateCurrentUserStream()
{
    mpCurUserStrm.reset( mrStg->OpenSotStream( "Current User" ) );
    if ( !mpCurUserStrm )
        return false;
    char pUserName[] = "Current User";
    sal_uInt32 nLenOfUserName = strlen( pUserName );
    sal_uInt32 nSizeOfRecord = 0x14 + ( ( nLenOfUserName + 4 ) & ~ 3 );

    mpCurUserStrm->WriteUInt16( 0 ).WriteUInt16( EPP_CurrentUserAtom ).WriteUInt32( nSizeOfRecord );
    mpCurUserStrm->WriteUInt32( 0x14 )                  // Len
                  .WriteUInt32( 0xe391c05f );           // Magic

    sal_uInt32 nEditPos = mpCurUserStrm->Tell();
    mpCurUserStrm->WriteUInt32( 0x0 )                   // OffsetToCurrentEdit;
                  .WriteUInt16( nLenOfUserName )
                  .WriteUInt16( 0x3f4 )                 // DocFileVersion
                  .WriteUChar( 3 )                      // MajorVersion
                  .WriteUChar( 0 )                      // MinorVersion
                  .WriteUInt16( 0 );                    // Pad Word
    pUserName[ nLenOfUserName ] = 8;
    mpCurUserStrm->WriteBytes(pUserName, nLenOfUserName + 1);
    for ( sal_uInt32 i = 0x15 + nLenOfUserName; i < nSizeOfRecord; i++ )
    {
        mpCurUserStrm->WriteUChar( 0 );                 // pad bytes
    }
    mpCurUserStrm->Seek( nEditPos );
    return true;
};

void PPTWriter::ImplCreateDocumentSummaryInformation()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mXModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());

    if (!xDocProps.is())
        return;

    // no idea what this is...
    static const sal_Int8 aGuid[ 0x52 ] =
    {
        0x4e, 0x00, 0x00, 0x00,
        '{',0,'D',0,'B',0,'1',0,'A',0,'C',0,'9',0,'6',0,'4',0,'-',0,
        'E',0,'3',0,'9',0,'C',0,'-',0,'1',0,'1',0,'D',0,'2',0,'-',0,
        'A',0,'1',0,'E',0,'F',0,'-',0,'0',0,'0',0,'6',0,'0',0,'9',0,
        '7',0,'D',0,'A',0,'5',0,'6',0,'8',0,'9',0,'}',0
    };
    uno::Sequence<sal_Int8> aGuidSeq(aGuid, 0x52);

    SvMemoryStream  aHyperBlob;
    ImplCreateHyperBlob( aHyperBlob );

    auto nHyperLength = static_cast<sal_Int32>(aHyperBlob.Tell());
    const sal_Int8* pBlob(
        static_cast<const sal_Int8*>(aHyperBlob.GetData()));
    auto aHyperSeq = comphelper::arrayToSequence<sal_Int8>(pBlob, nHyperLength);

    if ( mnCnvrtFlags & 0x8000 )
    {
        uno::Sequence<sal_Int8> aThumbSeq;
        if ( GetPageByIndex( 0, NORMAL ) && ImplGetPropertyValue( mXPagePropSet, "PreviewBitmap" ) )
        {
            aThumbSeq = *o3tl::doAccess<uno::Sequence<sal_Int8>>(mAny);
        }
        sfx2::SaveOlePropertySet( xDocProps, mrStg.get(),
                &aThumbSeq, &aGuidSeq, &aHyperSeq);
    }
    else
    {
        sfx2::SaveOlePropertySet( xDocProps, mrStg.get(),
                nullptr, &aGuidSeq, &aHyperSeq );
    }
}

void PPTWriter::ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId )
{
    if ( rSt.Tell() )
    {
        aBuExOutlineStream.WriteUInt32( ( EPP_PST_ExtendedParagraphHeaderAtom << 16 )
                                        | ( nRef << 4 ) )
                           .WriteUInt32( 8 )
                           .WriteUInt32( nSlideId )
                           .WriteUInt32( nInstance );
        aBuExOutlineStream.WriteBytes(rSt.GetData(), rSt.Tell());
    }
}

void PPTWriter::ImplCreateHeaderFooterStrings( SvStream& rStrm, css::uno::Reference< css::beans::XPropertySet > const & rXPagePropSet )
{
    if ( !rXPagePropSet.is() )
        return;

    OUString aString;
    css::uno::Any aAny;
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "HeaderText", true ) )
    {
        if ( aAny >>= aString )
            PPTWriter::WriteCString( rStrm, aString, 1 );
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "FooterText", true ) )
    {
        if ( aAny >>= aString )
            PPTWriter::WriteCString( rStrm, aString, 2 );
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "DateTimeText", true ) )
    {
        if ( aAny >>= aString )
            PPTWriter::WriteCString( rStrm, aString );
    }
}

void PPTWriter::ImplCreateHeaderFooters( css::uno::Reference< css::beans::XPropertySet > const & rXPagePropSet )
{
    if ( !rXPagePropSet.is() )
        return;

    bool bVal = false;
    sal_uInt32 nVal = 0;
    css::uno::Any aAny;
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "IsHeaderVisible", true ) )
    {
        if ( ( aAny >>= bVal ) && bVal )
            nVal |= 0x100000;
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "IsFooterVisible", true ) )
    {
        if ( ( aAny >>= bVal ) && bVal )
            nVal |= 0x200000;
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "IsDateTimeVisible", true ) )
    {
        if ( ( aAny >>= bVal ) && bVal )
            nVal |= 0x010000;
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "IsPageNumberVisible", true ) )
    {
        if ( ( aAny >>= bVal ) && bVal )
            nVal |= 0x080000;
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "IsDateTimeFixed", true ) )
    {
        if ( ( aAny >>= bVal ) && !bVal )
            nVal |= 0x20000;
        else
            nVal |= 0x40000;
    }
    if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, "DateTimeFormat", true ) )
    {
        sal_Int32 nFormat = *o3tl::doAccess<sal_Int32>(aAny);
        SvxDateFormat eDateFormat = static_cast<SvxDateFormat>( nFormat & 0xf );
        SvxTimeFormat eTimeFormat = static_cast<SvxTimeFormat>( ( nFormat >> 4 ) & 0xf );
        switch( eDateFormat )
        {
            case SvxDateFormat::F :
                nFormat = 1;
            break;
            case SvxDateFormat::D :
                nFormat = 2;
            break;
            case SvxDateFormat::C :
                nFormat = 4;
            break;
            default:
            case SvxDateFormat::A :
                nFormat = 0;
        }
        switch( eTimeFormat )
        {
            case SvxTimeFormat::HH24_MM :
                nFormat = 9;
            break;
            case SvxTimeFormat::HH12_MM :
                nFormat = 11;
            break;
            case SvxTimeFormat::HH24_MM_SS :
                nFormat = 10;
            break;
            case SvxTimeFormat::HH12_MM_SS :
                nFormat = 12;
            break;
            default:
                break;
        }
        nVal |= nFormat;
    }

    mpPptEscherEx->OpenContainer( EPP_HeadersFooters );
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    mpStrm->WriteUInt32( nVal );
    ImplCreateHeaderFooterStrings( *mpStrm, rXPagePropSet );
    mpPptEscherEx->CloseContainer();
}

bool PPTWriter::ImplCreateDocument()
{
    sal_uInt32 i;
    sal_uInt16 nSlideType = EPP_SLIDESIZE_TYPECUSTOM;

    sal_uInt32 nWidth = maDestPageSize.Width;
    sal_uInt32 nHeight = maDestPageSize.Height;

    if ( ( nWidth == 0x1680 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEONSCREEN;
    else if ( ( nWidth == 0x1200 ) && ( nHeight == 0x240 ) )
        nSlideType = EPP_SLIDESIZE_TYPEBANNER;
    else if ( ( nWidth == 0x1950 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPE35MM;
    else if ( ( nWidth == 0x1860 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEA4PAPER;

    mpPptEscherEx->OpenContainer( EPP_Document );
    // CREATE DOCUMENT ATOM
    mpPptEscherEx->AddAtom( 40, EPP_DocumentAtom, 1 );
    mpStrm->WriteUInt32( nWidth )                           // Slide Size in Master coordinates X
           .WriteUInt32( nHeight )                          //   "     "   "    "        "      Y
           .WriteInt32( maNotesPageSize.Width )     // Notes Page Size                  X
           .WriteInt32( maNotesPageSize.Height )    //   "     "   "                    Y
           .WriteInt32( 1 ).WriteInt32( 2 );            // the scale used when the Powerpoint document is embedded. the default is 1:2
    mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, mpStrm->Tell() );
    mpStrm->WriteUInt32( 0 )                        // Reference to NotesMaster ( 0 if none );
           .WriteUInt32( 0 )                        // Reference to HandoutMaster ( 0 if none );
           .WriteInt16( 1 )                         // Number of the first slide;
           .WriteUInt16( nSlideType )                           // Size of the document slides ( default: EPP_SLIDESIZETYPEONSCREEN )
           .WriteUChar( 0 )                         // bool1 indicates if document was saved with embedded true type fonts
           .WriteUChar( 0 )                         // bool1 indicates if the placeholders on the title slide are omitted
           .WriteUChar( 0 )                         // bool1 right to left ( flag for Bidi version )
           .WriteUChar( 1 );                            // bool1 visibility of comments shapes

    mpPptEscherEx->PtInsert( EPP_Persist_Document, mpStrm->Tell() );

    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 3 );  //Master footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    mpStrm->WriteUInt32( 0x25000d );
    if ( GetPageByIndex( 0, MASTER ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();
    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 4 );  //NotesMaster footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    mpStrm->WriteUInt32( 0x3d000d );
    if ( GetPageByIndex( 0, NOTICE ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText );      // animation information for the slides

    for ( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, mpStrm->Tell() );
        mpStrm->WriteUInt32( 0 )                                // psrReference - logical reference to the slide persist object ( EPP_MAINSLIDE_PERSIST_KEY )
               .WriteUInt32( 4 )                                // flags - only bit 3 used, if set then slide contains shapes other than placeholders
               .WriteInt32( 0 )                                     // numberTexts - number of placeholder texts stored with the persist object.  Allows to display outline view without loading the slide persist objects
               .WriteInt32( i + 0x100 )                             // slideId - Unique slide identifier, used for OLE link monikers for example
               .WriteUInt32( 0 );                               // reserved, usually 0

        if ( !GetPageByIndex( i, NORMAL ) )                     // very exciting: once again through all pages
            return false;
        SetCurrentStyleSheet( GetMasterIndex( NORMAL ) );

        css::uno::Reference< css::container::XNamed >
            aXName( mXDrawPage, css::uno::UNO_QUERY );

        if ( aXName.is() )
            maSlideNameList.push_back( aXName->getName() );
        else
            maSlideNameList.emplace_back( );
    }
    mpPptEscherEx->CloseContainer();    // EPP_SlideListWithText

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText, 2 );   // animation information for the notes
    for( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, mpStrm->Tell() );
        mpStrm->WriteUInt32( 0 )
               .WriteUInt32( 4 )
               .WriteInt32( 0 )
               .WriteInt32( i + 0x100 )
               .WriteUInt32( 0 );
    }
    mpPptEscherEx->CloseContainer();        // EPP_SlideListWithText

    css::uno::Reference< css::presentation::XPresentationSupplier >
        aXPresSupplier( mXModel, css::uno::UNO_QUERY );
    if ( aXPresSupplier.is() )
    {
        css::uno::Reference< css::presentation::XPresentation > aXPresentation( aXPresSupplier->getPresentation() );
        if ( aXPresentation.is() )
        {
            mXPropSet.set( aXPresentation, css::uno::UNO_QUERY );
            if ( mXPropSet.is() )
            {
                OUString aCustomShow;
                sal_uInt32 const nPenColor = 0x1000000;
                sal_Int32 const  nRestartTime = 0x7fffffff;
                sal_Int16   nStartSlide = 0;
                sal_Int16   nEndSlide = 0;
                sal_uInt32  nFlags = 0;             // Bit 0:   Auto advance
                                                    // Bit 1    Skip builds ( do not allow slide effects )
                                                    // Bit 2    Use slide range
                                                    // Bit 3    Use named show
                                                    // Bit 4    Browse mode on
                                                    // Bit 5    Kiosk mode on
                                                    // Bit 6    Skip narration
                                                    // Bit 7    loop continuously
                                                    // Bit 8    show scrollbar

                if ( ImplGetPropertyValue( "CustomShow" ) )
                {
                    aCustomShow = *o3tl::doAccess<OUString>(mAny);
                    if ( !aCustomShow.isEmpty() )
                    {
                        nFlags |= 8;
                    }
                }
                if ( ( nFlags & 8 ) == 0 )
                {
                    if ( ImplGetPropertyValue( "FirstPage" ) )
                    {
                        auto aSlideName = o3tl::doAccess<OUString>(mAny);

                        std::vector<OUString>::const_iterator pIter = std::find(
                                    maSlideNameList.begin(),maSlideNameList.end(), *aSlideName);

                        if (pIter != maSlideNameList.end())
                        {
                            nStartSlide = pIter - maSlideNameList.begin() + 1;
                            nFlags |= 4;
                            nEndSlide = static_cast<sal_uInt16>(mnPages);
                        }
                    }
                }

                if ( ImplGetPropertyValue( "IsAutomatic" ) )
                {
                    bool bBool = false;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 1;
                }

                if ( ImplGetPropertyValue( "IsEndless" ) )
                {
                    bool bBool = false;
                    mAny >>= bBool;
                    if ( bBool )
                        nFlags |= 0x80;
                }
                if ( ImplGetPropertyValue( "IsFullScreen" ) )
                {
                    bool bBool = false;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 0x11;
                }

                mpPptEscherEx->AddAtom( 80, EPP_SSDocInfoAtom, 1 );
                mpStrm->WriteUInt32( nPenColor ).WriteInt32( nRestartTime ).WriteInt16( nStartSlide ).WriteInt16( nEndSlide );

                sal_uInt32 nCustomShowNameLen = aCustomShow.getLength();
                if ( nCustomShowNameLen > 31 )
                    nCustomShowNameLen = 31;
                if ( nCustomShowNameLen )       // named show identifier
                {
                    const sal_Unicode* pCustomShow = aCustomShow.getStr();
                    for ( i = 0; i < nCustomShowNameLen; i++ )
                    {
                        mpStrm->WriteUInt16( pCustomShow[ i ] );
                    }
                }
                for ( i = nCustomShowNameLen; i < 32; i++, mpStrm->WriteUInt16( 0 ) ) ;

                mpStrm->WriteUInt32( nFlags );
                css::uno::Reference< css::presentation::XCustomPresentationSupplier > aXCPSup( mXModel, css::uno::UNO_QUERY );
                if ( aXCPSup.is() )
                {
                    css::uno::Reference< css::container::XNameContainer > aXCont( aXCPSup->getCustomPresentations() );
                    if ( aXCont.is() )
                    {
                        const css::uno::Sequence< OUString> aNameSeq( aXCont->getElementNames() );
                        if ( aNameSeq.hasElements() )
                        {
                            mpPptEscherEx->OpenContainer( EPP_NamedShows );
                            sal_uInt32 nCustomShowIndex = 0;
                            for( OUString const & customShowName : aNameSeq )
                            {
                                if ( !customShowName.isEmpty() )
                                {
                                    mpPptEscherEx->OpenContainer( EPP_NamedShow, nCustomShowIndex++ );

                                    sal_uInt32 nNamedShowLen = customShowName.getLength();
                                    if ( nNamedShowLen > 31 )
                                        nNamedShowLen = 31;
                                    mpPptEscherEx->AddAtom( nNamedShowLen << 1, EPP_CString );
                                    const sal_Unicode* pCustomShowName = customShowName.getStr();
                                    for ( sal_uInt32 k = 0; k < nNamedShowLen; ++k )
                                        mpStrm->WriteUInt16( pCustomShowName[ k ] );
                                    mAny = aXCont->getByName( customShowName );
                                    css::uno::Reference< css::container::XIndexContainer > aXIC;
                                    if ( mAny >>= aXIC )
                                    {
                                        mpPptEscherEx->BeginAtom();

                                        sal_Int32 nSlideCount = aXIC->getCount();
                                        for ( sal_Int32 j = 0; j < nSlideCount; j++ )   // number of slides
                                        {
                                            mAny = aXIC->getByIndex( j );
                                            css::uno::Reference< css::drawing::XDrawPage > aXDrawPage;
                                            if ( mAny >>= aXDrawPage )
                                            {
                                                css::uno::Reference< css::container::XNamed > aXName( aXDrawPage, css::uno::UNO_QUERY );
                                                if ( aXName.is() )
                                                {
                                                    OUString aSlideName( aXName->getName() );
                                                    std::vector<OUString>::const_iterator pIter = std::find(
                                                        maSlideNameList.begin(),maSlideNameList.end(),aSlideName);

                                                    if (pIter != maSlideNameList.end())
                                                    {
                                                        sal_uInt32 nPageNumber = pIter - maSlideNameList.begin();
                                                        mpStrm->WriteUInt32( nPageNumber + 0x100 ); // unique slide id
                                                    }
                                                }
                                            }
                                        }
                                        mpPptEscherEx->EndAtom( EPP_NamedShowSlides );
                                    }
                                    mpPptEscherEx->CloseContainer();            // EPP_NamedShow
                                }
                            }
                            mpPptEscherEx->CloseContainer();                // EPP_NamedShows
                        }
                    }
                }
            }
        }
    }
    mpPptEscherEx->AddAtom( 0, EPP_EndDocument );
    mpPptEscherEx->CloseContainer();    // EPP_Document
    return true;
};

void PPTWriter::ImplCreateHyperBlob( SvMemoryStream& rStrm )
{
    sal_uInt32 nCurrentOfs, nParaOfs, nParaCount = 0;

    nParaOfs = rStrm.Tell();
    rStrm.WriteUInt32( 0 );         // property size
    rStrm.WriteUInt32( 0 );         // property count

    for ( const auto& rHyperlink : maHyperlink )
    {
        nParaCount += 6;
        rStrm  .WriteUInt32( 3 )    // Type VT_I4
               .WriteUInt32( 7 )    // (VTI4 - Private1)
               .WriteUInt32( 3 )    // Type VT_I4
               .WriteUInt32( 6 )    // (VTI4 - Private2)
               .WriteUInt32( 3 )    // Type VT_I4
               .WriteUInt32( 0 );   // (VTI4 - Private3)

        // INFO
        // HIWORD:  = 0 : do not change anything
        //          = 1 : replace the hyperlink with the target and subaddress in the following two VTLPWSTR
        //          = 2 : delete the hyperlink
        // LOWORD:  = 0 : graphic shown as background (link)
        //          = 1 : graphic shown as shape (link)
        //          = 2 : graphic is used to fill a shape
        //          = 3 : graphic used to fill a shape outline (future use)
        //          = 4 : hyperlink attached to a shape
        //          = 5 :    "         "      " " (Word) field
        //          = 6 :    "         "      " " (Excel) range
        //          = 7 :    "         "      " " (PPT) text range
        //          = 8 :    "         "      " " (Project) task

        sal_Int32 nUrlLen = rHyperlink.aURL.getLength();
        const OUString& rUrl = rHyperlink.aURL;

        sal_uInt32 const nInfo = 7;

        rStrm  .WriteUInt32( 3 )    // Type VT_I4
               .WriteUInt32( nInfo );       // Info

        switch( rHyperlink.nType & 0xff )
        {
            case 1 :        // click action to slidenumber
            {
                rStrm.WriteUInt32( 0x1f ).WriteUInt32( 1 ).WriteUInt32( 0 );    // path
                rStrm.WriteUInt32( 0x1f ).WriteUInt32( nUrlLen + 1 );
                for ( sal_Int32 i = 0; i < nUrlLen; i++ )
                {
                    rStrm.WriteUInt16( rUrl[ i ] );
                }
                rStrm.WriteUInt16( 0 );
            }
            break;
            case 2 :
            {
                sal_Int32 i;

                rStrm  .WriteUInt32( 0x1f )
                       .WriteUInt32( nUrlLen + 1 );
                for ( i = 0; i < nUrlLen; i++ )
                {
                    rStrm.WriteUInt16( rUrl[ i ] );
                }
                if ( ! ( i & 1 ) )
                    rStrm.WriteUInt16( 0 );
                rStrm  .WriteUInt16( 0 )
                       .WriteUInt32( 0x1f )
                       .WriteUInt32( 1 )
                       .WriteUInt32( 0 );
            }
            break;
        }
    }
    nCurrentOfs = rStrm.Tell();
    rStrm.Seek( nParaOfs );
    rStrm.WriteUInt32( nCurrentOfs - ( nParaOfs + 4 ) );
    rStrm.WriteUInt32( nParaCount );
    rStrm.Seek( nCurrentOfs );
}

bool PPTWriter::ImplCreateMainNotes()
{
    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainNotes, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    mpStrm->WriteUInt32( 0x80000001 )                                               // Number that identifies this slide
           .WriteUInt32( 0 );                                                       // follow nothing
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(nullptr,nullptr);

    ImplWritePage( GetLayout( 20 ), aSolverContainer, NOTICE, true );

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, ShapeFlag::Background | ShapeFlag::HaveShapeProperty );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );                             // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                            // if true, this is the background shape
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x99cc00 ).WriteUInt32( 0xcc3333 ).WriteUInt32( 0xffcccc ).WriteUInt32( 0xb2b2b2 );
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return true;
}

static OUString getInitials( const OUString& rName )
{
    OUStringBuffer sInitials;

    const sal_Unicode * pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();

    while( nLength )
    {
        // skip whitespace
        while( nLength && (*pStr <= ' ') )
        {
            nLength--; pStr++;
        }

        // take letter
        if( nLength )
        {
            sInitials.append( *pStr );
            nLength--; pStr++;
        }

        // skip letters until whitespace
        while( nLength && (*pStr > ' ') )
        {
            nLength--; pStr++;
        }
    }

    return sInitials.makeStringAndClear();
}

void ImplExportComments( const uno::Reference< drawing::XDrawPage >& xPage, SvMemoryStream& rBinaryTagData10Atom )
{
    try
    {
        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( xPage, uno::UNO_QUERY_THROW );
        uno::Reference< office::XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );

        sal_Int32 nIndex = 1;

        while( xAnnotationEnumeration->hasMoreElements() )
        {
            EscherExContainer aComment10( rBinaryTagData10Atom, EPP_Comment10 );
            {
                uno::Reference< office::XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement() );

                geometry::RealPoint2D aRealPoint2D( xAnnotation->getPosition() );
                MapMode aMapDest( MapUnit::MapInch, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) );
                Point aPoint( OutputDevice::LogicToLogic( Point( static_cast< sal_Int32 >( aRealPoint2D.X * 100.0 ),
                    static_cast<sal_Int32>(aRealPoint2D.Y * 100.0)), MapMode(MapUnit::Map100thMM), aMapDest));

                OUString sAuthor( xAnnotation->getAuthor() );
                uno::Reference< text::XText > xText( xAnnotation->getTextRange() );
                OUString sText( xText->getString() );
                OUString sInitials( getInitials( sAuthor ) );
                util::DateTime aDateTime( xAnnotation->getDateTime() );
                if ( !sAuthor.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sAuthor );
                if ( !sText.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sText, 1 );
                if ( !sInitials.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sInitials, 2 );

                sal_Int16 nMilliSeconds = static_cast<sal_Int16>(::rtl::math::round(static_cast<double>(aDateTime.NanoSeconds) / 1000000000.0));
                EscherExAtom aCommentAtom10( rBinaryTagData10Atom, EPP_CommentAtom10 );
                rBinaryTagData10Atom.WriteInt32( nIndex++ )
                                    .WriteInt16( aDateTime.Year )
                                    .WriteUInt16( aDateTime.Month )
                                    .WriteUInt16( aDateTime.Day )   // todo: day of week
                                    .WriteUInt16( aDateTime.Day )
                                    .WriteUInt16( aDateTime.Hours )
                                    .WriteUInt16( aDateTime.Minutes )
                                    .WriteUInt16( aDateTime.Seconds )
                                    .WriteInt16( nMilliSeconds )
                                    .WriteInt32( aPoint.X() )
                                    .WriteInt32( aPoint.Y() );
            }
        }
    }
    catch ( uno::Exception& )
    {
    }
}

void PPTWriter::ImplWriteNotes( sal_uInt32 nPageNum )
{
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Notes | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    mpStrm->WriteUInt32( nPageNum + 0x100 )
           .WriteUInt16( 3 )                                        // follow master...
           .WriteUInt16( 0 );

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(nullptr,nullptr);

    ImplWritePage( GetLayout( 20 ), aSolverContainer, NOTICE, false );  // the shapes of the pages are created in the PPT document

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, ShapeFlag::Background | ShapeFlag::HaveShapeProperty );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );     // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    mpStrm->WriteUInt32( 0xffffff ).WriteUInt32( 0x000000 ).WriteUInt32( 0x808080 ).WriteUInt32( 0x000000 ).WriteUInt32( 0x99cc00 ).WriteUInt32( 0xcc3333 ).WriteUInt32( 0xffcccc ).WriteUInt32( 0xb2b2b2 );
    mpPptEscherEx->CloseContainer();    // EPP_Notes
};

void PPTWriter::ImplWriteBackground( css::uno::Reference< css::beans::XPropertySet > const & rXPropSet )
{
    //************************ ******
    //** DEFAULT BACKGROUND SHAPE **

    sal_uInt32 nFillColor = 0xffffff;
    sal_uInt32 nFillBackColor = 0;

    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, ShapeFlag::Background | ShapeFlag::HaveShapeProperty );

    // #i121183# Use real PageSize in 100th mm
    ::tools::Rectangle aRect(Point(0, 0), Size(maPageSize.Width, maPageSize.Height));

    EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm.get(), aRect );
    aPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );
    css::drawing::FillStyle aFS( css::drawing::FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, "FillStyle" ) )
        mAny >>= aFS;

    switch( aFS )
    {
        case css::drawing::FillStyle_GRADIENT :
        {
            aPropOpt.CreateGradientProperties( rXPropSet );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x1f001e );
            aPropOpt.GetOpt( ESCHER_Prop_fillColor, nFillColor );
            aPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
        }
        break;

        case css::drawing::FillStyle_BITMAP :
            aPropOpt.CreateGraphicProperties( rXPropSet, "FillBitmap", true );
        break;

        case css::drawing::FillStyle_HATCH :
            aPropOpt.CreateGraphicProperties( rXPropSet, "FillHatch", true );
        break;

        case css::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( rXPropSet, "FillColor" ) )
            {
                nFillColor = EscherEx::GetColor( *o3tl::doAccess<sal_uInt32>(mAny) );
                nFillBackColor = nFillColor ^ 0xffffff;
            }
            [[fallthrough]];
        }
        case css::drawing::FillStyle_NONE :
        default:
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        break;
    }
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, nFillColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Height ) );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_bwWhite );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
}

void PPTWriter::ImplWriteVBA()
{
    if ( mpVBA )
    {
        sal_uInt32 nLen = mpVBA->TellEnd();
        if ( nLen > 8 )
        {
            nLen -= 8;
            mnVBAOleOfs = mpStrm->Tell();
            mpPptEscherEx->BeginAtom();
            mpStrm->WriteBytes(static_cast<sal_Int8 const *>(mpVBA->GetData()) + 8, nLen);
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

void PPTWriter::ImplWriteOLE( )
{

    SvxMSExportOLEObjects aOleExport( mnCnvrtFlags );

    for ( const auto& rxExOleObjEntry : maExOleObj )
    {
        PPTExOleObjEntry* pPtr = rxExOleObjEntry.get();
        std::unique_ptr<SvMemoryStream> pStrm;
        pPtr->nOfsB = mpStrm->Tell();
        switch ( pPtr->eType )
        {
            case NORMAL_OLE_OBJECT :
            {
                SdrObject* pSdrObj = GetSdrObjectFromXShape( pPtr->xShape );
                if ( auto pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pSdrObj) )
                {
                    const ::uno::Reference < embed::XEmbeddedObject >& xObj( pSdrOle2Obj->GetObjRef() );
                    if( xObj.is() )
                    {
                        tools::SvRef<SotStorage> xTempStorage( new SotStorage( new SvMemoryStream(), true ) );
                        aOleExport.ExportOLEObject( xObj, *xTempStorage );

                        //TODO/MBA: testing
                        SvMemoryStream aStream;
                        tools::SvRef<SotStorage> xCleanStorage( new SotStorage( false, aStream ) );
                        xTempStorage->CopyTo( xCleanStorage.get() );
                        // create a dummy content stream, the dummy content is necessary for ppt, but not for
                        // doc files, so we can't share code.
                        tools::SvRef<SotStorageStream> xStm = xCleanStorage->OpenSotStream( SVEXT_PERSIST_STREAM );
                        xStm->WriteUInt32( 0 )        // no ClipboardId
                               .WriteUInt32( 4 )        // no target device
                               .WriteUInt32( 1 )        // aspect ratio
                               .WriteInt32( -1 )        // L-Index
                               .WriteUInt32( 0 )        // Advanced Flags
                               .WriteUInt32( 0 )        // compression
                               .WriteUInt32( 0 )        // Size
                               .WriteUInt32( 0 )        //  "
                               .WriteUInt32( 0 );
                        pStrm = xCleanStorage->CreateMemoryStream();
                    }
                }
            }
            break;

            case OCX_CONTROL :
            {
                if ( pPtr->xControlModel.is() )
                {
                    OUString aName;
                    //Initialize the graphic size which will be used on export
                    css::awt::Size  aSize( pPtr->xShape->getSize() );
                    tools::SvRef<SotStorage> xDest( new SotStorage( new SvMemoryStream(), true ) );
                    bool bOk = oox::ole::MSConvertOCXControls::WriteOCXStream( mXModel, xDest, pPtr->xControlModel, aSize, aName );
                    if ( bOk )
                        pStrm = xDest->CreateMemoryStream();
                }
            }
        }
        if ( pStrm )
        {
            mpPptEscherEx->BeginAtom();
            pStrm->Seek( STREAM_SEEK_TO_END );
            sal_uInt32 npStrmSize = pStrm->Tell();
            mpStrm->WriteUInt32( npStrmSize );                  // uncompressed size

            pStrm->Seek( 0 );
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Compress( *pStrm, *mpStrm );
            aZCodec.EndCompression();
            pStrm.reset();
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// write PersistantTable and UserEditAtom

void PPTWriter::ImplWriteAtomEnding()
{

#define EPP_LastViewTypeSlideView   1

    sal_uInt32  i, nPos, nOfs, nPersistOfs = mpStrm->Tell();
    sal_uInt32  nPersistEntrys = 0;
    mpStrm->WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 );         // skip record header and first entry

    // write document persist
    nPersistEntrys++;
    mpStrm->WriteUInt32( 0 );
    // write MasterPages persists
    for ( i = 0; i < mnMasterPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainMaster | i );
        if ( nOfs )
        {
            mpStrm->WriteUInt32( nOfs );
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // write MainNotesMaster persist
    nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainNotes );
    if ( nOfs )
    {
        mpStrm->WriteUInt32( nOfs );
        mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, ++nPersistEntrys );
    }
    // write slide persists -> we have to write a valid value into EPP_SlidePersistAtome too
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Slide | i );
        if ( nOfs )
        {
            mpStrm->WriteUInt32( nOfs );
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // write Notes persists
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Notes | i );
        if ( nOfs )
        {
            mpStrm->WriteUInt32( nOfs );
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Ole persists
    for ( const auto& rxExOleObjEntry : maExOleObj )
    {
        PPTExOleObjEntry* pPtr = rxExOleObjEntry.get();
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_ExObj );
        if ( nOfs )
        {
            nPersistEntrys++;
            mpStrm->WriteUInt32( pPtr->nOfsB );
            sal_uInt32 nOldPos, nPersOfs = nOfs + pPtr->nOfsA + 16 + 8;     // 8 bytes atom header, +16 to the persist entry
            nOldPos = mpStrm->Tell();
            mpStrm->Seek( nPersOfs );
            mpStrm->WriteUInt32( nPersistEntrys );
            mpStrm->Seek( nOldPos );
        }
    }
    // VB persist
    if ( mnVBAOleOfs && mpVBA )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_VBAInfoAtom );
        if ( nOfs )
        {
            nPersistEntrys++;
            sal_uInt32 n1, n2;

            mpVBA->Seek( 0 );
            mpVBA->ReadUInt32( n1 )
                  .ReadUInt32( n2 );

            mpStrm->WriteUInt32( mnVBAOleOfs );
            sal_uInt32 nOldPos = mpStrm->Tell();
            mpStrm->Seek( nOfs );               // Fill the VBAInfoAtom with the correct index to the persisttable
            mpStrm->WriteUInt32( nPersistEntrys )
                   .WriteUInt32( n1 )
                   .WriteInt32( 2 );
            mpStrm->Seek( nOldPos );

        }
    }
    nPos = mpStrm->Tell();
    mpStrm->Seek( nPersistOfs );
    mpPptEscherEx->AddAtom( ( nPersistEntrys + 1 ) << 2, EPP_PersistPtrIncrementalBlock );      // insert Record Header
    mpStrm->WriteUInt32( ( nPersistEntrys << 20 ) | 1 );
    mpStrm->Seek( nPos );

    mpCurUserStrm->WriteUInt32( nPos );             // set offset to current edit
    mpPptEscherEx->AddAtom( 28, EPP_UserEditAtom );
    mpStrm->WriteInt32( 0x100 )                     // last slide ID
           .WriteUInt32( 0x03000dbc )               // minor and major app version that did the save
           .WriteUInt32( 0 )                        // offset last save, 0 after a full save
           .WriteUInt32( nPersistOfs )                      // File offset to persist pointers for this save operation
           .WriteUInt32( 1 )                        // Persist reference to the document persist object
           .WriteUInt32( nPersistEntrys )           // max persists written, Seed value for persist object id management
           .WriteInt16( EPP_LastViewTypeSlideView ) // last view type
           .WriteInt16( 0x12 );                     // padword
}

// - exported function -

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool ExportPPT( const std::vector< css::beans::PropertyValue >& rMediaData,
                    tools::SvRef<SotStorage> const & rSvStorage,
                    css::uno::Reference< css::frame::XModel > const & rXModel,
                    css::uno::Reference< css::task::XStatusIndicator > const & rXStatInd,
                    SvMemoryStream* pVBA,
                    sal_uInt32 nCnvrtFlags )
{
    std::unique_ptr<PPTWriter> pPPTWriter(new PPTWriter( rSvStorage, rXModel, rXStatInd, pVBA, nCnvrtFlags ));
    pPPTWriter->exportPPT(rMediaData);
    bool bStatus = pPPTWriter->IsValid();
    return bStatus;
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SaveVBA( SfxObjectShell& rDocShell, SvMemoryStream*& pBas )
{
    tools::SvRef<SotStorage> xDest( new SotStorage( new SvMemoryStream(), true ) );
    SvxImportMSVBasic aMSVBas( rDocShell, *xDest );
    aMSVBas.SaveOrDelMSVBAStorage( true, "_MS_VBA_Overhead" );

    tools::SvRef<SotStorage> xOverhead = xDest->OpenSotStorage( "_MS_VBA_Overhead" );
    if ( xOverhead.is() && ( xOverhead->GetError() == ERRCODE_NONE ) )
    {
        tools::SvRef<SotStorage> xOverhead2 = xOverhead->OpenSotStorage( "_MS_VBA_Overhead" );
        if ( xOverhead2.is() && ( xOverhead2->GetError() == ERRCODE_NONE ) )
        {
            tools::SvRef<SotStorageStream> xTemp = xOverhead2->OpenSotStream( "_MS_VBA_Overhead2" );
            if ( xTemp.is() && ( xTemp->GetError() == ERRCODE_NONE ) )
            {
                sal_uInt32 nLen = xTemp->GetSize();
                if ( nLen )
                {
                    char* pTemp = new char[ nLen ];
                    xTemp->Seek( STREAM_SEEK_TO_BEGIN );
                    xTemp->ReadBytes(pTemp, nLen);
                    pBas = new SvMemoryStream( pTemp, nLen, StreamMode::READ );
                    pBas->ObjectOwnsMemory( true );
                    return true;
                }
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
