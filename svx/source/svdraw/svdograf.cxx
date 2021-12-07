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

#include <unotools/streamwrap.hxx>

#include <sfx2/lnkbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/linkmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
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
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

class SdrGraphicLink : public sfx2::SvBaseLink
{
    SdrGrafObj&         rGrafObj;

public:
    explicit            SdrGraphicLink(SdrGrafObj& rObj);

    virtual void        Closed() override;

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    void                Connect() { GetRealObject(); }
};

SdrGraphicLink::SdrGraphicLink(SdrGrafObj& rObj)
: ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB )
, rGrafObj( rObj )
{
    SetSynchron( false );
}

::sfx2::SvBaseLink::UpdateResult SdrGraphicLink::DataChanged(
    const OUString& rMimeType, const css::uno::Any & rValue )
{
    SdrModel& rModel(rGrafObj.getSdrModelFromSdrObject());
    sfx2::LinkManager* pLinkManager(rModel.GetLinkManager());

    if( pLinkManager && rValue.hasValue() )
    {
        sfx2::LinkManager::GetDisplayNames( this, nullptr, &rGrafObj.aFileName, nullptr, &rGrafObj.aFilterName );

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
    rGrafObj.pGraphicLink=nullptr;
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

    const std::deque<css::uno::Reference<css::graphic::XPrimitive2D>>& rContainer(rVectorGraphicDataPtr->getPrimitive2DSequence());

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
    ,pGraphicLink(nullptr)
    ,bMirrored(false)
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
    ,pGraphicLink(nullptr)
{
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    mbNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;

    aFileName = rSource.aFileName;
    bMirrored = rSource.bMirrored;

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
        mpBarCode = std::make_unique<css::drawing::BarCode>(*rSource.mpBarCode);
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
        SetGraphicLink( aFileName );
    }

    ImpSetAttrToGrafInfo();
}

SdrGrafObj::SdrGrafObj(
    SdrModel& rSdrModel,
    const Graphic& rGraphic,
    const tools::Rectangle& rRect)
:   SdrRectObj(rSdrModel, rRect)
    ,mpGraphicObject(new GraphicObject(rGraphic))
    ,pGraphicLink(nullptr)
    ,bMirrored(false)
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
    ,pGraphicLink(nullptr)
    ,bMirrored(false)
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
            const_cast<SdrGrafObj*>(this)->mpReplacementGraphicObject.reset(new GraphicObject(mpGraphicObject->GetGraphic().GetBitmapEx()));
        }
    }

    return mpReplacementGraphicObject.get();
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
        aFileName = rGraphic.getOriginURL();
        aFilterName = "";
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

Graphic SdrGrafObj::GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags ) const
{
    // Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality
    MapMode aDestMap(
        getSdrModelFromSdrObject().GetScaleUnit(),
        Point(),
        getSdrModelFromSdrObject().GetScaleFraction(),
        getSdrModelFromSdrObject().GetScaleFraction());
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
            (maGeo.nRotationAngle && maGeo.nRotationAngle != 18000_deg100);

        // Need cropping info earlier
        const_cast<SdrGrafObj*>(this)->ImpSetAttrToGrafInfo();

        // Actually transform the graphic only in this case.
        // Cropping always happens, though.
        aActAttr = aGrafInfo;

        if( bMirror )
        {
            sal_uInt16      nMirrorCase = ( maGeo.nRotationAngle == 18000_deg100 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
            bool bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            bool bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BmpMirrorFlags::Horizontal : BmpMirrorFlags::NONE ) | ( bVMirr ? BmpMirrorFlags::Vertical : BmpMirrorFlags::NONE ) );
        }

        if( bRotate )
            aActAttr.SetRotation( to<Degree10>(maGeo.nRotationAngle ) );
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

    if (aGrafInfo.IsCropped())
    {
        const tools::Long aCroppedTop(OutputDevice::LogicToLogic(aGrafInfo.GetTopCrop(), getSdrModelFromSdrObject().GetScaleUnit(), GetGrafPrefMapMode().GetMapUnit()));
        const tools::Long aCroppedBottom(OutputDevice::LogicToLogic(aGrafInfo.GetBottomCrop(), getSdrModelFromSdrObject().GetScaleUnit(), GetGrafPrefMapMode().GetMapUnit()));
        const tools::Long aCroppedLeft(OutputDevice::LogicToLogic(aGrafInfo.GetLeftCrop(), getSdrModelFromSdrObject().GetScaleUnit(), GetGrafPrefMapMode().GetMapUnit()));
        const tools::Long aCroppedRight(OutputDevice::LogicToLogic(aGrafInfo.GetRightCrop(), getSdrModelFromSdrObject().GetScaleUnit(), GetGrafPrefMapMode().GetMapUnit()));
        const tools::Long aCroppedWidth(aSize.getWidth() - aCroppedLeft + aCroppedRight);
        const tools::Long aCroppedHeight(aSize.getHeight() - aCroppedTop + aCroppedBottom);

        aSize = Size ( aCroppedWidth, aCroppedHeight);
    }

    if ( GetGrafPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
        aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, MapMode(getSdrModelFromSdrObject().GetScaleUnit()));
    else
        aSize = OutputDevice::LogicToLogic(aSize, GetGrafPrefMapMode(), MapMode(getSdrModelFromSdrObject().GetScaleUnit()));

    return aSize;
}

// TODO Remove
void SdrGrafObj::ForceSwapIn() const
{
    if (pGraphicLink && (mpGraphicObject->GetType() == GraphicType::NONE  ||
                         mpGraphicObject->GetType() == GraphicType::Default) )
    {
        pGraphicLink->Update();
    }
}

void SdrGrafObj::ImpRegisterLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrObject().GetLinkManager());

    if( pLinkManager != nullptr && pGraphicLink == nullptr )
    {
        if (!aFileName.isEmpty())
        {
            pGraphicLink = new SdrGraphicLink( *this );
            pLinkManager->InsertFileLink(
                *pGraphicLink, sfx2::SvBaseLinkObjectType::ClientGraphic, aFileName, (aFilterName.isEmpty() ? nullptr : &aFilterName));
            pGraphicLink->Connect();
        }
    }
}

void SdrGrafObj::ImpDeregisterLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrObject().GetLinkManager());

    if( pLinkManager != nullptr && pGraphicLink!=nullptr)
    {
        // When using Remove, the *pGraphicLink is implicitly deleted
        pLinkManager->Remove( pGraphicLink );
        pGraphicLink=nullptr;
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
    aFileName.clear();
    aFilterName.clear();

    auto aGraphic = mpGraphicObject->GetGraphic();
    aGraphic.setOriginURL("");
    SetGraphic(aGraphic);
}

bool SdrGrafObj::IsLinkedGraphic() const
{
    return !mpGraphicObject->GetGraphic().getOriginURL().isEmpty();
}

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoPresGrf = ( mpGraphicObject->GetType() != GraphicType::NONE ) && !m_bEmptyPresObj;

    rInfo.bResizeFreeAllowed = maGeo.nRotationAngle.get() % 9000 == 0 ||
                               maGeo.nRotationAngle.get() % 18000 == 0 ||
                               maGeo.nRotationAngle.get() % 27000 == 0;

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
    return OBJ_GRAF;
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
        sName.append(" '");
        sName.append(aName);
        sName.append('\'' );
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
        sName.append(" '");
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

SdrObjectUniquePtr SdrGrafObj::getFullDragClone() const
{
    // call parent
    SdrObjectUniquePtr pRetval = SdrRectObj::getFullDragClone();

    // #i103116# the full drag clone leads to problems
    // with linked graphics, so reset the link in this
    // temporary interaction object and load graphic
    if(pRetval && IsLinkedGraphic())
    {
        static_cast< SdrGrafObj* >(pRetval.get())->ReleaseGraphicLink();
    }

    return pRetval;
}

SdrGrafObj* SdrGrafObj::CloneSdrObject(SdrModel& rTargetModel) const
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

void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize( rRef, xFact, yFact );

    bool bMirrX = xFact.GetNumerator() < 0;
    bool bMirrY = yFact.GetNumerator() < 0;

    if( bMirrX != bMirrY )
        bMirrored = !bMirrored;
}

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrRectObj::NbcMirror(rRef1,rRef2);
    bMirrored = !bMirrored;
}

std::unique_ptr<SdrObjGeoData> SdrGrafObj::NewGeoData() const
{
    return std::make_unique<SdrGrafObjGeoData>();
}

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=static_cast<SdrGrafObjGeoData&>(rGeo);
    rGGeo.bMirrored=bMirrored;
}

void SdrGrafObj::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestoreGeoData(rGeo);
    const SdrGrafObjGeoData& rGGeo=static_cast<const SdrGrafObjGeoData&>(rGeo);
    bMirrored=rGGeo.bMirrored;
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

        if( pGraphicLink != nullptr )
            ImpDeregisterLink();
    }

    // call parent
    SdrRectObj::handlePageChange(pOldPage, pNewPage);

    if (!aFileName.isEmpty() && bInsert)
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
        const MapMode aMap(
            getSdrModelFromSdrObject().GetScaleUnit(),
            Point(),
            getSdrModelFromSdrObject().GetScaleFraction(),
            getSdrModelFromSdrObject().GetScaleFraction());

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

SdrObjectUniquePtr SdrGrafObj::DoConvertToPolyObj(bool bBezier, bool bAddText ) const
{
    SdrObject* pRetval = nullptr;
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
                maRect);
            SdrObjGroup* pGrp = new SdrObjGroup(getSdrModelFromSdrObject());

            if(aFilter.DoImport(aMtf, *pGrp->GetSubList(), 0))
            {
                {
                        // copy transformation
                    GeoStat aGeoStat(GetGeoStat());

                    if(aGeoStat.nShearAngle)
                    {
                        aGeoStat.RecalcTan();
                        pGrp->NbcShear(maRect.TopLeft(), aGeoStat.nShearAngle, aGeoStat.mfTanShearAngle, false);
                    }

                    if(aGeoStat.nRotationAngle)
                    {
                        aGeoStat.RecalcSinCos();
                        pGrp->NbcRotate(maRect.TopLeft(), aGeoStat.nRotationAngle, aGeoStat.mfSinRotationAngle, aGeoStat.mfCosRotationAngle);
                    }
                }

                pRetval = pGrp;
                pGrp->NbcSetLayer(GetLayer());

                if(bAddText)
                {
                    pRetval = ImpConvertAddText(SdrObjectUniquePtr(pRetval), bBezier).release();
                }

                // convert all children
                if( pRetval )
                {
                    SdrObject* pHalfDone = pRetval;
                    pRetval = pRetval->DoConvertToPolyObj(bBezier, bAddText).release();
                    SdrObject::Free( pHalfDone ); // resulting object is newly created

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
            {
                // always use SdrObject::Free(...) for SdrObjects (!)
                SdrObject* pTemp(pGrp);
                SdrObject::Free(pTemp);
            }

            // #i118485# convert line and fill
            SdrObjectUniquePtr pLineFill = SdrRectObj::DoConvertToPolyObj(bBezier, false);

            if(pLineFill)
            {
                if(pRetval)
                {
                    pGrp = dynamic_cast< SdrObjGroup* >(pRetval);

                    if(!pGrp)
                    {
                        pGrp = new SdrObjGroup(getSdrModelFromSdrObject());
                        pGrp->NbcSetLayer(GetLayer());
                        pGrp->GetSubList()->NbcInsertObject(pRetval);
                    }

                    pGrp->GetSubList()->NbcInsertObject(pLineFill.release(), 0);
                }
                else
                {
                    pRetval = pLineFill.release();
                }
            }

            break;
        }
        case GraphicType::Bitmap:
        {
            // create basic object and add fill
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText).release();

            // save bitmap as an attribute
            if(pRetval)
            {
                // retrieve bitmap for the fill
                SfxItemSet aSet(GetObjectItemSet());

                aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                const BitmapEx aBitmapEx(GetTransformedGraphic().GetBitmapEx());
                aSet.Put(XFillBitmapItem(OUString(), Graphic(aBitmapEx)));
                aSet.Put(XFillBmpTileItem(false));

                pRetval->SetMergedItemSet(aSet);
            }
            break;
        }
        case GraphicType::NONE:
        case GraphicType::Default:
        {
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText).release();
            break;
        }
    }

    return SdrObjectUniquePtr(pRetval);
}

void SdrGrafObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SetXPolyDirty();
    SdrRectObj::Notify( rBC, rHint );
    ImpSetAttrToGrafInfo();
}


void SdrGrafObj::SetMirrored( bool _bMirrored )
{
    bMirrored = _bMirrored;
}

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const sal_uInt16 nTrans = rSet.Get( SDRATTR_GRAFTRANSPARENCE ).GetValue();
    const SdrGrafCropItem&  rCrop = rSet.Get( SDRATTR_GRAFCROP );

    aGrafInfo.SetLuminance( rSet.Get( SDRATTR_GRAFLUMINANCE ).GetValue() );
    aGrafInfo.SetContrast( rSet.Get( SDRATTR_GRAFCONTRAST ).GetValue() );
    aGrafInfo.SetChannelR( rSet.Get( SDRATTR_GRAFRED ).GetValue() );
    aGrafInfo.SetChannelG( rSet.Get( SDRATTR_GRAFGREEN ).GetValue() );
    aGrafInfo.SetChannelB( rSet.Get( SDRATTR_GRAFBLUE ).GetValue() );
    aGrafInfo.SetGamma( rSet.Get( SDRATTR_GRAFGAMMA ).GetValue() * 0.01 );
    aGrafInfo.SetAlpha( 255 - static_cast<sal_uInt8>(FRound( std::min( nTrans, sal_uInt16(100) )  * 2.55 )) );
    aGrafInfo.SetInvert( rSet.Get( SDRATTR_GRAFINVERT ).GetValue() );
    aGrafInfo.SetDrawMode( rSet.Get( SDRATTR_GRAFMODE ).GetValue() );
    aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

    SetXPolyDirty();
    SetBoundAndSnapRectsDirty();
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
        aPos = maRect.TopLeft();

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

Reference< XInputStream > SdrGrafObj::getInputStream() const
{
    Reference< XInputStream > xStream;

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

    if (!xStream.is() && !aFileName.isEmpty())
    {
        SvFileStream* pStream = new SvFileStream( aFileName, StreamMode::READ );
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

        if(!basegfx::fTools::equalZero(fShearX))
        {
            // shearX is used, correct it
            fShearX = -fShearX;
        }

        aMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
            aScale,
            fShearX,
            fRotate,
            aTranslate);
    }

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

            rTarget.AddHdl(
                std::make_unique<SdrCropViewHdl>(
                    aMatrixForCropViewHdl,
                    GetGraphicObject().GetGraphic(),
                    fCropLeft,
                    fCropTop,
                    fCropRight,
                    fCropBottom));
        }
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
