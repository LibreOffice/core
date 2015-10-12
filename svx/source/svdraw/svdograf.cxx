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
#include <math.h>
#include <tools/helpers.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <svl/style.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/urihelper.hxx>
#include <svtools/grfmgr.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdetc.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdviter.hxx"
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/svdundo.hxx>
#include "svdfmtf.hxx"
#include <svx/sdgcpitm.hxx>
#include <editeng/eeitem.hxx>
#include <sdr/properties/graphicproperties.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/thread.hxx>
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

const Graphic ImpLoadLinkedGraphic( const OUString& aFileName, const OUString& aReferer, const OUString& aFilterName )
{
    Graphic aGraphic;

    SfxMedium xMed( aFileName, aReferer, STREAM_STD_READ );
    xMed.Download();

    SvStream* pInStrm = xMed.GetInStream();
    if ( pInStrm )
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();

        const sal_uInt16 nFilter = !aFilterName.isEmpty() && rGF.GetImportFormatCount()
            ? rGF.GetImportFormatNumber( aFilterName )
            : GRFILTER_FORMAT_DONTKNOW;

        css::uno::Sequence< css::beans::PropertyValue > aFilterData( 1 );

        // TODO: Room for improvement:
        // As this is a linked graphic the GfxLink is not needed if saving/loading our own format.
        // But this link is required by some filters to access the native graphic (PDF export/MS export),
        // there we should create a new service to provide this data if needed
        aFilterData[ 0 ].Name = "CreateNativeLink";
        aFilterData[ 0 ].Value = Any( sal_True );

        // #i123042# for e.g SVG the path is needed, so hand it over here. I have no real idea
        // what consequences this may have; maybe this is not handed over by purpose here. Not
        // handing it over means that any GraphicFormat that internallv needs a path as base
        // to interpret included links may fail.
        // Alternatively the path may be set at the result after this call when it is known
        // that it is a SVG graphic, but only because no one yet tried to interpret it.
        rGF.ImportGraphic( aGraphic, aFileName, *pInStrm, nFilter, NULL, GraphicFilterImportFlags::NONE, &aFilterData );
    }
    return aGraphic;
}

class SdrGraphicUpdater;
class SdrGraphicLink : public sfx2::SvBaseLink
{
    SdrGrafObj&         rGrafObj;
    SdrGraphicUpdater*  pGraphicUpdater;

public:
    explicit            SdrGraphicLink(SdrGrafObj& rObj);
    virtual             ~SdrGraphicLink();

    virtual void        Closed() override;

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue ) override;
    void                DataChanged( const Graphic& rGraphic );

    bool                Connect() { return 0 != GetRealObject(); }
    void                UpdateAsynchron();
    void                RemoveGraphicUpdater();

    OUString getReferer() const { return rGrafObj.aReferer; }
};

class SdrGraphicUpdater : public ::osl::Thread
{
public:
    SdrGraphicUpdater( const OUString& rFileName, const OUString& rFilterName, SdrGraphicLink& );
    virtual ~SdrGraphicUpdater();

    void SAL_CALL Terminate();

    bool GraphicLinkChanged( const OUString& rFileName ){ return maFileName != rFileName;    };

protected:

    /** is called from the inherited create method and acts as the
        main function of this thread.
    */
    virtual void SAL_CALL run() override;

    /** Called after the thread is terminated via the terminate
        method.  Used to kill the thread by calling delete on this.
    */
    virtual void SAL_CALL onTerminated() override;

private:

    const OUString  maFileName;
    const OUString  maFilterName;
    SdrGraphicLink& mrGraphicLink;

    volatile bool   mbIsTerminated;
};

SdrGraphicUpdater::SdrGraphicUpdater( const OUString& rFileName, const OUString& rFilterName, SdrGraphicLink& rGraphicLink )
: maFileName( rFileName )
, maFilterName( rFilterName )
, mrGraphicLink( rGraphicLink )
, mbIsTerminated( false )
{
    create();
}

SdrGraphicUpdater::~SdrGraphicUpdater()
{
}

void SdrGraphicUpdater::Terminate()
{
    mbIsTerminated = true;
}

void SAL_CALL SdrGraphicUpdater::onTerminated()
{
    delete this;
}

void SAL_CALL SdrGraphicUpdater::run()
{
    osl_setThreadName("SdrGraphicUpdater");

    Graphic aGraphic( ImpLoadLinkedGraphic( maFileName, mrGraphicLink.getReferer(), maFilterName ) );
    SolarMutexGuard aSolarGuard;
    if ( !mbIsTerminated )
    {
        mrGraphicLink.DataChanged( aGraphic );
        mrGraphicLink.RemoveGraphicUpdater();
    }
}

SdrGraphicLink::SdrGraphicLink(SdrGrafObj& rObj)
: ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB )
, rGrafObj( rObj )
, pGraphicUpdater( NULL )
{
    SetSynchron( false );
}

SdrGraphicLink::~SdrGraphicLink()
{
    if ( pGraphicUpdater )
        pGraphicUpdater->Terminate();
}

void SdrGraphicLink::DataChanged( const Graphic& rGraphic )
{
    rGrafObj.ImpSetLinkedGraphic( rGraphic );
}

void SdrGraphicLink::RemoveGraphicUpdater()
{
    pGraphicUpdater = NULL;
}

::sfx2::SvBaseLink::UpdateResult SdrGraphicLink::DataChanged(
    const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue )
{
    SdrModel*       pModel      = rGrafObj.GetModel();
    sfx2::LinkManager* pLinkManager= pModel  ? pModel->GetLinkManager() : 0;

    if( pLinkManager && rValue.hasValue() )
    {
        sfx2::LinkManager::GetDisplayNames( this, 0, &rGrafObj.aFileName, 0, &rGrafObj.aFilterName );

        Graphic aGraphic;
        if( sfx2::LinkManager::GetGraphicFromAny( rMimeType, rValue, aGraphic ))
        {
               rGrafObj.NbcSetGraphic( aGraphic );
            rGrafObj.ActionChanged();
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
    rGrafObj.pGraphicLink=NULL;
    rGrafObj.ReleaseGraphicLink();
    SvBaseLink::Closed();
}

void SdrGraphicLink::UpdateAsynchron()
{
    if( GetObj() )
    {
        if ( pGraphicUpdater )
        {
            if ( pGraphicUpdater->GraphicLinkChanged( rGrafObj.GetFileName() ) )
            {
                pGraphicUpdater->Terminate();
                pGraphicUpdater = new SdrGraphicUpdater( rGrafObj.GetFileName(), rGrafObj.GetFilterName(), *this );
            }
        }
        else
            pGraphicUpdater = new SdrGraphicUpdater( rGrafObj.GetFileName(), rGrafObj.GetFilterName(), *this );
    }
}

sdr::properties::BaseProperties* SdrGrafObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::GraphicProperties(*this);
}


// DrawContact section

sdr::contact::ViewContact* SdrGrafObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGraphic(*this);
}


// check if SVG and if try to get ObjectInfoPrimitive2D and extract info

void SdrGrafObj::onGraphicChanged()
{
    if (!pGraphic || pGraphic->IsSwappedOut()) // don't force swap-in for this
        return;

    const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

    if (!rSvgDataPtr.get())
        return;

    const drawinglayer::primitive2d::Primitive2DSequence aSequence(rSvgDataPtr->getPrimitive2DSequence());

    if (!aSequence.hasElements())
        return;

    drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    drawinglayer::processor2d::ObjectInfoPrimitiveExtractor2D aProcessor(aViewInformation2D);

    aProcessor.process(aSequence);

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

TYPEINIT1(SdrGrafObj,SdrRectObj);

SdrGrafObj::SdrGrafObj()
:   SdrRectObj(),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject;
    mpReplacementGraphic = 0;
    pGraphic->SetSwapStreamHdl( LINK(this, SdrGrafObj, ImpSwapHdl) );
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    bNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect)
:   SdrRectObj      ( rRect ),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject( rGrf );
    mpReplacementGraphic = 0;
    pGraphic->SetSwapStreamHdl( LINK(this, SdrGrafObj, ImpSwapHdl) );
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    bNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::SdrGrafObj( const Graphic& rGrf )
:   SdrRectObj(),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject( rGrf );
    mpReplacementGraphic = 0;
    pGraphic->SetSwapStreamHdl( LINK(this, SdrGrafObj, ImpSwapHdl) );
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    bNoShear = false;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

SdrGrafObj::~SdrGrafObj()
{
    delete pGraphic;
    delete mpReplacementGraphic;
    ImpLinkAbmeldung();
}

void SdrGrafObj::SetGraphicObject( const GraphicObject& rGrfObj )
{
    *pGraphic = rGrfObj;
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;
    pGraphic->SetSwapStreamHdl( LINK(this, SdrGrafObj, ImpSwapHdl) );
    pGraphic->SetUserData();
    mbIsPreview = false;
    SetChanged();
    BroadcastObjectChange();
    onGraphicChanged();
}

const GraphicObject& SdrGrafObj::GetGraphicObject(bool bForceSwapIn) const
{
    if(bForceSwapIn)
    {
        ForceSwapIn();
    }

    return *pGraphic;
}

const GraphicObject* SdrGrafObj::GetReplacementGraphicObject() const
{
    if(!mpReplacementGraphic && pGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            const_cast< SdrGrafObj* >(this)->mpReplacementGraphic = new GraphicObject(rSvgDataPtr->getReplacement());
        }
    }

    return mpReplacementGraphic;
}

void SdrGrafObj::NbcSetGraphic( const Graphic& rGrf )
{
    pGraphic->SetGraphic( rGrf );
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;
    pGraphic->SetUserData();
    mbIsPreview = false;
    onGraphicChanged();
}

void SdrGrafObj::SetGraphic( const Graphic& rGrf )
{
    NbcSetGraphic(rGrf);
    SetChanged();
    BroadcastObjectChange();
}

const Graphic& SdrGrafObj::GetGraphic() const
{
    ForceSwapIn();
    return pGraphic->GetGraphic();
}

Graphic SdrGrafObj::GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags ) const
{
    // Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality

    GraphicType     eType = GetGraphicType();
    MapMode         aDestMap( pModel->GetScaleUnit(), Point(), pModel->GetScaleFraction(), pModel->GetScaleFraction() );
    const Size      aDestSize( GetLogicRect().GetSize() );
    const bool      bMirror = bool( nTransformFlags & SdrGrafObjTransformsAttrs::MIRROR );
    const bool      bRotate = bool( nTransformFlags & SdrGrafObjTransformsAttrs::ROTATE ) &&
        ( aGeo.nRotationAngle && aGeo.nRotationAngle != 18000 ) && ( GRAPHIC_NONE != eType );

    // Need cropping info earlier
    const_cast<SdrGrafObj*>(this)->ImpSetAttrToGrafInfo();
    GraphicAttr aActAttr;

    if( SdrGrafObjTransformsAttrs::NONE != nTransformFlags &&
        GRAPHIC_NONE != eType )
    {
        // Actually transform the graphic only in this case.
        // Cropping always happens, though.
        aActAttr = aGrafInfo;

        if( bMirror )
        {
            sal_uInt16      nMirrorCase = ( aGeo.nRotationAngle == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
            bool bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            bool bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BmpMirrorFlags::Horizontal : BmpMirrorFlags::NONE ) | ( bVMirr ? BmpMirrorFlags::Vertical : BmpMirrorFlags::NONE ) );
        }

        if( bRotate )
            aActAttr.SetRotation( sal_uInt16(aGeo.nRotationAngle / 10) );
    }

    // Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

GraphicType SdrGrafObj::GetGraphicType() const
{
    return pGraphic->GetType();
}

bool SdrGrafObj::IsAnimated() const
{
    return pGraphic->IsAnimated();
}

bool SdrGrafObj::IsEPS() const
{
    return pGraphic->IsEPS();
}

bool SdrGrafObj::IsSwappedOut() const
{
    return mbIsPreview || pGraphic->IsSwappedOut();
}

const MapMode& SdrGrafObj::GetGrafPrefMapMode() const
{
    return pGraphic->GetPrefMapMode();
}

const Size& SdrGrafObj::GetGrafPrefSize() const
{
    return pGraphic->GetPrefSize();
}

void SdrGrafObj::SetGrafStreamURL( const OUString& rGraphicStreamURL )
{
    mbIsPreview = false;
    if( rGraphicStreamURL.isEmpty() )
    {
        pGraphic->SetUserData();
    }
    else if( pModel->IsSwapGraphics() )
    {
        pGraphic->SetUserData( rGraphicStreamURL );
    }
}

OUString SdrGrafObj::GetGrafStreamURL() const
{
    return pGraphic->GetUserData();
}

void SdrGrafObj::ForceSwapIn() const
{
    if( mbIsPreview && pGraphic->HasUserData() )
    {
        // removing preview graphic
        const OUString aUserData( pGraphic->GetUserData() );

        Graphic aEmpty;
        pGraphic->SetGraphic( aEmpty );
        pGraphic->SetUserData( aUserData );

        const_cast< SdrGrafObj* >( this )->mbIsPreview = false;
    }
    if ( pGraphicLink && pGraphic->IsSwappedOut() )
        ImpUpdateGraphicLink( false );
    else
        pGraphic->FireSwapInRequest();

    if( pGraphic->IsSwappedOut() ||
        ( pGraphic->GetType() == GRAPHIC_NONE ) ||
        ( pGraphic->GetType() == GRAPHIC_DEFAULT ) )
    {
        Graphic aDefaultGraphic;
        aDefaultGraphic.SetDefaultType();
        pGraphic->SetGraphic( aDefaultGraphic );
    }
}

void SdrGrafObj::ImpLinkAnmeldung()
{
    sfx2::LinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink == NULL )
    {
        if (!aFileName.isEmpty())
        {
            pGraphicLink = new SdrGraphicLink( *this );
            pLinkManager->InsertFileLink(
                *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, (aFilterName.isEmpty() ? NULL : &aFilterName), NULL);
            pGraphicLink->Connect();
        }
    }
}

void SdrGrafObj::ImpLinkAbmeldung()
{
    sfx2::LinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink!=NULL)
    {
        // When using Remove, the *pGraphicLink is implicitly deleted
        pLinkManager->Remove( pGraphicLink );
        pGraphicLink=NULL;
    }
}

void SdrGrafObj::SetGraphicLink(const OUString& rFileName, const OUString& rReferer, const OUString& rFilterName)
{
    ImpLinkAbmeldung();
    aFileName = rFileName;
    aReferer = rReferer;
    aFilterName = rFilterName;
    ImpLinkAnmeldung();
    pGraphic->SetUserData();
    pGraphic->SetSwapState();
}

void SdrGrafObj::ReleaseGraphicLink()
{
    ImpLinkAbmeldung();
    aFileName.clear();
    aReferer.clear();
    aFilterName.clear();
}

bool SdrGrafObj::IsLinkedGraphic() const
{
    return !aFileName.isEmpty();
}



void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoPresGrf = ( pGraphic->GetType() != GRAPHIC_NONE ) && !bEmptyPresObj;

    rInfo.bResizeFreeAllowed = aGeo.nRotationAngle % 9000 == 0 ||
                               aGeo.nRotationAngle % 18000 == 0 ||
                               aGeo.nRotationAngle % 27000 == 0;

    rInfo.bResizePropAllowed = true;
    rInfo.bRotateFreeAllowed = bNoPresGrf;
    rInfo.bRotate90Allowed = bNoPresGrf;
    rInfo.bMirrorFreeAllowed = bNoPresGrf;
    rInfo.bMirror45Allowed = bNoPresGrf;
    rInfo.bMirror90Allowed = !bEmptyPresObj;
    rInfo.bTransparenceAllowed = false;
    rInfo.bGradientAllowed = false;

    // #i118485# Shear allowed and possible now
    rInfo.bShearAllowed = true;

    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath = !IsEPS();
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
    rInfo.bCanConvToPoly = !IsEPS();
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrGrafObj::GetObjIdentifier() const
{
    return sal_uInt16( OBJ_GRAF );
}

/* The graphic of the GraphicLink will be loaded. If it is called with
   bAsynchron = true then the graphic will be set later via DataChanged
*/
bool SdrGrafObj::ImpUpdateGraphicLink( bool bAsynchron ) const
{
    bool bRet = false;
    if( pGraphicLink )
    {
        if ( bAsynchron )
            pGraphicLink->UpdateAsynchron();
        else
            pGraphicLink->DataChanged( ImpLoadLinkedGraphic( aFileName, aReferer, aFilterName ) );
        bRet = true;
    }
    return bRet;
}

void SdrGrafObj::ImpSetLinkedGraphic( const Graphic& rGraphic )
{
    const bool bIsChanged = GetModel()->IsChanged();
    NbcSetGraphic( rGraphic );
    ActionChanged();
    BroadcastObjectChange();
    GetModel()->SetChanged( bIsChanged );
}

OUString SdrGrafObj::TakeObjNameSingul() const
{
    if (!pGraphic)
        return OUString();

    const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

    OUStringBuffer sName;

    if(rSvgDataPtr.get())
    {
        sName.append(ImpGetResStr(STR_ObjNameSingulGRAFSVG));
    }
    else
    {
        switch( pGraphic->GetType() )
        {
            case GRAPHIC_BITMAP:
            {
                const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || static_cast<const SdrGrafTransparenceItem&>( GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                     ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPTRANSLNK : STR_ObjNameSingulGRAFBMPTRANS ) :
                                     ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPLNK : STR_ObjNameSingulGRAFBMP ) );

                sName.append(ImpGetResStr(nId));
            }
            break;

            case GRAPHIC_GDIMETAFILE:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNameSingulGRAFMTFLNK : STR_ObjNameSingulGRAFMTF));
            break;

            case GRAPHIC_NONE:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNameSingulGRAFNONELNK : STR_ObjNameSingulGRAFNONE));
            break;

            default:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNameSingulGRAFLNK : STR_ObjNameSingulGRAF));
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
    if(!pGraphic)
        return OUString();

    const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

    OUStringBuffer sName;

    if(rSvgDataPtr.get())
    {
        sName.append(ImpGetResStr(STR_ObjNamePluralGRAFSVG));
    }
    else
    {
        switch( pGraphic->GetType() )
        {
            case GRAPHIC_BITMAP:
            {
                const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || static_cast<const SdrGrafTransparenceItem&>( GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                     ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPTRANSLNK : STR_ObjNamePluralGRAFBMPTRANS ) :
                                     ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPLNK : STR_ObjNamePluralGRAFBMP ) );

                sName.append(ImpGetResStr(nId));
            }
            break;

            case GRAPHIC_GDIMETAFILE:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNamePluralGRAFMTFLNK : STR_ObjNamePluralGRAFMTF));
            break;

            case GRAPHIC_NONE:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNamePluralGRAFNONELNK : STR_ObjNamePluralGRAFNONE));
            break;

            default:
                sName.append(ImpGetResStr(IsLinkedGraphic() ? STR_ObjNamePluralGRAFLNK : STR_ObjNamePluralGRAF));
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

SdrObject* SdrGrafObj::getFullDragClone() const
{
    // call parent
    SdrGrafObj* pRetval = static_cast< SdrGrafObj* >(SdrRectObj::getFullDragClone());

    // #i103116# the full drag clone leads to problems
    // with linked graphics, so reset the link in this
    // temporary interaction object and load graphic
    if(pRetval && IsLinkedGraphic())
    {
        pRetval->ForceSwapIn();
        pRetval->ReleaseGraphicLink();
    }

    return pRetval;
}

SdrGrafObj* SdrGrafObj::Clone() const
{
    return CloneHelper< SdrGrafObj >();
}

SdrGrafObj& SdrGrafObj::operator=( const SdrGrafObj& rObj )
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator=( rObj );

    pGraphic->SetGraphic( rObj.GetGraphic(), &rObj.GetGraphicObject() );
    aFileName = rObj.aFileName;
    aFilterName = rObj.aFilterName;
    bMirrored = rObj.bMirrored;

    if( rObj.IsLinkedGraphic() )
    {
        SetGraphicLink( aFileName, rObj.aReferer, aFilterName );
    }

    ImpSetAttrToGrafInfo();
    return *this;
}

basegfx::B2DPolyPolygon SdrGrafObj::TakeXorPoly() const
{
    if(mbInsidePaint)
    {
        basegfx::B2DPolyPolygon aRetval;

        // take grown rectangle
        const sal_Int32 nHalfLineWidth(ImpGetLineWdt() / 2);
        const Rectangle aGrownRect(
            maRect.Left() - nHalfLineWidth,
            maRect.Top() - nHalfLineWidth,
            maRect.Right() + nHalfLineWidth,
            maRect.Bottom() + nHalfLineWidth);

        XPolygon aXPoly(ImpCalcXPoly(aGrownRect, GetEckenradius()));
        aRetval.append(aXPoly.getB2DPolygon());

        return aRetval;
    }
    else
    {
        // call parent
        return SdrRectObj::TakeXorPoly();
    }
}

sal_uInt32 SdrGrafObj::GetHdlCount() const
{
    return 8L;
}

SdrHdl* SdrGrafObj::GetHdl(sal_uInt32 nHdlNum) const
{
    return SdrRectObj::GetHdl( nHdlNum + 1L );
}

void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize( rRef, xFact, yFact );

    bool bMirrX = xFact.GetNumerator() < 0;
    bool bMirrY = yFact.GetNumerator() < 0;

    if( bMirrX != bMirrY )
        bMirrored = !bMirrored;
}

void SdrGrafObj::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    SdrRectObj::NbcRotate(rRef,nAngle,sn,cs);
}

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrRectObj::NbcMirror(rRef1,rRef2);
    bMirrored = !bMirrored;
}

void SdrGrafObj::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    // #i118485# Call Shear now, old version redirected to rotate
    SdrRectObj::NbcShear(rRef, nAngle, tn, bVShear);
}

void SdrGrafObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
}

void SdrGrafObj::NbcSetLogicRect( const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
}

SdrObjGeoData* SdrGrafObj::NewGeoData() const
{
    return new SdrGrafObjGeoData;
}

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=static_cast<SdrGrafObjGeoData&>(rGeo);
    rGGeo.bMirrored=bMirrored;
}

void SdrGrafObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    const SdrGrafObjGeoData& rGGeo=static_cast<const SdrGrafObjGeoData&>(rGeo);
    bMirrored=rGGeo.bMirrored;
}

void SdrGrafObj::SetPage( SdrPage* pNewPage )
{
    bool bRemove = pNewPage == NULL && pPage != NULL;
    bool bInsert = pNewPage != NULL && pPage == NULL;

    if( bRemove )
    {
        // No SwapIn necessary here, because if something's not loaded, it can't be animated either.
        if( pGraphic->IsAnimated())
            pGraphic->StopAnimation();

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    if(!pModel && !GetStyleSheet() && pNewPage && pNewPage->GetModel())
    {
        // #i119287# Set default StyleSheet for SdrGrafObj here, it is different from 'Default'. This
        // needs to be done before the style 'Default' is set from the :SetModel() call which is triggered
        // from the following :SetPage().
        // TTTT: Needs to be moved in branch aw080 due to having a SdrModel from the beginning, is at this
        // place for convenience currently (works in both versions, is not in the way)
        SfxStyleSheet* pSheet = pNewPage->GetModel()->GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();

        if(pSheet)
        {
            SetStyleSheet(pSheet, false);
        }
        else
        {
            SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
            SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
        }
    }

    SdrRectObj::SetPage( pNewPage );

    if (!aFileName.isEmpty() && bInsert)
        ImpLinkAnmeldung();
}

void SdrGrafObj::SetModel( SdrModel* pNewModel )
{
    bool bChg = pNewModel != pModel;

    if( bChg )
    {
        if( pGraphic->HasUserData() )
        {
            ForceSwapIn();
        }

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    // realize model
    SdrRectObj::SetModel(pNewModel);

    if (bChg && !aFileName.isEmpty())
        ImpLinkAnmeldung();
}

void SdrGrafObj::StartAnimation( OutputDevice* /*pOutDev*/, const Point& /*rPoint*/, const Size& /*rSize*/, long /*nExtraData*/)
{
    SetGrafAnimationAllowed(true);
}

bool SdrGrafObj::HasGDIMetaFile() const
{
    return( pGraphic->GetType() == GRAPHIC_GDIMETAFILE );
}

bool SdrGrafObj::isEmbeddedSvg() const
{
    return GRAPHIC_BITMAP == GetGraphicType() && GetGraphic().getSvgData().get();
}

GDIMetaFile SdrGrafObj::getMetafileFromEmbeddedSvg() const
{
    GDIMetaFile aRetval;

    if(isEmbeddedSvg() && GetModel())
    {
        ScopedVclPtrInstance< VirtualDevice > pOut;
        const Rectangle aBoundRect(GetCurrentBoundRect());
        const MapMode aMap(GetModel()->GetScaleUnit(), Point(), GetModel()->GetScaleFraction(), GetModel()->GetScaleFraction());

        pOut->EnableOutput(false);
        pOut->SetMapMode(aMap);
        aRetval.Record(pOut);
        SingleObjectPainter(*pOut.get());
        aRetval.Stop();
        aRetval.WindStart();
        aRetval.Move(-aBoundRect.Left(), -aBoundRect.Top());
        aRetval.SetPrefMapMode(aMap);
        aRetval.SetPrefSize(aBoundRect.GetSize());
    }

    return aRetval;
}

SdrObject* SdrGrafObj::DoConvertToPolyObj(bool bBezier, bool bAddText ) const
{
    SdrObject* pRetval = NULL;
    GraphicType aGraphicType(GetGraphicType());
    GDIMetaFile aMtf;

    if(isEmbeddedSvg())
    {
        // Embedded Svg
        // There is currently no helper to create SdrObjects from primitives (even if I'm thinking
        // about writing one for some time). To get the roundtrip to SdrObjects it is necessary to
        // use the old converter path over the MetaFile mechanism. Create Metafile from Svg
        // primitives here pretty directly
        aMtf = getMetafileFromEmbeddedSvg();
        aGraphicType = GRAPHIC_GDIMETAFILE;
    }
    else if(GRAPHIC_GDIMETAFILE == aGraphicType)
    {
        aMtf = GetTransformedGraphic(SdrGrafObjTransformsAttrs::COLOR|SdrGrafObjTransformsAttrs::MIRROR).GetGDIMetaFile();
    }

    switch(aGraphicType)
    {
        case GRAPHIC_GDIMETAFILE:
        {
            // Sort into group and return ONLY those objects that can be created from the MetaFile.
            ImpSdrGDIMetaFileImport aFilter(*GetModel(), GetLayer(), maRect);
            SdrObjGroup* pGrp = new SdrObjGroup();

            if(aFilter.DoImport(aMtf, *pGrp->GetSubList(), 0))
            {
                {
                        // copy transformation
                    GeoStat aGeoStat(GetGeoStat());

                    if(aGeoStat.nShearAngle)
                    {
                        aGeoStat.RecalcTan();
                        pGrp->NbcShear(maRect.TopLeft(), aGeoStat.nShearAngle, aGeoStat.nTan, false);
                    }

                    if(aGeoStat.nRotationAngle)
                    {
                        aGeoStat.RecalcSinCos();
                        pGrp->NbcRotate(maRect.TopLeft(), aGeoStat.nRotationAngle, aGeoStat.nSin, aGeoStat.nCos);
                    }
                }

                pRetval = pGrp;
                pGrp->NbcSetLayer(GetLayer());
                pGrp->SetModel(GetModel());

                if(bAddText)
                {
                    pRetval = ImpConvertAddText(pRetval, bBezier);
                }

                // convert all children
                if( pRetval )
                {
                    SdrObject* pHalfDone = pRetval;
                    pRetval = pHalfDone->DoConvertToPolyObj(bBezier, bAddText);
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
                delete pGrp;
            }

            // #i118485# convert line and fill
            SdrObject* pLineFill = SdrRectObj::DoConvertToPolyObj(bBezier, false);

            if(pLineFill)
            {
                if(pRetval)
                {
                    pGrp = dynamic_cast< SdrObjGroup* >(pRetval);

                    if(!pGrp)
                    {
                        pGrp = new SdrObjGroup();

                        pGrp->NbcSetLayer(GetLayer());
                        pGrp->SetModel(GetModel());
                        pGrp->GetSubList()->NbcInsertObject(pRetval);
                    }

                    pGrp->GetSubList()->NbcInsertObject(pLineFill, 0);
                }
                else
                {
                    pRetval = pLineFill;
                }
            }

            break;
        }
        case GRAPHIC_BITMAP:
        {
            // create basic object and add fill
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);

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
        case GRAPHIC_NONE:
        case GRAPHIC_DEFAULT:
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
    ImpSetAttrToGrafInfo();
}


void SdrGrafObj::SetMirrored( bool _bMirrored )
{
    bMirrored = _bMirrored;
}

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const sal_uInt16 nTrans = static_cast<const SdrGrafTransparenceItem&>( rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
    const SdrGrafCropItem&  rCrop = static_cast<const SdrGrafCropItem&>( rSet.Get( SDRATTR_GRAFCROP ) );

    aGrafInfo.SetLuminance( static_cast<const SdrGrafLuminanceItem&>( rSet.Get( SDRATTR_GRAFLUMINANCE ) ).GetValue() );
    aGrafInfo.SetContrast( static_cast<const SdrGrafContrastItem&>( rSet.Get( SDRATTR_GRAFCONTRAST ) ).GetValue() );
    aGrafInfo.SetChannelR( static_cast<const SdrGrafRedItem&>( rSet.Get( SDRATTR_GRAFRED ) ).GetValue() );
    aGrafInfo.SetChannelG( static_cast<const SdrGrafGreenItem&>( rSet.Get( SDRATTR_GRAFGREEN ) ).GetValue() );
    aGrafInfo.SetChannelB( static_cast<const SdrGrafBlueItem&>( rSet.Get( SDRATTR_GRAFBLUE ) ).GetValue() );
    aGrafInfo.SetGamma( static_cast<const SdrGrafGamma100Item&>( rSet.Get( SDRATTR_GRAFGAMMA ) ).GetValue() * 0.01 );
    aGrafInfo.SetTransparency( (sal_uInt8) FRound( std::min( nTrans, (sal_uInt16) 100 )  * 2.55 ) );
    aGrafInfo.SetInvert( static_cast<const SdrGrafInvertItem&>( rSet.Get( SDRATTR_GRAFINVERT ) ).GetValue() );
    aGrafInfo.SetDrawMode( static_cast<const SdrGrafModeItem&>( rSet.Get( SDRATTR_GRAFMODE ) ).GetValue() );
    aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

    SetXPolyDirty();
    SetRectsDirty();
}

void SdrGrafObj::AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly )
{
    Size aSize;
    Size aMaxSize( rMaxRect.GetSize() );
    if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        aSize = Application::GetDefaultDevice()->PixelToLogic( pGraphic->GetPrefSize(), MAP_100TH_MM );
    else
        aSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                            pGraphic->GetPrefMapMode(),
                                            MapMode( MAP_100TH_MM ) );

    if( aSize.Height() != 0 && aSize.Width() != 0 )
    {
        Point aPos( rMaxRect.TopLeft() );

        // if the graphic is too large, fit it to page
        if ( (!bShrinkOnly                          ||
             ( aSize.Height() > aMaxSize.Height() ) ||
             ( aSize.Width()  > aMaxSize.Width()  ) )&&
             aSize.Height() && aMaxSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aMaxSize.Width() /
                            (float)aMaxSize.Height();

            // Scale graphic to page size
            if ( fGrfWH < fWinWH )
            {
                aSize.Width() = (long)(aMaxSize.Height() * fGrfWH);
                aSize.Height()= aMaxSize.Height();
            }
            else if ( fGrfWH > 0.F )
            {
                aSize.Width() = aMaxSize.Width();
                aSize.Height()= (long)(aMaxSize.Width() / fGrfWH);
            }

            aPos = rMaxRect.Center();
        }

        if( bShrinkOnly )
            aPos = maRect.TopLeft();

        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        SetLogicRect( Rectangle( aPos, aSize ) );
    }
}

IMPL_LINK_TYPED( SdrGrafObj, ImpSwapHdl, const GraphicObject*, pO, SvStream* )
{
    SvStream* pRet = GRFMGR_AUTOSWAPSTREAM_NONE;

    if( pO->IsInSwapOut() )
    {
        if( pModel && !mbIsPreview && pModel->IsSwapGraphics() && pGraphic->GetSizeBytes() > 20480 )
        {
            // test if this object is visualized from someone
            // ## test only if there are VOCs other than the preview renderer
            if(!GetViewContact().HasViewObjectContacts(true))
            {
                const SdrSwapGraphicsMode nSwapMode = pModel->GetSwapGraphicsMode();

                if( ( pGraphicLink ) &&
                    ( nSwapMode & SdrSwapGraphicsMode::PURGE ) )
                {
                    pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
                }
                else if( nSwapMode & SdrSwapGraphicsMode::TEMP )
                {
                    pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
                    pGraphic->SetUserData();
                }

                // #i102380#
                sdr::contact::ViewContactOfGraphic* pVC = dynamic_cast< sdr::contact::ViewContactOfGraphic* >(&GetViewContact());

                if(pVC)
                {
                    pVC->flushGraphicObjects();
                }
            }
        }
    }
    else if( pO->IsInSwapIn() )
    {
        // can be loaded from the original document stream later
        if( pModel != NULL )
        {
            if( pGraphic->HasUserData() )
            {
                ::comphelper::LifecycleProxy proxy;
                OUString aUserData = pGraphic->GetUserData();
                uno::Reference<io::XInputStream> const xStream(
                    pModel->GetDocumentStream(aUserData, proxy));

                std::unique_ptr<SvStream> const pStream( (xStream.is())
                        ? ::utl::UcbStreamHelper::CreateStream(xStream)
                        : 0 );

                if( pStream != 0 )
                {
                    Graphic aGraphic;

                    std::unique_ptr<com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > pFilterData;

                    if(mbInsidePaint && !GetViewContact().HasViewObjectContacts(true))
                    {
                        pFilterData.reset(new com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >( 3 ));

                        const com::sun::star::awt::Size aPreviewSizeHint( 64, 64 );
                        const bool bAllowPartialStreamRead = true;
                        // create <GfxLink> instance also for previews in order to avoid that its corresponding
                        // data is cleared in the graphic cache entry in case that the preview data equals the complete graphic data
                        const bool bCreateNativeLink = true;
                        (*pFilterData)[ 0 ].Name = "PreviewSizeHint";
                        (*pFilterData)[ 0 ].Value <<= aPreviewSizeHint;
                        (*pFilterData)[ 1 ].Name = "AllowPartialStreamRead";
                        (*pFilterData)[ 1 ].Value <<= bAllowPartialStreamRead;
                        (*pFilterData)[ 2 ].Name = "CreateNativeLink";
                        (*pFilterData)[ 2 ].Value <<= bCreateNativeLink;

                        mbIsPreview = true;
                    }

                    if(!GraphicFilter::GetGraphicFilter().ImportGraphic(
                        aGraphic, aUserData, *pStream,
                        GRFILTER_FORMAT_DONTKNOW, NULL, GraphicFilterImportFlags::NONE, pFilterData.get()))
                    {
                        const OUString aNewUserData( pGraphic->GetUserData() );
                        pGraphic->SetGraphic( aGraphic );
                        if( mbIsPreview )
                        {
                            pGraphic->SetUserData(aNewUserData);
                        }
                        else
                        {
                            pGraphic->SetUserData();
                        }

                        // Graphic successfully swapped in.
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    pFilterData.reset();

                    pStream->ResetError();
                }
            }
            else if( !ImpUpdateGraphicLink( false ) )
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
            }
            else
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
            }
        }
        else
            pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
    }

    return pRet;
}

void SdrGrafObj::SetGrafAnimationAllowed(bool bNew)
{
    if(mbGrafAnimationAllowed != bNew)
    {
        mbGrafAnimationAllowed = bNew;
        ActionChanged();
    }
}

Reference< XInputStream > SdrGrafObj::getInputStream()
{
    Reference< XInputStream > xStream;

    if( pModel )
    {
        if( pGraphic && GetGraphic().IsLink() )
        {
            Graphic aGraphic( GetGraphic() );
            GfxLink aLink( aGraphic.GetLink() );
            sal_uInt32 nSize = aLink.GetDataSize();
            const void* pSourceData = static_cast<const void*>(aLink.GetData());
            if( nSize && pSourceData )
            {
                sal_uInt8 * pBuffer = new sal_uInt8[ nSize ];
                memcpy( pBuffer, pSourceData, nSize );

                SvMemoryStream* pStream = new SvMemoryStream( static_cast<void*>(pBuffer), (sal_Size)nSize, StreamMode::READ );
                pStream->ObjectOwnsMemory( true );
                xStream.set( new utl::OInputStreamWrapper( pStream, true ) );
            }
        }

        if (!xStream.is() && !aFileName.isEmpty())
        {
            SvFileStream* pStream = new SvFileStream( aFileName, StreamMode::READ );
            xStream.set( new utl::OInputStreamWrapper( pStream ) );
        }
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

        aMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
            aScale,
            fShearX,
            fRotate,
            aTranslate);
    }

    // get crop values
    const SdrGrafCropItem& rCrop = static_cast< const SdrGrafCropItem& >(GetMergedItem(SDRATTR_GRAFCROP));

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
                new SdrCropViewHdl(
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
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPLFT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 0.0);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPPER, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.0);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPRGT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 0.5);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LEFT , fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.5);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_RIGHT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 1.0);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LWLFT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 1.0);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LOWER, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 1.0);
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LWRGT, fShearX, fRotate));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
