/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <algorithm>
#include <cmath>
#include <unotools/streamwrap.hxx>

#include <sfx2/lnkbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/graphichelper.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include "svdfmtf.hxx"
#include <sdgcoitm.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/sdggaitm.hxx>
#include <sdginitm.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <sdgtritm.hxx>
#include <sdr/properties/graphicproperties.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <o3tl/numeric.hxx>
#include <o3tl/unit_conversion.hxx>
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <memory>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <svdpdf.hxx>
#include <svx/unoapi.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

class SdrGraphicLink : public sfx2::SvBaseLink
{
    SdrGrafObj&         rGrafObj;

public:
    explicit            SdrGraphicLink(SdrGrafObj& rObj);

    virtual void        Closed() override;

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const cpo::uno::Any & rValue ) override;

    void                Connect() { GetRealObject(); }
};

SdrGraphicLink::SdrGraphicLink(SdrGrafObj& rObj)
: ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB )
, rGrafObj( rObj )
{
    SetSynchron( false );
}

::sfx2::SvBaseLink::UpdateResult SdrGraphicLink::DataChanged(
    const OUString& rMimeType, const cpo::uno::Any & rValue )
{
    SdrModel& rModel(rGrafObj.getSdrModelFromSdrObject());
    sfx2::LinkManager* pLinkManager(rModel.GetLinkManager());

    if( pLinkManager && rValue.hasValue() )
    {
        sfx2::LinkManager::GetDisplayNames( this, nullptr, &rGrafObj.m_aFileName, nullptr, &rGrafObj.m_aFilterName );

        Graphic aGraphic;
        if (pLinkManager->GetGraphicFromAny(rMimeType, rValue, aGraphic, nullptr))
        {
            rGrafObj.ImpSetLinkedGraphic(aGraphic);
        }
        else if( SotExchange::GetFormatIdFromMimeType( rMimeType ) != sfx2::LinkManager::RegisterStatusInfoId() )
        {
            // broadcasting, to update slide sorter
            rGrafObj.BroadcastObjectChange();
        }
    }
    return SUCCESS;
}

void SdrGraphicLink::Closed()
{
    // close connection; set pLink of the object to NULL, as link instance is just about getting destructed.
    rGrafObj.ForceSwapIn();
    rGrafObj.m_pGraphicLink=nullptr;
    rGrafObj.ReleaseGraphicLink();
    SvBaseLink::Closed();
}

std::unique_ptr<sdr::properties::BaseProperties> SdrGrafObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::GraphicProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrGrafObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfGraphic>(*this);
}


// check if SVG and if try to get ObjectInfoPrimitive2D and extract info

void SdrGrafObj::onGraphicChanged()
{
    if (!mpGraphicObject || !mpGraphicObject->GetGraphic().isAvailable())
        return;

    auto const & rVectorGraphicDataPtr = mpGraphicObject->GetGraphic().getVectorGraphicData();

    if (!rVectorGraphicDataPtr)
        return;

    // Skip for PDF as it is only a bitmap primitive in a sequence and
    // doesn't contain metadata. However getting the primitive sequence
    // will also trigger a premature rendering of the PDF.
    if (rVectorGraphicDataPtr->getType() == VectorGraphicDataType::Pdf)
        return;

    const std::deque<uno::Reference<graphic::XPrimitive2D>>& rContainer(rVectorGraphicDataPtr->getPrimitive2DSequence());

    if (rContainer.empty())
        return;

    drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    drawinglayer::processor2d::ObjectInfoPrimitiveExtractor2D aProcessor(aViewInformation2D);

    aProcessor.process(rContainer);

    const drawinglayer::primitive2d::ObjectInfoPrimitive2D* pResult = aProcessor.getResult();

    if (!pResult)
        return;

    OUString aName = pResult->getName();
    OUString aTitle = pResult->getTitle();
    OUString aDesc = pResult->getDesc();

    if(!aName.isEmpty())
    {
        SetName(aName);
    }

    if(!aTitle.isEmpty())
    {
        SetTitle(aTitle);
    }

    if(!aDesc.isEmpty())
    {
        SetDescription(aDesc);
    }
}

SdrGrafObj::SdrGrafObj(SdrModel& rSdrModel)
:   SdrRectObj(rSdrModel)
    ,mpGraphicObject(new GraphicObject)
    ,m_pGraphicLink(nullptr)
    ,m_bMirrored(false)
    ,mbIsSignatureLine(false)
    ,mbIsSignatureLineShowSignDate(true)
    ,mbIsSignatureLineCanAddComment(false)
    ,mbSignatureLineIsSigned(false)
{
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    mbNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::SdrGrafObj(SdrModel& rSdrModel, SdrGrafObj const & rSource)
:   SdrRectObj(rSdrModel, rSource)
    ,mpGraphicObject(new GraphicObject)
    ,m_pGraphicLink(nullptr)
{
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    mbNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;

    m_aFileName = rSource.m_aFileName;
    m_bMirrored = rSource.m_bMirrored;

    mbIsSignatureLine = rSource.mbIsSignatureLine;
    maSignatureLineId = rSource.maSignatureLineId;
    maSignatureLineSuggestedSignerName = rSource.maSignatureLineSuggestedSignerName;
    maSignatureLineSuggestedSignerTitle = rSource.maSignatureLineSuggestedSignerTitle;
    maSignatureLineSuggestedSignerEmail = rSource.maSignatureLineSuggestedSignerEmail;
    maSignatureLineSigningInstructions = rSource.maSignatureLineSigningInstructions;
    mbIsSignatureLineShowSignDate = rSource.mbIsSignatureLineShowSignDate;
    mbIsSignatureLineCanAddComment = rSource.mbIsSignatureLineCanAddComment;
    mbSignatureLineIsSigned = false;
    mpSignatureLineUnsignedGraphic = rSource.mpSignatureLineUnsignedGraphic;

    if(rSource.mpBarCode)
    {
        mpBarCode = std::make_unique<drawing::BarCode>(*rSource.mpBarCode);
    }
    else
    {
        mpBarCode.reset();
    }

    if (mbIsSignatureLine && rSource.mpSignatureLineUnsignedGraphic)
        mpGraphicObject->SetGraphic(rSource.mpSignatureLineUnsignedGraphic);
    else
        mpGraphicObject->SetGraphic( rSource.GetGraphic() );

    if( rSource.IsLinkedGraphic() )
    {
        SetGraphicLink( m_aFileName );
    }
}

SdrGrafObj::SdrGrafObj(
    SdrModel& rSdrModel,
    const Graphic& rGraphic,
    const tools::Rectangle& rRect)
:   SdrRectObj(rSdrModel, rRect)
    ,mpGraphicObject(new GraphicObject(rGraphic))
    ,m_pGraphicLink(nullptr)
    ,m_bMirrored(false)
    ,mbIsSignatureLine(false)
    ,mbIsSignatureLineShowSignDate(true)
    ,mbIsSignatureLineCanAddComment(false)
    ,mbSignatureLineIsSigned(false)
{
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    mbNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::SdrGrafObj(
    SdrModel& rSdrModel,
    const Graphic& rGraphic)
:   SdrRectObj(rSdrModel)
    ,mpGraphicObject(new GraphicObject(rGraphic))
    ,m_pGraphicLink(nullptr)
    ,m_bMirrored(false)
    ,mbIsSignatureLine(false)
    ,mbIsSignatureLineShowSignDate(true)
    ,mbIsSignatureLineCanAddComment(false)
    ,mbSignatureLineIsSigned(false)
{
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    mbNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::~SdrGrafObj()
{
    ImpDeregisterLink();
}

void SdrGrafObj::SetGraphicObject(const GraphicObject& rGraphicObject)
{
    mpGraphicObject.reset(new GraphicObject(rGraphicObject));
    mpReplacementGraphicObject.reset();
    mpGraphicObject->SetUserData();
    SetChanged();
    BroadcastObjectChange();
    onGraphicChanged();
}

const GraphicObject& SdrGrafObj::GetGraphicObject(bool bForceSwapIn) const
{
    if (bForceSwapIn)
        ForceSwapIn();
    return *mpGraphicObject;
}

const GraphicObject* SdrGrafObj::GetReplacementGraphicObject() const
{
    if (!mpReplacementGraphicObject && mpGraphicObject)
    {
        auto const & rVectorGraphicDataPtr = mpGraphicObject->GetGraphic().getVectorGraphicData();

        if (rVectorGraphicDataPtr)
        {
            const_cast< SdrGrafObj* >(this)->mpReplacementGraphicObject.reset(new GraphicObject(rVectorGraphicDataPtr->getReplacement()));
        }
        else if (mpGraphicObject->GetGraphic().GetType() == GraphicType::GdiMetafile)
        {
            // Replacement graphic for PDF and metafiles is just the bitmap.
            const_cast<SdrGrafObj*>(this)->mpReplacementGraphicObject.reset(new GraphicObject(mpGraphicObject->GetGraphic().GetBitmap()));
        }
    }

    return mpReplacementGraphicObject.get();
}

cpo::uno::Reference<css::lang::XComponent> SdrGrafObj::GetReplacementGraphicModel() const
{
    if (!mpGraphicObject)
        return nullptr;

    const Graphic& rGraphic = mpGraphicObject->GetGraphic();
    auto const & rVectorGraphicDataPtr = rGraphic.getVectorGraphicData();
    if (!rVectorGraphicDataPtr)
        return nullptr;

    if (rVectorGraphicDataPtr->getType() != VectorGraphicDataType::Pdf)
        return nullptr;

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return nullptr;

    // Create an empty Draw component.
    uno::Reference<cpo::uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XModel> xModel(xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.drawing.DrawingDocument"_ustr, xContext),
        uno::UNO_QUERY);
    if (!xModel)
        return nullptr;
    uno::Reference<css::frame::XLoadable> xModelLoad(xModel, uno::UNO_QUERY);
    if (xModelLoad)
        xModelLoad->initNew();
    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xModel, uno::UNO_QUERY);
    if (!xDrawPagesSupplier)
        return nullptr;
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    if (!xDrawPages)
        return nullptr;
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    SdrPage* pSdrPage = GetSdrPageFromXDrawPage(xDrawPage);
    if (!pSdrPage)
        return nullptr;

    SdrModel& rSdrModel = pSdrPage->getSdrModelFromSdrPage();

    tools::Rectangle aLogicRect = GetLogicRect();

    ImpSdrPdfImport aFilter(rSdrModel, GetLayer(), aLogicRect, rGraphic);
    if (rGraphic.getPageNumber() < aFilter.GetPageCount())
    {
        aFilter.DoImport(*pSdrPage, 0, rGraphic.getPageNumber(), nullptr);
    }

    // Group shapes together, including invisible objects
    size_t nSrcObjCount = pSdrPage->GetObjCount();
    if (!nSrcObjCount)
        return xModel;

    rtl::Reference<SdrObject> xGrp(new SdrObjGroup(rSdrModel));
    SdrObjList* pDstList = xGrp->GetSubList();

    while (nSrcObjCount > 0)
    {
        --nSrcObjCount;
        rtl::Reference<SdrObject> xObj(pSdrPage->GetObj(nSrcObjCount));
        pSdrPage->RemoveObject(nSrcObjCount);
        pDstList->InsertObject(xObj.get(), 0);
    }

    pSdrPage->InsertObject(xGrp.get(), 0);

    return xModel;
}

void SdrGrafObj::NbcSetGraphic(const Graphic& rGraphic)
{
    mpGraphicObject->SetGraphic(rGraphic);
    mpReplacementGraphicObject.reset();
    mpGraphicObject->SetUserData();
    onGraphicChanged();
}

void SdrGrafObj::SetGraphic( const Graphic& rGraphic )
{
    if (!rGraphic.getOriginURL().isEmpty())
    {
        ImpDeregisterLink();
        m_aFileName = rGraphic.getOriginURL();
        m_aFilterName = u""_ustr;
    }
    NbcSetGraphic(rGraphic);
    if (!rGraphic.getOriginURL().isEmpty())
    {
        ImpRegisterLink();
        mpGraphicObject->SetUserData();
    }
    SetChanged();
    BroadcastObjectChange();
    ForceSwapIn();
}

const Graphic& SdrGrafObj::GetGraphic() const
{
    return mpGraphicObject->GetGraphic();
}

OUString SdrGrafObj::getGraphicExtension() const
{
    OUString aExtension;
    GraphicHelper::GetPreferredExtension(aExtension, GetGraphic());
    return aExtension;
}

Graphic SdrGrafObj::GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags ) const
{
    // Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality
    MapMode aDestMap(getSdrModelFromSdrObject().GetScaleUnit());
    const Size aDestSize( GetLogicRect().GetSize() );
    GraphicAttr aActAttr = GetGraphicAttr(nTransformFlags);

    // Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

GraphicType SdrGrafObj::GetGraphicType() const
{
    return mpGraphicObject->GetType();
}

GraphicAttr SdrGrafObj::GetGraphicAttr( SdrGrafObjTransformsAttrs nTransformFlags ) const
{
    GraphicAttr aActAttr;

    GraphicType eType = GetGraphicType();
    if( SdrGrafObjTransformsAttrs::NONE != nTransformFlags &&
        GraphicType::NONE != eType )
    {
        const bool      bMirror = bool( nTransformFlags & SdrGrafObjTransformsAttrs::MIRROR );
        const bool      bRotate = bool( nTransformFlags & SdrGrafObjTransformsAttrs::ROTATE ) &&
            (maGeo.m_nRotationAngle && maGeo.m_nRotationAngle != 18000_deg100);

        // Actually transform the graphic only in this case.
        // Cropping always happens, though.
        const SfxItemSet& rSet = GetObjectItemSet();
        const sal_uInt16 nTrans = rSet.Get( SDRATTR_GRAFTRANSPARENCE ).GetValue();
        const SdrGrafCropItem&  rCrop = rSet.Get( SDRATTR_GRAFCROP );

        aActAttr.SetLuminance( rSet.Get( SDRATTR_GRAFLUMINANCE ).GetValue() );
        aActAttr.SetContrast( rSet.Get( SDRATTR_GRAFCONTRAST ).GetValue() );
        aActAttr.SetChannelR( rSet.Get( SDRATTR_GRAFRED ).GetValue() );
        aActAttr.SetChannelG( rSet.Get( SDRATTR_GRAFGREEN ).GetValue() );
        aActAttr.SetChannelB( rSet.Get( SDRATTR_GRAFBLUE ).GetValue() );
        aActAttr.SetGamma( rSet.Get( SDRATTR_GRAFGAMMA ).GetValue() * 0.01 );
        aActAttr.SetAlpha(255 - basegfx::fround<sal_uInt8>(nTrans * 2.55));
        aActAttr.SetInvert( rSet.Get( SDRATTR_GRAFINVERT ).GetValue() );
        aActAttr.SetDrawMode( rSet.Get( SDRATTR_GRAFMODE ).GetValue() );
        aActAttr.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

        if( bMirror )
        {
            sal_uInt16      nMirrorCase = ( maGeo.m_nRotationAngle == 18000_deg100 ) ? ( m_bMirrored ? 3 : 4 ) : ( m_bMirrored ? 2 : 1 );
            bool bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            bool bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BmpMirrorFlags::Horizontal : BmpMirrorFlags::NONE ) | ( bVMirr ? BmpMirrorFlags::Vertical : BmpMirrorFlags::NONE ) );
        }

        if( bRotate )
            aActAttr.SetRotation( to<Degree10>(maGeo.m_nRotationAngle ) );
    }

    return aActAttr;
}

bool SdrGrafObj::IsAnimated() const
{
    return mpGraphicObject->IsAnimated();
}

bool SdrGrafObj::IsEPS() const
{
    return mpGraphicObject->IsEPS();
}

MapMode SdrGrafObj::GetGrafPrefMapMode() const
{
    return mpGraphicObject->GetPrefMapMode();
}

Size SdrGrafObj::GetGrafPrefSize() const
{
    return mpGraphicObject->GetPrefSize();
}

void SdrGrafObj::SetGrafStreamURL( const OUString& rGraphicStreamURL )
{
    if( rGraphicStreamURL.isEmpty() )
    {
        mpGraphicObject->SetUserData();
    }
    else if(getSdrModelFromSdrObject().IsSwapGraphics() )
    {
        mpGraphicObject->SetUserData( rGraphicStreamURL );
    }
}

OUString const & SdrGrafObj::GetGrafStreamURL() const
{
    return mpGraphicObject->GetUserData();
}

Size SdrGrafObj::getOriginalSize() const
{
    Size aSize = GetGrafPrefSize();

    if (GetGrafPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
        aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, MapMode(getSdrModelFromSdrObject().GetScaleUnit()));
    else
        aSize = OutputDevice::LogicToLogic(aSize, GetGrafPrefMapMode(), MapMode(getSdrModelFromSdrObject().GetScaleUnit()));

    const SfxItemSet& rSet = GetObjectItemSet();
    const SdrGrafCropItem&  rCrop = rSet.Get( SDRATTR_GRAFCROP );
    if ( rCrop.GetLeft() != 0 || rCrop.GetTop() != 0 || rCrop.GetRight() != 0 || rCrop.GetBottom() != 0 ) // if is cropped
    {
        const tools::Long aCroppedWidth(aSize.getWidth() - rCrop.GetLeft()
                                        - rCrop.GetRight());
        const tools::Long aCroppedHeight(aSize.getHeight() - rCrop.GetTop()
                                         - rCrop.GetBottom());

        aSize = Size(aCroppedWidth, aCroppedHeight);
    }

    return aSize;
}

// TODO Remove
void SdrGrafObj::ForceSwapIn() const
{
    if (m_pGraphicLink && (mpGraphicObject->GetType() == GraphicType::NONE  ||
                         mpGraphicObject->GetType() == GraphicType::Default) )
    {
        // Don't fetch linked graphic URLs if the user has not yet allowed
        // link updates. They have not been prompted about external content.
        sfx2::LinkManager* pLinkManager(getSdrModelFromSdrObject().GetLinkManager());
        SfxObjectShell* pShell = pLinkManager ? pLinkManager->GetPersist() : nullptr;
        if (!pShell || pShell->getEmbeddedObjectContainer().getUserAllowsLinkUpdate())
            m_pGraphicLink->Update();
    }
}

void SdrGrafObj::ImpRegisterLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrObject().GetLinkManager());

    if( pLinkManager != nullptr && m_pGraphicLink == nullptr )
    {
        if (!m_aFileName.isEmpty())
        {
            m_pGraphicLink = new SdrGraphicLink( *this );
            pLinkManager->InsertFileLink(
                *m_pGraphicLink, sfx2::SvBaseLinkObjectType::ClientGraphic, m_aFileName, (m_aFilterName.isEmpty() ? nullptr : &m_aFilterName));
            m_pGraphicLink->Connect();
        }
    }
}

void SdrGrafObj::ImpDeregisterLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrObject().GetLinkManager());

    if( pLinkManager != nullptr && m_pGraphicLink!=nullptr)
    {
        // When using Remove, the *pGraphicLink is implicitly deleted
        pLinkManager->Remove( m_pGraphicLink );
        m_pGraphicLink=nullptr;
    }
}

void SdrGrafObj::SetGraphicLink(const OUString& rFileName)
{
    Graphic aGraphic;
    aGraphic.setOriginURL(rFileName);
    SetGraphic(aGraphic);
}

void SdrGrafObj::ReleaseGraphicLink()
{
    ImpDeregisterLink();
    m_aFileName.clear();
    m_aFilterName.clear();

    auto aGraphic = mpGraphicObject->GetGraphic();
    aGraphic.setOriginURL(u""_ustr);
    SetGraphic(aGraphic);
}

bool SdrGrafObj::IsLinkedGraphic() const
{
    return !mpGraphicObject->GetGraphic().getOriginURL().isEmpty();
}

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoPresGrf = ( mpGraphicObject->GetType() != GraphicType::NONE ) && !m_bEmptyPresObj;

    rInfo.bResizeFreeAllowed = maGeo.m_nRotationAngle.get() % 9000 == 0 ||
                               maGeo.m_nRotationAngle.get() % 18000 == 0 ||
                               maGeo.m_nRotationAngle.get() % 27000 == 0;

    rInfo.bResizePropAllowed = true;
    rInfo.bRotateFreeAllowed = bNoPresGrf;
    rInfo.bRotate90Allowed = bNoPresGrf;
    rInfo.bMirrorFreeAllowed = bNoPresGrf;
    rInfo.bMirror45Allowed = bNoPresGrf;
    rInfo.bMirror90Allowed = !m_bEmptyPresObj;
    rInfo.bTransparenceAllowed = false;

    // #i118485# Shear allowed and possible now
    rInfo.bShearAllowed = true;

    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath = !IsEPS();
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
    rInfo.bCanConvToPoly = !IsEPS();
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

SdrObjKind SdrGrafObj::GetObjIdentifier() const
{
    return SdrObjKind::Graphic;
}

void SdrGrafObj::ImpSetLinkedGraphic( const Graphic& rGraphic )
{
    const bool bIsChanged(getSdrModelFromSdrObject().IsChanged());
    NbcSetGraphic( rGraphic );
    ActionChanged();
    BroadcastObjectChange();
    getSdrModelFromSdrObject().SetChanged(bIsChanged);
}

OUString SdrGrafObj::TakeObjNameSingul() const
{
    if (!mpGraphicObject)
        return OUString();

    auto const & rVectorGraphicDataPtr = mpGraphicObject->GetGraphic().getVectorGraphicData();

    OUStringBuffer sName;

    if (rVectorGraphicDataPtr)
    {
        switch (rVectorGraphicDataPtr->getType())
        {
        case VectorGraphicDataType::Svg:
        {
            sName.append(SvxResId(STR_ObjNameSingulGRAFSVG));
            break;
        }
        case VectorGraphicDataType::Wmf:
        {
            sName.append(SvxResId(STR_ObjNameSingulGRAFWMF));
            break;
        }
        case VectorGraphicDataType::Emf:
        {
            sName.append(SvxResId(STR_ObjNameSingulGRAFEMF));
            break;
        }
        case VectorGraphicDataType::Pdf:
        {
            sName.append(SvxResId(STR_ObjNameSingulGRAFPDF));
            break;
        }
        } //no default, see tdf#137813
    }
    else
    {
        switch( mpGraphicObject->GetType() )
        {
            case GraphicType::Bitmap:
            {
                TranslateId pId = ( ( mpGraphicObject->IsTransparent() || GetObjectItem( SDRATTR_GRAFTRANSPARENCE ).GetValue() ) ?
                                     ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPTRANSLNK : STR_ObjNameSingulGRAFBMPTRANS ) :
                                     ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPLNK : STR_ObjNameSingulGRAFBMP ) );

                sName.append(SvxResId(pId));
            }
            break;

            case GraphicType::GdiMetafile:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNameSingulGRAFMTFLNK : STR_ObjNameSingulGRAFMTF));
            break;

            case GraphicType::NONE:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNameSingulGRAFNONELNK : STR_ObjNameSingulGRAFNONE));
            break;

            default:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNameSingulGRAFLNK : STR_ObjNameSingulGRAF));
            break;
        }
    }

    const OUString aName(GetName());

    if (!aName.isEmpty())
    {
        sName.append(" '" + aName + "\'" );
    }

    return sName.makeStringAndClear();
}

OUString SdrGrafObj::TakeObjNamePlural() const
{
    if (!mpGraphicObject)
        return OUString();

    auto const & rVectorGraphicDataPtr = mpGraphicObject->GetGraphic().getVectorGraphicData();

    OUStringBuffer sName;

    if (rVectorGraphicDataPtr)
    {
        switch (rVectorGraphicDataPtr->getType())
        {
        case VectorGraphicDataType::Svg:
        {
            sName.append(SvxResId(STR_ObjNamePluralGRAFSVG));
            break;
        }
        case VectorGraphicDataType::Wmf:
        {
            sName.append(SvxResId(STR_ObjNamePluralGRAFWMF));
            break;
        }
        case VectorGraphicDataType::Emf:
        {
            sName.append(SvxResId(STR_ObjNamePluralGRAFEMF));
            break;
        }
        case VectorGraphicDataType::Pdf:
        {
            sName.append(SvxResId(STR_ObjNamePluralGRAFPDF));
            break;
        }
        } //no default, see tdf#137813
    }
    else
    {
        switch(mpGraphicObject->GetType())
        {
            case GraphicType::Bitmap:
            {
                TranslateId pId = ( ( mpGraphicObject->IsTransparent() || GetObjectItem( SDRATTR_GRAFTRANSPARENCE ).GetValue() ) ?
                                     ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPTRANSLNK : STR_ObjNamePluralGRAFBMPTRANS ) :
                                     ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPLNK : STR_ObjNamePluralGRAFBMP ) );

                sName.append(SvxResId(pId));
            }
            break;

            case GraphicType::GdiMetafile:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNamePluralGRAFMTFLNK : STR_ObjNamePluralGRAFMTF));
            break;

            case GraphicType::NONE:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNamePluralGRAFNONELNK : STR_ObjNamePluralGRAFNONE));
            break;

            default:
                sName.append(SvxResId(IsLinkedGraphic() ? STR_ObjNamePluralGRAFLNK : STR_ObjNamePluralGRAF));
            break;
        }
    }

    const OUString aName(GetName());

    if (!aName.isEmpty())
    {
        sName.append(" '" + aName + "\'");
    }

    return sName.makeStringAndClear();
}

rtl::Reference<SdrObject> SdrGrafObj::getFullDragClone() const
{
    // call parent
    rtl::Reference<SdrObject> pRetval = SdrRectObj::getFullDragClone();

    // #i103116# the full drag clone leads to problems
    // with linked graphics, so reset the link in this
    // temporary interaction object and load graphic
    if(pRetval && IsLinkedGraphic())
    {
        static_cast< SdrGrafObj* >(pRetval.get())->ReleaseGraphicLink();
    }

    return pRetval;
}

rtl::Reference<SdrObject> SdrGrafObj::implCloneSdrObject(SdrModel& rTargetModel) const
{
    return new SdrGrafObj(rTargetModel, *this);
}

sal_uInt32 SdrGrafObj::GetHdlCount() const
{
    return 8;
}

void SdrGrafObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    SdrHdlList tempList(nullptr);
    SdrRectObj::AddToHdlList( tempList );
    tempList.RemoveHdl(0);
    tempList.MoveTo(rHdlList);
}

void SdrGrafObj::NbcResize(const Point& rRef, double xFact, double yFact)
{
    SdrRectObj::NbcResize( rRef, xFact, yFact );

    bool bMirrX = xFact < 0;
    bool bMirrY = yFact < 0;

    if( bMirrX != bMirrY )
        m_bMirrored = !m_bMirrored;
}

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrRectObj::NbcMirror(rRef1,rRef2);
    m_bMirrored = !m_bMirrored;
}

std::unique_ptr<SdrObjGeoData> SdrGrafObj::NewGeoData() const
{
    return std::make_unique<SdrGrafObjGeoData>();
}

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=static_cast<SdrGrafObjGeoData&>(rGeo);
    rGGeo.bMirrored=m_bMirrored;
}

void SdrGrafObj::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestoreGeoData(rGeo);
    const SdrGrafObjGeoData& rGGeo=static_cast<const SdrGrafObjGeoData&>(rGeo);
    m_bMirrored=rGGeo.bMirrored;
}

void SdrGrafObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    const bool bRemove(pNewPage == nullptr && pOldPage != nullptr);
    const bool bInsert(pNewPage != nullptr && pOldPage == nullptr);

    if( bRemove )
    {
        // No SwapIn necessary here, because if something's not loaded, it can't be animated either.
        if( mpGraphicObject->IsAnimated())
            mpGraphicObject->StopAnimation();

        if( m_pGraphicLink != nullptr )
            ImpDeregisterLink();
    }

    // call parent
    SdrRectObj::handlePageChange(pOldPage, pNewPage);

    if (!m_aFileName.isEmpty() && bInsert)
    {
        ImpRegisterLink();
    }
}

void SdrGrafObj::StartAnimation()
{
    SetGrafAnimationAllowed(true);
}

bool SdrGrafObj::HasGDIMetaFile() const
{
    return( mpGraphicObject->GetType() == GraphicType::GdiMetafile );
}

bool SdrGrafObj::isEmbeddedVectorGraphicData() const
{
    return GraphicType::Bitmap == GetGraphicType() && GetGraphic().getVectorGraphicData();
}

GDIMetaFile SdrGrafObj::getMetafileFromEmbeddedVectorGraphicData() const
{
    GDIMetaFile aRetval;

    if(isEmbeddedVectorGraphicData())
    {
        ScopedVclPtrInstance< VirtualDevice > pOut;
        const tools::Rectangle aBoundRect(GetCurrentBoundRect());
        const MapMode aMap(getSdrModelFromSdrObject().GetScaleUnit());

        pOut->EnableOutput(false);
        pOut->SetMapMode(aMap);
        aRetval.Record(pOut);
        SingleObjectPainter(*pOut);
        aRetval.Stop();
        aRetval.WindStart();
        aRetval.Move(-aBoundRect.Left(), -aBoundRect.Top());
        aRetval.SetPrefMapMode(aMap);
        aRetval.SetPrefSize(aBoundRect.GetSize());
    }

    return aRetval;
}

GDIMetaFile SdrGrafObj::GetMetaFile(GraphicType &rGraphicType) const
{
    if (isEmbeddedVectorGraphicData())
    {
        // Embedded Vector Graphic Data
        // There is currently no helper to create SdrObjects from primitives (even if I'm thinking
        // about writing one for some time). To get the roundtrip to SdrObjects it is necessary to
        // use the old converter path over the MetaFile mechanism. Create Metafile from Svg
        // primitives here pretty directly
        rGraphicType = GraphicType::GdiMetafile;
        return getMetafileFromEmbeddedVectorGraphicData();
    }
    else if (GraphicType::GdiMetafile == rGraphicType)
    {
        return GetTransformedGraphic(SdrGrafObjTransformsAttrs::MIRROR).GetGDIMetaFile();
    }
    return GDIMetaFile();
}

rtl::Reference<SdrObject> SdrGrafObj::DoConvertToPolyObj(bool bBezier, bool bAddText ) const
{
    rtl::Reference<SdrObject> pRetval;
    GraphicType aGraphicType(GetGraphicType());
    GDIMetaFile aMtf(GetMetaFile(aGraphicType));
    switch(aGraphicType)
    {
        case GraphicType::GdiMetafile:
        {
            // Sort into group and return ONLY those objects that can be created from the MetaFile.
            ImpSdrGDIMetaFileImport aFilter(
                getSdrModelFromSdrObject(),
                GetLayer(),
                getRectangle());
            rtl::Reference<SdrObjGroup> pGrp = new SdrObjGroup(getSdrModelFromSdrObject());

            if(aFilter.DoImport(aMtf, *pGrp->GetSubList(), 0))
            {
                {
                        // copy transformation
                    GeoStat aGeoStat(GetGeoStat());

                    if(aGeoStat.m_nShearAngle)
                    {
                        aGeoStat.RecalcTan();
                        pGrp->NbcShear(getRectangle().TopLeft(), aGeoStat.m_nShearAngle, aGeoStat.mfTanShearAngle, false);
                    }

                    if(aGeoStat.m_nRotationAngle)
                    {
                        aGeoStat.RecalcSinCos();
                        pGrp->NbcRotate(getRectangle().TopLeft(), aGeoStat.m_nRotationAngle, aGeoStat.mfSinRotationAngle, aGeoStat.mfCosRotationAngle);
                    }
                }

                pRetval = pGrp;
                pGrp->NbcSetLayer(GetLayer());

                if(bAddText)
                {
                    pRetval = ImpConvertAddText(pRetval, bBezier);
                }

                // convert all children
                if( pRetval )
                {
                    pRetval = pRetval->DoConvertToPolyObj(bBezier, bAddText);

                    if( pRetval )
                    {
                        // flatten subgroups. As we call
                        // DoConvertToPolyObj() on the resulting group
                        // objects, subgroups can exist (e.g. text is
                        // a group object for every line).
                        SdrObjList* pList = pRetval->GetSubList();
                        if( pList )
                            pList->FlattenGroups();
                    }
                }
            }
            else
                pGrp.clear();

            // #i118485# convert line and fill
            rtl::Reference<SdrObject> pLineFill = SdrRectObj::DoConvertToPolyObj(bBezier, false);

            if(pLineFill)
            {
                if(pRetval)
                {
                    pGrp = dynamic_cast< SdrObjGroup* >(pRetval.get());

                    if(!pGrp)
                    {
                        pGrp = new SdrObjGroup(getSdrModelFromSdrObject());
                        pGrp->NbcSetLayer(GetLayer());
                        pGrp->GetSubList()->NbcInsertObject(pRetval.get());
                    }

                    pGrp->GetSubList()->NbcInsertObject(pLineFill.get(), 0);
                }
                else
                {
                    pRetval = std::move(pLineFill);
                }
            }

            break;
        }
        case GraphicType::Bitmap:
        {
            // create basic object and add fill
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);

            // save bitmap as an attribute
            if(pRetval)
            {
                // retrieve bitmap for the fill
                SfxItemSet aSet(GetObjectItemSet());

                aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                const Bitmap aBitmap(GetTransformedGraphic().GetBitmap());
                aSet.Put(XFillBitmapItem(OUString(), Graphic(aBitmap)));
                aSet.Put(XFillBmpTileItem(false));

                pRetval->SetMergedItemSet(aSet);
            }
            break;
        }
        case GraphicType::NONE:
        case GraphicType::Default:
        {
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);
            break;
        }
    }

    return pRetval;
}

void SdrGrafObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SetXPolyDirty();
    SdrRectObj::Notify( rBC, rHint );
}


void SdrGrafObj::SetMirrored( bool _bMirrored )
{
    m_bMirrored = _bMirrored;
}

void SdrGrafObj::AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly )
{
    Size aSize;
    Size aMaxSize( rMaxRect.GetSize() );
    if (mpGraphicObject->GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
        aSize = Application::GetDefaultDevice()->PixelToLogic(mpGraphicObject->GetPrefSize(), MapMode(MapUnit::Map100thMM));
    else
        aSize = OutputDevice::LogicToLogic( mpGraphicObject->GetPrefSize(),
                                            mpGraphicObject->GetPrefMapMode(),
                                            MapMode( MapUnit::Map100thMM ) );

    if( aSize.IsEmpty() )
        return;

    Point aPos( rMaxRect.TopLeft() );

    // if the graphic is too large, fit it to page
    if ( (!bShrinkOnly                          ||
         ( aSize.Height() > aMaxSize.Height() ) ||
         ( aSize.Width()  > aMaxSize.Width()  ) )&&
         aSize.Height() && aMaxSize.Height() )
    {
        float fGrfWH =  static_cast<float>(aSize.Width()) /
                        static_cast<float>(aSize.Height());
        float fWinWH =  static_cast<float>(aMaxSize.Width()) /
                        static_cast<float>(aMaxSize.Height());

        // Scale graphic to page size
        if ( fGrfWH < fWinWH )
        {
            aSize.setWidth( static_cast<tools::Long>(aMaxSize.Height() * fGrfWH) );
            aSize.setHeight( aMaxSize.Height() );
        }
        else if ( fGrfWH > 0.F )
        {
            aSize.setWidth( aMaxSize.Width() );
            aSize.setHeight( static_cast<tools::Long>(aMaxSize.Width() / fGrfWH) );
        }

        aPos = rMaxRect.Center();
    }

    if( bShrinkOnly )
        aPos = getRectangle().TopLeft();

    aPos.AdjustX( -(aSize.Width() / 2) );
    aPos.AdjustY( -(aSize.Height() / 2) );
    SetLogicRect( tools::Rectangle( aPos, aSize ) );
}

void SdrGrafObj::SetGrafAnimationAllowed(bool bNew)
{
    if(mbGrafAnimationAllowed != bNew)
    {
        mbGrafAnimationAllowed = bNew;
        ActionChanged();
    }
}

uno::Reference<io::XInputStream> SdrGrafObj::getInputStream() const
{
    uno::Reference<io::XInputStream> xStream;

    if (mpGraphicObject && GetGraphic().IsGfxLink())
    {
        Graphic aGraphic( GetGraphic() );
        GfxLink aLink( aGraphic.GetGfxLink() );
        sal_uInt32 nSize = aLink.GetDataSize();
        const void* pSourceData = static_cast<const void*>(aLink.GetData());
        if( nSize && pSourceData )
        {
            sal_uInt8 * pBuffer = new sal_uInt8[ nSize ];
            memcpy( pBuffer, pSourceData, nSize );

            SvMemoryStream* pStream = new SvMemoryStream( static_cast<void*>(pBuffer), static_cast<std::size_t>(nSize), StreamMode::READ );
            pStream->ObjectOwnsMemory( true );
            xStream.set( new utl::OInputStreamWrapper( pStream, true ) );
        }
    }

    if (!xStream.is() && !m_aFileName.isEmpty())
    {
        SvFileStream* pStream = new SvFileStream( m_aFileName, StreamMode::READ );
        xStream.set( new utl::OInputStreamWrapper( pStream ) );
    }

    return xStream;
}

// moved crop handle creation here; this is the object type using them
void SdrGrafObj::addCropHandles(SdrHdlList& rTarget) const
{
    basegfx::B2DHomMatrix aMatrix;
    basegfx::B2DPolyPolygon aPolyPolygon;

    // get object transformation
    TRGetBaseGeometry(aMatrix, aPolyPolygon);

    // part of object transformation correction, but used later, so defined outside next scope
    double fShearX(0.0), fRotate(0.0);

    {   // TTTT correct shear, it comes currently mirrored from TRGetBaseGeometry, can be removed with aw080
        basegfx::B2DTuple aScale;
        basegfx::B2DTuple aTranslate;

        aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

        aMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
            aScale,
            -fShearX,
            fRotate,
            aTranslate);
    }

    basegfx::B2DPoint aPos;
    aPos = aMatrix * basegfx::B2DPoint(0.0, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::UpperLeft, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Upper, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::UpperRight, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 0.5);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Left , fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.5);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Right, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::LowerLeft, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Lower, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::LowerRight, fShearX, fRotate));

    // get crop values
    const SdrGrafCropItem& rCrop = GetMergedItem(SDRATTR_GRAFCROP);
    if(rCrop.GetLeft() || rCrop.GetTop() || rCrop.GetRight() ||rCrop.GetBottom())
    {
        // decompose object transformation to have current translate and scale
        basegfx::B2DVector aScale, aTranslate;
        double fLclRotate, fLclShearX;

        aMatrix.decompose(aScale, aTranslate, fLclRotate, fLclShearX);

        if(!aScale.equalZero())
        {
            // get crop scale
            const basegfx::B2DVector aCropScaleFactor(
                GetGraphicObject().calculateCropScaling(
                    aScale.getX(),
                    aScale.getY(),
                    rCrop.GetLeft(),
                    rCrop.GetTop(),
                    rCrop.GetRight(),
                    rCrop.GetBottom()));

            // apply crop scale
            const double fCropLeft(rCrop.GetLeft() * aCropScaleFactor.getX());
            const double fCropTop(rCrop.GetTop() * aCropScaleFactor.getY());
            const double fCropRight(rCrop.GetRight() * aCropScaleFactor.getX());
            const double fCropBottom(rCrop.GetBottom() * aCropScaleFactor.getY());
            basegfx::B2DHomMatrix aMatrixForCropViewHdl(aMatrix);

            if(IsMirrored())
            {
                // create corrected new matrix, TTTT can be removed with aw080
                // the old mirror only can mirror horizontally; the vertical mirror
                // is faked by using the horizontal and 180 degree rotation. Since
                // the object can be rotated differently from 180 degree, this is
                // not safe to detect. Just correct horizontal mirror (which is
                // in IsMirrored()) and keep the rotation angle
                // caution: Do not modify aMatrix, it is used below to calculate
                // the exact handle positions
                basegfx::B2DHomMatrix aPreMultiply;

                // mirrored X, apply
                aPreMultiply.translate(-0.5, 0.0);
                aPreMultiply.scale(-1.0, 1.0);
                aPreMultiply.translate(0.5, 0.0);

                aMatrixForCropViewHdl = aMatrixForCropViewHdl * aPreMultiply;
            }

            rTarget.AddHdl(std::make_unique<SdrCropViewHdl>(
                aMatrixForCropViewHdl, GetGraphicObject().GetGraphic(), fCropLeft, fCropTop,
                fCropRight, fCropBottom));
        }
    }
}

namespace
{
/** The mapping with its shear turned the other way. The shear comes mirrored from
    TRGetBaseGeometry and goes back the same way into TRSetBaseGeometry, so a crop works on the
    mapping turned over and turns it back when it is done. A mapping without shear stays as it is,
    and bTurned says whether there was any. */
basegfx::B2DHomMatrix withShearTurnedOver(const basegfx::B2DHomMatrix& rMapping, bool& bTurned)
{
    const basegfx::utils::B2DHomMatrixBufferedDecompose aDecompose(rMapping);
    bTurned = !basegfx::fTools::equalZero(aDecompose.getShearX());
    if (!bTurned)
        return rMapping;

    return basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
        aDecompose.getScale(), -aDecompose.getShearX(), aDecompose.getRotate(),
        aDecompose.getTranslate());
}
}

void SdrGrafObj::cropChangeFor(SdrHdlKind eHandle, const basegfx::B2DPoint& rTo,
                               SdrGrafCropChange& rChange) const
{
    // get the original objects transformation
    basegfx::B2DPolyPolygon aPolyPolygon;
    rChange.mbShearCorrected = false;
    TRGetBaseGeometry(rChange.maOriginal, aPolyPolygon);

    // correct shear, it comes currently mirrored from TRGetBaseGeometry, can be removed with aw080
    rChange.maOriginal = withShearTurnedOver(rChange.maOriginal, rChange.mbShearCorrected);

    // generate start point of original drag vector in unit coordinates (the
    // vis-a-vis of the drag point)
    basegfx::B2DPoint aLocalStart(0.0, 0.0);
    bool bOnAxis(false);

    switch(eHandle)
    {
        case SdrHdlKind::UpperLeft: aLocalStart.setX(1.0); aLocalStart.setY(1.0); break;
        case SdrHdlKind::Upper: aLocalStart.setX(0.5); aLocalStart.setY(1.0); bOnAxis = true; break;
        case SdrHdlKind::UpperRight: aLocalStart.setX(0.0); aLocalStart.setY(1.0); break;
        case SdrHdlKind::Left : aLocalStart.setX(1.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case SdrHdlKind::Right: aLocalStart.setX(0.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case SdrHdlKind::LowerLeft: aLocalStart.setX(1.0); aLocalStart.setY(0.0); break;
        case SdrHdlKind::Lower: aLocalStart.setX(0.5); aLocalStart.setY(0.0); bOnAxis = true; break;
        case SdrHdlKind::LowerRight: aLocalStart.setX(0.0); aLocalStart.setY(0.0); break;
        default: break;
    }

    rChange.maLocalStart = aLocalStart;

    // create the current drag position in unit coordinates. To get there,
    // transform back the DragPoint to UnitCoordinates
    basegfx::B2DHomMatrix aInverse(rChange.maOriginal);
    aInverse.invert();
    basegfx::B2DPoint aLocalCurrent(aInverse * rTo);

    // if one of the edge handles is used, limit to X or Y drag only
    if(bOnAxis)
    {
        if(basegfx::fTools::equal(aLocalStart.getX(), 0.5))
        {
            aLocalCurrent.setX(aLocalStart.getX());
        }
        else
        {
            aLocalCurrent.setY(aLocalStart.getY());
        }
    }

    // create internal change in unit coordinates
    basegfx::B2DHomMatrix aDiscreteChangeMatrix;

    if(!basegfx::fTools::equal(aLocalCurrent.getX(), aLocalStart.getX()))
    {
        if(aLocalStart.getX() < 0.5)
        {
            aDiscreteChangeMatrix.scale(aLocalCurrent.getX(), 1.0);
        }
        else
        {
            aDiscreteChangeMatrix.scale(1.0 - aLocalCurrent.getX(), 1.0);
            aDiscreteChangeMatrix.translate(aLocalCurrent.getX(), 0.0);
        }
    }

    if(!basegfx::fTools::equal(aLocalCurrent.getY(), aLocalStart.getY()))
    {
        if(aLocalStart.getY() < 0.5)
        {
            aDiscreteChangeMatrix.scale(1.0, aLocalCurrent.getY());
        }
        else
        {
            aDiscreteChangeMatrix.scale(1.0, 1.0 - aLocalCurrent.getY());
            aDiscreteChangeMatrix.translate(0.0, aLocalCurrent.getY());
        }
    }

    rChange.maChange = aDiscreteChangeMatrix;
}

bool SdrGrafObj::cropChangeForWindow(const basegfx::B2DPoint& rCorner,
                                     const basegfx::B2DPoint& rOppositeCorner,
                                     SdrGrafCropChange& rChange) const
{
    // A crop by the upper left handle takes the picture's mapping onto the page, with the
    // shear set right, and the window is read through that same mapping.
    cropChangeFor(SdrHdlKind::UpperLeft, rCorner, rChange);
    basegfx::B2DHomMatrix aInverse(rChange.maOriginal);
    aInverse.invert();
    // The two corners are opposite ones, and the window runs between them on each axis.
    const basegfx::B2DPoint aFirst(aInverse * rCorner);
    const basegfx::B2DPoint aSecond(aInverse * rOppositeCorner);
    basegfx::B2DPoint aStart(std::min(aFirst.getX(), aSecond.getX()),
                             std::min(aFirst.getY(), aSecond.getY()));
    basegfx::B2DPoint aEnd(std::max(aFirst.getX(), aSecond.getX()),
                           std::max(aFirst.getY(), aSecond.getY()));

    // A twip on the page, as a share of the window along each axis. The corners come in whole
    // twips, so an edge within that of where it is now is the edge as it is.
    const basegfx::utils::B2DHomMatrixBufferedDecompose aDecompose(rChange.maOriginal);
    const double fTwip(o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100));
    const double fAcross(fTwip / std::max(std::abs(aDecompose.getScale().getX()), 1.0));
    const double fDown(fTwip / std::max(std::abs(aDecompose.getScale().getY()), 1.0));
    const auto kept = [](double fEdge, double fNow, double fNear) {
        return std::abs(fEdge - fNow) <= fNear ? fNow : fEdge;
    };
    aStart = basegfx::B2DPoint(kept(aStart.getX(), 0.0, fAcross), kept(aStart.getY(), 0.0, fDown));
    aEnd = basegfx::B2DPoint(kept(aEnd.getX(), 1.0, fAcross), kept(aEnd.getY(), 1.0, fDown));

    if (aEnd.getX() <= aStart.getX() || aEnd.getY() <= aStart.getY())
        return false;

    rChange.maChange = basegfx::utils::createScaleTranslateB2DHomMatrix(
        aEnd.getX() - aStart.getX(), aEnd.getY() - aStart.getY(), aStart.getX(), aStart.getY());
    return true;
}

void SdrGrafObj::cropWindowRanges(const SdrGrafCropChange& rChange, basegfx::B2DRange& rBefore,
                                  basegfx::B2DRange& rAfter)
{
    // Create the unrotated original rectangle and the unrotated modified
    // rectangle as Ranges
    const basegfx::utils::B2DHomMatrixBufferedDecompose aOriginalMatrixDecomp(rChange.maOriginal);

    // prepare unsheared/unrotated versions of the old and new transformation
    const basegfx::B2DHomMatrix aOriginalMatrixNoShearNoRotate(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            basegfx::absolute(aOriginalMatrixDecomp.getScale()),
            aOriginalMatrixDecomp.getTranslate()));

    // create the ranges for these
    rBefore = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
    rAfter = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
    rBefore.transform(aOriginalMatrixNoShearNoRotate);
    rAfter.transform(aOriginalMatrixNoShearNoRotate * rChange.maChange);
}

bool SdrGrafObj::applyCropChange(const SdrGrafCropChange& rChange)
{
    // prepare matrix to apply to object. Evtl. back-correct shear
    basegfx::B2DHomMatrix aNewObjectMatrix(rChange.maOriginal * rChange.maChange);

    bool bTurned(false);
    if(rChange.mbShearCorrected)
    {
        // back-correct shear
        aNewObjectMatrix = withShearTurnedOver(aNewObjectMatrix, bTurned);
    }

    // apply change to object by applying the unit coordinate change followed
    // by the original change
    basegfx::B2DPolyPolygon aPolyPolygon;
    TRSetBaseGeometry(aNewObjectMatrix, aPolyPolygon);

    /*
        The model holds the geometry in whole units, its turn and its shear included, so the
        window it laid down is not exactly the one asked for. The crop values are worked out from
        the window it reached, read back the way the change was worked out, so the graphic keeps
        its place and its scale however narrow the window is.
    */
    SdrGrafCropChange aReached(rChange);
    {
        basegfx::B2DHomMatrix aMappingReached;
        TRGetBaseGeometry(aMappingReached, aPolyPolygon);
        if (rChange.mbShearCorrected)
            aMappingReached = withShearTurnedOver(aMappingReached, bTurned);

        /*
            The reached window is measured on the unit square of the old one by where its corner
            went and how long its two sides are along the two axes. A shear or a turn that came
            back a hair off from what was asked for adds nothing to that, where it would widen a
            narrow window's box by as much as the window is high against its width.
        */
        basegfx::B2DHomMatrix aToTheUnitSquare(rChange.maOriginal);
        if (aToTheUnitSquare.invert())
        {
            const basegfx::B2DHomMatrix aOnTheSquare(aToTheUnitSquare * aMappingReached);
            const basegfx::B2DPoint aCorner(aOnTheSquare * basegfx::B2DPoint(0.0, 0.0));
            const basegfx::B2DPoint aAcross(aOnTheSquare * basegfx::B2DPoint(1.0, 0.0));
            const basegfx::B2DPoint aDown(aOnTheSquare * basegfx::B2DPoint(0.0, 1.0));
            aReached.maChange = basegfx::utils::createScaleTranslateB2DHomMatrix(
                aAcross.getX() - aCorner.getX(), aDown.getY() - aCorner.getY(), aCorner.getX(),
                aCorner.getY());
        }
    }

    // extract the old Rectangle structures
    basegfx::B2DRange aRangeOriginalNoShearNoRotate;
    basegfx::B2DRange aRangeNewNoShearNoRotate;
    cropWindowRanges(aReached, aRangeOriginalNoShearNoRotate, aRangeNewNoShearNoRotate);

    // The window's edges are taken as they are, not rounded, and the crop values are rounded
    // once at the end, so a narrow window keeps the scale its graphic is drawn at.
    if (basegfx::fTools::equalZero(aRangeOriginalNoShearNoRotate.getWidth())
        || basegfx::fTools::equalZero(aRangeOriginalNoShearNoRotate.getHeight()))
    {
        throw o3tl::divide_by_zero();
    }

    if((GetGraphicType() == GraphicType::NONE) || (GetGraphicType() == GraphicType::Default))
    {
        return false;
    }

    const GraphicObject& rGraphicObject(GetGraphicObject());
    // tdf#117145 Usually Writer will go the bExternal path (see above), but more correct for
    // the future is to use the MapMode from the SdrModel/SfxItemPool if the Writer's current
    // special handling should be unified to this path in the future. Usually it *should* be
    // MapUnit::Map100thMM, but better do not mix up Units.
    // Checked now what SwVirtFlyDrawObj::NbcCrop is doing - it calculates everything forced
    // to MapUnit::Map100thMM, but extracts/packs Twips to the used SdrGrafCropItem in Writer.
    const MapMode aMapModePool(getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
    Size aGraphicSize(rGraphicObject.GetPrefSize());

    if(MapUnit::MapPixel == rGraphicObject.GetPrefMapMode().GetMapUnit())
    {
        aGraphicSize = Application::GetDefaultDevice()->PixelToLogic(aGraphicSize, aMapModePool);
    }
    else
    {
        aGraphicSize = OutputDevice::LogicToLogic(aGraphicSize, rGraphicObject.GetPrefMapMode(),
                                                  aMapModePool);
    }

    if(0 == aGraphicSize.Width() || 0 == aGraphicSize.Height())
    {
        return false;
    }

    const SdrGrafCropItem& rOldCrop = GetMergedItem(SDRATTR_GRAFCROP);
    const basegfx::B2DRange& rOld(aRangeOriginalNoShearNoRotate);
    const basegfx::B2DRange& rNew(aRangeNewNoShearNoRotate);
    const double fScaleX
        = (aGraphicSize.Width() - rOldCrop.GetLeft() - rOldCrop.GetRight()) / rOld.getWidth();
    const double fScaleY
        = (aGraphicSize.Height() - rOldCrop.GetTop() - rOldCrop.GetBottom()) / rOld.getHeight();

    double fDiffLeft = rNew.getMinX() - rOld.getMinX();
    const double fDiffTop = rNew.getMinY() - rOld.getMinY();
    double fDiffRight = rNew.getMaxX() - rOld.getMaxX();
    const double fDiffBottom = rNew.getMaxY() - rOld.getMaxY();

    if(IsMirrored())
    {
        // mirrored X or Y, for old stuff, exchange X
        // check for aw080
        const double fTmp(fDiffLeft);
        fDiffLeft = -fDiffRight;
        fDiffRight = -fTmp;
    }

    const sal_Int32 nLeftCrop = basegfx::fround( rOldCrop.GetLeft() + fDiffLeft * fScaleX );
    const sal_Int32 nTopCrop = basegfx::fround( rOldCrop.GetTop() + fDiffTop * fScaleY );
    const sal_Int32 nRightCrop = basegfx::fround( rOldCrop.GetRight() - fDiffRight * fScaleX );
    const sal_Int32 nBottomCrop = basegfx::fround( rOldCrop.GetBottom() - fDiffBottom * fScaleY );

    SetMergedItem( SdrGrafCropItem( nLeftCrop, nTopCrop, nRightCrop, nBottomCrop ) );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
