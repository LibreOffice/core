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

#include <eppt.hxx>
#include "epptdef.hxx"
#include <tools/globname.hxx>
#include <tools/poly.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/gradient.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <vcl/fltcall.hxx>
#include <sfx2/docfile.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <tools/zcodec.hxx>
#include <editeng/svxenum.hxx>
#include <sot/storinfo.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wmf.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/docinf.hxx>
#include <oox/export/utils.hxx>
#include <oox/ole/olehelper.hxx>
#include <rtl/math.hxx>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::beans::XPropertySet;

//============================ PPTWriter ==================================

PPTWriter::PPTWriter( SvStorageRef& rSvStorage,
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
            ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags ) :
    PPTWriterBase           ( rXModel, rXStatInd ),
    mnCnvrtFlags            ( nCnvrtFlags ),
    mbStatus                ( sal_False ),
    mbUseNewAnimations      ( sal_True ),
    mnLatestStatValue       ( 0 ),
    mrStg                   ( rSvStorage ),
    mpCurUserStrm           ( NULL ),
    mpStrm                  ( NULL ),
    mpPicStrm               ( NULL ),
    mpPptEscherEx           ( NULL ),
    mnVBAOleOfs             ( 0 ),
    mpVBA                   ( pVBA ),
    mnExEmbed               ( 0 ),
    mpExEmbed               ( new SvMemoryStream ),
    mnPagesWritten          ( 0 ),
    mnTxId                  ( 0x7a2f64 )
{
}

void PPTWriter::exportPPTPre( const std::vector< com::sun::star::beans::PropertyValue >& rMediaData )
{
    if ( !mrStg.Is() )
        return;

    // master pages + slides and notes + notes master page
    mnDrawings = mnMasterPages + ( mnPages << 1 ) + 1;

    if ( mXStatusIndicator.is() )
    {
        mbStatusIndicator = sal_True;
        mnStatMaxValue = ( mnPages + mnMasterPages ) * 5;
        mXStatusIndicator->start( OUString( "PowerPoint Export" ), mnStatMaxValue + ( mnStatMaxValue >> 3 ) );
    }

    SvGlobalName aGName( 0x64818d10L, 0x4f9b, 0x11cf, 0x86, 0xea, 0x00, 0xaa, 0x00, 0xb9, 0x29, 0xe8 );
    mrStg->SetClass( aGName, 0,  OUString("MS PowerPoint 97") );

    if ( !ImplCreateCurrentUserStream() )
        return;

    mpStrm = mrStg->OpenSotStream( OUString( "PowerPoint Document" ) );
    if ( !mpStrm )
        return;

    if ( !mpPicStrm )
        mpPicStrm = mrStg->OpenSotStream( OUString( "Pictures" ) );

    for (std::vector< com::sun::star::beans::PropertyValue >::const_iterator aIter( rMediaData.begin() ), aEnd( rMediaData.end() );
        aIter != aEnd ; ++aIter)
    {
        if ( (*aIter).Name == "BaseURI" )
        {
            (*aIter).Value >>= maBaseURI;
            break;
        }
    }
    mpPptEscherEx = new PptEscherEx( *mpStrm, maBaseURI );
}

void PPTWriter::exportPPTPost( )
{
    if ( !ImplCloseDocument() )
        return;

    if ( mbStatusIndicator )
    {
        mXStatusIndicator->setText( OUString( "PowerPoint Export" ) );
        sal_uInt32 nValue = mnStatMaxValue + ( mnStatMaxValue >> 3 );
        if ( nValue > mnLatestStatValue )
        {
            mXStatusIndicator->setValue( nValue );
            mnLatestStatValue = nValue;
        }
    }

    ImplWriteOLE();

    ImplWriteVBA();

    if ( !ImplWriteAtomEnding() )
        return;

    if ( !ImplCreateDocumentSummaryInformation() )
        return;

    mbStatus = sal_True;
};

// ---------------------------------------------------------------------------------------------

void ImplExportComments( uno::Reference< drawing::XDrawPage > xPage, SvMemoryStream& rBinaryTagData10Atom );

void PPTWriter::ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                sal_Bool bHasBackground, Reference< XPropertySet > aXBackgroundPropSet )
{
    Any aAny;

    const PHLayout& rLayout = GetLayout( mXPagePropSet );
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Slide | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Slide );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << rLayout.nLayout;
    mpStrm->Write( rLayout.nPlaceHolder, 8 );       // placeholderIDs (8 parts)
    *mpStrm << (sal_uInt32)(nMasterNum | 0x80000000)// master ID (equals 0x80000000 on a master page)
            << (sal_uInt32)nPageNum + 0x100         // notes ID (equals null if no notes are present)
            << nMode
            << (sal_uInt16)0;                       // padword

    mnDiaMode = 0;
    sal_Bool bVisible = sal_True;
    ::com::sun::star::presentation::FadeEffect eFe = ::com::sun::star::presentation::FadeEffect_NONE;

    if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Visible" ) ) )
        aAny >>= bVisible;
    if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Change" ) ) )
    {
        switch ( *(sal_Int32*)aAny.getValue() )
        {
            case 1 :        // automatic
                mnDiaMode++;
            case 2 :        // semi-automatic
                mnDiaMode++;
            default :
            case 0 :        // manual
            break;
        }
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Effect" ) ) )
        aAny >>= eFe;

    sal_uInt32  nSoundRef = 0;
    sal_Bool    bIsSound = sal_False;
    sal_Bool    bStopSound = sal_False;
    sal_Bool    bLoopSound = sal_False;

    if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Sound" ) ) )
    {
        OUString aSoundURL;
        if ( aAny >>= aSoundURL )
        {
            nSoundRef = maSoundCollection.GetId( aSoundURL );
            bIsSound = sal_True;
        }
        else
            aAny >>= bStopSound;
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "LoopSound" ) ) )
        aAny >>= bLoopSound;

    sal_Bool bNeedsSSSlideInfoAtom = ( bVisible == sal_False )
                                    || ( mnDiaMode == 2 )
                                    || ( bIsSound )
                                    || ( bStopSound )
                                    || ( eFe != ::com::sun::star::presentation::FadeEffect_NONE );
    if ( bNeedsSSSlideInfoAtom )
    {
        sal_uInt8   nDirection = 0;
        sal_uInt8   nTransitionType = 0;
        sal_uInt16  nBuildFlags = 1;        // advange by mouseclick
        sal_Int32       nSlideTime = 0;         // still has to !!!
        sal_uInt8   nSpeed = 1;

        if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Speed" ) ) )
        {
            ::com::sun::star::presentation::AnimationSpeed aAs;
            aAny >>= aAs;
            nSpeed = (sal_uInt8)aAs;
        }
        sal_Int16 nTT = 0;
        if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "TransitionType" ) )
            && ( aAny >>= nTT ) )
        {
            sal_Int16 nTST = 0;
            if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "TransitionSubtype" ) )
                && ( aAny >>= nTST ) )
                nTransitionType = GetTransition( nTT, nTST, eFe, nDirection );

        }
        if ( !nTransitionType )
            nTransitionType = GetTransition( eFe, nDirection );
        if ( mnDiaMode == 2 )                                   // automatic ?
            nBuildFlags |= 0x400;
        if ( bVisible == sal_False )
            nBuildFlags |= 4;
        if ( bIsSound )
            nBuildFlags |= 16;
        if ( bLoopSound )
            nBuildFlags |= 64;
        if ( bStopSound )
            nBuildFlags |= 256;

        if ( GetPropertyValue( aAny, mXPagePropSet, OUString( "Duration" ) ) )// duration of this slide
            nSlideTime = *(sal_Int32*)aAny.getValue() << 10;        // in ticks

        mpPptEscherEx->AddAtom( 16, EPP_SSSlideInfoAtom );
        *mpStrm << nSlideTime       // standtime in ticks
                << nSoundRef
                << nDirection
                << nTransitionType
                << nBuildFlags
                << nSpeed
                << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0;
    }

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);
    ImplWritePage( rLayout, aSolverContainer, NORMAL, sal_False, nPageNum );    // the shapes of the pages are created in the PPT document
    mpPptEscherEx->LeaveGroup();

    if ( bHasBackground )
        ImplWriteBackground( aXBackgroundPropSet );
    else
    {
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );             // Flags: Connector | Background | HasSpt
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
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;

    SvMemoryStream aBinaryTagData10Atom;
    ImplExportComments( mXDrawPage, aBinaryTagData10Atom );
    if ( mbUseNewAnimations )
    {
        SvMemoryStream amsofbtAnimGroup;
        ppt::AnimationExporter aExporter( aSolverContainer, maSoundCollection );
        aExporter.doexport( mXDrawPage, amsofbtAnimGroup );
        sal_uInt32 nmsofbtAnimGroupSize = amsofbtAnimGroup.Tell();
        if ( nmsofbtAnimGroupSize )
        {
            {
                EscherExAtom aMagic2( aBinaryTagData10Atom, 0x2eeb );
                aBinaryTagData10Atom << (sal_uInt32)0x01c45df9
                                     << (sal_uInt32)0xe1471b30;
            }
            {
                EscherExAtom aMagic( aBinaryTagData10Atom, 0x2b00 );
                aBinaryTagData10Atom << (sal_uInt32)0;
            }
            aBinaryTagData10Atom.Write( amsofbtAnimGroup.GetData(), amsofbtAnimGroup.Tell() );
            {
                EscherExContainer aMagic2( aBinaryTagData10Atom, 0x2b02 );
            }
        }
    }
    if ( aBinaryTagData10Atom.Tell() )
    {
        EscherExContainer aProgTags     ( *mpStrm, EPP_ProgTags );
        EscherExContainer aProgBinaryTag( *mpStrm, EPP_ProgBinaryTag );
        {
            EscherExAtom aCString( *mpStrm, EPP_CString );
            *mpStrm << (sal_uInt32)0x5f005f
                    << (sal_uInt32)0x50005f
                    << (sal_uInt32)0x540050
                    << (sal_uInt16)0x31
                    << (sal_uInt16)0x30;
        }
        {
            EscherExAtom aBinaryTagData( *mpStrm, EPP_BinaryTagData );
            mpStrm->Write( aBinaryTagData10Atom.GetData(), aBinaryTagData10Atom.Tell() );
        }
    }
    mpPptEscherEx->CloseContainer();    // EPP_Slide
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteSlideMaster( sal_uInt32 nPageNum, Reference< XPropertySet > aXBackgroundPropSet )
{
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainMaster | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_MainMaster );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << (sal_Int32)EPP_LAYOUT_TITLEANDBODYSLIDE  // slide layout -> title and body slide
            << (sal_uInt8)1 << (sal_uInt8)2 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0     // placeholderID
            << (sal_uInt32)0        // master ID (equals null at a master page)
            << (sal_uInt32)0        // notes ID (equals null if no notes are present)
            << (sal_uInt16)0        // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
            << (sal_uInt16)0;       // padword

    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xff0000 << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x00ffff << (sal_uInt32)0x0099ff << (sal_uInt32)0xffff00 << (sal_uInt32)0x0000ff << (sal_uInt32)0x969696;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xccffff << (sal_uInt32)0x000000 << (sal_uInt32)0x336666 << (sal_uInt32)0x008080 << (sal_uInt32)0x339933 << (sal_uInt32)0x000080 << (sal_uInt32)0xcc3300 << (sal_uInt32)0x66ccff;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x333333 << (sal_uInt32)0x000000 << (sal_uInt32)0xdddddd << (sal_uInt32)0x808080 << (sal_uInt32)0x4d4d4d << (sal_uInt32)0xeaeaea;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x66ccff << (sal_uInt32)0xff0000 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xc0c0c0;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xc0c0c0 << (sal_uInt32)0xff6600 << (sal_uInt32)0x0000ff << (sal_uInt32)0x009900;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xff9933 << (sal_uInt32)0xccff99 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xb2b2b2;

    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        // the auto color is dependent to the page background,so we have to set a page that is in the right context
        if ( nInstance == EPP_TEXTTYPE_Notes )
            GetPageByIndex( 0, NOTICE );
        else
            GetPageByIndex( 0, MASTER );

        mpPptEscherEx->BeginAtom();

        sal_Bool bFirst = sal_True;
        sal_Bool bSimpleText = sal_False;

        *mpStrm << (sal_uInt16)5;                           // paragraph count

        for ( sal_uInt16 nLev = 0; nLev < 5; nLev++ )
        {
            if ( nInstance >= EPP_TEXTTYPE_CenterBody )
            {
                bFirst = sal_False;
                bSimpleText = sal_True;
                *mpStrm << nLev;
            }
            mpStyleSheet->mpParaSheet[ nInstance ]->Write( *mpStrm, mpPptEscherEx, nLev, bFirst, bSimpleText, mXPagePropSet );
            mpStyleSheet->mpCharSheet[ nInstance ]->Write( *mpStrm, mpPptEscherEx, nLev, bFirst, bSimpleText, mXPagePropSet );
            bFirst = sal_False;
        }
        mpPptEscherEx->EndAtom( EPP_TxMasterStyleAtom, 0, nInstance );
    }
    GetPageByIndex( nPageNum, MASTER );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );

    mpPptEscherEx->EnterGroup(0,0);
    ImplWritePage( GetLayout( 0 ), aSolverContainer, MASTER, sal_True );    // the shapes of the pages are created in the PPT document
    mpPptEscherEx->LeaveGroup();

    ImplWriteBackground( aXBackgroundPropSet );

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;

    if ( aBuExMasterStream.Tell() )
    {
        ImplProgTagContainer( mpStrm, &aBuExMasterStream );
    }
    mpPptEscherEx->CloseContainer();    // EPP_MainMaster
};

// ---------------------------------------------------------------------------------------------

PPTWriter::~PPTWriter()
{
    delete mpExEmbed;
    delete mpPptEscherEx;
    delete mpCurUserStrm;
    delete mpPicStrm;
    delete mpStrm;

    std::vector< PPTExStyleSheet* >::iterator aStyleSheetIter( maStyleSheetList.begin() );
    while( aStyleSheetIter < maStyleSheetList.end() )
        delete *aStyleSheetIter++;

    for ( std::vector<PPTExOleObjEntry*>::const_iterator it = maExOleObj.begin(); it != maExOleObj.end(); ++it )
        delete *it;

    if ( mbStatusIndicator )
        mXStatusIndicator->end();
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateCurrentUserStream()
{
    mpCurUserStrm = mrStg->OpenSotStream( OUString( "Current User" ) );
    if ( !mpCurUserStrm )
        return sal_False;
    char pUserName[] = "Current User";
    sal_uInt32 nLenOfUserName = strlen( pUserName );
    sal_uInt32 nSizeOfRecord = 0x14 + ( ( nLenOfUserName + 4 ) & ~ 3 );

    *mpCurUserStrm << (sal_uInt16)0 << (sal_uInt16)EPP_CurrentUserAtom << nSizeOfRecord;
    *mpCurUserStrm << (sal_uInt32)0x14                  // Len
                   << (sal_uInt32)0xe391c05f;           // Magic

    sal_uInt32 nEditPos = mpCurUserStrm->Tell();
    *mpCurUserStrm << (sal_uInt32)0x0                   // OffsetToCurrentEdit;
                   << (sal_uInt16)nLenOfUserName        //
                   << (sal_uInt16)0x3f4                 // DocFileVersion
                   << (sal_uInt8)3                      // MajorVersion
                   << (sal_uInt8)0                      // MinorVersion
                   << (sal_uInt16)0;                    // Pad Word
    pUserName[ nLenOfUserName ] = 8;
    mpCurUserStrm->Write( pUserName, nLenOfUserName + 1 );
    for ( sal_uInt32 i = 0x15 + nLenOfUserName; i < nSizeOfRecord; i++ )
    {
        *mpCurUserStrm << (sal_uInt8)0;                 // pad bytes
    };
    mpCurUserStrm->Seek( nEditPos );
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocumentSummaryInformation()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mXModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());

    if (xDocProps.is()) {

        // no idea what this is...
        static const sal_uInt8 aGuid[ 0x52 ] =
        {
            0x4e, 0x00, 0x00, 0x00,
            '{',0,'D',0,'B',0,'1',0,'A',0,'C',0,'9',0,'6',0,'4',0,'-',0,
            'E',0,'3',0,'9',0,'C',0,'-',0,'1',0,'1',0,'D',0,'2',0,'-',0,
            'A',0,'1',0,'E',0,'F',0,'-',0,'0',0,'0',0,'6',0,'0',0,'9',0,
            '7',0,'D',0,'A',0,'5',0,'6',0,'8',0,'9',0,'}',0
        };
        uno::Sequence<sal_uInt8> aGuidSeq(aGuid, 0x52);

        SvMemoryStream  aHyperBlob;
        ImplCreateHyperBlob( aHyperBlob );

        uno::Sequence<sal_uInt8> aHyperSeq(aHyperBlob.Tell());
        const sal_uInt8* pBlob(
            static_cast<const sal_uInt8*>(aHyperBlob.GetData()));
        for (sal_Int32 j = 0; j < aHyperSeq.getLength(); ++j) {
            aHyperSeq[j] = pBlob[j];
        }

        if ( mnCnvrtFlags & 0x8000 )
        {
            uno::Sequence<sal_uInt8> aThumbSeq;
            if ( GetPageByIndex( 0, NORMAL ) && ImplGetPropertyValue( mXPagePropSet, OUString( "PreviewBitmap" ) ) )
            {
                aThumbSeq =
                    *static_cast<const uno::Sequence<sal_uInt8>*>(mAny.getValue());
            }
            sfx2::SaveOlePropertySet( xDocProps, mrStg,
                    &aThumbSeq, &aGuidSeq, &aHyperSeq);
        }
        else
        {
            sfx2::SaveOlePropertySet( xDocProps, mrStg,
                    NULL, &aGuidSeq, &aHyperSeq );
        }
    }

    return sal_True;
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId )
{
    if ( rSt.Tell() )
    {
        aBuExOutlineStream << (sal_uInt32)( ( EPP_PST_ExtendedParagraphHeaderAtom << 16 )
                                | ( nRef << 4 ) )
                            << (sal_uInt32)8
                            << (sal_uInt32)nSlideId
                            << (sal_uInt32)nInstance;
        aBuExOutlineStream.Write( rSt.GetData(), rSt.Tell() );
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplCreateHeaderFooterStrings( SvStream& rStrm, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet )
{
    if ( rXPagePropSet.is() )
    {
        OUString aString;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "HeaderText" ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 1 );
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "FooterText" ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 2 );
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "DateTimeText" ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 0 );
        }
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplCreateHeaderFooters( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet )
{
    if ( rXPagePropSet.is() )
    {
        sal_Bool bVal = sal_False;
        sal_uInt32 nVal = 0;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "IsHeaderVisible" ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x100000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "IsFooterVisible" ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x200000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "IsDateTimeVisible" ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x010000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "IsPageNumberVisible" ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x080000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "IsDateTimeFixed" ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && !bVal )
                nVal |= 0x20000;
            else
                nVal |= 0x40000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, OUString( "DateTimeFormat" ), sal_True ) )
        {
            sal_Int32 nFormat = *(sal_Int32*)aAny.getValue();
            SvxDateFormat eDateFormat = (SvxDateFormat)( nFormat & 0xf );
            SvxTimeFormat eTimeFormat = (SvxTimeFormat)( ( nFormat >> 4 ) & 0xf );
            switch( eDateFormat )
            {
                case SVXDATEFORMAT_F :
                    nFormat = 1;
                break;
                case SVXDATEFORMAT_D :
                    nFormat = 2;
                break;
                case SVXDATEFORMAT_C :
                    nFormat = 4;
                break;
                default:
                case SVXDATEFORMAT_A :
                    nFormat = 0;
            }
            switch( eTimeFormat )
            {
                case SVXTIMEFORMAT_24_HM :
                    nFormat = 9;
                break;
                case SVXTIMEFORMAT_12_HM :
                    nFormat = 11;
                break;
                case SVXTIMEFORMAT_24_HMS :
                    nFormat = 10;
                break;
                case SVXTIMEFORMAT_12_HMS :
                    nFormat = 12;
                break;
                default:
                    break;
            }
            nVal |= nFormat;
        }

        mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 0 );
        mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
        *mpStrm << nVal;
        ImplCreateHeaderFooterStrings( *mpStrm, rXPagePropSet );
        mpPptEscherEx->CloseContainer();
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocument()
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
    *mpStrm << nWidth                           // Slide Size in Master coordinates X
            << nHeight                          //   "     "   "    "        "      Y
            << (sal_Int32)maNotesPageSize.Width     // Notes Page Size                  X
            << (sal_Int32)maNotesPageSize.Height    //   "     "   "                    Y
            << (sal_Int32)1 << (sal_Int32)2;            // the scale used when the Powerpoint document is embedded. the default is 1:2
    mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, mpStrm->Tell() );
    *mpStrm << (sal_uInt32)0                        // Reference to NotesMaster ( 0 if none );
            << (sal_uInt32)0                        // Reference to HandoutMaster ( 0 if none );
            << (sal_Int16)1                         // Number of the first slide;
            << nSlideType                           // Size of the document slides ( default: EPP_SLIDESIZETYPEONSCREEN )
            << (sal_uInt8)0                         // bool1 indicates if document was saved with embedded true type fonts
            << (sal_uInt8)0                         // bool1 indicates if the placeholders on the title slide are omitted
            << (sal_uInt8)0                         // bool1 right to left ( flag for Bidi version )
            << (sal_uInt8)1;                            // bool1 visibility of comments shapes

    mpPptEscherEx->PtInsert( EPP_Persist_Document, mpStrm->Tell() );

    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 3 );  //Master footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    *mpStrm << (sal_uInt32)0x25000d;
    if ( GetPageByIndex( 0, MASTER ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();
    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 4 );  //NotesMaster footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    *mpStrm << (sal_uInt32)0x3d000d;
    if ( GetPageByIndex( 0, NOTICE ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText );      // animation information for the slides

    for ( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0                                // psrReference - logical reference to the slide persist object ( EPP_MAINSLIDE_PERSIST_KEY )
                << (sal_uInt32)4                                // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                << (sal_Int32)0                                     // numberTexts - number of placeholder texts stored with the persist object.  Allows to display outline view without loading the slide persist objects
                << (sal_Int32)i + 0x100                             // slideId - Unique slide identifier, used for OLE link monikers for example
                << (sal_uInt32)0;                               // reserved, usualy 0

        if ( !GetPageByIndex( i, NORMAL ) )                     // very exciting: once again through all pages
            return sal_False;
        SetCurrentStyleSheet( GetMasterIndex( NORMAL ) );

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
            aXName( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

        if ( aXName.is() )
            maSlideNameList.push_back( aXName->getName() );
        else
            maSlideNameList.push_back( OUString() );
    }
    mpPptEscherEx->CloseContainer();    // EPP_SlideListWithText

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText, 2 );   // animation information for the notes
    for( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0
                << (sal_uInt32)4
                << (sal_Int32)0
                << (sal_Int32)i + 0x100
                << (sal_uInt32)0;
    }
    mpPptEscherEx->CloseContainer();        // EPP_SlideListWithText

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationSupplier >
        aXPresSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );            ;
    if ( aXPresSupplier.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation >
            aXPresentation( aXPresSupplier->getPresentation() );
        if ( aXPresentation.is() )
        {
            mXPropSet = ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >
                    ( aXPresentation, ::com::sun::star::uno::UNO_QUERY );
            if ( mXPropSet.is() )
            {
                OUString aCustomShow;
                sal_uInt32  nPenColor = 0x1000000;
                sal_Int32   nRestartTime = 0x7fffffff;
                sal_Int16   nStartSlide = 0;
                sal_Int16   nEndSlide = 0;
                sal_uInt32  nFlags = 0;             // Bit 0:   Auto advance
                                                    // Bit 1    Skip builds ( do not allow slide effects )
                                                    // Bit 2    Use slide range
                                                    // Bit 3    Use named show
                                                    // Bit 4    Browse mode on
                                                    // Bit 5    Kiosk mode on
                                                    // Bit 7    loop continously
                                                    // Bit ?    show scrollbar

                if ( ImplGetPropertyValue( OUString( "CustomShow" ) ) )
                {
                    aCustomShow = ( *(OUString*)mAny.getValue() );
                    if ( !aCustomShow.isEmpty() )
                    {
                        nFlags |= 8;
                    }
                }
                if ( ( nFlags & 8 ) == 0 )
                {
                    if ( ImplGetPropertyValue( OUString( "FirstPage" ) ) )
                    {
                        OUString aSlideName( *(OUString*)mAny.getValue() );

                        std::vector<OUString>::const_iterator pIter = std::find(
                                    maSlideNameList.begin(),maSlideNameList.end(),aSlideName);

                        if (pIter != maSlideNameList.end())
                        {
                            nStartSlide = pIter - maSlideNameList.begin() + 1;
                            nFlags |= 4;
                            nEndSlide = (sal_uInt16)mnPages;
                        }
                    }
                }

                if ( ImplGetPropertyValue( OUString("IsAutomatic" ) ) )
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 1;
                }

                if ( ImplGetPropertyValue( OUString( "IsEndless" ) ) ) // the correct name would be IsNotEndless: WTF?
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( bBool )
                        nFlags |= 0x80;
                }
                if ( ImplGetPropertyValue( OUString( "IsFullScreen" ) ) )
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 0x11;
                }

                mpPptEscherEx->AddAtom( 80, EPP_SSDocInfoAtom, 1 );
                *mpStrm << nPenColor << nRestartTime << nStartSlide << nEndSlide;

                sal_uInt32 nCustomShowNameLen = aCustomShow.getLength();
                if ( nCustomShowNameLen > 31 )
                    nCustomShowNameLen = 31;
                if ( nCustomShowNameLen )       // named show identifier
                {
                    const sal_Unicode* pCustomShow = aCustomShow.getStr();
                    for ( i = 0; i < nCustomShowNameLen; i++ )
                    {
                        *mpStrm << (sal_uInt16)( pCustomShow[ i ] );
                    }
                }
                for ( i = nCustomShowNameLen; i < 32; i++, *mpStrm << (sal_uInt16)0 ) ;

                *mpStrm << nFlags;
                ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XCustomPresentationSupplier >
                    aXCPSup( mXModel, ::com::sun::star::uno::UNO_QUERY );
                if ( aXCPSup.is() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        aXCont( aXCPSup->getCustomPresentations() );
                    if ( aXCont.is() )
                    {
                        ::com::sun::star::uno::Sequence< OUString> aNameSeq( aXCont->getElementNames() );
                        const OUString* pUString = aNameSeq.getArray();
                        sal_uInt32 nCount = aNameSeq.getLength();
                        if ( nCount )
                        {
                            mpPptEscherEx->OpenContainer( EPP_NamedShows );
                            sal_uInt32 nCustomShowIndex = 0;
                            for( i = 0; i < nCount; i++ )        // number of custom shows
                            {
                                if ( !pUString[ i ].isEmpty() )
                                {
                                    mpPptEscherEx->OpenContainer( EPP_NamedShow, nCustomShowIndex++ );

                                    sal_uInt32 nNamedShowLen = pUString[ i ].getLength();
                                    if ( nNamedShowLen > 31 )
                                        nNamedShowLen = 31;
                                    mpPptEscherEx->AddAtom( nNamedShowLen << 1, EPP_CString );
                                    const sal_Unicode* pCustomShowName = pUString[ i ].getStr();
                                    for ( sal_uInt32 k = 0; k < nNamedShowLen; *mpStrm << (sal_uInt16)( pCustomShowName[ k++ ] ) ) ;
                                    mAny = aXCont->getByName( pUString[ i ] );
                                    if ( mAny.getValue() )
                                    {

                                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > aXIC;
                                        if ( mAny >>= aXIC )
                                        {
                                            mpPptEscherEx->BeginAtom();

                                            sal_Int32 nSlideCount = aXIC->getCount();
                                            for ( sal_Int32 j = 0; j < nSlideCount; j++ )   // number of slides
                                            {
                                                mAny = aXIC->getByIndex( j );
                                                if ( mAny.getValue() )
                                                {
                                                    ::com::sun::star::uno::Reference<
                                                        ::com::sun::star::drawing::XDrawPage > aXDrawPage;
                                                    if ( mAny >>= aXDrawPage )
                                                    {
                                                        ::com::sun::star::uno::Reference<
                                                            ::com::sun::star::container::XNamed >
                                                            aXName( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );
                                                        if ( aXName.is() )
                                                        {
                                                            OUString aSlideName( aXName->getName() );
                                                            std::vector<OUString>::const_iterator pIter = std::find(
                                                                        maSlideNameList.begin(),maSlideNameList.end(),aSlideName);

                                                            if (pIter != maSlideNameList.end())
                                                            {
                                                                sal_uInt32 nPageNumber = pIter - maSlideNameList.begin();
                                                                *mpStrm << (sal_uInt32)( nPageNumber + 0x100 ); // unique slide id
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            mpPptEscherEx->EndAtom( EPP_NamedShowSlides );
                                        }
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
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateHyperBlob( SvMemoryStream& rStrm )
{
    sal_uInt32 nCurrentOfs, nParaOfs, nParaCount = 0;

    nParaOfs = rStrm.Tell();
    rStrm << (sal_uInt32)0;         // property size
    rStrm << (sal_uInt32)0;         // property count

    for ( std::vector<EPPTHyperlink>::const_iterator pIter = maHyperlink.begin(); pIter != maHyperlink.end(); ++pIter )
    {
        nParaCount += 6;
        rStrm   << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)7    // (VTI4 - Private1)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)6    // (VTI4 - Private2)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)0;   // (VTI4 - Private3)

        // INFO
        // HIWORD:  = 0 : do not change anything
        //          = 1 : replace the hyperlink with the target and subadress in the following two VTLPWSTR
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

        sal_Int32 nUrlLen = pIter->aURL.getLength();
        const OUString& rUrl = pIter->aURL;

        sal_uInt32 nInfo = 7;

        rStrm   << (sal_uInt32)3    // Type VT_I4
                << nInfo;       // Info

        switch( pIter->nType & 0xff )
        {
            case 1 :        // click action to slidenumber
            {
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)1 << (sal_uInt32)0;    // path
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)( nUrlLen + 1 );
                for ( sal_Int32 i = 0; i < nUrlLen; i++ )
                {
                    rStrm << rUrl[ i ];
                }
                rStrm << (sal_uInt16)0;
            }
            break;
            case 2 :
            {
                sal_Int32 i;

                rStrm   << (sal_uInt32)0x1f
                        << (sal_uInt32)( nUrlLen + 1 );
                for ( i = 0; i < nUrlLen; i++ )
                {
                    rStrm << rUrl[ i ];
                }
                if ( ! ( i & 1 ) )
                    rStrm << (sal_uInt16)0;
                rStrm   << (sal_uInt16)0
                        << (sal_uInt32)0x1f
                        << (sal_uInt32)1
                        << (sal_uInt32)0;
            }
            break;
        }
    }
    nCurrentOfs = rStrm.Tell();
    rStrm.Seek( nParaOfs );
    rStrm << (sal_uInt32)( nCurrentOfs - ( nParaOfs + 4 ) );
    rStrm << nParaCount;
    rStrm.Seek( nCurrentOfs );
    return sal_True;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateMainNotes()
{
    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainNotes, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)0x80000001                                               // Number that identifies this slide
            << (sal_uInt32)0;                                                       // follow nothing
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);

    ImplWritePage( GetLayout( 20 ), aSolverContainer, NOTICE, sal_True );

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );
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
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return sal_True;
}

// ---------------------------------------------------------------------------------------------

static OUString getInitials( const OUString& rName )
{
    OUString sInitials;

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
            sInitials += OUString( *pStr );
            nLength--; pStr++;
        }

        // skip letters until whitespace
        while( nLength && (*pStr > ' ') )
        {
            nLength--; pStr++;
        }
    }

    return sInitials;
}

void ImplExportComments( uno::Reference< drawing::XDrawPage > xPage, SvMemoryStream& rBinaryTagData10Atom )
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
                MapMode aMapDest( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) );
                Point aPoint( OutputDevice::LogicToLogic( Point( static_cast< sal_Int32 >( aRealPoint2D.X * 100.0 ),
                    static_cast< sal_Int32 >( aRealPoint2D.Y * 100.0 ) ), MAP_100TH_MM, aMapDest ) );

                OUString sAuthor( xAnnotation->getAuthor() );
                uno::Reference< text::XText > xText( xAnnotation->getTextRange() );
                OUString sText( xText->getString() );
                OUString sInitials( getInitials( sAuthor ) );
                util::DateTime aDateTime( xAnnotation->getDateTime() );
                if ( !sAuthor.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sAuthor, 0 );
                if ( !sText.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sText, 1 );
                if ( !sInitials.isEmpty() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sInitials, 2 );

                sal_Int16 nMilliSeconds = static_cast<sal_Int16>(::rtl::math::round(static_cast<double>(aDateTime.NanoSeconds) / 1000000000.0));
                EscherExAtom aCommentAtom10( rBinaryTagData10Atom, EPP_CommentAtom10 );
                rBinaryTagData10Atom << nIndex++
                                     << aDateTime.Year
                                     << aDateTime.Month
                                     << aDateTime.Day   // todo: day of week
                                     << aDateTime.Day
                                     << aDateTime.Hours
                                     << aDateTime.Minutes
                                     << aDateTime.Seconds
                                     << nMilliSeconds
                                     << static_cast< sal_Int32 >( aPoint.X() )
                                     << static_cast< sal_Int32 >( aPoint.Y() );
            }
        }
    }
    catch ( uno::Exception& )
    {
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteNotes( sal_uInt32 nPageNum )
{
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Notes | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)nPageNum + 0x100
            << (sal_uInt16)3                                        // follow master ....
            << (sal_uInt16)0;

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);

    ImplWritePage( GetLayout( 20 ), aSolverContainer, NOTICE, sal_False );  // the shapes of the pages are created in the PPT document

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 ); // Flags: Connector | Background | HasSpt
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
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
};

void PPTWriter::ImplWriteBackground( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    //************************ ******
    //** DEFAULT BACKGROUND SHAPE **
    //******************************

    sal_uInt32 nFillColor = 0xffffff;
    sal_uInt32 nFillBackColor = 0;

    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );                     // Flags: Connector | Background | HasSpt

    // #i121183# Use real PageSize in 100th mm
    Rectangle aRect(Point(0, 0), Size(maPageSize.Width, maPageSize.Height));

    EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm, aRect );
    aPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );
    ::com::sun::star::drawing::FillStyle aFS( ::com::sun::star::drawing::FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, OUString( "FillStyle" ) ) )
        mAny >>= aFS;

    switch( aFS )
    {
        case ::com::sun::star::drawing::FillStyle_GRADIENT :
        {
            aPropOpt.CreateGradientProperties( rXPropSet );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x1f001e );
            aPropOpt.GetOpt( ESCHER_Prop_fillColor, nFillColor );
            aPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
        }
        break;

        case ::com::sun::star::drawing::FillStyle_BITMAP :
            aPropOpt.CreateGraphicProperties( rXPropSet, OUString( "FillBitmapURL" ), sal_True );
        break;

        case ::com::sun::star::drawing::FillStyle_HATCH :
            aPropOpt.CreateGraphicProperties( rXPropSet, OUString( "FillHatch" ), sal_True );
        break;

        case ::com::sun::star::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( rXPropSet, OUString( "FillColor" ) ) )
            {
                nFillColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                nFillBackColor = nFillColor ^ 0xffffff;
            }
        }   // PASSTHROUGH INTENDED
        case ::com::sun::star::drawing::FillStyle_NONE :
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
        mpVBA->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nLen = mpVBA->Tell();
        if ( nLen > 8 )
        {
            nLen -= 8;
            mnVBAOleOfs = mpStrm->Tell();
            mpPptEscherEx->BeginAtom();
            mpStrm->Write( (sal_Int8*)mpVBA->GetData() + 8, nLen );
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteOLE( )
{

    SvxMSExportOLEObjects aOleExport( mnCnvrtFlags );

    for ( std::vector<PPTExOleObjEntry*>::const_iterator it = maExOleObj.begin(); it != maExOleObj.end(); ++it )
    {
        PPTExOleObjEntry* pPtr = *it;
        SvMemoryStream* pStrm = NULL;
        pPtr->nOfsB = mpStrm->Tell();
        switch ( pPtr->eType )
        {
            case NORMAL_OLE_OBJECT :
            {
                SdrObject* pSdrObj = GetSdrObjectFromXShape( pPtr->xShape );
                if ( pSdrObj && pSdrObj->ISA( SdrOle2Obj ) )
                {
                    ::uno::Reference < embed::XEmbeddedObject > xObj( ( (SdrOle2Obj*) pSdrObj )->GetObjRef() );
                    if( xObj.is() )
                    {
                        SvStorageRef xTempStorage( new SvStorage( new SvMemoryStream(), sal_True ) );
                        aOleExport.ExportOLEObject( xObj, *xTempStorage );

                        //TODO/MBA: testing
                        OUString aPersistStream( SVEXT_PERSIST_STREAM );
                        SvMemoryStream aStream;
                        SvStorageRef xCleanStorage( new SvStorage( sal_False, aStream ) );
                        xTempStorage->CopyTo( xCleanStorage );
                        // create a dummy content stream, the dummy content is necessary for ppt, but not for
                        // doc files, so we can't share code.
                        SotStorageStreamRef xStm = xCleanStorage->OpenSotStream( aPersistStream, STREAM_STD_READWRITE );
                        *xStm   << (sal_uInt32)0        // no ClipboardId
                                << (sal_uInt32)4        // no target device
                                << (sal_uInt32)1        // aspect ratio
                                << (sal_Int32)-1        // L-Index
                                << (sal_uInt32)0        // Advanced Flags
                                << (sal_uInt32)0        // compression
                                << (sal_uInt32)0        // Size
                                << (sal_uInt32)0        //  "
                                << (sal_uInt32)0;
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
                    ::com::sun::star::awt::Size  aSize( pPtr->xShape->getSize() );
                    SvStorageRef xDest( new SvStorage( new SvMemoryStream(), sal_True ) );
                    sal_Bool bOk = oox::ole::MSConvertOCXControls::WriteOCXStream( mXModel, xDest, pPtr->xControlModel, aSize, aName );
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
            *mpStrm << npStrmSize;                  // uncompressed size

            pStrm->Seek( 0 );
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Compress( *pStrm, *mpStrm );
            aZCodec.EndCompression();
            delete pStrm;
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------
// write PersistantTable and UserEditAtom

sal_Bool PPTWriter::ImplWriteAtomEnding()
{

#define EPP_LastViewTypeSlideView   1

    sal_uInt32  i, nPos, nOfs, nPersistOfs = mpStrm->Tell();
    sal_uInt32  nPersistEntrys = 0;
    *mpStrm << (sal_uInt32)0 << (sal_uInt32)0 << (sal_uInt32)0;         // skip record header and first entry

    // write document persist
        nPersistEntrys++;
        *mpStrm << (sal_uInt32)0;
    // write MasterPages persists
    for ( i = 0; i < mnMasterPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainMaster | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // write MainNotesMaster persist
    nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainNotes );
    if ( nOfs )
    {
        *mpStrm << nOfs;
        mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, ++nPersistEntrys );
    }
    // write slide persists -> we have to write a valid value into EPP_SlidePersistAtome too
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Slide | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // write Notes persists
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Notes | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Ole persists
    for ( std::vector<PPTExOleObjEntry*>::const_iterator it = maExOleObj.begin(); it != maExOleObj.end(); ++it )
    {
        PPTExOleObjEntry* pPtr = *it;
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_ExObj );
        if ( nOfs )
        {
            nPersistEntrys++;
            *mpStrm << pPtr->nOfsB;
            sal_uInt32 nOldPos, nPersOfs = nOfs + pPtr->nOfsA + 16 + 8;     // 8 bytes atom header, +16 to the persist entry
            nOldPos = mpStrm->Tell();
            mpStrm->Seek( nPersOfs );
            *mpStrm << nPersistEntrys;
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
            *mpVBA >> n1
                   >> n2;

            *mpStrm << mnVBAOleOfs;
            sal_uInt32 nOldPos = mpStrm->Tell();
            mpStrm->Seek( nOfs );               // Fill the VBAInfoAtom with the correct index to the persisttable
            *mpStrm << nPersistEntrys
                    << n1
                    << sal_Int32(2);
            mpStrm->Seek( nOldPos );

        }
    }
    nPos = mpStrm->Tell();
    mpStrm->Seek( nPersistOfs );
    mpPptEscherEx->AddAtom( ( nPersistEntrys + 1 ) << 2, EPP_PersistPtrIncrementalBlock );      // insert Record Header
    *mpStrm << (sal_uInt32)( ( nPersistEntrys << 20 ) | 1 );
    mpStrm->Seek( nPos );

    *mpCurUserStrm << (sal_uInt32)nPos;             // set offset to current edit
    mpPptEscherEx->AddAtom( 28, EPP_UserEditAtom );
    *mpStrm << (sal_Int32)0x100                     // last slide ID
            << (sal_uInt32)0x03000dbc               // minor and major app version that did the save
            << (sal_uInt32)0                        // offset last save, 0 after a full save
            << nPersistOfs                      // File offset to persist pointers for this save operation
            << (sal_uInt32)1                        // Persist reference to the document persist object
            << (sal_uInt32)nPersistEntrys           // max persists written, Seed value for persist object id management
            << (sal_Int16)EPP_LastViewTypeSlideView // last view type
            << (sal_Int16)0x12;                     // padword

    return sal_True;
}

// ---------------------
// - exported function -
// ---------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL ExportPPT( const std::vector< com::sun::star::beans::PropertyValue >& rMediaData, SvStorageRef& rSvStorage,
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
                        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
                            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags )
{
    PPTWriter*  pPPTWriter;
    sal_Bool bStatus = sal_False;

    pPPTWriter = new PPTWriter( rSvStorage, rXModel, rXStatInd, pVBA, nCnvrtFlags );
    if ( pPPTWriter )
    {
        pPPTWriter->exportPPT(rMediaData);
        bStatus = ( pPPTWriter->IsValid() == sal_True );
        delete pPPTWriter;
    }

    return bStatus;
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL SaveVBA( SfxObjectShell& rDocShell, SvMemoryStream*& pBas )
{
    SvStorageRef xDest( new SvStorage( new SvMemoryStream(), sal_True ) );
    SvxImportMSVBasic aMSVBas( rDocShell, *xDest );
    aMSVBas.SaveOrDelMSVBAStorage( sal_True, OUString( "_MS_VBA_Overhead" ) );

    SvStorageRef xOverhead = xDest->OpenSotStorage( OUString( "_MS_VBA_Overhead") );
    if ( xOverhead.Is() && ( xOverhead->GetError() == SVSTREAM_OK ) )
    {
        SvStorageRef xOverhead2 = xOverhead->OpenSotStorage( OUString( "_MS_VBA_Overhead") );
        if ( xOverhead2.Is() && ( xOverhead2->GetError() == SVSTREAM_OK ) )
        {
            SvStorageStreamRef xTemp = xOverhead2->OpenSotStream( OUString( "_MS_VBA_Overhead2") );
            if ( xTemp.Is() && ( xTemp->GetError() == SVSTREAM_OK ) )
            {
                sal_uInt32 nLen = xTemp->GetSize();
                if ( nLen )
                {
                    char* pTemp = new char[ nLen ];
                    if ( pTemp )
                    {
                        xTemp->Seek( STREAM_SEEK_TO_BEGIN );
                        xTemp->Read( pTemp, nLen );
                        pBas = new SvMemoryStream( pTemp, nLen, STREAM_READ );
                        pBas->ObjectOwnsMemory( sal_True );
                        return sal_True;
                    }
                }
            }
        }
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
