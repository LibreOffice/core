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
#include <mdiexp.hxx>
#include <tools/helpers.hxx>
#include <tools/urlobj.hxx>
#include <tools/fract.hxx>
#include <svl/undo.hxx>
#include <svl/fstathelper.hxx>
#include <vcl/imap.hxx>
#include <vcl/graphicfilter.hxx>
#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/boxitem.hxx>
#include <sot/formats.hxx>
#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <o3tl/deleter.hxx>
#include <vcl/svapp.hxx>
#include <retrieveinputstreamconsumer.hxx>
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>

using namespace com::sun::star;

SwGrfNode::SwGrfNode(
        const SwNodeIndex & rWhere,
        const OUString& rGrfName,
        const OUString& rFltName,
        const Graphic* pGraphic,
        SwGrfFormatColl *pGrfColl,
        SwAttrSet const * pAutoAttr ) :
    SwNoTextNode( rWhere, SwNodeType::Grf, pGrfColl, pAutoAttr ),
    maGrfObj(),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( false )
{
    bInSwapIn = bChgTwipSize =
        bFrameInPaint = bScaleImageMap = false;

    ReRead(rGrfName, rFltName, pGraphic, false);
}

SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                      const GraphicObject& rGrfObj,
                      SwGrfFormatColl *pGrfColl,
                      SwAttrSet const * pAutoAttr ) :
    SwNoTextNode( rWhere, SwNodeType::Grf, pGrfColl, pAutoAttr ),
    maGrfObj(rGrfObj),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( false )
{
    bInSwapIn = bChgTwipSize  =
        bFrameInPaint = bScaleImageMap = false;
}

/** Create new SW/G reader.
 *
 * Use this ctor if you want to read a linked graphic.
 *
 * @note Does not read/open the image itself!
 */
SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                      const OUString& rGrfName,
                      const OUString& rFltName,
                      SwGrfFormatColl *pGrfColl,
                      SwAttrSet const * pAutoAttr ) :
    SwNoTextNode( rWhere, SwNodeType::Grf, pGrfColl, pAutoAttr ),
    maGrfObj(),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( false )
{
    Graphic aGrf; aGrf.SetDefaultType();
    maGrfObj.SetGraphic( aGrf, rGrfName );

    bInSwapIn = bChgTwipSize =
        bFrameInPaint = bScaleImageMap = false;

    InsertLink( rGrfName, rFltName );
    if( IsLinkedFile() )
    {
        INetURLObject aUrl( rGrfName );
        if( INetProtocol::File == aUrl.GetProtocol() &&
            FStatHelper::IsDocument( aUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ))
        {
            // file exists, so create connection without an update
            static_cast<SwBaseLink*>( refLink.get() )->Connect();
        }
    }
}

bool SwGrfNode::ReRead(
    const OUString& rGrfName, const OUString& rFltName,
    const Graphic* pGraphic,
    bool bNewGrf )
{
    bool bReadGrf = false;
    bool bSetTwipSize = true;
    mpReplacementGraphic.reset();

    OSL_ENSURE( pGraphic || !rGrfName.isEmpty(),
            "GraphicNode without a name, Graphic or GraphicObject" );

    OUString sURLLink;
    if (pGraphic)
    {
        Graphic aGraphic(*pGraphic);

        sURLLink = aGraphic.getOriginURL();
        if (sURLLink.isEmpty() && !rGrfName.isEmpty())
        {
            sURLLink = rGrfName;
            aGraphic.setOriginURL(sURLLink);
        }
    }
    else
    {
        sURLLink = rGrfName;
    }

    // with name
    if( refLink.is() )
    {
        OSL_ENSURE( !bInSwapIn, "ReRead: I am still in SwapIn" );

        if( !sURLLink.isEmpty() )
        {
            // Note: If there is DDE in the FltName, than it is a DDE-linked graphic
            OUString sCmd( sURLLink );
            if( !rFltName.isEmpty() )
            {
                sal_uInt16 nNewType;
                if( rFltName == "DDE" )
                    nNewType = OBJECT_CLIENT_DDE;
                else
                {
                    sfx2::MakeLnkName( sCmd, nullptr, sURLLink, OUString(), &rFltName );
                    nNewType = OBJECT_CLIENT_GRF;
                }

                if( nNewType != refLink->GetObjType() )
                {
                    refLink->Disconnect();
                    static_cast<SwBaseLink*>( refLink.get() )->SetObjType( nNewType );
                }
            }

            refLink->SetLinkSourceName( sCmd );
        }
        else // no name anymore, so remove link
        {
            GetDoc()->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
            refLink.clear();
        }

        if( pGraphic )
        {
            maGrfObj.SetGraphic( *pGraphic, sURLLink );
            onGraphicChanged();
            bReadGrf = true;
        }
        else
        {
            // reset data of the old graphic so that the correct placeholder is
            // shown in case the new link could not be loaded
            Graphic aGrf; aGrf.SetDefaultType();
            maGrfObj.SetGraphic( aGrf, sURLLink );

            if( refLink.is() )
            {
                if( getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) )
                {
                    SwMsgPoolItem aMsgHint( RES_GRF_REREAD_AND_INCACHE );
                    ModifyNotification( &aMsgHint, &aMsgHint );
                }
                else if ( bNewGrf )
                {
                    //TODO refLink->setInputStream(getInputStream());
                    static_cast<SwBaseLink*>( refLink.get() )->SwapIn();
                }
            }
            onGraphicChanged();
            bSetTwipSize = false;
        }
    }
    else if( pGraphic && sURLLink.isEmpty() )
    {
        maGrfObj.SetGraphic( *pGraphic );
        onGraphicChanged();
        bReadGrf = true;
    }
    // Was the graphic already loaded?
    else if( !bNewGrf && GraphicType::NONE != maGrfObj.GetType() )
        return true;
    else
    {
        // create new link for the graphic object
        InsertLink( sURLLink, rFltName );

        if( GetNodes().IsDocNodes() )
        {
            if( pGraphic )
            {
                maGrfObj.SetGraphic( *pGraphic, sURLLink );
                onGraphicChanged();
                bReadGrf = true;
                // create connection without update, as we have the graphic
                static_cast<SwBaseLink*>( refLink.get() )->Connect();
            }
            else
            {
                Graphic aGrf;
                aGrf.SetDefaultType();
                maGrfObj.SetGraphic( aGrf, sURLLink );
                onGraphicChanged();
                if ( bNewGrf )
                {
                    static_cast<SwBaseLink*>( refLink.get() )->SwapIn();
                }
            }
        }
    }

    // Bug 39281: Do not delete Size immediately - Events on ImageMaps should have
    // something to work with when swapping
    if( bSetTwipSize )
        SetTwipSize( ::GetGraphicSizeTwip( maGrfObj.GetGraphic(), nullptr ) );

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
    mpReplacementGraphic.reset();

    // #i73788#
    mpThreadConsumer.reset();

    SwDoc* pDoc = GetDoc();
    if( refLink.is() )
    {
        OSL_ENSURE( !bInSwapIn, "DTOR: I am still in SwapIn" );
        pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
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
    //#39289# delete frames already here since the Frames' dtor needs the graphic for its StopAnimation
    if( HasWriterListeners() )
        DelFrames(nullptr);
}

/// allow reaction on change of content of GraphicObject
void SwGrfNode::onGraphicChanged()
{
    // try to access SwFlyFrameFormat; since title/desc/name are set there, there is no
    // use to continue if it is not yet set. If not yet set, call onGraphicChanged()
    // when it is set.
    SwFlyFrameFormat* pFlyFormat = dynamic_cast< SwFlyFrameFormat* >(GetFlyFormat());

    if(pFlyFormat)
    {
        OUString aName;
        OUString aTitle;
        OUString aDesc;
        const VectorGraphicDataPtr& rVectorGraphicDataPtr = GetGrf().getVectorGraphicData();

        if(rVectorGraphicDataPtr.get())
        {
            const drawinglayer::primitive2d::Primitive2DContainer aSequence(rVectorGraphicDataPtr->getPrimitive2DSequence());

            if(!aSequence.empty())
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

        if(!aTitle.isEmpty())
        {
            SetTitle(aTitle);
        }
        else if (!aName.isEmpty())
        {
            SetTitle(aName);
        }

        if(!aDesc.isEmpty())
        {
            SetDescription(aDesc);
        }
    }
}

void SwGrfNode::SetGraphic(const Graphic& rGraphic)
{
    maGrfObj.SetGraphic(rGraphic, OUString());
    onGraphicChanged();
}

const Graphic& SwGrfNode::GetGrf(bool bWait) const
{
    const_cast<SwGrfNode*>(this)->SwapIn(bWait);
    return maGrfObj.GetGraphic();
}

const GraphicObject& SwGrfNode::GetGrfObj(bool bWait) const
{
    const_cast<SwGrfNode*>(this)->SwapIn(bWait);
    return maGrfObj;
}

const GraphicObject* SwGrfNode::GetReplacementGrfObj() const
{
    if(!mpReplacementGraphic)
    {
        const VectorGraphicDataPtr& rVectorGraphicDataPtr = GetGrfObj().GetGraphic().getVectorGraphicData();

        if(rVectorGraphicDataPtr.get())
        {
            const_cast< SwGrfNode* >(this)->mpReplacementGraphic.reset( new GraphicObject(rVectorGraphicDataPtr->getReplacement()) );
        }
        else if (GetGrfObj().GetGraphic().hasPdfData() ||
                 GetGrfObj().GetGraphic().GetType() == GraphicType::GdiMetafile)
        {
            // Replacement graphic for PDF and metafiles is just the bitmap.
            const_cast<SwGrfNode*>(this)->mpReplacementGraphic.reset( new GraphicObject(GetGrfObj().GetGraphic().GetBitmapEx()) );
        }
    }

    return mpReplacementGraphic.get();
}

SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const OUString& rGrfName,
                                const OUString& rFltName,
                                const Graphic* pGraphic,
                                SwGrfFormatColl* pGrfColl,
                                SwAttrSet const * pAutoAttr )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer is 0." );
    SwGrfNode *pNode;
    // create object delayed, only from a SW/G-reader
    if( !pGraphic )
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGrfColl, pAutoAttr );
    else
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGraphic, pGrfColl, pAutoAttr );
    return pNode;
}

SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const GraphicObject& rGrfObj,
                                SwGrfFormatColl* pGrfColl )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer is 0." );
    return new SwGrfNode( rWhere, rGrfObj, pGrfColl, nullptr );
}

Size SwGrfNode::GetTwipSize() const
{
    if( !nGrfSize.Width() && !nGrfSize.Height() )
    {
        const_cast<SwGrfNode*>(this)->SwapIn();
    }
    return nGrfSize;
}

/**
 * @return true if ReRead or reading successful,
 *         false if not loaded
 */
bool SwGrfNode::SwapIn(bool bWaitForData)
{
    if(bInSwapIn) // not recursively!
        return true;

    bool bRet = false;
    bInSwapIn = true;
    SwBaseLink* pLink = static_cast<SwBaseLink*>( refLink.get() );

    if( pLink )
    {
        if( GraphicType::NONE == maGrfObj.GetType() ||
            GraphicType::Default == maGrfObj.GetType() )
        {
            // link was not loaded yet
            if( pLink->SwapIn( bWaitForData ) )
            {
                bRet = true;
            }
            else if( GraphicType::Default == maGrfObj.GetType() )
            {
                // no default bitmap anymore, thus re-paint
                mpReplacementGraphic.reset();

                maGrfObj.SetGraphic( Graphic() );
                onGraphicChanged();
                SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
                ModifyNotification( &aMsgHint, &aMsgHint );
            }
        }
        else
        {
            bRet = true;
        }
    }
    else
        bRet = true;

    if (bRet)
    {
        if( !nGrfSize.Width() && !nGrfSize.Height() )
            SetTwipSize( ::GetGraphicSizeTwip( maGrfObj.GetGraphic(), nullptr ) );
    }
    bInSwapIn = false;
    return bRet;
}

bool SwGrfNode::GetFileFilterNms( OUString* pFileNm, OUString* pFilterNm ) const
{
    bool bRet = false;
    if( refLink.is() && refLink->GetLinkManager() )
    {
        sal_uInt16 nType = refLink->GetObjType();
        if( OBJECT_CLIENT_GRF == nType )
            bRet = sfx2::LinkManager::GetDisplayNames(
                    refLink.get(), nullptr, pFileNm, nullptr, pFilterNm );
        else if( OBJECT_CLIENT_DDE == nType && pFileNm && pFilterNm )
        {
            OUString sApp;
            OUString sTopic;
            OUString sItem;
            if( sfx2::LinkManager::GetDisplayNames(
                    refLink.get(), &sApp, &sTopic, &sItem ) )
            {
                *pFileNm = sApp + OUStringLiteral1(sfx2::cTokenSeparator)
                         + sTopic + OUStringLiteral1(sfx2::cTokenSeparator)
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
bool SwGrfNode::SavePersistentData()
{
    if( refLink.is() )
    {
        OSL_ENSURE( !bInSwapIn, "SavePersistentData: I am still in SwapIn" );
        GetDoc()->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
        return true;
    }

    // swap in first if in storage
    if( HasEmbeddedStreamName() && !SwapIn() )
        return false;

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
    return true;
}

bool SwGrfNode::RestorePersistentData()
{
    if( refLink.is() )
    {
        IDocumentLinksAdministration& rIDLA = getIDocumentLinksAdministration();
        refLink->SetVisible( rIDLA.IsVisibleLinks() );
        rIDLA.GetLinkManager().InsertDDELink( refLink.get() );
        if( getIDocumentLayoutAccess().GetCurrentLayout() )
            refLink->Update();
    }
    return true;
}

void SwGrfNode::InsertLink( const OUString& rGrfName, const OUString& rFltName )
{
    refLink = new SwBaseLink( SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::GDIMETAFILE, this );

    IDocumentLinksAdministration& rIDLA = getIDocumentLinksAdministration();
    if( GetNodes().IsDocNodes() )
    {
        refLink->SetVisible( rIDLA.IsVisibleLinks() );
        if( rFltName == "DDE" )
        {
            sal_Int32 nTmp = 0;
            const OUString sApp{ rGrfName.getToken( 0, sfx2::cTokenSeparator, nTmp ) };
            const OUString sTopic{ rGrfName.getToken( 0, sfx2::cTokenSeparator, nTmp ) };
            const OUString sItem{ rGrfName.copy( nTmp ) };
            rIDLA.GetLinkManager().InsertDDELink( refLink.get(), sApp, sTopic, sItem );
        }
        else
        {
            const bool bSync = rFltName == "SYNCHRON";
            refLink->SetSynchron( bSync );
            refLink->SetContentType( SotClipboardFormatId::SVXB );

            rIDLA.GetLinkManager().InsertFileLink( *refLink,
                                            OBJECT_CLIENT_GRF, rGrfName,
                                (!bSync && !rFltName.isEmpty() ? &rFltName : nullptr) );
        }
    }
}

void SwGrfNode::ReleaseLink()
{
    if( refLink.is() )
    {
        Graphic aLocalGraphic(maGrfObj.GetGraphic());
        const bool bHasOriginalData(aLocalGraphic.IsGfxLink());

        {
            bInSwapIn = true;
            SwBaseLink* pLink = static_cast<SwBaseLink*>( refLink.get() );
            pLink->SwapIn( true, true );
            bInSwapIn = false;
        }

        getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
        refLink.clear();
        aLocalGraphic.setOriginURL("");

        // #i15508# added extra processing after getting rid of the link. Use whatever is
        // known from the formerly linked graphic to get to a state as close to a directly
        // unlinked inserted graphic as possible. Goal is to have a valid GfxLink at the
        // ImplGraphic (see there) that holds temporary data to the original data and type
        // information about the original data. Only when this is given will
        // SvXMLGraphicHelper::ImplInsertGraphicURL which is used at export use that type
        // and use the original graphic at export for the ODF, without evtl. recoding
        // of the bitmap graphic data to something without loss (e.g. PNG) but bigger
        if(bHasOriginalData)
        {
            // #i15508# if we have the original data at the Graphic, let it survive
            // by using that Graphic again, this time at a GraphicObject without link.
            // This happens e.g. when inserting a linked graphic and breaking the link
            maGrfObj.SetGraphic(aLocalGraphic);
        }
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
        SetScaleImageMap( false );
    }
}

void SwGrfNode::ScaleImageMap()
{
    if( !nGrfSize.Width() || !nGrfSize.Height() )
        return;

    // re-scale Image-Map
    SwFrameFormat* pFormat = GetFlyFormat();

    if( !pFormat )
        return;

    SwFormatURL aURL( pFormat->GetURL() );
    if ( !aURL.GetMap() )
        return;

    bool bScale = false;
    Fraction aScaleX( 1, 1 );
    Fraction aScaleY( 1, 1 );

    const SwFormatFrameSize& rFrameSize = pFormat->GetFrameSize();
    const SvxBoxItem& rBox = pFormat->GetBox();

    if( !rFrameSize.GetWidthPercent() )
    {
        SwTwips nWidth = rFrameSize.GetWidth();

        nWidth -= rBox.CalcLineSpace(SvxBoxItemLine::LEFT) +
                  rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);

        OSL_ENSURE( nWidth>0, "Do any 0 twip wide graphics exist!?" );

        if( nGrfSize.Width() != nWidth )
        {
            aScaleX = Fraction( nGrfSize.Width(), nWidth );
            bScale = true;
        }
    }
    if( !rFrameSize.GetHeightPercent() )
    {
        SwTwips nHeight = rFrameSize.GetHeight();

        nHeight -= rBox.CalcLineSpace(SvxBoxItemLine::TOP) +
                   rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM);

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
        pFormat->SetFormatAttr( aURL );
    }
}

SwContentNode* SwGrfNode::MakeCopy(SwDoc* pDoc, const SwNodeIndex& rIdx, bool) const
{
    // copy formats into the other document
    SwGrfFormatColl* pColl = pDoc->CopyGrfColl( *GetGrfColl() );

    Graphic aTmpGrf = GetGrf();

    OUString sFile, sFilter;
    if( IsLinkedFile() )
        sfx2::LinkManager::GetDisplayNames( refLink.get(), nullptr, &sFile, nullptr, &sFilter );
    else if( IsLinkedDDE() )
    {
        OUString sTmp1, sTmp2;
        sfx2::LinkManager::GetDisplayNames( refLink.get(), &sTmp1, &sTmp2, &sFilter );
        sfx2::MakeLnkName( sFile, &sTmp1, sTmp2, sFilter );
        sFilter = "DDE";
    }

    SwGrfNode* pGrfNd = SwNodes::MakeGrfNode( rIdx, sFile, sFilter,
                                              &aTmpGrf, pColl,
                                              GetpSwAttrSet() );
    pGrfNd->SetTitle( GetTitle() );
    pGrfNd->SetDescription( GetDescription() );
    pGrfNd->SetContour( HasContour(), HasAutomaticContour() );
    return pGrfNd;
}

/// returns the Graphic-Attr-Structure filled with our graphic attributes
GraphicAttr& SwGrfNode::GetGraphicAttr( GraphicAttr& rGA,
                                        const SwFrame* pFrame ) const
{
    const SwAttrSet& rSet = GetSwAttrSet();

    rGA.SetDrawMode( rSet.GetDrawModeGrf().GetValue() );

    const SwMirrorGrf & rMirror = rSet.GetMirrorGrf();
    BmpMirrorFlags nMirror = BmpMirrorFlags::NONE;
    if( rMirror.IsGrfToggle() && pFrame && !pFrame->FindPageFrame()->OnRightPage() )
    {
        switch( rMirror.GetValue() )
        {
        case MirrorGraph::Dont:
            nMirror = BmpMirrorFlags::Horizontal;
            break;
        case MirrorGraph::Vertical:
            nMirror = BmpMirrorFlags::NONE;
            break;
        case MirrorGraph::Horizontal:
            nMirror = BmpMirrorFlags::Horizontal|BmpMirrorFlags::Vertical;
            break;
        default:
            nMirror = BmpMirrorFlags::Vertical;
            break;
        }
    }
    else
        switch( rMirror.GetValue() )
        {
        case MirrorGraph::Both:
            nMirror = BmpMirrorFlags::Horizontal|BmpMirrorFlags::Vertical;
            break;
        case MirrorGraph::Vertical:
            nMirror = BmpMirrorFlags::Horizontal;
            break;
        case MirrorGraph::Horizontal:
            nMirror = BmpMirrorFlags::Vertical;
            break;
        default: break;
        }

    rGA.SetMirrorFlags( nMirror );

    const SwCropGrf& rCrop = rSet.GetCropGrf();
    rGA.SetCrop( convertTwipToMm100( rCrop.GetLeft() ),
                 convertTwipToMm100( rCrop.GetTop() ),
                 convertTwipToMm100( rCrop.GetRight() ),
                 convertTwipToMm100( rCrop.GetBottom() ));

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
    rGA.SetTransparency( static_cast<sal_uInt8>(FRound(
                                std::min( nTrans, sal_uInt16(100) )  * 2.55 )) );

    return rGA;
}

bool SwGrfNode::IsTransparent() const
{
    return maGrfObj.IsTransparent() ||
        GetSwAttrSet().GetTransparencyGrf().GetValue() != 0;
}

void SwGrfNode::TriggerAsyncRetrieveInputStream()
{
    if ( !IsLinkedFile() )
    {
        OSL_FAIL( "<SwGrfNode::TriggerAsyncLoad()> - Method is misused. Method call is only valid for graphic nodes, which refer a linked graphic file" );
        return;
    }

    if (mpThreadConsumer == nullptr)
    {
        mpThreadConsumer.reset(new SwAsyncRetrieveInputStreamThreadConsumer(*this), o3tl::default_delete<SwAsyncRetrieveInputStreamThreadConsumer>());

        OUString sGrfNm;
        sfx2::LinkManager::GetDisplayNames( refLink.get(), nullptr, &sGrfNm );
        OUString sReferer;
        SfxObjectShell * sh = GetDoc()->GetPersist();
        if (sh != nullptr && sh->HasName())
        {
            sReferer = sh->GetMedium()->GetName();
        }
        mpThreadConsumer->CreateThread( sGrfNm, sReferer );
    }
}


void SwGrfNode::ApplyInputStream(
    const css::uno::Reference<css::io::XInputStream>& xInputStream,
    const bool bIsStreamReadOnly )
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
        sfx2::LinkManager::GetDisplayNames( refLink.get(), nullptr, &sGrfNm );
        if ( !sGrfNm.startsWith( "vnd.sun.star.pkg:" ) )
        {
            bRet = true;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
