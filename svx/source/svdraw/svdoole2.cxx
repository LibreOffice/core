/*************************************************************************
 *
 *  $RCSfile: svdoole2.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 11:01:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdoole2.hxx"

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODE_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDMISC_HPP_
#include <com/sun/star/embed/EmbedMisc.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/embedhlp.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/ipclient.hxx>
#include <tools/stream.hxx>

#ifndef _SVDPAGV_HXX
#include <svdpagv.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SV_JOBSET_HXX
#include <vcl/jobset.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#include <sot/formats.hxx>

#ifndef _SVXLINKMGR_HXX //autogen
#include <linkmgr.hxx>
#endif
#ifndef SVTOOLS_TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#include <cppuhelper/implbase2.hxx>

#include <svtools/solar.hrc>
#include "svdxout.hxx"
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include "svdpagv.hxx"
#include "svdmodel.hxx"
#include "svdio.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "unomlstr.hxx"
#include "impgrf.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrOle2ObjImpl
{
public:
    // TODO/LATER: do we really need this pointer?
    GraphicObject*  pGraphicObject;
    String          aPersistName;       // name of object in persist

    // #107645#
    // New local var to avoid repeated loading if load of OLE2 fails
    sal_Bool        mbLoadingOLEObjectFailed;
    bool            mbConnected;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Predicate determining whether the given OLE is an internal math
// object
static bool ImplIsMathObj( const uno::Reference < embed::XEmbeddedObject >& rObjRef )
{
    if ( !rObjRef.is() )
        return false;

    SvGlobalName aClassName( rObjRef->getClassID() );
    if( aClassName == SvGlobalName(SO3_SM_CLASSID_30) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_40) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_50) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_60) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID)      )
    {
        return true;
    }
    else
    {
        return false;
    }
}

// -----------------------------------------------------------------------------

TYPEINIT1(SdrOle2Obj,SdrRectObj);

SdrOle2Obj::SdrOle2Obj(FASTBOOL bFrame_)
{
    bInDestruction = FALSE;
    Init();
    bFrame=bFrame_;
}

// -----------------------------------------------------------------------------
SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef& rNewObjRef, FASTBOOL bFrame_)
    : xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef& rNewObjRef, const XubString& rNewObjName, FASTBOOL bFrame_)
    : xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    mpImpl->aPersistName = rNewObjName;
    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef&  rNewObjRef, const XubString& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_)
    : SdrRectObj(rNewRect)
    , xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    mpImpl->aPersistName = rNewObjName;
    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::Init()
{
    mpImpl = new SdrOle2ObjImpl;
    pModifyListener = NULL;
    pGraphic=NULL;
    mpImpl->pGraphicObject=NULL;

    // #107645#
    // init to start situation, loading did not fail
    mpImpl->mbLoadingOLEObjectFailed = sal_False;
    mpImpl->mbConnected = false;
    xObjRef.Lock( TRUE );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::~SdrOle2Obj()
{
    bInDestruction = TRUE;

    if ( mpImpl->mbConnected )
        Disconnect();

    if( pGraphic!=NULL )
        delete pGraphic;

    if(mpImpl->pGraphicObject!=NULL)
        delete mpImpl->pGraphicObject;

    if(pModifyListener)
    {
        pModifyListener->invalidate();
        pModifyListener->release();
        pModifyListener = NULL;
    }
    delete mpImpl;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetGraphic_Impl(const Graphic* pGrf)
{
    if ( pGraphic )
    {
        delete pGraphic;
        pGraphic = NULL;
        delete mpImpl->pGraphicObject;
        mpImpl->pGraphicObject = NULL;
    }

    if (pGrf!=NULL)
    {
        pGraphic = new Graphic(*pGrf);
        mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
    }

    SetChanged();
    BroadcastObjectChange();

    //if ( ppObjRef->Is() && pGrf )
    //  BroadcastObjectChange();
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    // only for setting a preview graphic
    SetGraphic_Impl( pGrf );
}

// -----------------------------------------------------------------------------

FASTBOOL SdrOle2Obj::IsEmpty() const
{
    return !(xObjRef.is());
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::Connect()
{
    if( mpImpl->mbConnected )
    {
        DBG_ERROR("Connect() called on connected object!");
        return;
    }

    Connect_Impl();
    AddListeners_Impl();
}

void SdrOle2Obj::Connect_Impl()
{
    if( pModel && mpImpl->aPersistName.Len() )
    {
        SfxObjectShell* pPers=pModel->GetPersist();
        if ( pPers )
        {
            comphelper::EmbeddedObjectContainer& rContainer = pPers->GetEmbeddedObjectContainer();
            if ( !rContainer.HasEmbeddedObject( mpImpl->aPersistName ) )
            {
                // object not known to container document
                // No object -> disaster!
                DBG_ASSERT( xObjRef.is(), "No object in connect!");
                if ( xObjRef.is() )
                {
                    // object came from the outside, now add it to the container
                    ::rtl::OUString aTmp;
                    rContainer.InsertEmbeddedObject( xObjRef.GetObject(), aTmp );
                    mpImpl->aPersistName = aTmp;
                }
            }
            else if ( !xObjRef.is() )
                xObjRef.Assign( rContainer.GetEmbeddedObject( mpImpl->aPersistName ), xObjRef.GetViewAspect() );

            if ( xObjRef.GetObject().is() )
            {
                xObjRef.AssignToContainer( &rContainer, mpImpl->aPersistName );
                mpImpl->mbConnected = true;
                xObjRef.Lock( TRUE );
            }
        }

        // In Cache einfuegen
        if ( xObjRef.is() )
            GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
    }

    //TODO/LATER: wait for definition of MiscStatus RESIZEONPRINTERCHANGE
    //if ( xObjRef.is() && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
    {
        //TODO/LATER: needs a new handling for OnPrinterChanged
        /*
        if (pModel && pModel->GetRefDevice() &&
            pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
        {
            // Kein RefDevice oder RefDevice kein Printer
            BOOL bModified = (*ppObjRef)->IsModified();
            Printer* pPrinter = (Printer*) pModel->GetRefDevice();
            (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
            (*ppObjRef)->SetModified( bModified );
        }*/
    }
}

void SdrOle2Obj::AddListeners_Impl()
{
    if( xObjRef.is() )
    {
        // register modify listener
        if( pModifyListener == NULL )
        {
            ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
            pModifyListener->acquire();
        }

        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->addModifyListener( xListener );
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::Disconnect()
{
    if( !mpImpl->mbConnected )
    {
        DBG_ERROR("Connect() called on disconnected object!");
        return;
    }

    RemoveListeners_Impl();
    Disconnect_Impl();
}

void SdrOle2Obj::RemoveListeners_Impl()
{
    if( xObjRef.is() && mpImpl->aPersistName.Len() )
    {
        sal_Int32 nState = xObjRef->getCurrentState();
        if ( nState != embed::EmbedStates::LOADED )
        {
            uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
            if( xBC.is() && pModifyListener )
            {
                uno::Reference< util::XModifyListener > xListener( pModifyListener );
                xBC->removeModifyListener( xListener );
            }
        }
    }
}

void SdrOle2Obj::Disconnect_Impl()
{
    if ( pModel && mpImpl->aPersistName.Len() )
    {
        if( pModel->IsInDestruction() )
        {
            // TODO/LATER: here we must assume that the destruction of the model is enough to make clear that we will not
            // remove the object from the container, even if the DrawingObject itself is not destroyed (unfortunately this
            // happens later than the destruction of the model, so we can't assert that).
            //DBG_ASSERT( bInDestruction, "Model is destroyed, but not me?!" );
            //TODO/LATER: should be make sure that the ObjectShell also forgets the object, because we will close it soon?
            /*
            uno::Reference < util::XCloseable > xClose( xObjRef, uno::UNO_QUERY );
            if ( xClose.is() )
            {
                try
                {
                    xClose->close( sal_True );
                }
                catch ( util::CloseVetoException& )
                {
                    // there's still someone who needs the object!
                }
            }

            xObjRef = NULL;*/
        }
        else if ( xObjRef.is() )
        {
            SfxObjectShell* pPers = pModel->GetPersist();
            if ( pPers )
            {
                // remove object, but don't close it
                comphelper::EmbeddedObjectContainer& rContainer = pPers->GetEmbeddedObjectContainer();
                xObjRef.AssignToContainer( NULL, mpImpl->aPersistName );
                rContainer.RemoveEmbeddedObject( xObjRef.GetObject(), sal_False);
            }
        }

        // Aus Cache entfernen
        GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
    }

    mpImpl->mbConnected = false;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetModel(SdrModel* pNewModel)
{
    SfxObjectShell* pDestPers = pNewModel ? pNewModel->GetPersist() : 0;
    SfxObjectShell* pSrcPers  = pModel ? pModel->GetPersist() : 0;

    DBG_ASSERT( pSrcPers || !mpImpl->mbConnected, "Connected object without a model?!" );

    if( pDestPers != pSrcPers && mpImpl->mbConnected )
        // disconnect from old container
        Disconnect();
    else
        RemoveListeners_Impl();

    if( pDestPers && pSrcPers && ( pDestPers != pSrcPers ) )
    {
        ImpCopyObject( pSrcPers, pDestPers, mpImpl->aPersistName );
    }

    SdrRectObj::SetModel( pNewModel );

    if( pDestPers && pDestPers != pSrcPers )
        Connect();
    else
        AddListeners_Impl();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
    FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;

    if (bRemove && mpImpl->mbConnected )
        Disconnect();

    SdrRectObj::SetPage(pNewPage);

    if (bInsert && !mpImpl->mbConnected )
        Connect();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetObjRef( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rNewObjRef )
{
    DBG_ASSERT( !rNewObjRef.is() || !xObjRef.GetObject().is(), "SetObjRef called on already initialized object!");
    if( rNewObjRef == xObjRef.GetObject() )
        return;

    // MBA: the caller of the method is responsible to control the old object, it will not be closed here
    // Otherwise WW8 import crashes because it tranfers control to OLENode by this method
    if ( xObjRef.GetObject().is() )
        xObjRef.Lock( FALSE );

    // MBA: avoid removal of object in Disconnect! It is definitely a HACK to call SetObjRef(0)!
    xObjRef.Clear();

    if ( mpImpl->mbConnected )
        Disconnect();

    xObjRef.Assign( rNewObjRef, GetAspect() );

    if ( xObjRef.is() )
    {
        DELETEZ( pGraphic );

        if ( (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
            SetResizeProtect(TRUE);

        // #108759# For math objects, set closed state to transparent
        if( ImplIsMathObj( rNewObjRef ) )
            SetClosedObj( false );

        Connect();
    }

    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetClosedObj( bool bIsClosed )
{
    // TODO/LATER: do we still need this hack?
    // #108759# Allow changes to the closed state of OLE objects
    bClosedObj = bIsClosed;
}

// -----------------------------------------------------------------------------

FASTBOOL SdrOle2Obj::HasSetName() const
{
    return TRUE;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetName(const XubString& rStr)
{
    aName = rStr;
    SetChanged();
}

// -----------------------------------------------------------------------------

XubString SdrOle2Obj::GetName() const
{
    return aName;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetPersistName( const String& rPersistName )
{
    DBG_ASSERT( !mpImpl->aPersistName.Len(), "Persist name changed!");

    mpImpl->aPersistName = rPersistName;
    mpImpl->mbLoadingOLEObjectFailed = false;

    Connect();
    SetChanged();
}

void SdrOle2Obj::AbandonObject()
{
    mpImpl->aPersistName.Erase();
    mpImpl->mbLoadingOLEObjectFailed = false;
    SetObjRef(0);
}

// -----------------------------------------------------------------------------

String SdrOle2Obj::GetPersistName() const
{
    return mpImpl->aPersistName;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoOrthoDesired   =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bCanConvToContour = FALSE;
}

// -----------------------------------------------------------------------------

UINT16 SdrOle2Obj::GetObjIdentifier() const
{
    return bFrame ? UINT16(OBJ_FRAME) : UINT16(OBJ_OLE2);
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    sal_Bool bOk(sal_True);

    if( !GetGraphic() )
        ( (SdrOle2Obj*) this)->GetObjRef_Impl();    // try to create embedded object

    if ( xObjRef.is() )
    {
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        if( !bSizProt && (nMiscStatus & embed::EmbedMisc::EMBED_NEVERRESIZE) )
            ( (SdrOle2Obj*) this)->bSizProt = TRUE;

        OutputDevice* pOut = rOut.GetOutDev();

        //TODO/LATER: currently it's not possible to compare the windows, the XOutDev contains a virtual device
        sal_Int32 nState = xObjRef->getCurrentState();
        //if ( ( nState != embed::EmbedStates::INPLACE_ACTIVE && nState != embed::EmbedStates::UI_ACTIVE ) ||
        //       pModel && SfxInPlaceClient::GetActiveWindow( pModel->GetPersist(), xObjRef ) != pOut )
        {
            if ((rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTGRAF) ==0)
            {
                if ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE )
                {
                    // PlugIn-Objekt connecten
                    if (rInfoRec.pPV!=NULL)
                    {
                        SdrOle2Obj* pOle2Obj = (SdrOle2Obj*) this;
                        SdrView* pSdrView = (SdrView*) &rInfoRec.pPV->GetView();
                        pSdrView->DoConnect(pOle2Obj);
                    }
                }

                // #108759# Temporarily set the current background
                // color, since OLEs rely on that during
                // auto-colored text rendering
                Wallpaper aOldBg( pOut->GetBackground() );

                if( rInfoRec.pPV && GetPage() )
                    pOut->SetBackground( rInfoRec.pPV->GetView().CalcBackgroundColor( GetSnapRect(),
                                                                                      rInfoRec.pPV->GetVisibleLayers(),
                                                                                      *GetPage() ) );

                pOut->Push( PUSH_CLIPREGION );
                pOut->IntersectClipRegion( aRect );

                GetGraphic();
                PaintGraphic_Impl( rOut, rInfoRec, nState == embed::EmbedStates::ACTIVE );

                /*
                if ( !mpImpl->pMetaFile )
                    GetGDIMetaFile();
                if ( mpImpl->pMetaFile )
                    mpImpl->pMetaFile->Play( pOut, aRect.TopLeft(), aRect.GetSize() );
                //(*ppObjRef)->DoDraw(pOut,aRect.TopLeft(),aRect.GetSize(),JobSetup());
                */

                pOut->Pop();

                // #108759# Restore old background
                pOut->SetBackground( aOldBg );
            }
            else if( ( rInfoRec.nPaintMode & SDRPAINTMODE_HIDEDRAFTGRAF ) == 0 )
            { // sonst SDRPAINTMODE_DRAFTGRAF
                Polygon aPoly(Rect2Poly(aRect,aGeo));
                pOut->SetLineColor(Color(COL_BLACK));
                pOut->DrawPolyLine(aPoly);
                pOut->DrawLine(aPoly[0],aPoly[2]);
                pOut->DrawLine(aPoly[1],aPoly[3]);
            }
        }
    }
    else if ( GetGraphic() )
    {
        PaintGraphic_Impl( rOut, rInfoRec );
    }
    // #100499# OLE without context and without bitmap, do the same as
    // for empty groups, additionally draw empty OLE bitmap
    else
    {
        if(!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet((sal_uInt8)nLayerId))
        {
            OutputDevice* pOutDev = rOut.GetOutDev();

            pOutDev->SetFillColor();
            pOutDev->SetLineColor(Color(COL_LIGHTGRAY));
            pOutDev->DrawRect(aOutRect);

            Bitmap aBitmap(ResId(BMP_OLEOBJ, ImpGetResMgr()));
            Rectangle aSnapRect(GetSnapRect());
            Size aBitmapSize(pOutDev->PixelToLogic(aBitmap.GetSizePixel()));

            pOutDev->DrawBitmap(
                aSnapRect.Center() - Point(aBitmapSize.Width() / 2, aBitmapSize.Height() / 2),
                aBitmapSize,
                aBitmap);
        }
    }

    if (HasText())
    {
        bOk = SdrTextObj::DoPaintObject(rOut, rInfoRec);
    }
    return bOk;
}

void SdrOle2Obj::PaintGraphic_Impl( XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec, sal_Bool bActive ) const
{
    Graphic* pGr = GetGraphic();
    OutputDevice* pOutDev=rOut.GetOutDev();
    if ( pGr && pGr->GetType() != GRAPHIC_NONE )
    {
        if( IsEmptyPresObj() )
        {
            const MapMode   aDstMapMode( pOutDev->GetMapMode().GetMapUnit() );
            Point           aPos(aRect.Center());
            Size            aDstSize;

            if( pGr->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aDstSize = pOutDev->PixelToLogic( pGr->GetPrefSize(), aDstMapMode );
            else
                aDstSize = pOutDev->LogicToLogic( pGr->GetPrefSize(), pGr->GetPrefMapMode(), aDstMapMode );

            aPos.X()-=aDstSize.Width() /2;
            aPos.Y()-=aDstSize.Height()/2;
            if (aPos.X() >= aRect.Left() && aPos.Y() >= aRect.Top())
                pGr->Draw(pOutDev,aPos, aDstSize);

            svtools::ColorConfig aColorConfig;
            svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( svtools::OBJECTBOUNDARIES ) );

            if( aColor.bIsVisible )
            {
                pOutDev->SetFillColor();
                pOutDev->SetLineColor( aColor.nColor );
                pOutDev->DrawRect(aRect);
            }
        }
        else
            pGr->Draw( pOutDev, aRect.TopLeft(), aRect.GetSize() );

        // shade the representation if the object is activated outplace
        if ( bActive )
            ::svt::EmbeddedObjectRef::DrawShading( aRect, pOutDev );
    }
    else
        ::svt::EmbeddedObjectRef::DrawPaintReplacement( aRect, mpImpl->aPersistName, pOutDev );
}

// -----------------------------------------------------------------------------

SdrObject* SdrOle2Obj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt,nTol,pVisiLayer,TRUE,TRUE);
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(bFrame ? STR_ObjNameSingulFrame : STR_ObjNameSingulOLE2);

    if( aName.Len() )
    {
        rName.AppendAscii(" '");
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(bFrame ? STR_ObjNamePluralFrame : STR_ObjNamePluralOLE2);
}

// -----------------------------------------------------------------------------
// #116235#
//void SdrOle2Obj::ImpAssign( const SdrObject& rObj, SdrPage* pNewPage, SdrModel* pNewModel, sal_Bool bRestoreModel )
//{
//    const SdrOle2Obj& rOle2Obj = static_cast< const SdrOle2Obj& >( rObj );
//  SdrModel* pOldModel = pModel;
//
//  if( pModel )
//        Disconnect();
//
//    SdrRectObj::operator=( rObj );
//
//    // #108867# Manually copying bClosedObj attribute
//    SetClosedObj( rObj.IsClosedObj() );
//
//    if( pNewPage )
//        pPage = pNewPage;
//
//    if( pNewModel )
//        pModel = pNewModel;
//
//    aName = rOle2Obj.aName;
//  mpImpl->aPersistName = rOle2Obj.mpImpl->aPersistName;
//  aProgName = rOle2Obj.aProgName;
//  bFrame = rOle2Obj.bFrame;
//
//  if( rOle2Obj.pGraphic )
//  {
//      if( pGraphic )
//      {
//          delete pGraphic;
//          delete mpImpl->pGraphicObject;
//      }
//
//        pGraphic = new Graphic( *rOle2Obj.pGraphic );
//      mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
//  }
//
//  if( pModel && rObj.GetModel() )
//    {
//      SvPersist* pDestPers = pModel->GetPersist();
//      SvPersist* pSrcPers = rObj.GetModel()->GetPersist();
//
//        if( pDestPers && pSrcPers )
//        {
//            ImpCopyObject( *pSrcPers, *pDestPers, mpImpl->aPersistName );
//
//            if( rOle2Obj.ppObjRef->Is() && ppObjRef->Is() &&
//                ( (*rOle2Obj.ppObjRef)->GetMapUnit() == (*ppObjRef)->GetMapUnit() ) )
//            {
//                    (*ppObjRef)->SetVisArea( (*rOle2Obj.ppObjRef)->GetVisArea() );
//            }
//        }
//
//        Connect();
//  }
//
//  if(bRestoreModel)
//  {
//      pModel = pOldModel;
//  }
//}

// -----------------------------------------------------------------------------

void SdrOle2Obj::ImpCopyObject( SfxObjectShell* pSrcPersist, SfxObjectShell* pDstPersist, String& rPersistName )
{
    DBG_ASSERT( !xObjRef.is(), "Object already existing!" );
    comphelper::EmbeddedObjectContainer& rContainer = pSrcPersist->GetEmbeddedObjectContainer();
    uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( rPersistName );
    if ( xObj.is() )
    {
        ::rtl::OUString aTmp;
        xObjRef.Assign( pDstPersist->GetEmbeddedObjectContainer().CopyEmbeddedObject( xObj, aTmp ), GetAspect() );
        rPersistName = aTmp;
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::operator=(const SdrObject& rObj)
{
    //TODO/LATER: who takes over control of my old object?!
    if( &rObj != this )
    {
        // #116235#
        // ImpAssign( rObj );
        const SdrOle2Obj& rOle2Obj = static_cast< const SdrOle2Obj& >( rObj );

        uno::Reference < util::XCloseable > xClose( xObjRef.GetObject(), uno::UNO_QUERY );

        if( pModel && mpImpl->mbConnected )
            Disconnect();

        SdrRectObj::operator=( rObj );

        // #108867# Manually copying bClosedObj attribute
        SetClosedObj( rObj.IsClosedObj() );

        aName = rOle2Obj.aName;
        mpImpl->aPersistName = rOle2Obj.mpImpl->aPersistName;
        aProgName = rOle2Obj.aProgName;
        bFrame = rOle2Obj.bFrame;

        if( rOle2Obj.pGraphic )
        {
            if( pGraphic )
            {
                delete pGraphic;
                delete mpImpl->pGraphicObject;
            }

            pGraphic = new Graphic( *rOle2Obj.pGraphic );
            mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
        }

        if( pModel && rObj.GetModel() )
        {
            SfxObjectShell* pDestPers = pModel->GetPersist();
            SfxObjectShell* pSrcPers = rObj.GetModel()->GetPersist();
            if( pDestPers && pSrcPers )
            {
                ImpCopyObject( pSrcPers, pDestPers, mpImpl->aPersistName );
                if( xObjRef.is() && rOle2Obj.xObjRef.is() && rOle2Obj.xObjRef->getMapUnit( rOle2Obj.GetAspect() ) == xObjRef->getMapUnit( GetAspect() ) )
                {
                    //TODO/LATER: possible optimization: allow to do that in loaded state
                    svt::EmbeddedObjectRef::TryRunningState( rOle2Obj.xObjRef.GetObject() );
                    svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() );
                    xObjRef->setVisualAreaSize( GetAspect(), rOle2Obj.xObjRef->getVisualAreaSize( GetAspect() ) );
                }
            }

            if( !mpImpl->mbConnected )
                Connect();
        }
    }
}

// -----------------------------------------------------------------------------

// #116235#
//SdrObject* SdrOle2Obj::Clone( SdrPage* pNewPage, SdrModel* pNewModel ) const
//{
//  SdrOle2Obj* pObj = static_cast< SdrOle2Obj* >( SdrObjFactory::MakeNewObject( GetObjInventor(), GetObjIdentifier(),NULL ) );
//
//    if( pObj )
//    {
//        pObj->ImpAssign( *this, pNewPage, pNewModel, sal_True );
//
//        if( pNewModel )
//          pObj->SetModel( pNewModel );
//    }
//
//  return pObj;
//}

// -----------------------------------------------------------------------------

FASTBOOL SdrOle2Obj::HasSpecialDrag() const
{
    return FALSE;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::ImpSetVisAreaSize()
{
    //TODO/LATER: possible optimization: allow to do that in loaded state
    GetObjRef();    // try to load inplace object
    if ( xObjRef.is())
    {
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        if ( nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE)
        {
            // Server resized selbst (StarChart)
            // Neue VisArea setzen
            //TODO/LATER: is it enough to use only size, not rectangle?
            MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
            Rectangle aVisArea = OutputDevice::LogicToLogic( aRect, pModel->GetScaleUnit(), aMapUnit);
            awt::Size aSz;
            aSz.Width = aVisArea.GetSize().Width();
            aSz.Height = aVisArea.GetSize().Height();
            xObjRef->setVisualAreaSize( GetAspect(), aSz );

            // Wurde die VisArea akzeptiert?
            //TODO/LATER: is it enough to use only size, not rectangle?
            aSz = xObjRef->getVisualAreaSize( GetAspect() );
            Rectangle aAcceptedVisArea;
            aAcceptedVisArea.SetSize( Size( aSz.Width, aSz.Height ) );
            //Rectangle aAcceptedVisArea(rIPRef->GetVisArea());
            if (aVisArea.GetSize() != aAcceptedVisArea.GetSize())
            {
                // VisArea wurde nicht akzeptiert -> korrigieren
                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(), aMapUnit, pModel->GetScaleUnit()));
            }

            xObjRef.UpdateReplacement();
        }
        else
        {
            /**********************************************************************
            * Nun wird nicht mehr die VisArea gesetzt, sondern es erfolgt eine
            * Skalierung
            **********************************************************************/
            SfxInPlaceClient* pClient = SfxInPlaceClient::GetClient( pModel->GetPersist(), xObjRef.GetObject() );
            if ( pClient && xObjRef.is() )
            {
                awt::Size aObjSize = xObjRef->getVisualAreaSize( GetAspect() );

                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                Size aObjAreaSize(aObjSize.Width, aObjSize.Height);
                aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize, aMapUnit, pModel->GetScaleUnit() );

                Size aSize = aRect.GetSize();
                Fraction aScaleWidth (aSize.Width(),  aObjAreaSize.Width() );
                Fraction aScaleHeight(aSize.Height(), aObjAreaSize.Height() );
                // Nun auf 10 Binaerstellen kuerzen
                Kuerzen(aScaleHeight, 10);
                Kuerzen(aScaleWidth,  10);

                pClient->SetSizeScale(aScaleWidth, aScaleHeight);

                Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
                pClient->SetObjArea(aScaleRect);
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // kleine Korrekturen
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000) {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }
        aGeo.nDrehWink=0;
        aGeo.nShearWink=0;
        aGeo.nSin=0.0;
        aGeo.nCos=1.0;
        aGeo.nTan=0.0;
        SetRectsDirty();
    }
    if( (NULL == pModel) || !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::SetGeoData(rGeo);
    if( (NULL == pModel) || !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
    if( (NULL == pModel) || !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    if( (NULL == pModel) || !pModel->isLocked() )
        ImpSetVisAreaSize();
}

Graphic* SdrOle2Obj::GetGraphic() const
{
    if ( xObjRef.is() )
        return xObjRef.GetGraphic();
    return pGraphic;
}

void SdrOle2Obj::GetNewReplacement()
{
    if ( xObjRef.is() )
        xObjRef.UpdateReplacement();
}

// -----------------------------------------------------------------------------

//BFS01void SdrOle2Obj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01 SdrRectObj::WriteData(rOut);
//BFS01 SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrOle2Obj");
//BFS01#endif
//BFS01
//BFS01 // UNICODE: rOut<<mpImpl->aPersistName;
//BFS01 rOut.WriteByteString(mpImpl->aPersistName);
//BFS01
//BFS01 // UNICODE: rOut<<aProgName;
//BFS01 rOut.WriteByteString(aProgName);
//BFS01
//BFS01 GetObjRef();
//BFS01 BOOL bObjRefValid=ppObjRef->Is();
//BFS01 rOut<<bObjRefValid;
//BFS01 BOOL bPreview = FALSE;
//BFS01 if( !IsEmptyPresObj() && pModel && pModel->IsSaveOLEPreview() )
//BFS01     bPreview = TRUE;
//BFS01
//BFS01 if( bPreview )
//BFS01 {
//BFS01     // set preview graphic (not for empty presentation objects)
//BFS01     GetGDIMetaFile();
//BFS01     if( mpImpl->pMetaFile )
//BFS01     {
//BFS01         Graphic aNewGraphic( *mpImpl->pMetaFile );
//BFS01         ( (SdrOle2Obj*) this )->SetGraphic( &aNewGraphic );
//BFS01     }
//BFS01 }
//BFS01
//BFS01 BOOL bHasGraphic=pGraphic!=NULL;
//BFS01 rOut<<bHasGraphic;
//BFS01 if (bHasGraphic)
//BFS01 {
//BFS01     SdrDownCompat aGrafCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
//BFS01#ifdef DBG_UTIL
//BFS01     aGrafCompat.SetID("SdrOle2Obj(Graphic)");
//BFS01#endif
//BFS01     rOut<<*pGraphic;
//BFS01 }
//BFS01
//BFS01 if( bPreview )
//BFS01     ( (SdrOle2Obj*) this )->SetGraphic( NULL );     // remove preview graphic
//BFS01}

// -----------------------------------------------------------------------------

//BFS01void SdrOle2Obj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 rIn.SetError( 0 );
//BFS01
//BFS01 if (rIn.GetError()!=0) return;
//BFS01 SdrRectObj::ReadData(rHead,rIn);
//BFS01 SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrOle2Obj");
//BFS01#endif
//BFS01
//BFS01 // UNICODE: rIn >> mpImpl->aPersistName;
//BFS01 rIn.ReadByteString(mpImpl->aPersistName);
//BFS01
//BFS01 // UNICODE: rIn >> aProgName;
//BFS01 rIn.ReadByteString(aProgName);
//BFS01
//BFS01 BOOL bObjRefValid;
//BFS01 rIn>>bObjRefValid;
//BFS01
//BFS01 BOOL bHasGraphic;
//BFS01 rIn>>bHasGraphic;
//BFS01 if (bHasGraphic)
//BFS01 {
//BFS01     if(pGraphic==NULL)
//BFS01         pGraphic=new Graphic;
//BFS01
//BFS01     if(rHead.GetVersion()>=11)
//BFS01     { // ab V11 eingepackt
//BFS01         SdrDownCompat aGrafCompat(rIn,STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01         aGrafCompat.SetID("SdrOle2Obj(Graphic)");
//BFS01#endif
//BFS01         rIn>>*pGraphic;
//BFS01     }
//BFS01     else
//BFS01         rIn>>*pGraphic;
//BFS01
//BFS01     if( mpImpl->pGraphicObject )
//BFS01         delete mpImpl->pGraphicObject;
//BFS01
//BFS01     mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
//BFS01 }
//BFS01}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    if( (NULL == pModel) || !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

BOOL SdrOle2Obj::Unload()
{
    BOOL bUnloaded = FALSE;

    if( xObjRef.is() )
    {
        //TODO/LATER: no refcounting tricks anymore!
        //"customers" must register as state change listeners
        //Nicht notwendig im Doc DTor (MM)
        //ULONG nRefCount = (*ppObjRef)->GetRefCount();
        // prevent Unload if there are external references
        //if( nRefCount > 2 )
        //    return FALSE;
        //DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
    }
    else
        bUnloaded = TRUE;

    if ( pModel && xObjRef.is() )
    {
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        sal_Int32 nState = xObjRef->getCurrentState();
        if ( nState == embed::EmbedStates::LOADED )
        {
            bUnloaded = TRUE;
            return bUnloaded;
        }

        uno::Reference < util::XModifiable > xModifiable( xObjRef->getComponent(), uno::UNO_QUERY );

        if ( embed::EmbedMisc::MS_EMBED_ALWAYSRUN != nMiscStatus    &&
        //TODO/LATER: no refcounting tricks anymore!
        //1 < (*ppObjRef)->GetRefCount()                   &&
        !( xModifiable.is() && xModifiable->isModified() ) &&
        !( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE ) )
        {
            xObjRef->changeState( embed::EmbedStates::LOADED );
            bUnloaded = TRUE;
        }
    }

    return bUnloaded;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::CreatePersistName( SfxObjectShell* pPers )
{
    mpImpl->aPersistName = pPers->GetEmbeddedObjectContainer().CreateUniqueObjectName();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::GetObjRef_Impl()
{
    if ( !xObjRef.is() && mpImpl->aPersistName.Len() && pModel && pModel->GetPersist() )
    {
        // #107645#
        // Only try loading if it did not went wrong up to now
        if(!mpImpl->mbLoadingOLEObjectFailed)
        {
            xObjRef.Assign( pModel->GetPersist()->GetEmbeddedObjectContainer().GetEmbeddedObject( mpImpl->aPersistName ) );

            // #107645#
            // If loading of OLE object failed, remember that to not invoke a endless
            // loop trying to load it again and again.
            if( xObjRef.is() )
            {
                mpImpl->mbLoadingOLEObjectFailed = sal_True;
            }

            // #108759# For math objects, set closed state to transparent
            if( ImplIsMathObj( xObjRef.GetObject() ) )
                SetClosedObj( false );
        }

        if ( xObjRef.is() )
        {
            if( !IsEmptyPresObj() )
            {
                // #75637# remember modified status of model
                const sal_Bool bWasChanged(pModel ? pModel->IsChanged() : sal_False);

                // perhaps preview not valid anymore
                // #75637# This line changes the modified state of the model
                SetGraphic_Impl( NULL );

                // #75637# if status was not set before, force it back
                // to not set, so that SetGraphic(0L) above does not
                // set the modified state of the model.
                if(!bWasChanged && pModel && pModel->IsChanged())
                {
                    pModel->SetChanged( sal_False );
                }
            }

            sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
            //TODO/LATER: wait until ResizeOnPrinterChange is defined
            //if ( nMiscStatus & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
            {
                if (pModel && pModel->GetRefDevice() &&
                    pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
                {
                    if(!bInDestruction)
                    {
                        //TODO/LATER: printerchange notification
                        /*
                        // prevent SetModified (don't want no update here)
                        sal_Bool bWasEnabled = (*ppObjRef)->IsEnableSetModified();
                        if ( bWasEnabled )
                            (*ppObjRef)->EnableSetModified( sal_False );

                        // Kein RefDevice oder RefDevice kein Printer
                        Printer* pPrinter = (Printer*) pModel->GetRefDevice();
                        (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );

                        // reset state
                        (*ppObjRef)->EnableSetModified( bWasEnabled );*/
                    }
                }
            }
        }

        if ( xObjRef.is() )
            Connect();
    }
}

uno::Reference < embed::XEmbeddedObject > SdrOle2Obj::GetObjRef() const
{
    const_cast<SdrOle2Obj*>(this)->GetObjRef_Impl();
    if ( xObjRef.is() )
        svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() );
    return xObjRef.GetObject();
}

// -----------------------------------------------------------------------------

uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
{
    GetObjRef();
    if ( xObjRef.is() )
        return uno::Reference< frame::XModel >( xObjRef->getComponent(), uno::UNO_QUERY );
    else
        return uno::Reference< frame::XModel >();
}

// -----------------------------------------------------------------------------

// #109985#
sal_Bool SdrOle2Obj::IsChart() const
{
    if ( !xObjRef.is() )
        return false;

    SvGlobalName aObjClsId( xObjRef->getClassID() );
    if(
        SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId)
    {
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------------

Rectangle SdrOle2Obj::GetVisibleArea()
{
    Rectangle aRect;
    if( xObjRef.is() )
    {
        //TODO/LATER: possible optimization: allow to do that in loaded state
        svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() );
        awt::Size aSize = xObjRef->getVisualAreaSize( GetAspect() );
        aRect.SetSize( Size( aSize.Width, aSize.Height ) );
    }

    return aRect;
}

/// #110015# sets the visible area at the SvInPlaceObject and SvEmbeddedInfoObject
void SdrOle2Obj::SetVisibleArea( const Rectangle& rVisArea )
{
    if( xObjRef.is() )
    {
        //TODO/LATER: possible optimization: allow to do that in loaded state
        svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() );
        awt::Size aSize;
        aSize.Width = rVisArea.GetWidth();
        aSize.Height = rVisArea.GetHeight();
        xObjRef->setVisualAreaSize( GetAspect(), aSize );

        //TODO/LATER: should we continue hacking?!
        //if( pModel && (!pModel->GetPersist()->IsEnableSetModified()) )
        //    xInplace->SetModified(FALSE);
    }

    /*
    if(pModel && mpImpl->aPersistName.Len() )
    {
        SvPersist* pPers = pModel->GetPersist();

        if (pPers)
        {
            SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);
            SvEmbeddedInfoObject * pEmbed = PTR_CAST(SvEmbeddedInfoObject, pInfo );

            if( pEmbed )
                pEmbed->SetInfoVisArea( rVisArea );
        }
    }*/
}

// eof
