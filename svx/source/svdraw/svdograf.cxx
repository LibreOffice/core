/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <svtools/filter.hxx>
#include <svl/urihelper.hxx>
#include <svtools/grfmgr.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdetc.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdviter.hxx"
#include <svx/svdview.hxx>
#include "svtools/filter.hxx"
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/svdundo.hxx>
#include "svdfmtf.hxx"
#include <svx/sdgcpitm.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/properties/graphicproperties.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/thread.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

// -----------
// - Defines -
// -----------

#define SWAPGRAPHIC_TIMEOUT     5000


// ------------------
// - SdrGraphicLink -
// ------------------


const Graphic ImpLoadLinkedGraphic( const String aFileName, const String aFilterName )
{
    Graphic aGraphic;

    SfxMedium xMed( aFileName, STREAM_STD_READ );
    xMed.DownLoad();

    SvStream* pInStrm = xMed.GetInStream();
    if ( pInStrm )
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();

        const sal_uInt16 nFilter = aFilterName.Len() && rGF.GetImportFormatCount()
            ? rGF.GetImportFormatNumber( aFilterName )
            : GRFILTER_FORMAT_DONTKNOW;

        String aEmptyStr;
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aFilterData( 1 );

        // TODO: Room for improvement:
        // As this is a linked graphic the GfxLink is not needed if saving/loading our own format.
        // But this link is required by some filters to access the native graphic (PDF export/MS export),
        // there we should create a new service to provide this data if needed
        aFilterData[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
        aFilterData[ 0 ].Value = Any( true );
        rGF.ImportGraphic( aGraphic, aEmptyStr, *pInStrm, nFilter, NULL, 0, &aFilterData );
    }
    return aGraphic;
}

class SdrGraphicUpdater;
class SdrGraphicLink : public sfx2::SvBaseLink
{
    SdrGrafObj*         pGrafObj;
    SdrGraphicUpdater*  pGraphicUpdater;

public:
                        SdrGraphicLink(SdrGrafObj* pObj);
    virtual             ~SdrGraphicLink();

    virtual void        Closed();

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );
    void                DataChanged( const Graphic& rGraphic );

    sal_Bool                Connect() { return 0 != GetRealObject(); }
    void                UpdateAsynchron();
    void                RemoveGraphicUpdater();
};

class SdrGraphicUpdater : public ::osl::Thread
{
public:
    SdrGraphicUpdater( const String& rFileName, const String& rFilterName, SdrGraphicLink& );
    virtual ~SdrGraphicUpdater( void );

    void SAL_CALL Terminate( void );

    sal_Bool GraphicLinkChanged( const String& rFileName ){ return maFileName != rFileName;    };

protected:

    /** is called from the inherited create method and acts as the
        main function of this thread.
    */
    virtual void SAL_CALL run(void);

    /** Called after the thread is terminated via the terminate
        method.  Used to kill the thread by calling delete on this.
    */
    virtual void SAL_CALL onTerminated(void);

private:

    const String    maFileName;
    const String    maFilterName;
    SdrGraphicLink& mrGraphicLink;

    volatile bool   mbIsTerminated;
};

SdrGraphicUpdater::SdrGraphicUpdater( const String& rFileName, const String& rFilterName, SdrGraphicLink& rGraphicLink )
: maFileName( rFileName )
, maFilterName( rFilterName )
, mrGraphicLink( rGraphicLink )
, mbIsTerminated( false )
{
    create();
}

SdrGraphicUpdater::~SdrGraphicUpdater( void )
{
}

void SdrGraphicUpdater::Terminate()
{
    mbIsTerminated = true;
}

void SAL_CALL SdrGraphicUpdater::onTerminated(void)
{
    delete this;
}

void SAL_CALL SdrGraphicUpdater::run(void)
{
    Graphic aGraphic( ImpLoadLinkedGraphic( maFileName, maFilterName ) );
    SolarMutexGuard aSolarGuard;
    if ( !mbIsTerminated )
    {
        mrGraphicLink.DataChanged( aGraphic );
        mrGraphicLink.RemoveGraphicUpdater();
    }
}

// -----------------------------------------------------------------------------

SdrGraphicLink::SdrGraphicLink(SdrGrafObj* pObj)
: ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB )
, pGrafObj( pObj )
, pGraphicUpdater( NULL )
{
    SetSynchron( false );
}

// -----------------------------------------------------------------------------

SdrGraphicLink::~SdrGraphicLink()
{
    if ( pGraphicUpdater )
        pGraphicUpdater->Terminate();
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::DataChanged( const Graphic& rGraphic )
{
    pGrafObj->ImpSetLinkedGraphic( rGraphic );
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::RemoveGraphicUpdater()
{
    pGraphicUpdater = NULL;
}

// -----------------------------------------------------------------------------

::sfx2::SvBaseLink::UpdateResult SdrGraphicLink::DataChanged(
    const String& rMimeType, const ::com::sun::star::uno::Any & rValue )
{
    SdrModel*       pModel      = pGrafObj ? pGrafObj->GetModel() : 0;
    sfx2::LinkManager* pLinkManager= pModel  ? pModel->GetLinkManager() : 0;

    if( pLinkManager && rValue.hasValue() )
    {
        pLinkManager->GetDisplayNames( this, 0, &pGrafObj->aFileName, 0, &pGrafObj->aFilterName );

        Graphic aGraphic;
        if( sfx2::LinkManager::GetGraphicFromAny( rMimeType, rValue, aGraphic ))
        {
               pGrafObj->NbcSetGraphic( aGraphic );
            pGrafObj->ActionChanged();
        }
        else if( SotExchange::GetFormatIdFromMimeType( rMimeType ) != sfx2::LinkManager::RegisterStatusInfoId() )
        {
            // broadcasting, to update slide sorter
            pGrafObj->BroadcastObjectChange();
        }
    }
    return SUCCESS;
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::Closed()
{
    // close connection; set pLink of the object to NULL, as link instance is just about getting destructed.
    pGrafObj->ForceSwapIn();
    pGrafObj->pGraphicLink=NULL;
    pGrafObj->ReleaseGraphicLink();
    SvBaseLink::Closed();
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::UpdateAsynchron()
{
    if( GetObj() )
    {
        if ( pGraphicUpdater )
        {
            if ( pGraphicUpdater->GraphicLinkChanged( pGrafObj->GetFileName() ) )
            {
                pGraphicUpdater->Terminate();
                pGraphicUpdater = new SdrGraphicUpdater( pGrafObj->GetFileName(), pGrafObj->GetFilterName(), *this );
            }
        }
        else
            pGraphicUpdater = new SdrGraphicUpdater( pGrafObj->GetFileName(), pGrafObj->GetFilterName(), *this );
    }
}

// --------------
// - SdrGrafObj -
// --------------

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrGrafObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::GraphicProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrGrafObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGraphic(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrGrafObj,SdrRectObj);

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj()
:   SdrRectObj(),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = true;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect)
:   SdrRectObj      ( rRect ),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = true;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj( const Graphic& rGrf )
:   SdrRectObj(),
    pGraphicLink    ( NULL ),
    bMirrored       ( false )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = true;

    mbGrafAnimationAllowed = true;

    // #i25616#
    mbLineIsOutsideGeometry = true;
    mbInsidePaint = false;
    mbIsPreview = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;
}

// -----------------------------------------------------------------------------

SdrGrafObj::~SdrGrafObj()
{
    delete pGraphic;
    ImpLinkAbmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicObject( const GraphicObject& rGrfObj )
{
    *pGraphic = rGrfObj;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    pGraphic->SetUserData();
    mbIsPreview = false;
    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

const GraphicObject& SdrGrafObj::GetGraphicObject(bool bForceSwapIn) const
{
    if(bForceSwapIn)
    {
        ForceSwapIn();
    }

    return *pGraphic;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetGraphic( const Graphic& rGrf )
{
    pGraphic->SetGraphic( rGrf );
    pGraphic->SetUserData();
    mbIsPreview = false;
}

void SdrGrafObj::SetGraphic( const Graphic& rGrf )
{
    NbcSetGraphic(rGrf);
    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

const Graphic& SdrGrafObj::GetGraphic() const
{
    ForceSwapIn();
    return pGraphic->GetGraphic();
}

// -----------------------------------------------------------------------------

Graphic SdrGrafObj::GetTransformedGraphic( sal_uIntPtr nTransformFlags ) const
{
    // Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality

    GraphicType     eType = GetGraphicType();
    MapMode         aDestMap( pModel->GetScaleUnit(), Point(), pModel->GetScaleFraction(), pModel->GetScaleFraction() );
    const Size      aDestSize( GetLogicRect().GetSize() );
    const sal_Bool      bMirror = ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_MIRROR ) != 0;
    const sal_Bool      bRotate = ( ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_ROTATE ) != 0 ) &&
        ( aGeo.nDrehWink && aGeo.nDrehWink != 18000 ) && ( GRAPHIC_NONE != eType );

    // Need cropping info earlier
    ( (SdrGrafObj*) this )->ImpSetAttrToGrafInfo();
    GraphicAttr aActAttr;

    if( SDRGRAFOBJ_TRANSFORMATTR_NONE != nTransformFlags &&
        GRAPHIC_NONE != eType )
    {
        // Actually transform the graphic only in this case.
        // Cropping always happens, though.
        aActAttr = aGrafInfo;

        if( bMirror )
        {
            sal_uInt16      nMirrorCase = ( aGeo.nDrehWink == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
            bool bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            bool bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
        }

        if( bRotate )
            aActAttr.SetRotation( sal_uInt16(aGeo.nDrehWink / 10) );
    }

    // Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

// -----------------------------------------------------------------------------

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

bool SdrGrafObj::IsRenderGraphic() const
{
    return pGraphic->IsRenderGraphic();
}

bool SdrGrafObj::HasRenderGraphic() const
{
    return pGraphic->HasRenderGraphic();
}

bool SdrGrafObj::IsSwappedOut() const
{
    return mbIsPreview ? true : pGraphic->IsSwappedOut();
}

const MapMode& SdrGrafObj::GetGrafPrefMapMode() const
{
    return pGraphic->GetPrefMapMode();
}

const Size& SdrGrafObj::GetGrafPrefSize() const
{
    return pGraphic->GetPrefSize();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGrafStreamURL( const String& rGraphicStreamURL )
{
    mbIsPreview = false;
    if( !rGraphicStreamURL.Len() )
    {
        pGraphic->SetUserData();
    }
    else if( pModel->IsSwapGraphics() )
    {
        pGraphic->SetUserData( rGraphicStreamURL );

        // set state of graphic object to 'swapped out'
        if( pGraphic->GetType() == GRAPHIC_NONE )
            pGraphic->SetSwapState();
    }
}

// -----------------------------------------------------------------------------

String SdrGrafObj::GetGrafStreamURL() const
{
    return pGraphic->GetUserData();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapIn() const
{
    if( mbIsPreview )
    {
        // removing preview graphic
        const String aUserData( pGraphic->GetUserData() );

        Graphic aEmpty;
        pGraphic->SetGraphic( aEmpty );
        pGraphic->SetUserData( aUserData );
        pGraphic->SetSwapState();

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

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapOut() const
{
    pGraphic->FireSwapOutRequest();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAnmeldung()
{
    sfx2::LinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink == NULL )
    {
        if (!aFileName.isEmpty())
        {
            pGraphicLink = new SdrGraphicLink( this );
            pLinkManager->InsertFileLink(
                *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, (aFilterName.isEmpty() ? NULL : &aFilterName), NULL);
            pGraphicLink->Connect();
        }
    }
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicLink(const rtl::OUString& rFileName, const String& rFilterName)
{
    ImpLinkAbmeldung();
    aFileName = rFileName;
    aFilterName = rFilterName;
    ImpLinkAnmeldung();
    pGraphic->SetUserData();

    // A linked graphic is per definition swapped out (has to be loaded)
    pGraphic->SetSwapState();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ReleaseGraphicLink()
{
    ImpLinkAbmeldung();
    aFileName = rtl::OUString();
    aFilterName = rtl::OUString();
}

bool SdrGrafObj::IsLinkedGraphic() const
{
    return !aFileName.isEmpty();
}

const rtl::OUString& SdrGrafObj::GetFileName() const
{
    return aFileName;
}

const rtl::OUString& SdrGrafObj::GetFilterName() const
{
    return aFilterName;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bAnim = pGraphic->IsAnimated();
    bool bRenderGraphic = pGraphic->HasRenderGraphic();
    bool bNoPresGrf = ( pGraphic->GetType() != GRAPHIC_NONE ) && !bEmptyPresObj;

    rInfo.bResizeFreeAllowed = aGeo.nDrehWink % 9000 == 0 ||
                               aGeo.nDrehWink % 18000 == 0 ||
                               aGeo.nDrehWink % 27000 == 0;

    rInfo.bResizePropAllowed = true;
    rInfo.bRotateFreeAllowed = bNoPresGrf && !bAnim && !bRenderGraphic;
    rInfo.bRotate90Allowed = bNoPresGrf && !bAnim && !bRenderGraphic;
    rInfo.bMirrorFreeAllowed = bNoPresGrf && !bAnim && !bRenderGraphic;
    rInfo.bMirror45Allowed = bNoPresGrf && !bAnim && !bRenderGraphic;
    rInfo.bMirror90Allowed = !bEmptyPresObj && !bRenderGraphic;
    rInfo.bTransparenceAllowed = false;
    rInfo.bGradientAllowed = false;
    rInfo.bShearAllowed = false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath = false;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
    rInfo.bCanConvToPoly = !IsEPS() && !bRenderGraphic;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

// -----------------------------------------------------------------------------

sal_uInt16 SdrGrafObj::GetObjIdentifier() const
{
    return sal_uInt16( OBJ_GRAF );
}

// -----------------------------------------------------------------------------

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
            pGraphicLink->DataChanged( ImpLoadLinkedGraphic( aFileName, aFilterName ) );
        bRet = true;
    }
    return bRet;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetLinkedGraphic( const Graphic& rGraphic )
{
    const sal_Bool bIsChanged = GetModel()->IsChanged();
    NbcSetGraphic( rGraphic );
    ActionChanged();
    BroadcastObjectChange();
    GetModel()->SetChanged( bIsChanged );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNameSingul(XubString& rName) const
{
    switch( pGraphic->GetType() )
    {
        case GRAPHIC_BITMAP:
        {
            const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                 ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPTRANSLNK : STR_ObjNameSingulGRAFBMPTRANS ) :
                                 ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPLNK : STR_ObjNameSingulGRAFBMP ) );

            rName=ImpGetResStr( nId );
        }
        break;

        case GRAPHIC_GDIMETAFILE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFMTFLNK : STR_ObjNameSingulGRAFMTF );
        break;

        case GRAPHIC_NONE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFNONELNK : STR_ObjNameSingulGRAFNONE );
        break;

        default:
            rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNameSingulGRAFLNK : STR_ObjNameSingulGRAF );
        break;
    }

    const String aName(GetName());

    if( aName.Len() )
    {
        rName.AppendAscii( " '" );
        rName += aName;
        rName += sal_Unicode( '\'' );
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNamePlural( XubString& rName ) const
{
    switch( pGraphic->GetType() )
    {
        case GRAPHIC_BITMAP:
        {
            const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                 ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPTRANSLNK : STR_ObjNamePluralGRAFBMPTRANS ) :
                                 ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPLNK : STR_ObjNamePluralGRAFBMP ) );

            rName=ImpGetResStr( nId );
        }
        break;

        case GRAPHIC_GDIMETAFILE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFMTFLNK : STR_ObjNamePluralGRAFMTF );
        break;

        case GRAPHIC_NONE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFNONELNK : STR_ObjNamePluralGRAFNONE );
        break;

        default:
            rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNamePluralGRAFLNK : STR_ObjNamePluralGRAF );
        break;
    }

    const String aName(GetName());

    if( aName.Len() )
    {
        rName.AppendAscii( " '" );
        rName += aName;
        rName += sal_Unicode( '\'' );
    }
}

// -----------------------------------------------------------------------------

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

    if( rObj.pGraphicLink != NULL)
    {
        SetGraphicLink( aFileName, aFilterName );
    }

    ImpSetAttrToGrafInfo();
    return *this;
}

// -----------------------------------------------------------------------------
// #i25616#

basegfx::B2DPolyPolygon SdrGrafObj::TakeXorPoly() const
{
    if(mbInsidePaint)
    {
        basegfx::B2DPolyPolygon aRetval;

        // take grown rectangle
        const sal_Int32 nHalfLineWidth(ImpGetLineWdt() / 2);
        const Rectangle aGrownRect(
            aRect.Left() - nHalfLineWidth,
            aRect.Top() - nHalfLineWidth,
            aRect.Right() + nHalfLineWidth,
            aRect.Bottom() + nHalfLineWidth);

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

// -----------------------------------------------------------------------------

sal_uInt32 SdrGrafObj::GetHdlCount() const
{
    return 8L;
}

// -----------------------------------------------------------------------------

SdrHdl* SdrGrafObj::GetHdl(sal_uInt32 nHdlNum) const
{
    return SdrRectObj::GetHdl( nHdlNum + 1L );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize( rRef, xFact, yFact );

    bool bMirrX = xFact.GetNumerator() < 0;
    bool bMirrY = yFact.GetNumerator() < 0;

    if( bMirrX != bMirrY )
        bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrRectObj::NbcRotate(rRef,nWink,sn,cs);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrRectObj::NbcMirror(rRef1,rRef2);
    bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SdrRectObj::NbcRotate( rRef, nWink, tn, bVShear );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetLogicRect( const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
}

// -----------------------------------------------------------------------------

SdrObjGeoData* SdrGrafObj::NewGeoData() const
{
    return new SdrGrafObjGeoData;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
    rGGeo.bMirrored=bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    Size        aSizMerk( aRect.GetSize() );

    SdrRectObj::RestGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
    bMirrored=rGGeo.bMirrored;
}

// -----------------------------------------------------------------------------

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

    SdrRectObj::SetPage( pNewPage );

    if (!aFileName.isEmpty() && bInsert)
        ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetModel( SdrModel* pNewModel )
{
    bool bChg = pNewModel != pModel;

    if( bChg )
    {
        if( pGraphic->HasUserData() )
        {
            ForceSwapIn();
            pGraphic->SetUserData();
        }

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    // realize model
    SdrRectObj::SetModel(pNewModel);

    if (bChg && !aFileName.isEmpty())
        ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StartAnimation( OutputDevice* /*pOutDev*/, const Point& /*rPoint*/, const Size& /*rSize*/, long /*nExtraData*/)
{
    SetGrafAnimationAllowed(true);
}

// -----------------------------------------------------------------------------

bool SdrGrafObj::HasGDIMetaFile() const
{
    return( pGraphic->GetType() == GRAPHIC_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::DoConvertToPolyObj(sal_Bool bBezier) const
{
    SdrObject* pRetval = NULL;

    switch( GetGraphicType() )
    {
        case GRAPHIC_GDIMETAFILE:
        {
            // Sort into group and return ONLY those objects that can be created from the MetaFile.
            SdrObjGroup*            pGrp = new SdrObjGroup();
            ImpSdrGDIMetaFileImport aFilter(*GetModel());
            Point                   aOutPos( aRect.TopLeft() );
            const Size              aOutSiz( aRect.GetSize() );

            aFilter.SetScaleRect(GetSnapRect());
            aFilter.SetLayer(GetLayer());

            sal_uInt32 nInsAnz = aFilter.DoImport(GetTransformedGraphic().GetGDIMetaFile(), *pGrp->GetSubList(), 0);
            if(nInsAnz)
            {
                pRetval = pGrp;
                pGrp->NbcSetLayer(GetLayer());
                pGrp->SetModel(GetModel());
                pRetval = ImpConvertAddText(pRetval, bBezier);

                // convert all children
                if( pRetval )
                {
                    SdrObject* pHalfDone = pRetval;
                    pRetval = pHalfDone->DoConvertToPolyObj(bBezier);
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
                delete pGrp;
            break;
        }
        case GRAPHIC_BITMAP:
        {
            // create basic object and add fill
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier);

            // save bitmap as an attribute
            if(pRetval)
            {
                // retrieve bitmap for the fill
                SfxItemSet aSet(GetObjectItemSet());

                aSet.Put(XFillStyleItem(XFILL_BITMAP));
                Bitmap aBitmap( GetTransformedGraphic().GetBitmap() );
                XOBitmap aXBmp(aBitmap, XBITMAP_STRETCH);
                aSet.Put(XFillBitmapItem(String(), aXBmp));
                aSet.Put(XFillBmpTileItem(false));

                pRetval->SetMergedItemSet(aSet);
            }
            break;
        }
        case GRAPHIC_NONE:
        case GRAPHIC_DEFAULT:
        {
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier);
            break;
        }
    }

    return pRetval;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SetXPolyDirty();
    SdrRectObj::Notify( rBC, rHint );
    ImpSetAttrToGrafInfo();
}

bool SdrGrafObj::IsMirrored() const
{
    return bMirrored;
}

void SdrGrafObj::SetMirrored( bool _bMirrored )
{
    bMirrored = _bMirrored;
}

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const sal_uInt16 nTrans = ( (SdrGrafTransparenceItem&) rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
    const SdrGrafCropItem&  rCrop = (const SdrGrafCropItem&) rSet.Get( SDRATTR_GRAFCROP );

    aGrafInfo.SetLuminance( ( (SdrGrafLuminanceItem&) rSet.Get( SDRATTR_GRAFLUMINANCE ) ).GetValue() );
    aGrafInfo.SetContrast( ( (SdrGrafContrastItem&) rSet.Get( SDRATTR_GRAFCONTRAST ) ).GetValue() );
    aGrafInfo.SetChannelR( ( (SdrGrafRedItem&) rSet.Get( SDRATTR_GRAFRED ) ).GetValue() );
    aGrafInfo.SetChannelG( ( (SdrGrafGreenItem&) rSet.Get( SDRATTR_GRAFGREEN ) ).GetValue() );
    aGrafInfo.SetChannelB( ( (SdrGrafBlueItem&) rSet.Get( SDRATTR_GRAFBLUE ) ).GetValue() );
    aGrafInfo.SetGamma( ( (SdrGrafGamma100Item&) rSet.Get( SDRATTR_GRAFGAMMA ) ).GetValue() * 0.01 );
    aGrafInfo.SetTransparency( (sal_uInt8) FRound( Min( nTrans, (sal_uInt16) 100 )  * 2.55 ) );
    aGrafInfo.SetInvert( ( (SdrGrafInvertItem&) rSet.Get( SDRATTR_GRAFINVERT ) ).GetValue() );
    aGrafInfo.SetDrawMode( ( (SdrGrafModeItem&) rSet.Get( SDRATTR_GRAFMODE ) ).GetValue() );
    aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

    SetXPolyDirty();
    SetRectsDirty();
}

// -----------------------------------------------------------------------------

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
            aPos = aRect.TopLeft();

        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        SetLogicRect( Rectangle( aPos, aSize ) );
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdrGrafObj, ImpSwapHdl, GraphicObject*, pO )
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
                const sal_uIntPtr   nSwapMode = pModel->GetSwapGraphicsMode();

                if( ( pGraphic->HasUserData() || pGraphicLink ) &&
                    ( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
                {
                    pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
                }
                else if( nSwapMode & SDR_SWAPGRAPHICSMODE_TEMP )
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
                uno::Reference<io::XInputStream> const xStream(
                    pModel->GetDocumentStream(pGraphic->GetUserData(), proxy));

                ::boost::scoped_ptr<SvStream> const pStream( (xStream.is())
                        ? ::utl::UcbStreamHelper::CreateStream(xStream)
                        : 0 );

                if( pStream != NULL )
                {
                    Graphic aGraphic;

                    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL;

                    if(mbInsidePaint && !GetViewContact().HasViewObjectContacts(true))
                    {
                        pFilterData = new com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >( 3 );

                        com::sun::star::awt::Size aPreviewSizeHint( 64, 64 );
                        sal_Bool bAllowPartialStreamRead = true;
                        sal_Bool bCreateNativeLink = false;
                        (*pFilterData)[ 0 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "PreviewSizeHint" ) );
                        (*pFilterData)[ 0 ].Value <<= aPreviewSizeHint;
                        (*pFilterData)[ 1 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "AllowPartialStreamRead" ) );
                        (*pFilterData)[ 1 ].Value <<= bAllowPartialStreamRead;
                        (*pFilterData)[ 2 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
                        (*pFilterData)[ 2 ].Value <<= bCreateNativeLink;

                        mbIsPreview = true;
                    }

                    if( !GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic,
                                                                          pGraphic->GetUserData(),
                                                                          *pStream,
                                                                          GRFILTER_FORMAT_DONTKNOW,
                                                                          NULL, 0, pFilterData ) )
                    {
                        const String aUserData( pGraphic->GetUserData() );

                        pGraphic->SetGraphic( aGraphic );
                        pGraphic->SetUserData( aUserData );

                        // Graphic successfully swapped in.
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    delete pFilterData;

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

    return (long)(void*) pRet;
}

// -----------------------------------------------------------------------------

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
        // can be loaded from the original document stream later
        if( pGraphic->HasUserData() )
        {
            ::comphelper::LifecycleProxy proxy;
            xStream.set(
                pModel->GetDocumentStream(pGraphic->GetUserData(), proxy));
            // fdo#46340: this may look completely insane, and it is,
            // but it also prevents a crash: the LifecycleProxy will go out
            // of scope, but the xStream must be returned; the UcbStreamHelper
            // will actually copy the xStream to a temp file (because it is
            // not seekable), which makes it not crash...
            SvStream *const pStream =
                utl::UcbStreamHelper::CreateStream(xStream);
            xStream.set(new utl::OInputStreamWrapper(pStream, true));
        }
        else if( pGraphic && GetGraphic().IsLink() )
        {
            Graphic aGraphic( GetGraphic() );
            GfxLink aLink( aGraphic.GetLink() );
            sal_uInt32 nSize = aLink.GetDataSize();
            const void* pSourceData = (const void*)aLink.GetData();
            if( nSize && pSourceData )
            {
                sal_uInt8 * pBuffer = new sal_uInt8[ nSize ];
                if( pBuffer )
                {
                    memcpy( pBuffer, pSourceData, nSize );

                    SvMemoryStream* pStream = new SvMemoryStream( (void*)pBuffer, (sal_Size)nSize, STREAM_READ );
                    pStream->ObjectOwnsMemory( true );
                    xStream.set( new utl::OInputStreamWrapper( pStream, true ) );
                }
            }
        }

        if (!xStream.is() && !aFileName.isEmpty())
        {
            SvFileStream* pStream = new SvFileStream( aFileName, STREAM_READ );
            if( pStream )
                xStream.set( new utl::OInputStreamWrapper( pStream ) );
        }
    }

    return xStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
