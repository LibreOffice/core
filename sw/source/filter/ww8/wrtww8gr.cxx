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


#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <rtl/math.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>
#include <svl/itemiter.hxx>
#include "svl/urihelper.hxx"

#include <svtools/embedhlp.hxx>

#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <editeng/lrspitem.hxx> // SvxLRSpaceItem
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdoole2.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <fmtanchr.hxx>
#include <ndgrf.hxx>
#include <frmfmt.hxx>       // class SwFlyFrmFmt
#include <grfatr.hxx>       // class SwCropGrf
#include <ndole.hxx>
#include <ndtxt.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>

#include <doctok/sprmids.hxx>

#include <doc.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8struc.hxx"
#include "wrtww8.hxx"
#include "ww8par.hxx"
#include "escher.hxx"
//Added for i120568
#include "ww8attributeoutput.hxx"
#include "fmturl.hxx"

#include "docsh.hxx"
#include <cstdio>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace ::com::sun::star;
using namespace nsFieldFlags;

// TODO:
// 5. convert the MapModes that Widows can't handle

// OutGrf () is called for every GrafNode in the document. Es wird ein PicLocFc-Sprm
// eingefuegt, der statt Adresse ein Magic sal_uLong enthaelt. Ausserdem wird
// in der Graf-Klasse der GrfNode-Ptr gemerkt ( fuers spaetere Ausgeben der
// Grafiken und Patchen der PicLocFc-Attribute )

void WW8Export::OutputGrfNode( const SwGrfNode& /*rNode*/ )
{
    OSL_TRACE("WW8Export::OutputGrfNode( const SwGrfNode& )" );
    OSL_ENSURE( mpParentFrame, "frame not set!" );
    if ( mpParentFrame )
    {
        OutGrf( *mpParentFrame );
        pFib->fHasPic = 1;
    }
}

bool WW8Export::TestOleNeedsGraphic(const SwAttrSet& rSet,
    SvStorageRef xOleStg, SvStorageRef xObjStg, String &rStorageName,
    SwOLENode *pOLENd)
{
    bool bGraphicNeeded = false;
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();

    do {
        switch (pItem->Which())
        {
            /*
            For an inline object these properties are irrelevent because they
            will be the same as the defaults that msword applies in their
            absence, so if that is all that there is for these inline objects
            then if there turns out to be enough information in the object
            itself to regenerate the correct size and preview of the object
            then we will not need to provide an additional graphics preview in
            the data stream, which can save a lot of disk space.
            */
            case RES_FRM_SIZE:
            case RES_CNTNT:
            case RES_VERT_ORIENT:
            case RES_ANCHOR:
                break;
            default:
                bGraphicNeeded = true;
        }
    } while( !bGraphicNeeded && !aIter.IsAtEnd() &&
        0 != ( pItem = aIter.NextItem() ) );

    /*
    Now we must see if the object contains a preview itself which is equal to
    the preview that we are currently using. If the graphics are equal then we
    dont need to store another preview
    */
    GDIMetaFile aWMF;
    long nX=0,nY=0;
    if (!bGraphicNeeded && SwWW8ImplReader::ImportOleWMF(xOleStg,aWMF,nX,nY))
    {
        // bGraphicNeeded set to true is right / fixes #i51670#.
        bGraphicNeeded = true;
        Point aTmpPoint;
        Rectangle aRect( aTmpPoint, Size( nX, nY ) );
        Graphic aGraph(aWMF);

        ErrCode nErr = ERRCODE_NONE;
        Rectangle aVisArea;
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        if ( pOLENd )
            nAspect = pOLENd->GetAspect();
        SdrOle2Obj *pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
            rStorageName,xObjStg,pDoc->GetDocStorage(),aGraph,aRect,aVisArea,0,nErr,0,nAspect);

        if (pRet)
        {
            uno::Reference< embed::XEmbeddedObject > xObj = pOLENd->GetOLEObj().GetOleRef();
            if ( xObj.is() )
            {
                SvStream* pGraphicStream = NULL;
                comphelper::EmbeddedObjectContainer aCnt( pDoc->GetDocStorage() );
                try
                {
                    uno::Reference< embed::XEmbedPersist > xPersist(
                            xObj,
                            uno::UNO_QUERY_THROW );

                    // it makes no sence to search the object in the container by reference since the object was created
                    // outside of the container and was not inserted there, only the name makes sence
                    pGraphicStream =
                            ::utl::UcbStreamHelper::CreateStream( aCnt.GetGraphicStream( xPersist->getEntryName() ) );
                }
                catch( const uno::Exception& )
                {}

                OSL_ENSURE( pGraphicStream && !pGraphicStream->GetError(), "No graphic stream available!" );
                if ( pGraphicStream && !pGraphicStream->GetError() )
                {
                    Graphic aGr1;
                    GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
                    if( rGF.ImportGraphic( aGr1, aEmptyStr, *pGraphicStream, GRFILTER_FORMAT_DONTKNOW ) == GRFILTER_OK )
                    {
                        Graphic aGr2;
                        delete pGraphicStream;
                        pGraphicStream =
                                ::utl::UcbStreamHelper::CreateStream( aCnt.GetGraphicStream( pRet->GetObjRef() ) );
                        if( rGF.ImportGraphic( aGr2, aEmptyStr, *pGraphicStream, GRFILTER_FORMAT_DONTKNOW ) == GRFILTER_OK )
                        {
                            if ( aGr1 == aGr2 )
                                bGraphicNeeded = false;
                        }
                    }
                }
                else
                    delete pGraphicStream;
            }

            delete pRet;
        }
    }
    else
        bGraphicNeeded = true;
    return bGraphicNeeded;
}

void WW8Export::OutputOLENode( const SwOLENode& rOLENode )
{
    OSL_TRACE("WW8Export::OutputOLENode( const SwOLENode& rOLENode )" );
    sal_uInt8 *pSpecOLE;
    sal_uInt8 *pDataAdr;
    short nSize;
    static sal_uInt8 aSpecOLE_WW8[] = {
            0x03, 0x6a, 0, 0, 0, 0, // sprmCPicLocation
            0x0a, 0x08, 1,          // sprmCFOLE2
            0x56, 0x08, 1           // sprmCFObj
        };
    static sal_uInt8 aSpecOLE_WW6[] = {
            68, 4, 0, 0, 0, 0,      // sprmCPicLocation (len is 4)
            75, 1,                  // sprmCFOLE2
            118, 1                  // sprmCFObj
        };

    if ( bWrtWW8 )
    {
        pSpecOLE = aSpecOLE_WW8;
        nSize = sizeof( aSpecOLE_WW8 );
    }
    else
    {
        pSpecOLE = aSpecOLE_WW6;
        nSize = sizeof( aSpecOLE_WW6 );
    }
    pDataAdr = pSpecOLE + 2; //WW6 sprm is 1 but has 1 byte len as well.

    SvStorageRef xObjStg = GetWriter().GetStorage().OpenSotStorage(
        OUString(SL::aObjectPool), STREAM_READWRITE |
        STREAM_SHARE_DENYALL );

    if( xObjStg.Is()  )
    {
        uno::Reference < embed::XEmbeddedObject > xObj(const_cast<SwOLENode&>(rOLENode).GetOLEObj().GetOleRef());
        if( xObj.is() )
        {
            const embed::XEmbeddedObject *pObj = xObj.get();
            WW8OleMap& rPointerToObjId = GetOLEMap();
            //Don't want to use pointer ids, as is traditional, because we need
            //to put this into a 32bit value, and on 64bit the bottom bits
            //might collide and two unrelated ole objects end up considered the
            //same.  Don't want to simply start at 0 which is a special value
            sal_Int32 nPictureId = SAL_MAX_INT32 - rPointerToObjId.size();
            WW8OleMap::value_type entry = std::make_pair(pObj, nPictureId);
            std::pair<WW8OleMap::iterator, bool> aRes = rPointerToObjId.insert(entry);
            bool bIsNotDuplicate = aRes.second; //.second is false when element already existed
            nPictureId = aRes.first->second;
            Set_UInt32(pDataAdr, nPictureId);
            String sStorageName = OUString('_');
            sStorageName += OUString::number( nPictureId );
            SvStorageRef xOleStg = xObjStg->OpenSotStorage( sStorageName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xOleStg.Is() )
            {
                /*
                If this object storage has been written already don't
                waste time rewriting it
                */
                if (bIsNotDuplicate)
                {
                    sal_Int64 nAspect = rOLENode.GetAspect();
                    svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
                    GetOLEExp().ExportOLEObject( aObjRef, *xOleStg );
                    if ( nAspect == embed::Aspects::MSOLE_ICON )
                    {
                        OUString aObjInfo( "\3ObjInfo" );
                        if ( !xOleStg->IsStream( aObjInfo ) )
                        {
                            const sal_uInt8 pObjInfoData[] = { 0x40, 0x00, 0x03, 0x00 };
                            SvStorageStreamRef rObjInfoStream = xOleStg->OpenSotStream( aObjInfo );
                            if ( rObjInfoStream.Is() && !rObjInfoStream->GetError() )
                            {
                                rObjInfoStream->Write( pObjInfoData, sizeof( pObjInfoData ) );
                                xOleStg->Commit();
                            }
                        }
                    }
                }

                // write as embedded field - the other things will be done
                // in the escher export
                String sServer(FieldString(ww::eEMBED));
                sServer += xOleStg->GetUserName();
                sServer += ' ';

                OutputField(0, ww::eEMBED, sServer, WRITEFIELD_START |
                    WRITEFIELD_CMD_START | WRITEFIELD_CMD_END);

                pChpPlc->AppendFkpEntry( Strm().Tell(),
                        nSize, pSpecOLE );

                bool bEndCR = true;
                /*
                In the word filter we only need a preview image for
                floating images, and then only (the usual case) if the
                object doesn't contain enough information to reconstruct
                what we need.

                We don't need a graphic for inline objects, so we don't
                even need the overhead of a graphic in that case.
                */
                bool bGraphicNeeded = false;

                if (mpParentFrame)
                {
                    bGraphicNeeded = true;

                    if (mpParentFrame->IsInline())
                    {
                        const SwAttrSet& rSet =
                            mpParentFrame->GetFrmFmt().GetAttrSet();
                        bEndCR = false;
                        bGraphicNeeded = TestOleNeedsGraphic(rSet,
                            xOleStg, xObjStg, sStorageName, const_cast<SwOLENode*>(&rOLENode));
                    }
                }

                if (!bGraphicNeeded)
                    WriteChar(0x1);
                else
                {
                    /*
                    ##897##
                    We need to insert the graphic representation of
                    this object for the inline case, otherwise word
                    has no place to find the dimensions of the ole
                    object, and will not be able to draw it
                    */
                    OutGrf(*mpParentFrame);
                }

                OutputField(0, ww::eEMBED, aEmptyStr,
                    WRITEFIELD_END | WRITEFIELD_CLOSE);

                if (bEndCR) //No newline in inline case
                    WriteCR();
            }
        }
    }
}

void WW8Export::OutputLinkedOLE( const OUString& rOleId )
{
    uno::Reference< embed::XStorage > xDocStg = pDoc->GetDocStorage();
    uno::Reference< embed::XStorage > xOleStg = xDocStg->openStorageElement( "OLELinks", embed::ElementModes::READ );
    SotStorageRef xObjSrc = SotStorage::OpenOLEStorage( xOleStg, rOleId, STREAM_READ );

    SotStorageRef xObjStg = GetWriter().GetStorage().OpenSotStorage(
        OUString(SL::aObjectPool), STREAM_READWRITE |
        STREAM_SHARE_DENYALL );

    if( xObjStg.Is() && xObjSrc.Is() )
    {
        SotStorageRef xOleDst = xObjStg->OpenSotStorage( rOleId,
                STREAM_READWRITE | STREAM_SHARE_DENYALL );
        if ( xOleDst.Is() )
            xObjSrc->CopyTo( xOleDst );

        if ( !xOleDst->GetError( ) )
        {
            xOleDst->Commit();

            // Ouput the cPicLocation attribute
            ww::bytes* pBuf = new ww::bytes();
            GetWriter().InsUInt16( *pBuf, NS_sprm::LN_CPicLocation );
            GetWriter().InsUInt32( *pBuf, rOleId.copy( 1 ).toInt32() );

            GetWriter().InsUInt16( *pBuf, NS_sprm::LN_CFOle2 );
            pBuf->push_back( 1 );

            GetWriter().InsUInt16( *pBuf, NS_sprm::LN_CFSpec );
            pBuf->push_back( 1 );

            GetWriter().InsUInt16( *pBuf, NS_sprm::LN_CFObj );
            pBuf->push_back( 1 );

            pChpPlc->AppendFkpEntry( Strm().Tell(), pBuf->size(), pBuf->data() );
            delete pBuf;
        }
    }
}

void WW8Export::OutGrf(const sw::Frame &rFrame)
{
    //Added for i120568,the hyperlink info within a graphic whose anchor type is "As character"
    //will be exported to ensure the fidelity
    const SwFmtURL& rURL = rFrame.GetFrmFmt().GetAttrSet().GetURL();
    bool bURLStarted = false;
    if( !rURL.GetURL().isEmpty() && rFrame.GetWriterType() == sw::Frame::eGraphic)
    {
        bURLStarted = true;
        m_pAttrOutput->StartURL( rURL.GetURL(), rURL.GetTargetFrameName() );
    }

    // Store the graphic settings in GrfNode so they may be written-out later
    pGrf->Insert(rFrame);

    pChpPlc->AppendFkpEntry( Strm().Tell(), pO->size(), pO->data() );
    pO->clear();

    // #i29408#
    // linked, as-character anchored graphics have to be exported as fields.
    const SwGrfNode* pGrfNd = rFrame.IsInline() && rFrame.GetContent()
                              ? rFrame.GetContent()->GetGrfNode() : 0;
    if ( pGrfNd && pGrfNd->IsLinkedFile() )
    {
        String sStr( FieldString(ww::eINCLUDEPICTURE) );
        sStr.AppendAscii(" \"");
        {
            if ( pGrfNd )
            {
                String aFileURL;
                pGrfNd->GetFileFilterNms( &aFileURL, 0 );
                sStr += aFileURL;
            }
        }
        sStr.AppendAscii("\" \\d");

        OutputField( 0, ww::eINCLUDEPICTURE, sStr,
                   WRITEFIELD_START | WRITEFIELD_CMD_START | WRITEFIELD_CMD_END );
    }

    WriteChar( (char)1 );   // paste graphic symbols in the main text

    sal_uInt8 aArr[ 18 ];
    sal_uInt8* pArr = aArr;

    const SwFrmFmt &rFlyFmt = rFrame.GetFrmFmt();
    const RndStdIds eAn = rFlyFmt.GetAttrSet().GetAnchor(false).GetAnchorId();
    if (eAn == FLY_AS_CHAR)
    {
        sal_Int16 eVert = rFlyFmt.GetVertOrient().GetVertOrient();
        if ((eVert == text::VertOrientation::CHAR_CENTER) || (eVert == text::VertOrientation::LINE_CENTER))
        {
            bool bVert = false;
            //The default for word in vertical text mode is to center,
            //otherwise a sub/super script hack is employed
            if (pOutFmtNode && pOutFmtNode->ISA(SwCntntNode) )
            {
                const SwTxtNode* pTxtNd = (const SwTxtNode*)pOutFmtNode;
                SwPosition aPos(*pTxtNd);
                bVert = pDoc->IsInVerticalText(aPos);
            }
            if (!bVert)
            {
                SwTwips nHeight = rFlyFmt.GetFrmSize().GetHeight();
                nHeight/=20; //nHeight was in twips, want it in half points, but
                             //then half of total height.
                long nFontHeight = ((const SvxFontHeightItem&)
                    GetItem(RES_CHRATR_FONTSIZE)).GetHeight();
                nHeight-=nFontHeight/20;

                if (bWrtWW8)
                    Set_UInt16( pArr, NS_sprm::LN_CHpsPos );
                else
                    Set_UInt8( pArr, 101 );
                Set_UInt16( pArr, -((sal_Int16)nHeight));
            }
        }
    }

    // sprmCFSpec
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x855 );
    else
        Set_UInt8( pArr, 117 );
    Set_UInt8( pArr, 1 );

    // sprmCPicLocation
    if( bWrtWW8 )
        Set_UInt16( pArr, NS_sprm::LN_CPicLocation );
    else
    {
        Set_UInt8( pArr, 68 );
        Set_UInt8( pArr, 4 );
    }
    Set_UInt32( pArr, GRF_MAGIC_321 );

    // vary Magic, so that different graphic attributes will not be merged
    static sal_uInt8 nAttrMagicIdx = 0;
    --pArr;
    Set_UInt8( pArr, nAttrMagicIdx++ );
    pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );

    // #i75464#
    // Check, if graphic isn't exported as-character anchored.
    // Otherwise, an additional paragraph is exported for a graphic, which is
    // forced to be treated as inline, because it's anchored inside another frame.
    if ( !rFrame.IsInline() &&
         ( ((eAn == FLY_AT_PARA) && ( bWrtWW8 || !IsInTable() )) ||
           (eAn == FLY_AT_PAGE)) )
    {
        WriteChar( (char)0x0d ); // close the surrounding frame with CR

        static sal_uInt8 nSty[2] = { 0, 0 };
        pO->insert( pO->end(), nSty, nSty+2 );     // Style #0
        bool bOldGrf = bOutGrf;
        bOutGrf = true;

        OutputFormat( rFrame.GetFrmFmt(), false, false, true ); // Fly-Attrs

        bOutGrf = bOldGrf;
        pPapPlc->AppendFkpEntry( Strm().Tell(), pO->size(), pO->data() );
        pO->clear();
    }
    // #i29408#
    // linked, as-character anchored graphics have to be exported as fields.
    else if ( pGrfNd && pGrfNd->IsLinkedFile() )
    {
        OutputField( 0, ww::eINCLUDEPICTURE, String(), WRITEFIELD_CLOSE );
    }
    //Added for i120568,the hyperlink info within a graphic whose anchor type is
    //"As character" will be exported to ensure the fidelity
    if( bURLStarted )
        m_pAttrOutput->EndURL();
}

GraphicDetails& GraphicDetails::operator=(const GraphicDetails &rOther)
{
    maFly = rOther.maFly;
    mnPos = rOther.mnPos;
    mnWid = rOther.mnWid;
    mnHei = rOther.mnHei;
    return *this;
}

void SwWW8WrGrf::Insert(const sw::Frame &rFly)
{
    const Size aSize( rFly.GetLayoutSize() );
    const sal_uInt16 nWidth = static_cast< sal_uInt16 >(aSize.Width());
    const sal_uInt16 nHeight = static_cast< sal_uInt16 >(aSize.Height());
    maDetails.push_back(GraphicDetails(rFly, nWidth, nHeight));
}

void SwWW8WrGrf::WritePICFHeader(SvStream& rStrm, const sw::Frame &rFly,
    sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight, const SwAttrSet* pAttrSet)
{
    sal_Int16 nXSizeAdd = 0, nYSizeAdd = 0;
    sal_Int16 nCropL = 0, nCropR = 0, nCropT = 0, nCropB = 0;

            // write Crop-Attribut content in Header ( if available )
    const SfxPoolItem* pItem;
    if (pAttrSet && (SFX_ITEM_ON
        == pAttrSet->GetItemState(RES_GRFATR_CROPGRF, false, &pItem)))
    {
        const SwCropGrf& rCr = *(SwCropGrf*)pItem;
        nCropL = (sal_Int16)rCr.GetLeft();
        nCropR = (sal_Int16)rCr.GetRight();
        nCropT = (sal_Int16)rCr.GetTop();
        nCropB = (sal_Int16)rCr.GetBottom();
        nXSizeAdd = nXSizeAdd - (sal_Int16)( rCr.GetLeft() + rCr.GetRight() );
        nYSizeAdd = nYSizeAdd - (sal_Int16)( rCr.GetTop() + rCr.GetBottom() );
    }

    Size aGrTwipSz(rFly.GetSize());
    bool bWrtWW8 = rWrt.bWrtWW8;
    sal_uInt16 nHdrLen = bWrtWW8 ? 0x44 : 0x3A;

    sal_uInt8 aArr[ 0x44 ] = { 0 };

    sal_uInt8* pArr = aArr + 0x2E;  // Do borders first

    const SwAttrSet& rAttrSet = rFly.GetFrmFmt().GetAttrSet();
    if (SFX_ITEM_ON == rAttrSet.GetItemState(RES_BOX, false, &pItem))
    {
        const SvxBoxItem* pBox = (const SvxBoxItem*)pItem;
        if( pBox )
        {
            bool bShadow = false;               // Shadow ?
            const SvxShadowItem* pSI =
                sw::util::HasItem<SvxShadowItem>(rAttrSet, RES_SHADOW);
            if (pSI)
            {
                bShadow = (pSI->GetLocation() != SVX_SHADOW_NONE) &&
                    (pSI->GetWidth() != 0);
            }

            sal_uInt8 aLnArr[4] = { BOX_LINE_TOP, BOX_LINE_LEFT,
                                BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
            for( sal_uInt8 i = 0; i < 4; ++i )
            {
                const ::editeng::SvxBorderLine* pLn = pBox->GetLine( aLnArr[ i ] );
                WW8_BRC aBrc;
                if (pLn)
                {
                    aBrc = rWrt.TranslateBorderLine( *pLn,
                        pBox->GetDistance( aLnArr[ i ] ), bShadow );
                }

                // use importer logic to determine how large the exported
                // border will really be in word and adjust accordingly
                short nSpacing;
                short nThick = aBrc.DetermineBorderProperties(!bWrtWW8,
                    &nSpacing);
                switch (aLnArr[ i ])
                {
                    case BOX_LINE_TOP:
                    case BOX_LINE_BOTTOM:
                        nHeight -= bShadow ? nThick*2 : nThick;
                        nHeight = nHeight - nSpacing;
                        break;
                    case BOX_LINE_LEFT:
                    case BOX_LINE_RIGHT:
                    default:
                        nWidth -= bShadow ? nThick*2 : nThick;
                        nWidth = nWidth - nSpacing;
                        break;
                }
                memcpy( pArr, &aBrc.aBits1, 2);
                pArr+=2;

                if( bWrtWW8 )
                {
                    memcpy( pArr, &aBrc.aBits2, 2);
                    pArr+=2;
                }
            }
        }
    }

    pArr = aArr + 4;                                // skip lcb
    Set_UInt16( pArr, nHdrLen );                    // set cbHeader

    Set_UInt16( pArr, mm );                         // set mm

    /*
    Just in case our original size is too big to fit inside a ushort we can
    substitute the final size and loose on retaining the scaling factor but
    still keep the correct display size anyway.
    */
    const bool bIsSubstitutedSize = (aGrTwipSz.Width() > SHRT_MAX) || (aGrTwipSz.Height() > SHRT_MAX) ||
                                    (aGrTwipSz.Width() < 0 ) || (aGrTwipSz.Height() < 0);
    if ( bIsSubstitutedSize )
    {
        aGrTwipSz.Width() = nWidth;
        aGrTwipSz.Height() = nHeight;
    }
    using namespace sw::types;
    // set xExt & yExt
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Width() * 254L / 144));
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Height() * 254L / 144));
    pArr += 16;
    // skip hMF & rcWinMF
    // set dxaGoal & dyaGoal
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Width()));
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Height()));

    if ( aGrTwipSz.Width() + nXSizeAdd )             // set mx
    {
        if ( !bIsSubstitutedSize )
        {
            const double fVal = nWidth * 1000.0 / (aGrTwipSz.Width() + nXSizeAdd );
            Set_UInt16( pArr, (sal_uInt16)::rtl::math::round(fVal) );
        }
        else
        {
            Set_UInt16( pArr, 1000 );
        }
    }
    else
    {
        pArr += 2;
    }

    if ( aGrTwipSz.Height() + nYSizeAdd )            // set my
    {
        if ( !bIsSubstitutedSize )
        {
            const double fVal = nHeight * 1000.0 / (aGrTwipSz.Height() + nYSizeAdd);
            Set_UInt16( pArr, (sal_uInt16)::rtl::math::round(fVal) );
        }
        else
        {
            Set_UInt16( pArr, 1000 );
        }
    }
    else
    {
        pArr += 2;
    }

    if ( !bIsSubstitutedSize )
    {
        Set_UInt16( pArr, nCropL );                     // set dxaCropLeft
        Set_UInt16( pArr, nCropT );                     // set dyaCropTop
        Set_UInt16( pArr, nCropR );                     // set dxaCropRight
        Set_UInt16( pArr, nCropB );                     // set dyaCropBottom
    }
    else
    {
        pArr += 8;
    }

    rStrm.Write( aArr, nHdrLen );
}

void SwWW8WrGrf::WriteGrfFromGrfNode(SvStream& rStrm, const SwGrfNode &rGrfNd,
    const sw::Frame &rFly, sal_uInt16 nWidth, sal_uInt16 nHeight)
{
    if (rGrfNd.IsLinkedFile())     // Linked File
    {
        String aFileN;
        rGrfNd.GetFileFilterNms( &aFileN, 0 );

            sal_uInt16 mm = 94;                    // 94 = BMP, GIF

        WritePICFHeader(rStrm, rFly, mm, nWidth, nHeight,
            rGrfNd.GetpSwAttrSet());
        rStrm << (sal_uInt8)aFileN.Len();    // write Pascal-String
        SwWW8Writer::WriteString8(rStrm, aFileN, false,
            RTL_TEXTENCODING_MS_1252);
    }
    else                                // Embedded File or DDE or something like that
    {
        if (rWrt.bWrtWW8)
        {
            WritePICFHeader(rStrm, rFly, 0x64, nWidth, nHeight,
                rGrfNd.GetpSwAttrSet());
            SwBasicEscherEx aInlineEscher(&rStrm, rWrt);
            aInlineEscher.WriteGrfFlyFrame(rFly.GetFrmFmt(), 0x401);
            aInlineEscher.WritePictures();
        }
        else
        {
            Graphic& rGrf = const_cast<Graphic&>(rGrfNd.GetGrf());
            bool bSwapped = rGrf.IsSwapOut() ? true : false;
            // immer ueber den Node einswappen!
            const_cast<SwGrfNode&>(rGrfNd).SwapIn();

            GDIMetaFile aMeta;
            switch (rGrf.GetType())
            {
                case GRAPHIC_BITMAP:        // Bitmap -> play in Metafile
                    {
                        VirtualDevice aVirt;
                        aMeta.Record(&aVirt);
                        aVirt.DrawBitmap( Point( 0,0 ), rGrf.GetBitmap() );
                        aMeta.Stop();
                        aMeta.WindStart();
                        aMeta.SetPrefMapMode( rGrf.GetPrefMapMode());
                        aMeta.SetPrefSize( rGrf.GetPrefSize());
                    }
                    break;
                case GRAPHIC_GDIMETAFILE :      // GDI ( =SV ) Metafile
                    aMeta = rGrf.GetGDIMetaFile();
                    break;
                default:
                    return;
            }

            WritePICFHeader(rStrm, rFly, 8, nWidth, nHeight,
                rGrfNd.GetpSwAttrSet());
            WriteWindowMetafileBits(rStrm, aMeta);

            if (bSwapped)
                rGrf.SwapOut();
        }
    }
}
//For i120928,export graphic info of bullet
void SwWW8WrGrf::WritePICBulletFHeader(SvStream& rStrm, const Graphic &rGrf,
            sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight)
{
    sal_Int16 nXSizeAdd = 0, nYSizeAdd = 0;
    sal_Int16 nCropL = 0, nCropR = 0, nCropT = 0, nCropB = 0;

    Size aGrTwipSz(rGrf.GetPrefSize());
    bool bWrtWW8 = rWrt.bWrtWW8;
    sal_uInt16 nHdrLen = bWrtWW8 ? 0x44 : 0x3A;

    sal_uInt8 aArr[ 0x44 ] = { 0 };

    sal_uInt8* pArr = aArr + 0x2E;  //Do borders first

    sal_uInt8 aLnArr[4] = { BOX_LINE_TOP, BOX_LINE_LEFT,
    BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    for( sal_uInt8 i = 0; i < 4; ++i )
    {
        WW8_BRC aBrc;

        short nSpacing;
        short nThick = aBrc.DetermineBorderProperties(!bWrtWW8,
        &nSpacing);
        switch (aLnArr[ i ])
        {
            case BOX_LINE_TOP:
            case BOX_LINE_BOTTOM:
            nHeight -= nThick;
            nHeight = nHeight - nSpacing;
            break;
            case BOX_LINE_LEFT:
            case BOX_LINE_RIGHT:
            default:
            nWidth -= nThick;
            nWidth = nWidth - nSpacing;
            break;
        }
        memcpy( pArr, &aBrc.aBits1, 2);
        pArr+=2;

        if( bWrtWW8 )
        {
            memcpy( pArr, &aBrc.aBits2, 2);
            pArr+=2;
        }
    }

    pArr = aArr + 4;                                //skip lcb
    Set_UInt16( pArr, nHdrLen );                    // set cbHeader

    Set_UInt16( pArr, mm );                         // set mm

    if ( (aGrTwipSz.Width() * 254L / 144 > USHRT_MAX) || (aGrTwipSz.Height()  * 254L / 144 > USHRT_MAX)
        || (aGrTwipSz.Width() < 0 ) || (aGrTwipSz.Height() < 0) )
    {
        aGrTwipSz.Width() = nWidth;
        aGrTwipSz.Height() = nHeight;
    }
    using namespace sw::types;
    // set xExt & yExt
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Width() * 254L / 144));
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Height() * 254L / 144));
    pArr += 16;
    // skip hMF & rcWinMF
    // set dxaGoal & dyaGoal
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Width()));
    Set_UInt16(pArr, msword_cast<sal_uInt16>(aGrTwipSz.Height()));

    if( aGrTwipSz.Width() + nXSizeAdd )             // set mx
    {
        double fVal = nWidth * 1000.0 / (aGrTwipSz.Width() + nXSizeAdd);
        Set_UInt16( pArr, (sal_uInt16)::rtl::math::round(fVal) );
    }
    else
        pArr += 2;

    if( aGrTwipSz.Height() + nYSizeAdd )            // set my
    {
        double fVal = nHeight * 1000.0 / (aGrTwipSz.Height() + nYSizeAdd);
        Set_UInt16( pArr, (sal_uInt16)::rtl::math::round(fVal) );
    }
    else
        pArr += 2;

    Set_UInt16( pArr, nCropL );                     // set dxaCropLeft
    Set_UInt16( pArr, nCropT );                     // set dyaCropTop
    Set_UInt16( pArr, nCropR );                     // set dxaCropRight
    Set_UInt16( pArr, nCropB );                     // set dyaCropBottom

    rStrm.Write( aArr, nHdrLen );
}

void SwWW8WrGrf::WriteGrfForBullet(SvStream& rStrm, const Graphic &rGrf, sal_uInt16 nWidth, sal_uInt16 nHeight)
{
    if (rWrt.bWrtWW8)
    {
        WritePICBulletFHeader(rStrm,rGrf, 0x64,nWidth,nHeight);
        SwBasicEscherEx aInlineEscher(&rStrm, rWrt);
        aInlineEscher.WriteGrfBullet(rGrf);
        aInlineEscher.WritePictures();
    }
    else
    {
        GDIMetaFile aMeta;
        switch (rGrf.GetType())
        {
            case GRAPHIC_BITMAP:        // Bitmap -> in Metafile abspielen
            {
                VirtualDevice aVirt;
                aMeta.Record(&aVirt);
                aVirt.DrawBitmap( Point( 0,0 ), rGrf.GetBitmap() );
                aMeta.Stop();
                aMeta.WindStart();
                aMeta.SetPrefMapMode( rGrf.GetPrefMapMode());
                aMeta.SetPrefSize( rGrf.GetPrefSize());
            }
            break;
            case GRAPHIC_GDIMETAFILE :      // GDI ( =SV ) Metafile
                aMeta = rGrf.GetGDIMetaFile();
            break;
            default:
                return;
        }
        WritePICBulletFHeader(rStrm, rGrf, 8, nWidth, nHeight);
        WriteWindowMetafileBits(rStrm, aMeta);
    }
}

void SwWW8WrGrf::WriteGraphicNode(SvStream& rStrm, const GraphicDetails &rItem)
{
    sal_uInt16 nWidth = rItem.mnWid;
    sal_uInt16 nHeight = rItem.mnHei;
    sal_uInt32 nPos = rStrm.Tell();         // store start of graphic

    const sw::Frame &rFly = rItem.maFly;
    switch (rFly.GetWriterType())
    {
        case sw::Frame::eGraphic:
        {
            const SwNode *pNode = rItem.maFly.GetContent();
            const SwGrfNode *pNd = pNode ? pNode->GetGrfNode() : 0;
            OSL_ENSURE(pNd, "Impossible");
            if (pNd)
                WriteGrfFromGrfNode(rStrm, *pNd, rItem.maFly, nWidth, nHeight);
        }
        break;
        //For i120928,add branch to export graphic of bullet
        case sw::Frame::eBulletGrf:
        {
            if (rItem.maFly.HasGraphic())
            {
                const Graphic& rGrf = rItem.maFly.GetGraphic();
                WriteGrfForBullet(rStrm, rGrf, nWidth, nHeight);
            }
        }
        break;

        case sw::Frame::eOle:
        {
#ifdef OLE_PREVIEW_AS_EMF
            const SwNode *pNode = rItem.maFly.GetContent();
            const SwOLENode *pNd = pNode ? pNode->GetOLENode() : 0;
            OSL_ENSURE(pNd, "Impossible");
            if (!rWrt.bWrtWW8)
            {
                SwOLENode *pOleNd = const_cast<SwOLENode*>(pNd);
                OSL_ENSURE( pOleNd, " Wer hat den OleNode versteckt ?" );
                SwOLEObj&                   rSObj= pOleNd->GetOLEObj();
                uno::Reference < embed::XEmbeddedObject > rObj(  rSObj.GetOleRef() );

                comphelper::EmbeddedObjectContainer aCnt( pOleNd->GetDoc()->GetDocStorage() );

                SvStream* pGraphicStream = ::utl::UcbStreamHelper::CreateStream( aCnt.GetGraphicStream( rObj ) );
                OSL_ENSURE( pGraphicStream && !pGraphicStream->GetError(), "No graphic stream available!" );
                if ( pGraphicStream && !pGraphicStream->GetError() )
                {
                    Graphic aGr;
                    GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
                    if( rGF.ImportGraphic( aGr, aEmptyStr, *pGraphicStream, GRFILTER_FORMAT_DONTKNOW ) == GRFILTER_OK )
                    {
                        //TODO/LATER: do we really want to use GDIMetafile?!
                        GDIMetaFile aMtf;
                        aMtf = aGr.GetGDIMetaFile();
                        aMtf.WindStart();
                        aMtf.Play(Application::GetDefaultDevice(), Point(0, 0),
                            Size(2880, 2880));
                        WritePICFHeader(rStrm, rFly, 8, nWidth, nHeight,
                            pNd->GetpSwAttrSet());
                        WriteWindowMetafileBits(rStrm, aMtf);
                    }
                }
                else
                    delete pGraphicStream;
            }
            else
            {
                //Convert this ole2 preview in ww8+ to an EMF for better unicode
                //support (note that at this moment this breaks StarSymbol
                //using graphics because I need to embed starsymbol in exported
                //documents.
                WritePICFHeader(rStrm, rFly, 0x64, nWidth, nHeight,
                    pNd->GetpSwAttrSet());
                SwBasicEscherEx aInlineEscher(&rStrm, rWrt);
                aInlineEscher.WriteOLEFlyFrame(rFly.GetFrmFmt(), 0x401);
                aInlineEscher.WritePictures();
            }
#else
            // cast away const
            SwOLENode *pOleNd = const_cast<SwOLENode*>(pNd);
            OSL_ENSURE( pOleNd, " Wer hat den OleNode versteckt ?" );
            SwOLEObj&                   rSObj= pOleNd->GetOLEObj();

            // TODO/LATER: do we need to load object?
            Graphic* pGr = SdrOle2Obj::GetGraphicFromObject( pOleNd->GetDoc()->GetDocStorage(), rObj );

            //TODO/LATER: do we really want to use GDIMetafile?!
            GDIMetaFile aMtf;
            if ( pGr )
                aMtf = pGr->GetGDIMetaFile();

            Size aS(aMtf.GetPrefSize());
            aMtf.WindStart();
            aMtf.Play(Application::GetDefaultDevice(), Point(0, 0),
                Size(2880, 2880));

            WritePICFHeader(rStrm, rFly, 8, nWidth, nHeight,
                pNd->GetpSwAttrSet());
            WriteWindowMetafileBits(rStrm, aMtf);
            delete pGr;
#endif
        }
        break;
        case sw::Frame::eDrawing:
        case sw::Frame::eTxtBox:
        case sw::Frame::eFormControl:
            OSL_ENSURE(rWrt.bWrtWW8,
                "You can't try and export these in WW8 format, a filter bug");
            /*
            #i3958# We only export an empty dummy picture frame here, this is
            what word does the escher export should contain an anchored to
            character element which is drawn over this dummy and the whole
            shebang surrounded with a SHAPE field. This isn't *my* hack :-),
            its what word does.
            */
            if (rWrt.bWrtWW8)
            {
                WritePICFHeader(rStrm, rFly, 0x64, nWidth, nHeight);
                SwBasicEscherEx aInlineEscher(&rStrm, rWrt);
                aInlineEscher.WriteEmptyFlyFrame(rFly.GetFrmFmt(), 0x401);
            }
            break;
        default:
            OSL_ENSURE(!this,
           "Some inline export not implemented, remind cmc before we ship :-)");
            break;
    }

    sal_uInt32 nPos2 = rStrm.Tell();                    // store the end
    rStrm.Seek( nPos );
    SVBT32 nLen;
    UInt32ToSVBT32( nPos2 - nPos, nLen );             // calculate graphic length
    rStrm.Write( nLen, 4 );                         // patch it in the header
    rStrm.Seek( nPos2 );                            // restore Pos
}

// SwWW8WrGrf::Write() is called after the text.
// It writes out all the graphics and remembers the file locations of the graphics,
// so when writing the attributes of the items it can be patched into PicLocFc-SPRMs.
// The search in the attributes for the Magic sal_uLong and patching
// happens when writing the attributes. Class SwWW8WrGrf-Klasse provides with
// GetFPos() sequentially the positions
void SwWW8WrGrf::Write()
{
    SvStream& rStrm = *rWrt.pDataStrm;
    myiter aEnd = maDetails.end();
    for (myiter aIter = maDetails.begin(); aIter != aEnd; ++aIter)
    {
        sal_uInt32 nPos = rStrm.Tell();                 // align to 4 Bytes
        if( nPos & 0x3 )
            SwWW8Writer::FillCount( rStrm, 4 - ( nPos & 0x3 ) );

        bool bDuplicated = false;
        for (myiter aIter2 = maDetails.begin(); aIter2 != aIter; ++aIter2)
        {
            if (*aIter2 == *aIter)
            {
                aIter->mnPos = aIter2->mnPos;
                bDuplicated = true;
                break;
            }
        }

        if (!bDuplicated)
        {
            aIter->mnPos = rStrm.Tell();
            WriteGraphicNode(rStrm, *aIter);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
