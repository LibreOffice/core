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

#include <hintids.hxx>
#include <tools/helpers.hxx>
#include <tools/urlobj.hxx>
#include <svl/undo.hxx>
#include <svl/fstathelper.hxx>
#include <svtools/imap.hxx>
#include <vcl/graphicfilter.hxx>
#include <sot/storage.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/boxitem.hxx>
#include <sot/formats.hxx>
#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <frmatr.hxx>
#include <grfatr.hxx>
#include <swtypes.hxx>
#include <ndgrf.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <swbaslnk.hxx>
#include <pagefrm.hxx>
#include <editsh.hxx>
#include <pam.hxx>

#include <rtl/ustring.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <vcl/svapp.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <retrieveinputstreamconsumer.hxx>
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>

using namespace com::sun::star;

SwGrfNode::SwGrfNode(
        const SwNodeIndex & rWhere,
        const String& rGrfName, const String& rFltName,
        const Graphic* pGraphic,
        SwGrfFmtColl *pGrfColl,
        SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    maGrfObj(),
    mpReplacementGraphic(0),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    maGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;

    bGrafikArrived = sal_True;
    ReRead(rGrfName,rFltName, pGraphic, 0, sal_False);
}

SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                          const GraphicObject& rGrfObj,
                      SwGrfFmtColl *pGrfColl, SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    maGrfObj(rGrfObj),
    mpReplacementGraphic(0),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    maGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    if( rGrfObj.HasUserData() && rGrfObj.IsSwappedOut() )
        maGrfObj.SetSwapState();
    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel= bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;
    bGrafikArrived = sal_True;
}

/** Create new SW/G reader.
 *
 * Use this ctor if you want to read a linked graphic.
 *
 * @note Does not read/open the image itself!
 */
SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                      const String& rGrfName, const String& rFltName,
                      SwGrfFmtColl *pGrfColl,
                      SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    maGrfObj(),
    mpReplacementGraphic(0),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    maGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );

    Graphic aGrf; aGrf.SetDefaultType();
    maGrfObj.SetGraphic( aGrf, rGrfName );

    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;
    bGrafikArrived = sal_True;

    InsertLink( rGrfName, rFltName );
    if( IsLinkedFile() )
    {
        INetURLObject aUrl( rGrfName );
        if( INET_PROT_FILE == aUrl.GetProtocol() &&
            FStatHelper::IsDocument( aUrl.GetMainURL( INetURLObject::NO_DECODE ) ))
        {
            // file exists, so create connection without an update
            ((SwBaseLink*)&refLink)->Connect();
        }
    }
}

sal_Bool SwGrfNode::ReRead(
    const String& rGrfName, const OUString& rFltName,
    const Graphic* pGraphic, const GraphicObject* pGrfObj,
    sal_Bool bNewGrf )
{
    sal_Bool bReadGrf = sal_False, bSetTwipSize = sal_True;
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;

    OSL_ENSURE( pGraphic || pGrfObj || rGrfName.Len(),
            "GraphicNode without a name, Graphic or GraphicObject" );

    // with name
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "ReRead: I am still in SwapIn" );

        if( rGrfName.Len() )
        {
            // Note: If there is DDE in the FltName, than it is a DDE-linked graphic
            OUString sCmd( rGrfName );
            if( !rFltName.isEmpty() )
            {
                sal_uInt16 nNewType;
                if( rFltName == "DDE" )
                    nNewType = OBJECT_CLIENT_DDE;
                else
                {
                    sfx2::MakeLnkName( sCmd, 0, rGrfName, aEmptyStr, &rFltName );
                    nNewType = OBJECT_CLIENT_GRF;
                }

                if( nNewType != refLink->GetObjType() )
                {
                    refLink->Disconnect();
                    ((SwBaseLink*)&refLink)->SetObjType( nNewType );
                }
            }

            refLink->SetLinkSourceName( sCmd );
        }
        else // no name anymore, so remove link
        {
            GetDoc()->GetLinkManager().Remove( refLink );
            refLink.Clear();
        }

        if( pGraphic )
        {
            maGrfObj.SetGraphic( *pGraphic, rGrfName );
            onGraphicChanged();
            bReadGrf = sal_True;
        }
        else if( pGrfObj )
        {
            maGrfObj = *pGrfObj;
            if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
                maGrfObj.SetSwapState();
            maGrfObj.SetLink( rGrfName );
            onGraphicChanged();
            bReadGrf = sal_True;
        }
        else
        {
            // reset data of the old graphic so that the correct placeholder is
            // shown in case the new link could not be loaded
            Graphic aGrf; aGrf.SetDefaultType();
            maGrfObj.SetGraphic( aGrf, rGrfName );

            if( refLink.Is() )
            {
                if( getLayoutFrm( GetDoc()->GetCurrentLayout() ) )
                {
                    SwMsgPoolItem aMsgHint( RES_GRF_REREAD_AND_INCACHE );
                    ModifyNotification( &aMsgHint, &aMsgHint );
                }
                // #i59688# - do not load linked graphic, if it isn't a new linked graphic.
                else if ( bNewGrf )
                {
                    //TODO refLink->setInputStream(getInputStream());
                    ((SwBaseLink*)&refLink)->SwapIn();
                }
            }
            onGraphicChanged();
            bSetTwipSize = sal_False;
        }
    }
    else if( pGraphic && !rGrfName.Len() )
    {
        // Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        maGrfObj.SetGraphic( *pGraphic );
        onGraphicChanged();
        bReadGrf = sal_True;
    }
    else if( pGrfObj && !rGrfName.Len() )
    {
        // Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        maGrfObj = *pGrfObj;
        onGraphicChanged();
        if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
            maGrfObj.SetSwapState();
        bReadGrf = sal_True;
    }
    // Was the graphic already loaded?
    else if( !bNewGrf && GRAPHIC_NONE != maGrfObj.GetType() )
        return sal_True;
    else
    {
        if( HasStreamName() )
            DelStreamName();

        // create new link for the graphic object
        InsertLink( rGrfName, rFltName );

        if( GetNodes().IsDocNodes() )
        {
            if( pGraphic )
            {
                maGrfObj.SetGraphic( *pGraphic, rGrfName );
                onGraphicChanged();
                bReadGrf = sal_True;
                // create connection without update, as we have the graphic
                ((SwBaseLink*)&refLink)->Connect();
            }
            else if( pGrfObj )
            {
                maGrfObj = *pGrfObj;
                maGrfObj.SetLink( rGrfName );
                onGraphicChanged();
                bReadGrf = sal_True;
                // create connection without update, as we have the graphic
                ((SwBaseLink*)&refLink)->Connect();
            }
            else
            {
                // reset data of the old graphic so that the correct placeholder is
                // shown in case the new link could not be loaded
                Graphic aGrf; aGrf.SetDefaultType();
                maGrfObj.SetGraphic( aGrf, rGrfName );
                onGraphicChanged();
                // #i59688# - do not load linked graphic, if it isn't a new linked graphic.
                if ( bNewGrf )
                {
                    ((SwBaseLink*)&refLink)->SwapIn();
                }
            }
        }
    }

    // Bug 39281: Do not delete Size immediately - Events on ImageMaps should have
    // something to work with when swapping
    if( bSetTwipSize )
        SetTwipSize( ::GetGraphicSizeTwip( maGrfObj.GetGraphic(), 0 ) );

    // create an updates for the frames
    if( bReadGrf && bNewGrf )
    {
        SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );
        ModifyNotification( &aMsgHint, &aMsgHint );
    }

    return bReadGrf;
}

SwGrfNode::~SwGrfNode()
{
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;

    // #i73788#
    mpThreadConsumer.reset();

    SwDoc* pDoc = GetDoc();
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "DTOR: I am still in SwapIn" );
        pDoc->GetLinkManager().Remove( refLink );
        refLink->Disconnect();
    }
    else
    {
        // #i40014# - A graphic node, which is in a linked
        // section, whose link is another section in the document, doesn't
        // have to remove the stream from the storage.
        // Because it's hard to detect this case here and it would only fix
        // one problem with shared graphic files - there are also problems,
        // a certain graphic file is referenced by two independent graphic nodes,
        // brush item or drawing objects, the stream isn't no longer removed here.
        // To do this stuff correctly, a reference counting on shared streams
        // inside one document has to be implemented.
    }
    //#39289# delete frames already here since the Frms' dtor needs the graphic for its StopAnimation
    if( GetDepends() )
        DelFrms();
}

/// allow reaction on change of content of GraphicObject
void SwGrfNode::onGraphicChanged()
{
    // try to access SwFlyFrmFmt; since title/desc/name are set there, there is no
    // use to continue if it is not yet set. If not yet set, call onGraphicChanged()
    // when it is set.
    SwFlyFrmFmt* pFlyFmt = dynamic_cast< SwFlyFrmFmt* >(GetFlyFmt());

    if(pFlyFmt)
    {
        String aName;
        String aTitle;
        String aDesc;
        const SvgDataPtr& rSvgDataPtr = GetGrf().getSvgData();

        if(rSvgDataPtr.get())
        {
            const drawinglayer::primitive2d::Primitive2DSequence aSequence(rSvgDataPtr->getPrimitive2DSequence());

            if(aSequence.hasElements())
            {
                drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                drawinglayer::processor2d::ObjectInfoPrimitiveExtractor2D aProcessor(aViewInformation2D);

                aProcessor.process(aSequence);

                const drawinglayer::primitive2d::ObjectInfoPrimitive2D* pResult = aProcessor.getResult();

                if(pResult)
                {
                    aName = pResult->getName();
                    aTitle = pResult->getTitle();
                    aDesc = pResult->getDesc();
                }
            }
        }

        if(aTitle.Len())
        {
            SetTitle(aTitle);
        }

        if(aDesc.Len())
        {
            SetDescription(aDesc);
        }
    }
}

void SwGrfNode::SetGraphic(const Graphic& rGraphic, const String& rLink)
{
    maGrfObj.SetGraphic(rGraphic, rLink);
    onGraphicChanged();
}

const GraphicObject* SwGrfNode::GetReplacementGrfObj() const
{
    if(!mpReplacementGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = GetGrfObj().GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            const_cast< SwGrfNode* >(this)->mpReplacementGraphic = new GraphicObject(rSvgDataPtr->getReplacement());
        }
    }

    return mpReplacementGraphic;
}

SwCntntNode *SwGrfNode::SplitCntntNode( const SwPosition & )
{
    return this;
}

SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const OUString& rGrfName,
                                const OUString& rFltName,
                                const Graphic* pGraphic,
                                SwGrfFmtColl* pGrfColl,
                                SwAttrSet* pAutoAttr,
                                sal_Bool bDelayed )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
    SwGrfNode *pNode;
    // create object delayed, only from a SW/G-reader
    if( bDelayed )
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGrfColl, pAutoAttr );
    else
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGraphic, pGrfColl, pAutoAttr );
    return pNode;
}

SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const GraphicObject& rGrfObj,
                                SwGrfFmtColl* pGrfColl,
                                SwAttrSet* pAutoAttr )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
    return new SwGrfNode( rWhere, rGrfObj, pGrfColl, pAutoAttr );
}

Size SwGrfNode::GetTwipSize() const
{
    return nGrfSize;
}

sal_Bool SwGrfNode::ImportGraphic( SvStream& rStrm )
{
    Graphic aGraphic;
    const String aURL(maGrfObj.GetUserData());

    if(!GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, rStrm))
    {
        delete mpReplacementGraphic;
        mpReplacementGraphic = 0;

        maGrfObj.SetGraphic( aGraphic );
        maGrfObj.SetUserData( aURL );
        onGraphicChanged();
        return sal_True;
    }

    return sal_False;
}

/**
 * @return -1 if ReRead successful,
 *          1 if reading successful,
 *          0 if not loaded
 */
short SwGrfNode::SwapIn( sal_Bool bWaitForData )
{
    if( bInSwapIn ) // not recursively!
        return !maGrfObj.IsSwappedOut();

    short nRet = 0;
    bInSwapIn = sal_True;
    SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;

    if( pLink )
    {
        if( GRAPHIC_NONE == maGrfObj.GetType() ||
            GRAPHIC_DEFAULT == maGrfObj.GetType() )
        {
            // link was not loaded yet
            if( pLink->SwapIn( bWaitForData ) )
                nRet = -1;
            else if( GRAPHIC_DEFAULT == maGrfObj.GetType() )
            {
                // no default bitmap anymore, thus re-paint
                delete mpReplacementGraphic;
                mpReplacementGraphic = 0;

                maGrfObj.SetGraphic( Graphic() );
                onGraphicChanged();
                SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
                ModifyNotification( &aMsgHint, &aMsgHint );
            }
        }
        else if( maGrfObj.IsSwappedOut() ) {
            // link to download
            nRet = pLink->SwapIn( bWaitForData ) ? 1 : 0;
        }
        else
            nRet = 1;
    }
    else if( maGrfObj.IsSwappedOut() )
    {
        // graphic is in storage or in a temp file
        if( !HasStreamName() )
            nRet = (short)maGrfObj.SwapIn();
        else
        {
            try
            {
                String aStrmName, aPicStgName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
                SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
                if ( pStrm )
                {
                    if ( ImportGraphic( *pStrm ) )
                        nRet = 1;
                    delete pStrm;
                }
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwGrfNode::SwapIn(..)> - unhandled exception!" );
            }
        }

        if( 1 == nRet )
        {
            SwMsgPoolItem aMsg( RES_GRAPHIC_SWAPIN );
            ModifyNotification( &aMsg, &aMsg );
        }
    }
    else
        nRet = 1;
    OSL_ENSURE( nRet, "Cannot swap in graphic" );

    if( nRet )
    {
        if( !nGrfSize.Width() && !nGrfSize.Height() )
            SetTwipSize( ::GetGraphicSizeTwip( maGrfObj.GetGraphic(), 0 ) );
    }
    bInSwapIn = sal_False;
    return nRet;
}

short SwGrfNode::SwapOut()
{
    if( maGrfObj.GetType() != GRAPHIC_DEFAULT &&
        maGrfObj.GetType() != GRAPHIC_NONE &&
        !maGrfObj.IsSwappedOut() && !bInSwapIn )
    {
        if( !refLink.Is() )
        {
            // Swapping is only needed for embedded pictures.
            // The graphic will be written into a temp file if it is new, i.e.
            // if there is no stream name in the storage yet
            if( !HasStreamName() )
                if( !maGrfObj.SwapOut() )
                    return 0;
        }
        // written graphics and links are removed here
        return (short) maGrfObj.SwapOut( NULL );
    }
    return 1;
}

bool SwGrfNode::GetFileFilterNms( OUString* pFileNm, OUString* pFilterNm ) const
{
    bool bRet = false;
    if( refLink.Is() && refLink->GetLinkManager() )
    {
        sal_uInt16 nType = refLink->GetObjType();
        if( OBJECT_CLIENT_GRF == nType )
            bRet = refLink->GetLinkManager()->GetDisplayNames(
                    refLink, 0, pFileNm, 0, pFilterNm );
        else if( OBJECT_CLIENT_DDE == nType && pFileNm && pFilterNm )
        {
            OUString sApp;
            OUString sTopic;
            OUString sItem;
            if( refLink->GetLinkManager()->GetDisplayNames(
                    refLink, &sApp, &sTopic, &sItem ) )
            {
                *pFileNm = sApp + OUString(sfx2::cTokenSeparator)
                         + sTopic + OUString(sfx2::cTokenSeparator)
                         + sItem;
                *pFilterNm = "DDE";
                bRet = true;
            }
        }
    }
    return bRet;
}

/** Make a graphic object ready for UNDO.
 *
 * If it is already in storage, it needs to be loaded.
 */
sal_Bool SwGrfNode::SavePersistentData()
{
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "SavePersistentData: I am still in SwapIn" );
        GetDoc()->GetLinkManager().Remove( refLink );
        return sal_True;
    }

    // swap in first if already in storage
    if( HasStreamName() && !SwapIn() )
        return sal_False;

    // #i44367#
    // Do not delete graphic file in storage, because the graphic file could
    // be referenced by other graphic nodes.
    // Because it's hard to detect this case here and it would only fix
    // one problem with shared graphic files - there are also problems, if
    // a certain graphic file is referenced by two independent graphic nodes,
    // brush item or drawing objects, the stream isn't no longer removed here.
    // To do this stuff correct, a reference counting on shared streams
    // inside one document has to be implemented.
    // Important note: see also fix for #i40014#

    // swap out into temp file
    return (sal_Bool) SwapOut();
}

sal_Bool SwGrfNode::RestorePersistentData()
{
    if( refLink.Is() )
    {
        IDocumentLinksAdministration* pIDLA = getIDocumentLinksAdministration();
        refLink->SetVisible( pIDLA->IsVisibleLinks() );
        pIDLA->GetLinkManager().InsertDDELink( refLink );
        if( getIDocumentLayoutAccess()->GetCurrentLayout() )    //swmod 080218
            refLink->Update();
    }
    return sal_True;
}

void SwGrfNode::InsertLink( const OUString& rGrfName, const OUString& rFltName )
{
    refLink = new SwBaseLink( sfx2::LINKUPDATE_ONCALL, FORMAT_GDIMETAFILE, this );

    IDocumentLinksAdministration* pIDLA = getIDocumentLinksAdministration();
    if( GetNodes().IsDocNodes() )
    {
        refLink->SetVisible( pIDLA->IsVisibleLinks() );
        if( rFltName == "DDE" )
        {
            sal_Int32 nTmp = 0;
            String sApp, sTopic, sItem;
            sApp = rGrfName.getToken( 0, sfx2::cTokenSeparator, nTmp );
            sTopic = rGrfName.getToken( 0, sfx2::cTokenSeparator, nTmp );
            sItem = rGrfName.copy( nTmp );
            pIDLA->GetLinkManager().InsertDDELink( refLink,
                                            sApp, sTopic, sItem );
        }
        else
        {
            sal_Bool bSync = rFltName == "SYNCHRON";
            refLink->SetSynchron( bSync );
            refLink->SetContentType( SOT_FORMATSTR_ID_SVXB );

            pIDLA->GetLinkManager().InsertFileLink( *refLink,
                                            OBJECT_CLIENT_GRF, rGrfName,
                                (!bSync && !rFltName.isEmpty() ? &rFltName : 0) );
        }
    }
    maGrfObj.SetLink( rGrfName );
}

void SwGrfNode::ReleaseLink()
{
    if( refLink.Is() )
    {
        {
            bInSwapIn = sal_True;
            SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;
            pLink->SwapIn( sal_True, sal_True );
            bInSwapIn = sal_False;
        }
        getIDocumentLinksAdministration()->GetLinkManager().Remove( refLink );
        refLink.Clear();
        maGrfObj.SetLink();
    }
}

void SwGrfNode::SetTwipSize( const Size& rSz )
{
    nGrfSize = rSz;
    if( IsScaleImageMap() && nGrfSize.Width() && nGrfSize.Height() )
    {
        // resize Image-Map to size of the graphic
        ScaleImageMap();

        // do not re-scale Image-Map
        SetScaleImageMap( sal_False );
    }
}

void SwGrfNode::ScaleImageMap()
{
    if( !nGrfSize.Width() || !nGrfSize.Height() )
        return;

    // re-scale Image-Map
    SwFrmFmt* pFmt = GetFlyFmt();

    if( !pFmt )
        return;

    SwFmtURL aURL( pFmt->GetURL() );
    if ( !aURL.GetMap() )
        return;

    bool bScale = false;
    Fraction aScaleX( 1, 1 );
    Fraction aScaleY( 1, 1 );

    const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
    const SvxBoxItem& rBox = pFmt->GetBox();

    if( !rFrmSize.GetWidthPercent() )
    {
        SwTwips nWidth = rFrmSize.GetWidth();

        nWidth -= rBox.CalcLineSpace(BOX_LINE_LEFT) +
                  rBox.CalcLineSpace(BOX_LINE_RIGHT);

        OSL_ENSURE( nWidth>0, "Do any 0 twip wide graphics exist!?" );

        if( nGrfSize.Width() != nWidth )
        {
            aScaleX = Fraction( nGrfSize.Width(), nWidth );
            bScale = true;
        }
    }
    if( !rFrmSize.GetHeightPercent() )
    {
        SwTwips nHeight = rFrmSize.GetHeight();

        nHeight -= rBox.CalcLineSpace(BOX_LINE_TOP) +
                   rBox.CalcLineSpace(BOX_LINE_BOTTOM);

        OSL_ENSURE( nHeight>0, "Do any 0 twip high graphics exist!?" );

        if( nGrfSize.Height() != nHeight )
        {
            aScaleY = Fraction( nGrfSize.Height(), nHeight );
            bScale = true;
        }
    }

    if( bScale )
    {
        aURL.GetMap()->Scale( aScaleX, aScaleY );
        pFmt->SetFmtAttr( aURL );
    }
}

void SwGrfNode::DelStreamName()
{
    if( HasStreamName() )
    {
        // then remove graphic from storage
        uno::Reference < embed::XStorage > xDocStg = GetDoc()->GetDocStorage();
        if( xDocStg.is() )
        {
            try
            {
                String aPicStgName, aStrmName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = xDocStg;
                if ( aPicStgName.Len() )
                    refPics = xDocStg->openStorageElement( aPicStgName, embed::ElementModes::READWRITE );
                refPics->removeElement( aStrmName );
                uno::Reference < embed::XTransactedObject > xTrans( refPics, uno::UNO_QUERY );
                if ( xTrans.is() )
                    xTrans->commit();
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwGrfNode::DelStreamName()> - unhandled exception!" );
            }
        }

        maGrfObj.SetUserData();
    }
}

/** helper method to get a substorage of the document storage for readonly access.

    OD, MAV 2005-08-17 #i53025#
    A substorage with the specified name will be opened readonly. If the provided
    name is empty the root storage will be returned.
*/
uno::Reference< embed::XStorage > SwGrfNode::_GetDocSubstorageOrRoot( const String& aStgName ) const
{
    uno::Reference < embed::XStorage > refStor =
        const_cast<SwGrfNode*>(this)->GetDoc()->GetDocStorage();
    OSL_ENSURE( refStor.is(), "Kein Storage am Doc" );

    if ( aStgName.Len() )
    {
        if( refStor.is() )
            return refStor->openStorageElement( aStgName, embed::ElementModes::READ );
    }

    return refStor;
}

/** helper method to determine stream for the embedded graphic.

    OD 2005-05-04 #i48434#
    Important note: caller of this method has to handle the thrown exceptions
    OD, MAV 2005-08-17 #i53025#
    Storage, which should contain the stream of the embedded graphic, is
    provided via parameter. Otherwise the returned stream will be closed
    after the method returns, because its parent stream is closed and deleted.
    Proposed name of embedded graphic stream is also provided by parameter.
*/
SvStream* SwGrfNode::_GetStreamForEmbedGrf(
            const uno::Reference< embed::XStorage >& _refPics,
            String& _aStrmName ) const
{
    SvStream* pStrm( 0L );

    if( _refPics.is() && _aStrmName.Len() )
    {
        // If stream doesn't exist in the storage, try access the graphic file by
        // re-generating its name.
        // A save action can have changed the filename of the embedded graphic,
        // because a changed unique ID of the graphic is calculated.
        // --> recursive calls of <GetUniqueID()> have to be avoided.
        // Thus, use local static boolean to assure this.
        if ( !_refPics->hasByName( _aStrmName ) ||
               !_refPics->isStreamElement( _aStrmName ) )
        {
            xub_StrLen nExtPos = _aStrmName.Search( '.' );
            String aExtStr = _aStrmName.Copy( nExtPos );
            if ( GetGrfObj().GetType() != GRAPHIC_NONE )
            {
                _aStrmName = OStringToOUString(GetGrfObj().GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US);
                _aStrmName += aExtStr;
            }
        }

        // assure that graphic file exist in the storage.
        if ( _refPics->hasByName( _aStrmName ) &&
             _refPics->isStreamElement( _aStrmName ) )
        {
            uno::Reference < io::XStream > refStrm = _refPics->openStreamElement( _aStrmName, embed::ElementModes::READ );
            pStrm = utl::UcbStreamHelper::CreateStream( refStrm );
        }
        else
        {
            OSL_FAIL( "<SwGrfNode::_GetStreamForEmbedGrf(..)> - embedded graphic file not found!" );
        }
    }

    return pStrm;
}

void SwGrfNode::_GetStreamStorageNames( String& rStrmName,
                                        String& rStorName ) const
{
    rStorName.Erase();
    rStrmName.Erase();

    String aUserData( maGrfObj.GetUserData() );
    if( !aUserData.Len() )
        return;

    if (aNewStrmName.Len()>0) {
        aUserData=aNewStrmName;
    }

    String aProt( "vnd.sun.star.Package:" );
    if( 0 == aUserData.CompareTo( aProt, aProt.Len() ) )
    {
        // 6.0 (XML) Package
        xub_StrLen nPos = aUserData.Search( '/' );
        if( STRING_NOTFOUND == nPos )
        {
            rStrmName = aUserData.Copy( aProt.Len() );
        }
        else
        {
            xub_StrLen nPathStart = aProt.Len();
            if( 0 == aUserData.CompareToAscii( "./", 2 ) )
                nPathStart += 2;
            rStorName = aUserData.Copy( nPathStart, nPos-nPathStart );
            rStrmName = aUserData.Copy( nPos+1 );
        }
    }
    else
    {
        OSL_FAIL( "<SwGrfNode::_GetStreamStorageNames(..)> - unknown graphic URL type. Code for handling 3.1 - 5.2 storages has been deleted by issue i53025." );
    }
    OSL_ENSURE( STRING_NOTFOUND == rStrmName.Search( '/' ),
            "invalid graphic stream name" );
}

SwCntntNode* SwGrfNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // copy formats into the other document
    SwGrfFmtColl* pColl = pDoc->CopyGrfColl( *GetGrfColl() );

    Graphic aTmpGrf;
    SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;
    if( !pLink && HasStreamName() )
    {
        try
        {
            String aStrmName, aPicStgName;
            _GetStreamStorageNames( aStrmName, aPicStgName );
            uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
            SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
            if ( pStrm )
            {
                const OUString aURL(maGrfObj.GetUserData());
                GraphicFilter::GetGraphicFilter().ImportGraphic(aTmpGrf, aURL, *pStrm);
                delete pStrm;
            }
        }
        catch (const uno::Exception& e)
        {
            // #i48434#
            SAL_WARN("sw.core", "<SwGrfNode::MakeCopy(..)> - unhandled exception!" << e.Message);
        }
    }
    else
    {
        if( maGrfObj.IsSwappedOut() )
            const_cast<SwGrfNode*>(this)->SwapIn();
        aTmpGrf = maGrfObj.GetGraphic();
    }

    const sfx2::LinkManager& rMgr = getIDocumentLinksAdministration()->GetLinkManager();
    OUString sFile, sFilter;
    if( IsLinkedFile() )
        rMgr.GetDisplayNames( refLink, 0, &sFile, 0, &sFilter );
    else if( IsLinkedDDE() )
    {
        OUString sTmp1, sTmp2;
        rMgr.GetDisplayNames( refLink, &sTmp1, &sTmp2, &sFilter );
        sfx2::MakeLnkName( sFile, &sTmp1, sTmp2, sFilter );
        sFilter = "DDE";
    }

    SwGrfNode* pGrfNd = pDoc->GetNodes().MakeGrfNode( rIdx, sFile, sFilter,
                                                    &aTmpGrf, pColl,
                                            (SwAttrSet*)GetpSwAttrSet() );
    pGrfNd->SetTitle( GetTitle() );
    pGrfNd->SetDescription( GetDescription() );
    pGrfNd->SetContour( HasContour(), HasAutomaticContour() );
    return pGrfNd;
}

IMPL_LINK( SwGrfNode, SwapGraphic, GraphicObject*, pGrfObj )
{
    SvStream* pRet;

    // Keep graphic while in swap in. That's at least important
    // when breaking links, because in this situation a reschedule call and
    // a DataChanged call lead to a paint of the graphic.
    if( pGrfObj->IsInSwapOut() && (IsSelected() || bInSwapIn) )
        pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
    else if( refLink.Is() )
    {
        if( pGrfObj->IsInSwapIn() )
        {
            // then make it by your self
            if( !bInSwapIn )
            {
                bool bIsModifyLocked = IsModifyLocked();
                LockModify();
                SwapIn( sal_False );
                if( !bIsModifyLocked )
                    UnlockModify();
            }
            pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
        }
        else
            pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
    }
    else
    {
        pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;

        if( HasStreamName() )
        {
            try
            {
                String aStrmName, aPicStgName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
                SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
                if ( pStrm )
                {
                    if( pGrfObj->IsInSwapOut() )
                    {
                        pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
                    }
                    else
                    {
                        ImportGraphic( *pStrm );
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    delete pStrm;
                }
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwapGraphic> - unhandled exception!" );
            }
        }
    }

    return (sal_IntPtr)pRet;
}

/// delete all QuickDraw-Bitmaps in the specified document
void DelAllGrfCacheEntries( SwDoc* pDoc )
{
    if( pDoc )
    {
        // delete all Graphic-Links with this name from cache
        const sfx2::LinkManager& rLnkMgr = pDoc->GetLinkManager();
        const ::sfx2::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
        SwGrfNode* pGrfNd;
        OUString sFileNm;
        for( sal_uInt16 n = rLnks.size(); n; )
        {
            ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                rLnkMgr.GetDisplayNames( pLnk, 0, &sFileNm ) &&
                pLnk->ISA( SwBaseLink ) && 0 != ( pGrfNd =
                ((SwBaseLink*)pLnk)->GetCntntNode()->GetGrfNode()) )
            {
                pGrfNd->ReleaseGraphicFromCache();
            }
        }
    }
}

/// returns the Graphic-Attr-Structure filled with our graphic attributes
GraphicAttr& SwGrfNode::GetGraphicAttr( GraphicAttr& rGA,
                                        const SwFrm* pFrm ) const
{
    const SwAttrSet& rSet = GetSwAttrSet();

    rGA.SetDrawMode( (GraphicDrawMode)rSet.GetDrawModeGrf().GetValue() );

    const SwMirrorGrf & rMirror = rSet.GetMirrorGrf();
    sal_uLong nMirror = BMP_MIRROR_NONE;
    if( rMirror.IsGrfToggle() && pFrm && !pFrm->FindPageFrm()->OnRightPage() )
    {
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRAPH_DONT:     nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRAPH_VERT:     nMirror = BMP_MIRROR_NONE; break;
        case RES_MIRROR_GRAPH_HOR:  nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        default:                    nMirror = BMP_MIRROR_VERT; break;
        }
    }
    else
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRAPH_BOTH:     nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        case RES_MIRROR_GRAPH_VERT: nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRAPH_HOR:  nMirror = BMP_MIRROR_VERT; break;
        }

    rGA.SetMirrorFlags( nMirror );

    const SwCropGrf& rCrop = rSet.GetCropGrf();
    rGA.SetCrop( TWIP_TO_MM100( rCrop.GetLeft() ),
                 TWIP_TO_MM100( rCrop.GetTop() ),
                 TWIP_TO_MM100( rCrop.GetRight() ),
                 TWIP_TO_MM100( rCrop.GetBottom() ));

    const SwRotationGrf& rRotation = rSet.GetRotationGrf();
    rGA.SetRotation( rRotation.GetValue() );

    rGA.SetLuminance( rSet.GetLuminanceGrf().GetValue() );
    rGA.SetContrast( rSet.GetContrastGrf().GetValue() );
    rGA.SetChannelR( rSet.GetChannelRGrf().GetValue() );
    rGA.SetChannelG( rSet.GetChannelGGrf().GetValue() );
    rGA.SetChannelB( rSet.GetChannelBGrf().GetValue() );
    rGA.SetGamma( rSet.GetGammaGrf().GetValue() );
    rGA.SetInvert( rSet.GetInvertGrf().GetValue() );

    const sal_uInt16 nTrans = rSet.GetTransparencyGrf().GetValue();
    rGA.SetTransparency( (sal_uInt8) FRound(
                                std::min( nTrans, (sal_uInt16) 100 )  * 2.55 ) );

    return rGA;
}

sal_Bool SwGrfNode::IsTransparent() const
{
    sal_Bool bRet = maGrfObj.IsTransparent();
    if( !bRet ) // ask the attribut
        bRet = 0 != GetSwAttrSet().GetTransparencyGrf().GetValue();

    return bRet;
}

sal_Bool SwGrfNode::IsSelected() const
{
    sal_Bool bRet = sal_False;
    const SwEditShell* pESh = GetDoc()->GetEditShell();
    if( pESh )
    {
        const SwNode* pN = this;
        const ViewShell* pV = pESh;
        do {
            if( pV->ISA( SwEditShell ) && pN == &((SwCrsrShell*)pV)
                                ->GetCrsr()->GetPoint()->nNode.GetNode() )
            {
                bRet = sal_True;
                break;
            }
        }
        while( pESh != ( pV = (ViewShell*)pV->GetNext() ));
    }
    return bRet;
}

// #i73788#
boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > SwGrfNode::GetThreadConsumer()
{
    return mpThreadConsumer;
}

void SwGrfNode::TriggerAsyncRetrieveInputStream()
{
    if ( !IsLinkedFile() )
    {
        OSL_FAIL( "<SwGrfNode::TriggerAsyncLoad()> - Method is misused. Method call is only valid for graphic nodes, which refer a linked graphic file" );
        return;
    }

    if ( mpThreadConsumer.get() == 0 )
    {
        mpThreadConsumer.reset( new SwAsyncRetrieveInputStreamThreadConsumer( *this ) );

        OUString sGrfNm;
        refLink->GetLinkManager()->GetDisplayNames( refLink, 0, &sGrfNm, 0, 0 );

        mpThreadConsumer->CreateThread( sGrfNm );
    }
}

bool SwGrfNode::IsLinkedInputStreamReady() const
{
    return mbLinkedInputStreamReady;
}

void SwGrfNode::ApplyInputStream(
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
    const sal_Bool bIsStreamReadOnly )
{
    if ( IsLinkedFile() )
    {
        if ( xInputStream.is() )
        {
            mxInputStream = xInputStream;
            mbIsStreamReadOnly = bIsStreamReadOnly;
            mbLinkedInputStreamReady = true;
            SwMsgPoolItem aMsgHint( RES_LINKED_GRAPHIC_STREAM_ARRIVED );
            ModifyNotification( &aMsgHint, &aMsgHint );
        }
    }
}

void SwGrfNode::UpdateLinkWithInputStream()
{
    // do not work on link, if a <SwapIn> has been triggered.
    if ( !bInSwapIn && IsLinkedFile() )
    {
        GetLink()->setStreamToLoadFrom( mxInputStream, mbIsStreamReadOnly );
        GetLink()->Update();
        SwMsgPoolItem aMsgHint( RES_GRAPHIC_ARRIVED );
        ModifyNotification( &aMsgHint, &aMsgHint );

        // #i88291#
        mxInputStream.clear();
        GetLink()->clearStreamToLoadFrom();
        mbLinkedInputStreamReady = false;
        mpThreadConsumer.reset();
    }
}

// #i90395#
bool SwGrfNode::IsAsyncRetrieveInputStreamPossible() const
{
    bool bRet = false;

    if ( IsLinkedFile() )
    {
        OUString sGrfNm;
        refLink->GetLinkManager()->GetDisplayNames( refLink, 0, &sGrfNm, 0, 0 );
        if ( !sGrfNm.startsWith( "vnd.sun.star.pkg:" ) )
        {
            bRet = true;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
