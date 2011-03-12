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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _ANIMATION
#include <unotools/streamwrap.hxx>

#include <sfx2/lnkbase.hxx>
#include <math.h>
#include <vcl/salbtype.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
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
#include <vos/mutex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

// -----------
// - Defines -
// -----------

#define GRAFSTREAMPOS_INVALID   0xffffffff
#define SWAPGRAPHIC_TIMEOUT     5000


// ------------------
// - SdrGraphicLink -
// ------------------


const Graphic ImpLoadLinkedGraphic( const String& rFileName, const String& rFilterName )
{
    Graphic aGraphic;

    SfxMedium xMed( rFileName, STREAM_STD_READ, sal_True );
    xMed.DownLoad();

    SvStream* pInStrm = xMed.GetInStream();
    if ( pInStrm )
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();

        const sal_uInt16 nFilter = rFilterName.Len() && pGF->GetImportFormatCount()
                            ? pGF->GetImportFormatNumber( rFilterName )
                            : GRFILTER_FORMAT_DONTKNOW;

        String aEmptyStr;
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aFilterData( 1 );

        // Room for improvment:
        // As this is a linked graphic the GfxLink is not needed if saving/loading our own format.
        // But this link is required by some filters to access the native graphic (pdf export/ms export),
        // there we should create a new service to provide this data if needed
        aFilterData[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
        aFilterData[ 0 ].Value = Any( sal_True );
        pGF->ImportGraphic( aGraphic, aEmptyStr, *pInStrm, nFilter, NULL, 0, &aFilterData );
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
    virtual void        DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
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

    sal_Bool GraphicLinkChanged( const String& rFileName ){ return mrFileName != rFileName; };

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

    ::osl::Mutex    maMutex;
    const String&   mrFileName;
    const String&   mrFilterName;
    SdrGraphicLink& mrGraphicLink;

    volatile bool mbIsTerminated;
};

SdrGraphicUpdater::SdrGraphicUpdater( const String& rFileName, const String& rFilterName, SdrGraphicLink& rGraphicLink )
: mrFileName( rFileName )
, mrFilterName( rFilterName )
, mrGraphicLink( rGraphicLink )
, mbIsTerminated( sal_False )
{
    create();
}

SdrGraphicUpdater::~SdrGraphicUpdater( void )
{
}

void SdrGraphicUpdater::Terminate()
{
    ::osl::MutexGuard aGuard( maMutex );
    mbIsTerminated = sal_True;
}

void SAL_CALL SdrGraphicUpdater::onTerminated(void)
{
    delete this;
}

void SAL_CALL SdrGraphicUpdater::run(void)
{
    Graphic aGraphic( ImpLoadLinkedGraphic( mrFileName, mrFilterName ) );
    ::osl::MutexGuard aGuard(maMutex);
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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
    SetSynchron( sal_False );
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

void SdrGraphicLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
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
            // broadcasting, to update slidesorter
            pGrafObj->BroadcastObjectChange();
        }
    }
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::Closed()
{
    // Die Verbindung wird aufgehoben; pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
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
    bMirrored       ( sal_False )
{
    pGraphic = new GraphicObject;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = sal_True;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;
    mbIsPreview = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect)
:   SdrRectObj      ( rRect ),
    pGraphicLink    ( NULL ),
    bMirrored       ( sal_False )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = sal_True;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;
    mbIsPreview = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj( const Graphic& rGrf )
:   SdrRectObj(),
    pGraphicLink    ( NULL ),
    bMirrored       ( sal_False )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), SWAPGRAPHIC_TIMEOUT );
    bNoShear = sal_True;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;
    mbIsPreview = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
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
    mbIsPreview = sal_False;
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
    mbIsPreview = sal_False;
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
    // #107947# Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality

    GraphicType     eType = GetGraphicType();
    MapMode         aDestMap( pModel->GetScaleUnit(), Point(), pModel->GetScaleFraction(), pModel->GetScaleFraction() );
    const Size      aDestSize( GetLogicRect().GetSize() );
    const sal_Bool      bMirror = ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_MIRROR ) != 0;
    const sal_Bool      bRotate = ( ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_ROTATE ) != 0 ) &&
        ( aGeo.nDrehWink && aGeo.nDrehWink != 18000 ) && ( GRAPHIC_NONE != eType );

    // #104115# Need cropping info earlier
    ( (SdrGrafObj*) this )->ImpSetAttrToGrafInfo();
    GraphicAttr aActAttr;

    if( SDRGRAFOBJ_TRANSFORMATTR_NONE != nTransformFlags &&
        GRAPHIC_NONE != eType )
    {
        // actually transform the graphic only in this case. On the
        // other hand, cropping will always happen
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

    // #107947# Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

// -----------------------------------------------------------------------------

GraphicType SdrGrafObj::GetGraphicType() const
{
    return pGraphic->GetType();
}

sal_Bool SdrGrafObj::IsAnimated() const
{
    return pGraphic->IsAnimated();
}

sal_Bool SdrGrafObj::IsEPS() const
{
    return pGraphic->IsEPS();
}

sal_Bool SdrGrafObj::IsSwappedOut() const
{
    return mbIsPreview ? sal_True : pGraphic->IsSwappedOut();
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
    mbIsPreview = sal_False;
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

void SdrGrafObj::SetFileName(const String& rFileName)
{
    aFileName = rFileName;
    SetChanged();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetFilterName(const String& rFilterName)
{
    aFilterName = rFilterName;
    SetChanged();
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

        const_cast< SdrGrafObj* >( this )->mbIsPreview = sal_False;
    }
    if ( pGraphicLink && pGraphic->IsSwappedOut() )
        ImpUpdateGraphicLink( sal_False );
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
        if( aFileName.Len() )
        {
            pGraphicLink = new SdrGraphicLink( this );
            pLinkManager->InsertFileLink( *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, ( aFilterName.Len() ? &aFilterName : NULL ), NULL );
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
        // Bei Remove wird *pGraphicLink implizit deleted
        pLinkManager->Remove( pGraphicLink );
        pGraphicLink=NULL;
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicLink( const String& rFileName, const String& rFilterName )
{
    ImpLinkAbmeldung();
    aFileName = rFileName;
    aFilterName = rFilterName;
    ImpLinkAnmeldung();
    pGraphic->SetUserData();

    // #92205# A linked graphic is per definition swapped out (has to be loaded)
    pGraphic->SetSwapState();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ReleaseGraphicLink()
{
    ImpLinkAbmeldung();
    aFileName = String();
    aFilterName = String();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bAnim = pGraphic->IsAnimated();
    bool bNoPresGrf = ( pGraphic->GetType() != GRAPHIC_NONE ) && !bEmptyPresObj;

    rInfo.bResizeFreeAllowed = aGeo.nDrehWink % 9000 == 0 ||
                               aGeo.nDrehWink % 18000 == 0 ||
                               aGeo.nDrehWink % 27000 == 0;

    rInfo.bResizePropAllowed = sal_True;
    rInfo.bRotateFreeAllowed = bNoPresGrf && !bAnim;
    rInfo.bRotate90Allowed = bNoPresGrf && !bAnim;
    rInfo.bMirrorFreeAllowed = bNoPresGrf && !bAnim;
    rInfo.bMirror45Allowed = bNoPresGrf && !bAnim;
    rInfo.bMirror90Allowed = !bEmptyPresObj;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed = sal_False;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bCanConvToPath = sal_False;
    rInfo.bCanConvToPathLineToArea = sal_False;
    rInfo.bCanConvToPolyLineToArea = sal_False;
    rInfo.bCanConvToPoly = !IsEPS();
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
sal_Bool SdrGrafObj::ImpUpdateGraphicLink( sal_Bool bAsynchron ) const
{
    sal_Bool bRet = sal_False;
    if( pGraphicLink )
    {
        if ( bAsynchron )
            pGraphicLink->UpdateAsynchron();
        else
            pGraphicLink->DataChanged( ImpLoadLinkedGraphic( aFileName, aFilterName ) );
        bRet = sal_True;
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

void SdrGrafObj::operator=( const SdrObject& rObj )
{
    SdrRectObj::operator=( rObj );

    const SdrGrafObj& rGraf = (SdrGrafObj&) rObj;

    pGraphic->SetGraphic( rGraf.GetGraphic(), &rGraf.GetGraphicObject() );
    aCropRect = rGraf.aCropRect;
    aFileName = rGraf.aFileName;
    aFilterName = rGraf.aFilterName;
    bMirrored = rGraf.bMirrored;

    if( rGraf.pGraphicLink != NULL)
    {
        SetGraphicLink( aFileName, aFilterName );
    }

    ImpSetAttrToGrafInfo();
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
        // hier kein SwapIn noetig, weil wenn nicht geladen, dann auch nicht animiert.
        if( pGraphic->IsAnimated())
            pGraphic->StopAnimation();

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    SdrRectObj::SetPage( pNewPage );

    if(aFileName.Len() && bInsert)
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

    // Model umsetzen
    SdrRectObj::SetModel(pNewModel);

    if( bChg && aFileName.Len() )
        ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StartAnimation( OutputDevice* /*pOutDev*/, const Point& /*rPoint*/, const Size& /*rSize*/, long /*nExtraData*/)
{
    // #111096#
    // use new graf animation
    SetGrafAnimationAllowed(sal_True);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StopAnimation(OutputDevice* /*pOutDev*/, long /*nExtraData*/)
{
    // #111096#
    // use new graf animation
    SetGrafAnimationAllowed(sal_False);
}

// -----------------------------------------------------------------------------

bool SdrGrafObj::HasGDIMetaFile() const
{
    return( pGraphic->GetType() == GRAPHIC_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

const GDIMetaFile* SdrGrafObj::GetGDIMetaFile() const
{
    OSL_FAIL( "Invalid return value! Don't use it! (KA)" );
    return &GetGraphic().GetGDIMetaFile();
}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::DoConvertToPolyObj(sal_Bool bBezier) const
{
    SdrObject* pRetval = NULL;

    switch( GetGraphicType() )
    {
        case GRAPHIC_GDIMETAFILE:
        {
            // NUR die aus dem MetaFile erzeugbaren Objekte in eine Gruppe packen und zurueckliefern
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
            // Grundobjekt kreieren und Fuellung ergaenzen
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier);

            // Bitmap als Attribut retten
            if(pRetval)
            {
                // Bitmap als Fuellung holen
                SfxItemSet aSet(GetObjectItemSet());

                aSet.Put(XFillStyleItem(XFILL_BITMAP));
                Bitmap aBitmap( GetTransformedGraphic().GetBitmap() );
                XOBitmap aXBmp(aBitmap, XBITMAP_STRETCH);
                aSet.Put(XFillBitmapItem(String(), aXBmp));
                aSet.Put(XFillBmpTileItem(sal_False));

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

void SdrGrafObj::ImpSetGrafInfoToAttr()
{
    SetObjectItem( SdrGrafLuminanceItem( aGrafInfo.GetLuminance() ) );
    SetObjectItem( SdrGrafContrastItem( aGrafInfo.GetContrast() ) );
    SetObjectItem( SdrGrafRedItem( aGrafInfo.GetChannelR() ) );
    SetObjectItem( SdrGrafGreenItem( aGrafInfo.GetChannelG() ) );
    SetObjectItem( SdrGrafBlueItem( aGrafInfo.GetChannelB() ) );
    SetObjectItem( SdrGrafGamma100Item( FRound( aGrafInfo.GetGamma() * 100.0 ) ) );
    SetObjectItem( SdrGrafTransparenceItem( (sal_uInt16) FRound( aGrafInfo.GetTransparency() / 2.55 ) ) );
    SetObjectItem( SdrGrafInvertItem( aGrafInfo.IsInvert() ) );
    SetObjectItem( SdrGrafModeItem( aGrafInfo.GetDrawMode() ) );
    SetObjectItem( SdrGrafCropItem( aGrafInfo.GetLeftCrop(), aGrafInfo.GetTopCrop(), aGrafInfo.GetRightCrop(), aGrafInfo.GetBottomCrop() ) );
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

        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ( (!bShrinkOnly                          ||
             ( aSize.Height() > aMaxSize.Height() ) ||
             ( aSize.Width()  > aMaxSize.Width()  ) )&&
             aSize.Height() && aMaxSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aMaxSize.Width() /
                            (float)aMaxSize.Height();

            // Grafik an Pagesize anpassen (skaliert)
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
        // kann aus dem original Doc-Stream nachgeladen werden...
        if( pModel != NULL )
        {
            if( pGraphic->HasUserData() )
            {
                SdrDocumentStreamInfo aStreamInfo;

                aStreamInfo.mbDeleteAfterUse = sal_False;
                aStreamInfo.maUserData = pGraphic->GetUserData();

                SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );

                if( pStream != NULL )
                {
                    Graphic aGraphic;

                    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL;

                    if(mbInsidePaint && !GetViewContact().HasViewObjectContacts(true))
                    {
//                          Rectangle aSnapRect(GetSnapRect());
//                          const Rectangle aSnapRectPixel(pOutDev->LogicToPixel(aSnapRect));

                        pFilterData = new com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >( 3 );

                        com::sun::star::awt::Size aPreviewSizeHint( 64, 64 );
                        sal_Bool bAllowPartialStreamRead = sal_True;
                        sal_Bool bCreateNativeLink = sal_False;
                        (*pFilterData)[ 0 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "PreviewSizeHint" ) );
                        (*pFilterData)[ 0 ].Value <<= aPreviewSizeHint;
                        (*pFilterData)[ 1 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "AllowPartialStreamRead" ) );
                        (*pFilterData)[ 1 ].Value <<= bAllowPartialStreamRead;
                        (*pFilterData)[ 2 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
                        (*pFilterData)[ 2 ].Value <<= bCreateNativeLink;

                        mbIsPreview = sal_True;
                    }

                    if( !GraphicFilter::GetGraphicFilter()->ImportGraphic( aGraphic, String(), *pStream,
                                                        GRFILTER_FORMAT_DONTKNOW, NULL, 0, pFilterData ) )
                    {
                        const String aUserData( pGraphic->GetUserData() );

                        pGraphic->SetGraphic( aGraphic );
                        pGraphic->SetUserData( aUserData );

                        // #142146# Graphic successfully swapped in.
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    delete pFilterData;

                    pStream->ResetError();

                    if( aStreamInfo.mbDeleteAfterUse || aStreamInfo.mxStorageRef.is() )
                    {
                        if ( aStreamInfo.mxStorageRef.is() )
                        {
                            aStreamInfo.mxStorageRef->dispose();
                            aStreamInfo.mxStorageRef = 0;
                        }

                        delete pStream;
                    }
                }
            }
            else if( !ImpUpdateGraphicLink( sal_False ) )
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

// #111096#
// Access to GrafAnimationAllowed flag
sal_Bool SdrGrafObj::IsGrafAnimationAllowed() const
{
    return mbGrafAnimationAllowed;
}

void SdrGrafObj::SetGrafAnimationAllowed(sal_Bool bNew)
{
    if(mbGrafAnimationAllowed != bNew)
    {
        mbGrafAnimationAllowed = bNew;
        ActionChanged();
    }
}

// #i25616#
sal_Bool SdrGrafObj::IsObjectTransparent() const
{
    if(((const SdrGrafTransparenceItem&)GetObjectItem(SDRATTR_GRAFTRANSPARENCE)).GetValue()
        || pGraphic->IsTransparent())
    {
        return sal_True;
    }

    return sal_False;
}

Reference< XInputStream > SdrGrafObj::getInputStream()
{
    Reference< XInputStream > xStream;

    if( pModel )
    {
//      if( !pGraphic->HasUserData() )
//          pGraphic->SwapOut();

        // kann aus dem original Doc-Stream nachgeladen werden...
        if( pGraphic->HasUserData() )
        {
            SdrDocumentStreamInfo aStreamInfo;

            aStreamInfo.mbDeleteAfterUse = sal_False;
            aStreamInfo.maUserData = pGraphic->GetUserData();

            SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );

            if( pStream )
                xStream.set( new utl::OInputStreamWrapper( pStream, sal_True ) );
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
                    pStream->ObjectOwnsMemory( sal_True );
                    xStream.set( new utl::OInputStreamWrapper( pStream, sal_True ) );
                }
            }
        }

        if( !xStream.is() && aFileName.Len() )
        {
            SvFileStream* pStream = new SvFileStream( aFileName, STREAM_READ );
            if( pStream )
                xStream.set( new utl::OInputStreamWrapper( pStream ) );
        }
    }

    return xStream;
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
