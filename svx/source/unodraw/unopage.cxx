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

#include <config_features.h>

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/mutex.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/classids.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <sfx2/objsh.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdstr.hrc>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/unopage.hxx>
#include "shapeimpl.hxx"
#include "svdglob.hxx"
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
#include <tools/globname.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

UNO3_GETIMPLEMENTATION_IMPL( SvxDrawPage );

SvxDrawPage::SvxDrawPage(SdrPage* pInPage) throw()
    : mrBHelper(getMutex())
    , mpPage(pInPage)
    , mpModel(mpPage->GetModel())  // register at broadcaster
    , mpView(new SdrView(mpModel)) // create (hidden) view

{
    mpView->SetDesignMode();
}

SvxDrawPage::~SvxDrawPage() throw()
{
    if( !mrBHelper.bDisposed )
    {
        assert("SvxDrawPage must be disposed!");
        acquire();
        dispose();
    }
}

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
        mpModel = nullptr;
    }

    if( mpView )
    {
        delete mpView;
        mpView = nullptr;
    }
    mpPage = nullptr;
}

// XComponent
void SvxDrawPage::dispose()
{
    SolarMutexGuard aSolarGuard;

    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it robust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multiple threading
    // Remark: It is an error to call dispose more than once
    bool bDoDispose = false;
    {
        osl::MutexGuard aGuard( mrBHelper.rMutex );
        if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
        {
            // only one call go into this section
            mrBHelper.bInDispose = true;
            bDoDispose = true;
        }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( static_cast<lang::XComponent *>(this) ) );
            css::document::EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automatically cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(const css::uno::Exception&)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            osl::MutexGuard aGuard( mrBHelper.rMutex );
            mrBHelper.bDisposed = true;
            mrBHelper.bInDispose = false;
            throw;
        }

        osl::MutexGuard aGuard( mrBHelper.rMutex );
        mrBHelper.bDisposed = true;
        mrBHelper.bInDispose = false;
    }

}

void SAL_CALL SvxDrawPage::addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    mrBHelper.addListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

void SAL_CALL SvxDrawPage::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    mrBHelper.removeListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

void SAL_CALL SvxDrawPage::add( const uno::Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    if ( ( mpModel == nullptr ) || ( mpPage == nullptr ) )
        throw lang::DisposedException();

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( nullptr == pShape )
        return;

    SdrObject *pObj = pShape->GetSdrObject();

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape );
        ENSURE_OR_RETURN_VOID( pObj != nullptr, "SvxDrawPage::add: no SdrObject was created!" );
    }
    else if ( !pObj->IsInserted() )
    {
        pObj->SetModel(mpModel);
        mpPage->InsertObject( pObj );
    }

    pShape->Create( pObj, this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj, "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    if ( !pObj->IsInserted() )
    {
        pObj->SetModel(mpModel);
        mpPage->InsertObject( pObj );
    }

    mpModel->SetChanged();
}

void SAL_CALL SvxDrawPage::addTop( const uno::Reference< drawing::XShape >& xShape )
{
    add(xShape);
}

void SAL_CALL SvxDrawPage::addBottom( const uno::Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    if ( ( mpModel == nullptr ) || ( mpPage == nullptr ) )
        throw lang::DisposedException();

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( nullptr == pShape )
        return;

    SdrObject *pObj = pShape->GetSdrObject();

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape, true );
        ENSURE_OR_RETURN_VOID( pObj != nullptr, "SvxDrawPage::add: no SdrObject was created!" );
    }
    else if ( !pObj->IsInserted() )
    {
        pObj->SetModel(mpModel);
        mpPage->InsertObject( pObj, 0 );
    }

    pShape->Create( pObj, this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj, "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    if ( !pObj->IsInserted() )
    {
        pObj->SetModel(mpModel);
        mpPage->InsertObject( pObj, 0 );
    }

    mpModel->SetChanged();
}

void SAL_CALL SvxDrawPage::remove( const Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if (pShape)
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if (pObj)
        {
            // remove SdrObject from page
            const size_t nCount = mpPage->GetObjCount();
            for( size_t nNum = 0; nNum < nCount; ++nNum )
            {
                if(mpPage->GetObj(nNum) == pObj)
                {
                    const bool bUndoEnabled = mpModel->IsUndoEnabled();

                    if (bUndoEnabled)
                    {
                        mpModel->BegUndo(ImpGetResStr(STR_EditDelete),
                            pObj->TakeObjNameSingul(), SdrRepeatFunc::Delete);

                        SdrUndoAction * pAction = mpModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj);
                        mpModel->AddUndo(pAction);
                    }

                    OSL_VERIFY( mpPage->RemoveObject( nNum ) == pObj );

                    if (!bUndoEnabled)
                        SdrObject::Free(pObj);

                    if (bUndoEnabled)
                        mpModel->EndUndo();

                    break;
                }
            }
        }
    }

    mpModel->SetChanged();
}

// css::container::XIndexAccess
sal_Int32 SAL_CALL SvxDrawPage::getCount()
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    return static_cast<sal_Int32>( mpPage->GetObjCount() );
}

uno::Any SAL_CALL SvxDrawPage::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    if ( Index < 0 || static_cast<size_t>(Index) >= mpPage->GetObjCount() )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pObj = mpPage->GetObj( Index );
    if( pObj == nullptr )
        throw uno::RuntimeException();


    return makeAny(Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY ));
}

// css::container::XElementAccess
uno::Type SAL_CALL SvxDrawPage::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SAL_CALL SvxDrawPage::hasElements()
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
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

// ATTENTION: SelectObjectsInView selects the css::drawing::Shapes
// only in the given SdrPageView. It hasn't to be the visible SdrPageView.
void SvxDrawPage::SelectObjectsInView( const Reference< drawing::XShapes > & aShapes, SdrPageView* pPageView ) throw ()
{
    SAL_WARN_IF(!pPageView, "svx", "SdrPageView is NULL!");
    SAL_WARN_IF(!mpView, "svx", "SdrView is NULL!");

    if(pPageView!=nullptr && mpView!=nullptr)
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

// ATTENTION: SelectObjectInView selects the shape only in the given SdrPageView.
// It hasn't to be the visible SdrPageView.
void SvxDrawPage::SelectObjectInView( const Reference< drawing::XShape > & xShape, SdrPageView* pPageView ) throw()
{
    SAL_WARN_IF(!pPageView, "svx", "SdrPageView is NULL!");
    SAL_WARN_IF(!mpView, "svx", "SdrView is NULL!");

    if(pPageView!=nullptr && mpView != nullptr)
    {
        mpView->UnmarkAllObj( pPageView );
        lcl_markSdrObjectOfShape( xShape, *mpView, *pPageView );
    }
}

Reference< drawing::XShapeGroup > SAL_CALL SvxDrawPage::group( const Reference< drawing::XShapes >& xShapes )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    SAL_WARN_IF(!mpPage , "svx", "SdrPage is NULL!");
    SAL_WARN_IF(!mpView, "svx", "SdrView is NULL!");

    Reference< css::drawing::XShapeGroup >  xShapeGroup;
    if(mpPage==nullptr||mpView==nullptr||!xShapes.is())
        return xShapeGroup;

    SdrPageView* pPageView = mpView->ShowSdrPage( mpPage );

    SelectObjectsInView( xShapes, pPageView );

    mpView->GroupMarked();

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
             xShapeGroup.set( pObj->getUnoShape(), UNO_QUERY );
    }

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();

    return xShapeGroup;
}

void SAL_CALL SvxDrawPage::ungroup( const Reference< drawing::XShapeGroup >& aGroup )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    SAL_WARN_IF(!mpPage, "svx", "SdrPage is NULL!");
    SAL_WARN_IF(!mpView, "svx", "SdrView is NULL!");

    if(mpPage==nullptr||mpView==nullptr||!aGroup.is())
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( mpPage );

    Reference< drawing::XShape > xShape( aGroup, UNO_QUERY );
    SelectObjectInView( xShape, pPageView );
    mpView->UnGroupMarked();

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();
}

SdrObject *SvxDrawPage::CreateSdrObject_(const Reference< drawing::XShape > & xShape)
{
    sal_uInt16 nType = 0;
    SdrInventor nInventor;

    GetTypeAndInventor( nType, nInventor, xShape->getShapeType() );
    if (!nType)
        return nullptr;

    awt::Size aSize = xShape->getSize();
    aSize.Width += 1;
    aSize.Height += 1;
    awt::Point aPos = xShape->getPosition();
    Rectangle aRect( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) );

    SdrObject* pNewObj = SdrObjFactory::MakeNewObject(nInventor, nType, aRect, mpPage);
    if (!pNewObj)
        return nullptr;

    if( dynamic_cast<const E3dPolyScene* >(pNewObj) !=  nullptr)
    {
        // initialise scene
        E3dScene* pScene = static_cast<E3dScene*>(pNewObj);

        double fW = (double)aSize.Width;
        double fH = (double)aSize.Height;

        Camera3D aCam(pScene->GetCamera());
        aCam.SetAutoAdjustProjection(false);
        aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
        basegfx::B3DPoint aLookAt;
        basegfx::B3DPoint aCamPos(0.0, 0.0, 10000.0);
        aCam.SetPosAndLookAt(aCamPos, aLookAt);
        aCam.SetFocalLength(100.0);
        aCam.SetDefaults(aCamPos, aLookAt, 10000.0);
        pScene->SetCamera(aCam);

        pScene->SetRectsDirty();
    }
    else if(dynamic_cast<const E3dExtrudeObj* >(pNewObj) !=  nullptr)
    {
        E3dExtrudeObj* pObj = static_cast<E3dExtrudeObj*>(pNewObj);
        basegfx::B2DPolygon aNewPolygon;
        aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
        aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
        aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
        aNewPolygon.setClosed(true);
        pObj->SetExtrudePolygon(basegfx::B2DPolyPolygon(aNewPolygon));

        // #107245# pObj->SetExtrudeCharacterMode(sal_True);
        pObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }
    else if(dynamic_cast<const E3dLatheObj* >(pNewObj) !=  nullptr)
    {
        E3dLatheObj* pObj = static_cast<E3dLatheObj*>(pNewObj);
        basegfx::B2DPolygon aNewPolygon;
        aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
        aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
        aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
        aNewPolygon.setClosed(true);
        pObj->SetPolyPoly2D(basegfx::B2DPolyPolygon(aNewPolygon));

        // #107245# pObj->SetLatheCharacterMode(sal_True);
        pObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    return pNewObj;
}

void SvxDrawPage::GetTypeAndInventor( sal_uInt16& rType, SdrInventor& rInventor, const OUString& aName ) throw()
{
    sal_uInt32 nTempType = UHashMap::getId( aName );

    if( nTempType == UHASHMAP_NOTFOUND )
    {
        if( aName == "com.sun.star.drawing.TableShape" ||
            aName == "com.sun.star.presentation.TableShape" )
        {
            rInventor = SdrInventor::Default;
            rType = OBJ_TABLE;
        }
#if HAVE_FEATURE_AVMEDIA
        else if ( aName == "com.sun.star.presentation.MediaShape" )
        {
            rInventor = SdrInventor::Default;
            rType = OBJ_MEDIA;
        }
#endif
    }
    else if(nTempType & E3D_INVENTOR_FLAG)
    {
        rInventor = SdrInventor::E3d;
        rType = (sal_uInt16)(nTempType & ~E3D_INVENTOR_FLAG);
    }
    else
    {
        rInventor = SdrInventor::Default;
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

SvxShape* SvxDrawPage::CreateShapeByTypeAndInventor( sal_uInt16 nType, SdrInventor nInventor, SdrObject *pObj, SvxDrawPage *mpPage, OUString const & referer )
{
    SvxShape* pRet = nullptr;
    switch( nInventor )
    {
        case SdrInventor::E3d:
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
                default: // unknown 3D-object on page
                    pRet = new SvxShape( pObj );
                    break;
            }
            break;
        }
        case SdrInventor::Default:
        {
            switch( nType )
            {
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
                    pRet = new SvxGraphicObject( pObj, referer );
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
                        if( pRet == nullptr )
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
                case OBJ_UNO:
                    pRet = new SvxShapeControl( pObj );
                    break;
                case OBJ_CUSTOMSHAPE:
                    pRet = new SvxCustomShape( pObj );
                    break;
#if HAVE_FEATURE_DESKTOP
                case OBJ_MEDIA:
                    pRet = new SvxMediaShape( pObj, referer );
                    break;
#endif
                case OBJ_TABLE:
                    pRet = new SvxTableShape( pObj );
                    break;
                default: // unknown 2D-object on page
                    OSL_FAIL("Not implemented Starone-Shape created! [CL]");
                    pRet = new SvxShapeText( pObj );
                    break;
            }
            break;
        }
        default: // unknown inventor
        {
            OSL_FAIL("AW: Unknown Inventor in SvxDrawPage::CreateShape()");
            break;
        }
    }

    if(pRet)
    {
        sal_uInt32 nObjId = nType;

        if( nInventor == SdrInventor::E3d )
            nObjId |= E3D_INVENTOR_FLAG;

        switch(nObjId)
        {
        case OBJ_CCUT:          // segment of circle
        case OBJ_CARC:          // arc of circle
        case OBJ_SECT:          // sector
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

Reference< drawing::XShape >  SvxDrawPage::CreateShape( SdrObject *pObj ) const
{
    Reference< drawing::XShape > xShape( CreateShapeByTypeAndInventor(pObj->GetObjIdentifier(),
                                              pObj->GetObjInventor(),
                                              pObj,
                                              const_cast<SvxDrawPage*>(this)));
    return xShape;
}

SdrObject *SvxDrawPage::CreateSdrObject( const Reference< drawing::XShape > & xShape, bool bBeginning ) throw()
{
    SdrObject* pObj = CreateSdrObject_( xShape );
    if( pObj)
    {
        pObj->SetModel(mpModel);
        if ( !pObj->IsInserted() && !pObj->IsDoNotInsertIntoPageAutomatically() )
        {
            if(bBeginning)
                mpPage->InsertObject( pObj, 0 );
            else
                mpPage->InsertObject( pObj );
        }
    }

    return pObj;
}

// css::lang::XServiceInfo
OUString SAL_CALL SvxDrawPage::getImplementationName()
{
    return OUString("SvxDrawPage");
}

sal_Bool SAL_CALL SvxDrawPage::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SvxDrawPage::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSeq { "com.sun.star.drawing.ShapeCollection" };
    return aSeq;
}

SvxShape* CreateSvxShapeByTypeAndInventor(sal_uInt16 nType, SdrInventor nInventor, OUString const & referer)
{
    return SvxDrawPage::CreateShapeByTypeAndInventor( nType, nInventor, nullptr, nullptr, referer );
}

void SvxDrawPage::ChangeModel( SdrModel* pNewModel )
{
    if( pNewModel != mpModel )
    {
        mpModel = pNewModel;

        if( mpView )
        {
            delete mpView;
            mpView = new SdrView( mpModel );
            mpView->SetDesignMode();
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
SdrPage* GetSdrPageFromXDrawPage( const uno::Reference< drawing::XDrawPage >& xDrawPage ) throw()
{
    if(xDrawPage.is())
    {
        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xDrawPage );

        if(pDrawPage)
        {
            return pDrawPage->GetSdrPage();
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
