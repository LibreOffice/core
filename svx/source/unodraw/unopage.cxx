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

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <osl/mutex.hxx>
#include <sfx2/dispatch.hxx>
#include <sot/clsids.hxx>
#include <comphelper/serviceinfohelper.hxx>

#include <sfx2/objsh.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unopage.hxx>
#include "shapeimpl.hxx"
#include "svx/globl3d.hxx"
#include <svx/polysc3d.hxx>
#include <svx/unoprov.hxx>
#include <svx/svdopath.hxx>
#include "svx/unoapi.hxx"
#include <svx/svdomeas.hxx>
#include <svx/extrud3d.hxx>
#include <svx/lathe3d.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

using ::rtl::OUString;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

/**********************************************************************
* class SvxDrawPage                                                   *
**********************************************************************/

UNO3_GETIMPLEMENTATION_IMPL( SvxDrawPage );
DBG_NAME(SvxDrawPage)
SvxDrawPage::SvxDrawPage( SdrPage* pInPage ) throw()
: mrBHelper( getMutex() )
, mpPage( pInPage )
, mpModel( 0 )
{
    DBG_CTOR(SvxDrawPage,NULL);
    // Am Broadcaster anmelden
    if( mpPage )
        mpModel = mpPage->GetModel();
    if( mpModel )
        StartListening( *mpModel );


    // Erzeugen der (hidden) ::com::sun::star::sdbcx::View
    mpView = new SdrView( mpModel );
    if( mpView )
        mpView->SetDesignMode(sal_True);
}

//----------------------------------------------------------------------
SvxDrawPage::~SvxDrawPage() throw()
{
    DBG_ASSERT( mrBHelper.bDisposed, "SvxDrawPage must be disposed!" );
    if( !mrBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
    DBG_DTOR(SvxDrawPage,NULL);
}

//----------------------------------------------------------------------

// XInterface
void SvxDrawPage::release() throw()
{
    OWeakAggObject::release();
}

// XComponent
void SvxDrawPage::disposing() throw()
{
    if( mpModel )
    {
        EndListening( *mpModel );
        mpModel = NULL;
    }

    if( mpView )
    {
        delete mpView;
        mpView = NULL;
    }
    mpPage = 0;
}

//----------------------------------------------------------------------
// XComponent
//----------------------------------------------------------------------

void SvxDrawPage::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it rubust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multible threading
    // Remark: It is an error to call dispose more than once
    sal_Bool bDoDispose = sal_False;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        // only one call go into this section
        mrBHelper.bInDispose = sal_True;
        bDoDispose = sal_True;
    }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( (lang::XComponent *)this ) );
            ::com::sun::star::document::EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automaticly cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(const ::com::sun::star::uno::Exception&)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        mrBHelper.bDisposed = sal_True;
        mrBHelper.bInDispose = sal_False;
    }

}

//----------------------------------------------------------------------

void SAL_CALL SvxDrawPage::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpModel == 0 )
        throw lang::DisposedException();

    mrBHelper.addListener( ::getCppuType( &aListener ) , aListener );
}

//----------------------------------------------------------------------

void SAL_CALL SvxDrawPage::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpModel == 0 )
        throw lang::DisposedException();

    mrBHelper.removeListener( ::getCppuType( &aListener ) , aListener );
}

//----------------------------------------------------------------------
// SfxListener
//----------------------------------------------------------------------

void SvxDrawPage::Notify( SfxBroadcaster&, const SfxHint& /*rHint*/ )
{
}

//----------------------------------------------------------------------
// ::com::sun::star::drawing::XShapes
//----------------------------------------------------------------------

void SAL_CALL SvxDrawPage::add( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if ( ( mpModel == NULL ) || ( mpPage == NULL ) )
        throw lang::DisposedException();

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( NULL == pShape )
        return;

    SdrObject *pObj = pShape->GetSdrObject();

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape );
        ENSURE_OR_RETURN_VOID( pObj != NULL, "SvxDrawPage::add: no SdrObject was created!" );
    }
    else if ( !pObj->IsInserted() )
    {
        pObj->SetModel(mpModel);
        mpPage->InsertObject( pObj );
    }

    pShape->Create( pObj, this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj, "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::remove( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(pShape)
    {
        SdrObject*  pObj = pShape->GetSdrObject();
        if(pObj)
        {
            // SdrObject aus der Page loeschen
            sal_uInt32 nCount = mpPage->GetObjCount();
            for( sal_uInt32 nNum = 0; nNum < nCount; nNum++ )
            {
                if(mpPage->GetObj(nNum) == pObj)
                {
                    OSL_VERIFY( mpPage->RemoveObject( nNum ) == pObj );
                    SdrObject::Free( pObj );
                    break;
                }
            }
        }
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
// ::com::sun::star::container::XIndexAccess
//----------------------------------------------------------------------

sal_Int32 SAL_CALL SvxDrawPage::getCount()
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    return( (sal_Int32) mpPage->GetObjCount() );
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxDrawPage::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    if ( Index < 0 || Index >= (sal_Int32)mpPage->GetObjCount() )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pObj = mpPage->GetObj( Index );
    if( pObj == NULL )
        throw uno::RuntimeException();


    return makeAny(Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY ));
}


//----------------------------------------------------------------------
// ::com::sun::star::container::XElementAccess
//----------------------------------------------------------------------

uno::Type SAL_CALL SvxDrawPage::getElementType()
    throw( uno::RuntimeException )
{
    return INTERFACE_TYPE( drawing::XShape );
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxDrawPage::hasElements()
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    return mpPage && mpPage->GetObjCount()>0;
}

namespace
{
    void lcl_markSdrObjectOfShape( const Reference< drawing::XShape >& _rxShape, SdrView& _rView, SdrPageView& _rPageView )
    {
        SvxShape* pShape = SvxShape::getImplementation( _rxShape );
        if ( !pShape )
            return;

        SdrObject* pObj = pShape->GetSdrObject();
        if ( !pObj )
            return;

        _rView.MarkObj( pObj, &_rPageView );
    }
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectsInView selektiert die ::com::sun::star::drawing::Shapes nur in der angegebennen
//         SdrPageView. Dies mu� nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectsInView( const Reference< drawing::XShapes > & aShapes, SdrPageView* pPageView ) throw ()
{
    DBG_ASSERT(pPageView,"SdrPageView ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(pPageView!=NULL && mpView!=NULL)
    {
        mpView->UnmarkAllObj( pPageView );

        long nCount = aShapes->getCount();
        for( long i = 0; i < nCount; i++ )
        {
            uno::Any aAny( aShapes->getByIndex(i) );
            Reference< drawing::XShape > xShape;
            if( aAny >>= xShape )
                lcl_markSdrObjectOfShape( xShape, *mpView, *pPageView );
        }
    }
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectInView selektiert das Shape *nur* in der angegebennen
//         SdrPageView. Dies mu� nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectInView( const Reference< drawing::XShape > & xShape, SdrPageView* pPageView ) throw()
{
    DBG_ASSERT(pPageView,"SdrPageView ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(pPageView!=NULL && mpView != NULL)
    {
        mpView->UnmarkAllObj( pPageView );
        lcl_markSdrObjectOfShape( xShape, *mpView, *pPageView );
    }
}

//----------------------------------------------------------------------
Reference< drawing::XShapeGroup > SAL_CALL SvxDrawPage::group( const Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    DBG_ASSERT(mpPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    Reference< ::com::sun::star::drawing::XShapeGroup >  xShapeGroup;
    if(mpPage==NULL||mpView==NULL||!xShapes.is())
        return xShapeGroup;

    SdrPageView* pPageView = mpView->ShowSdrPage( mpPage );

    _SelectObjectsInView( xShapes, pPageView );

    mpView->GroupMarked();

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
             xShapeGroup = Reference< drawing::XShapeGroup >::query( pObj->getUnoShape() );
    }

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();

    return xShapeGroup;
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::ungroup( const Reference< drawing::XShapeGroup >& aGroup )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    DBG_ASSERT(mpPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(mpPage==NULL||mpView==NULL||!aGroup.is())
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( mpPage );

    Reference< drawing::XShape > xShape( aGroup, UNO_QUERY );
    _SelectObjectInView( xShape, pPageView );
    mpView->UnGroupMarked();

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
SdrObject *SvxDrawPage::_CreateSdrObject( const Reference< drawing::XShape > & xShape ) throw()
{
    sal_uInt16 nType;
    sal_uInt32 nInventor;

    GetTypeAndInventor( nType, nInventor, xShape->getShapeType() );
    SdrObject* pNewObj = 0;

    if( nType != 0 )
    {
        awt::Size aSize = xShape->getSize();
        aSize.Width += 1;
        aSize.Height += 1;
        awt::Point aPos = xShape->getPosition();
        Rectangle aRect( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) );

        // special cases
        if( nInventor == SdrInventor )
        {
            switch( nType )
            {
            case OBJ_MEASURE:
                {
                    pNewObj = new SdrMeasureObj( aRect.TopLeft(), aRect.BottomRight() );
                    break;
                }
            case OBJ_LINE:
                {
                    basegfx::B2DPolygon aPoly;
                    aPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Top()));
                    aPoly.append(basegfx::B2DPoint(aRect.Right(), aRect.Bottom()));
                    pNewObj = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPoly));
                    break;
                }
            }
        }

        if( pNewObj == NULL )
            pNewObj = SdrObjFactory::MakeNewObject( nInventor, nType, mpPage );

        if(pNewObj)
        {
            pNewObj->SetSnapRect(aRect);

            if( pNewObj->ISA(E3dPolyScene))
            {
                // Szene initialisieren
                E3dScene* pScene = (E3dScene*)pNewObj;

                double fW = (double)aSize.Width;
                double fH = (double)aSize.Height;

                Camera3D aCam(pScene->GetCamera());
                aCam.SetAutoAdjustProjection(sal_False);
                aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
                basegfx::B3DPoint aLookAt;
                basegfx::B3DPoint aCamPos(0.0, 0.0, 10000.0);
                aCam.SetPosAndLookAt(aCamPos, aLookAt);
                aCam.SetFocalLength(100.0);
                aCam.SetDefaults(aCamPos, aLookAt, 10000.0);
                pScene->SetCamera(aCam);

                pScene->SetRectsDirty();
            }
            else if(pNewObj->ISA(E3dExtrudeObj))
            {
                E3dExtrudeObj* pObj = (E3dExtrudeObj*)pNewObj;
                basegfx::B2DPolygon aNewPolygon;
                aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
                aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
                aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
                aNewPolygon.setClosed(true);
                pObj->SetExtrudePolygon(basegfx::B2DPolyPolygon(aNewPolygon));

                // #107245# pObj->SetExtrudeCharacterMode(sal_True);
                pObj->SetMergedItem(Svx3DCharacterModeItem(sal_True));
            }
            else if(pNewObj->ISA(E3dLatheObj))
            {
                E3dLatheObj* pObj = (E3dLatheObj*)pNewObj;
                basegfx::B2DPolygon aNewPolygon;
                aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
                aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
                aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
                aNewPolygon.setClosed(true);
                pObj->SetPolyPoly2D(basegfx::B2DPolyPolygon(aNewPolygon));

                // #107245# pObj->SetLatheCharacterMode(sal_True);
                pObj->SetMergedItem(Svx3DCharacterModeItem(sal_True));
            }
        }
    }

    return pNewObj;
}

//----------------------------------------------------------------------
void SvxDrawPage::GetTypeAndInventor( sal_uInt16& rType, sal_uInt32& rInventor, const OUString& aName ) const throw()
{
    sal_uInt32 nTempType = UHashMap::getId( aName );

    if( nTempType == UHASHMAP_NOTFOUND )
    {
        if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TableShape")) ||
            aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.TableShape")) )
        {
            rInventor = SdrInventor;
            rType = OBJ_TABLE;
        }
        else if ( aName == "com.sun.star.presentation.MediaShape" )
        {
            rInventor = SdrInventor;
            rType = OBJ_MEDIA;
        }
    }
    else if(nTempType & E3D_INVENTOR_FLAG)
    {
        rInventor = E3dInventor;
        rType = (sal_uInt16)(nTempType & ~E3D_INVENTOR_FLAG);
    }
    else
    {
        rInventor = SdrInventor;
        rType = (sal_uInt16)nTempType;

        switch( rType )
        {
            case OBJ_FRAME:
            case OBJ_OLE2_PLUGIN:
            case OBJ_OLE2_APPLET:
                rType = OBJ_OLE2;
                break;
        }
    }
}

//----------------------------------------------------------------------
SvxShape* SvxDrawPage::CreateShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor, SdrObject *pObj, SvxDrawPage *mpPage ) throw()
{
    SvxShape* pRet = NULL;
    switch( nInventor )
    {
        case E3dInventor:
        {
            switch( nType )
            {
                case E3D_SCENE_ID :
                case E3D_POLYSCENE_ID :
                    pRet = new Svx3DSceneObject( pObj, mpPage );
                    break;
                case E3D_CUBEOBJ_ID :
                    pRet = new Svx3DCubeObject( pObj );
                    break;
                case E3D_SPHEREOBJ_ID :
                    pRet = new Svx3DSphereObject( pObj );
                    break;
                case E3D_LATHEOBJ_ID :
                    pRet = new Svx3DLatheObject( pObj );
                    break;
                case E3D_EXTRUDEOBJ_ID :
                    pRet = new Svx3DExtrudeObject( pObj );
                    break;
                case E3D_POLYGONOBJ_ID :
                    pRet = new Svx3DPolygonObject( pObj );
                    break;
                default: // unbekanntes 3D-Objekt auf der Page
                    pRet = new SvxShape( pObj );
                    break;
            }
            break;
        }
        case SdrInventor:
        {
            switch( nType )
            {
//              case OBJ_NONE:
//                  break;
                case OBJ_GRUP:
                    pRet = new SvxShapeGroup( pObj, mpPage );
                    break;
                case OBJ_LINE:
                    pRet = new SvxShapePolyPolygon( pObj , PolygonKind_LINE );
                    break;
                case OBJ_RECT:
                    pRet = new SvxShapeRect( pObj );
                    break;
                case OBJ_CIRC:
                case OBJ_SECT:
                case OBJ_CARC:
                case OBJ_CCUT:
                    pRet = new SvxShapeCircle( pObj );
                    break;
                case OBJ_POLY:
                    pRet = new SvxShapePolyPolygon( pObj , PolygonKind_POLY );
                    break;
                case OBJ_PLIN:
                    pRet = new SvxShapePolyPolygon( pObj , PolygonKind_PLIN );
                    break;
                case OBJ_SPLNLINE:
                case OBJ_PATHLINE:
                    pRet = new SvxShapePolyPolygonBezier( pObj , PolygonKind_PATHLINE );
                    break;
                case OBJ_SPLNFILL:
                case OBJ_PATHFILL:
                    pRet = new SvxShapePolyPolygonBezier( pObj , PolygonKind_PATHFILL );
                    break;
                case OBJ_FREELINE:
                    pRet = new SvxShapePolyPolygonBezier( pObj , PolygonKind_FREELINE );
                    break;
                case OBJ_FREEFILL:
                    pRet = new SvxShapePolyPolygonBezier( pObj , PolygonKind_FREEFILL );
                    break;
                case OBJ_CAPTION:
                    pRet = new SvxShapeCaption( pObj );
                    break;
                case OBJ_TITLETEXT:
                case OBJ_OUTLINETEXT:
                case OBJ_TEXT:
                    pRet = new SvxShapeText( pObj );
                    break;
                case OBJ_GRAF:
                    pRet = new SvxGraphicObject( pObj );
                    break;
                case OBJ_FRAME:
                    pRet = new SvxFrameShape( pObj );
                    break;
                case OBJ_OLE2_APPLET:
                    pRet = new SvxAppletShape( pObj );
                    break;
                case OBJ_OLE2_PLUGIN:
                    pRet = new SvxPluginShape( pObj );
                    break;
                 case OBJ_OLE2:
                     {
                        if( pObj && !pObj->IsEmptyPresObj() && mpPage )
                        {
                            SdrPage* pSdrPage = mpPage->GetSdrPage();
                            if( pSdrPage )
                            {
                                SdrModel* pSdrModel = pSdrPage->GetModel();
                                if( pSdrModel )
                                {
                                    ::comphelper::IEmbeddedHelper *pPersist = pSdrModel->GetPersist();
                                    if( pPersist )
                                    {
                                        uno::Reference < embed::XEmbeddedObject > xObject = pPersist->getEmbeddedObjectContainer().
                                                GetEmbeddedObject( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() );

                                        // TODO CL->KA: Why is this not working anymore?
                                        if( xObject.is() )
                                        {
                                            SvGlobalName aClassId( xObject->getClassID() );

                                            const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID );
                                            const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID );
                                            const SvGlobalName aIFrameClassId( SO3_IFRAME_CLASSID );

                                            if( aPluginClassId == aClassId )
                                            {
                                                pRet = new SvxPluginShape( pObj );
                                                nType = OBJ_OLE2_PLUGIN;
                                            }
                                            else if( aAppletClassId == aClassId )
                                            {
                                                pRet = new SvxAppletShape( pObj );
                                                nType = OBJ_OLE2_APPLET;
                                            }
                                            else if( aIFrameClassId == aClassId )
                                            {
                                                pRet = new SvxFrameShape( pObj );
                                                nType = OBJ_FRAME;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if( pRet == NULL )
                        {
                            SvxUnoPropertyMapProvider& rSvxMapProvider = getSvxMapProvider();
                            pRet = new SvxOle2Shape( pObj, rSvxMapProvider.GetMap(SVXMAP_OLE2),  rSvxMapProvider.GetPropertySet(SVXMAP_OLE2, SdrObject::GetGlobalDrawObjectItemPool()) );
                        }
                     }
                    break;
                case OBJ_EDGE:
                    pRet = new SvxShapeConnector( pObj );
                    break;
                case OBJ_PATHPOLY:
                    pRet = new SvxShapePolyPolygon( pObj , PolygonKind_PATHPOLY );
                    break;
                case OBJ_PATHPLIN:
                    pRet = new SvxShapePolyPolygon( pObj , PolygonKind_PATHPLIN );
                    break;
                case OBJ_PAGE:
                {
                    SvxUnoPropertyMapProvider& rSvxMapProvider = getSvxMapProvider();
                    pRet = new SvxShape( pObj, rSvxMapProvider.GetMap(SVXMAP_PAGE),  rSvxMapProvider.GetPropertySet(SVXMAP_PAGE, SdrObject::GetGlobalDrawObjectItemPool()) );
                }
                    break;
                case OBJ_MEASURE:
                    pRet = new SvxShapeDimensioning( pObj );
                    break;
//              case OBJ_DUMMY:
//                  break;
                case OBJ_UNO:
                    pRet = new SvxShapeControl( pObj );
                    break;
                case OBJ_CUSTOMSHAPE:
                    pRet = new SvxCustomShape( pObj );
                    break;
                case OBJ_MEDIA:
                    pRet = new SvxMediaShape( pObj );
                    break;
                case OBJ_TABLE:
                    pRet = new SvxTableShape( pObj );
                    break;
                default: // unbekanntes 2D-Objekt auf der Page
                    OSL_FAIL("Nicht implementierter Starone-Shape erzeugt! [CL]");
                    pRet = new SvxShapeText( pObj );
                    break;
            }
            break;
        }
        default: // Unbekannter Inventor
        {
            OSL_FAIL("AW: Unknown Inventor in SvxDrawPage::_CreateShape()");
            break;
        }
    }

    if(pRet)
    {
        sal_uInt32 nObjId = nType;

        if( nInventor == E3dInventor )
            nObjId |= E3D_INVENTOR_FLAG;

        switch(nObjId)
        {
        case OBJ_CCUT:          // Kreisabschnitt
        case OBJ_CARC:          // Kreisbogen
        case OBJ_SECT:          // Kreissektor
            nObjId = OBJ_CIRC;
            break;

        case E3D_SCENE_ID | E3D_INVENTOR_FLAG:
            nObjId = E3D_POLYSCENE_ID | E3D_INVENTOR_FLAG;
            break;

        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
            nObjId = OBJ_TEXT;
            break;
        }

        pRet->setShapeKind(nObjId);
    }

    return pRet;
}

//----------------------------------------------------------------------
Reference< drawing::XShape >  SvxDrawPage::_CreateShape( SdrObject *pObj ) const throw()
{
    Reference< drawing::XShape > xShape( CreateShapeByTypeAndInventor(pObj->GetObjIdentifier(),
                                              pObj->GetObjInventor(),
                                              pObj,
                                              (SvxDrawPage*)this));
    return xShape;
}

//----------------------------------------------------------------------
SdrObject *SvxDrawPage::CreateSdrObject( const Reference< drawing::XShape > & xShape ) throw()
{
    SdrObject* pObj = _CreateSdrObject( xShape );
    if( pObj && !pObj->IsInserted() )
        mpPage->InsertObject( pObj );

    return pObj;
}

//----------------------------------------------------------------------
// ::com::sun::star::lang::XServiceInfo
//----------------------------------------------------------------------
OUString SAL_CALL SvxDrawPage::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxDrawPage"));
}

sal_Bool SAL_CALL SvxDrawPage::supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxDrawPage::getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ShapeCollection" ));
    return aSeq;
}

SvxShape* CreateSvxShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor ) throw()
{
    return SvxDrawPage::CreateShapeByTypeAndInventor( nType, nInventor );
}

void SvxDrawPage::ChangeModel( SdrModel* pNewModel )
{
    if( pNewModel != mpModel )
    {
        if( mpModel )
            EndListening( *mpModel );

        if( pNewModel )
            StartListening( *pNewModel );

        mpModel = pNewModel;

        if( mpView )
        {
            delete mpView;
            mpView = new SdrView( mpModel );
            if( mpView )
                mpView->SetDesignMode(sal_True);
        }
    }
}

/** returns a StarOffice API wrapper for the given SdrPage */
uno::Reference< drawing::XDrawPage > GetXDrawPageForSdrPage( SdrPage* pPage ) throw ()
{
    if(pPage)
    {
        uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );

        return xDrawPage;
    }

    return uno::Reference< drawing::XDrawPage >();
}

/** returns the SdrObject from the given StarOffice API wrapper */
SdrPage* GetSdrPageFromXDrawPage( uno::Reference< drawing::XDrawPage > xDrawPage ) throw()
{
    if(xDrawPage.is())
    {
        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xDrawPage );

        if(pDrawPage)
        {
            return pDrawPage->GetSdrPage();
        }
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
