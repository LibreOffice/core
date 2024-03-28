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
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <o3tl/safeint.hxx>
#include <osl/mutex.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/strings.hrc>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/unopage.hxx>
#include "shapeimpl.hxx"
#include <svx/unodraw/SvxTableShape.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/unoprov.hxx>
#include <svx/unoapi.hxx>
#include <extrud3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/scene3d.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/globname.hxx>
#include <sal/log.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

UNO3_GETIMPLEMENTATION_IMPL( SvxDrawPage );

SvxDrawPage::SvxDrawPage(SdrPage* pInPage) // TTTT should be reference
:   mrBHelper(m_aMutex)
    ,mpPage(pInPage)
    ,mpModel(&pInPage->getSdrModelFromSdrPage())  // register at broadcaster
    ,mpView(new SdrView(pInPage->getSdrModelFromSdrPage())) // create (hidden) view
{
    mpView->SetDesignMode();
}

SvxDrawPage::~SvxDrawPage() noexcept
{
    if( !mrBHelper.bDisposed )
    {
        assert(!"SvxDrawPage must be disposed!");
        acquire();
        dispose();
    }
}

// XInterface
void SvxDrawPage::release() noexcept
{
    OWeakAggObject::release();
}

// XComponent
void SvxDrawPage::disposing() noexcept
{
    if( mpModel )
    {
        mpModel = nullptr;
    }

    mpView.reset();
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
    if( !bDoDispose )
        return;

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

    SvxShape* pShape = comphelper::getFromUnoTunnel<SvxShape>( xShape );

    if( nullptr == pShape )
    {
        assert(false && "adding a non-SvxShape to a page?");
        return;
    }

    rtl::Reference<SdrObject> pObj = pShape->GetSdrObject();
    bool bNeededToClone(false);

    if(pObj && &pObj->getSdrModelFromSdrObject() != &mpPage->getSdrModelFromSdrPage())
    {
        // TTTT UNO API tries to add an existing SvxShape to this SvxDrawPage,
        // but these use different SdrModels. It was possible before to completely
        // 'change' a SdrObject to another SdrModel (including dangerous MigrateItemPool
        // stuff), but is no longer. We need to Clone the SdrObject to the target model
        // and ::Create a new SvxShape (set SdrObject there, take over values, ...)
        rtl::Reference<SdrObject> pClonedSdrShape(pObj->CloneSdrObject(mpPage->getSdrModelFromSdrPage()));
        pObj->setUnoShape(nullptr);
        pClonedSdrShape->setUnoShape(pShape);
        // pShape->InvalidateSdrObject();
        // pShape->Create(pClonedSdrShape, this);
        pObj = pClonedSdrShape;
        bNeededToClone = true;
    }

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape );
        ENSURE_OR_RETURN_VOID( pObj != nullptr, "SvxDrawPage::add: no SdrObject was created!" );
    }
    else if ( !pObj->IsInserted() )
    {
        mpPage->InsertObject( pObj.get() );

        if(bNeededToClone)
        {
            // TTTT Unfortunately in SdrObject::SetPage (see there) the
            // xShape/UnoShape at the newly cloned SDrObject is *removed* again,
            // so re-set it here, the caller *may need it* (e.g. Writer)
            uno::Reference< drawing::XShape > xShapeCheck(pObj->getWeakUnoShape());

            if( !xShapeCheck.is() )
            {
                pObj->setUnoShape(pShape);
            }
        }
    }

    pShape->Create( pObj.get(), this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj.get(), "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    if ( !pObj->IsInserted() )
    {
        mpPage->InsertObject( pObj.get() );
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

    SvxShape* pShape = comphelper::getFromUnoTunnel<SvxShape>( xShape );

    if( nullptr == pShape )
    {
        assert(false && "adding a non-SvxShape to a page?");
        return;
    }

    rtl::Reference<SdrObject> pObj = pShape->GetSdrObject();

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape, true );
        ENSURE_OR_RETURN_VOID( pObj != nullptr, "SvxDrawPage::add: no SdrObject was created!" );
    }
    else if ( !pObj->IsInserted() )
    {
        mpPage->InsertObject( pObj.get(), 0 );
    }

    pShape->Create( pObj.get(), this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj.get(), "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    if ( !pObj->IsInserted() )
    {
        mpPage->InsertObject( pObj.get(), 0 );
    }

    mpModel->SetChanged();
}

void SAL_CALL SvxDrawPage::remove( const Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpPage == nullptr) )
        throw lang::DisposedException();

    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xShape );
    if (!pObj)
        return;

    // remove SdrObject from page
    const size_t nCount = mpPage->GetObjCount();
    for( size_t nNum = 0; nNum < nCount; ++nNum )
    {
        if(mpPage->GetObj(nNum) == pObj)
        {
            const bool bUndoEnabled = mpModel->IsUndoEnabled();

            if (bUndoEnabled)
            {
                mpModel->BegUndo(SvxResId(STR_EditDelete),
                    pObj->TakeObjNameSingul(), SdrRepeatFunc::Delete);

                mpModel->AddUndo(mpModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
            }

            OSL_VERIFY( mpPage->RemoveObject( nNum ) == pObj );

            if (bUndoEnabled)
                mpModel->EndUndo();

            break;
        }
    }

    mpModel->SetChanged();
}

void SvxDrawPage::sort( const css::uno::Sequence< sal_Int32 >& sortOrder )
{
    SolarMutexGuard aGuard;

    if ((mpModel == nullptr) || (mpPage == nullptr))
        throw lang::DisposedException();

    auto newOrder = comphelper::sequenceToContainer<std::vector<sal_Int32>>(sortOrder);
    mpPage->sort(newOrder);
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
        throw lang::DisposedException("Model or Page was already disposed!");

    if ( Index < 0 || o3tl::make_unsigned(Index) >= mpPage->GetObjCount() )
        throw lang::IndexOutOfBoundsException("Index (" + OUString::number(Index)
                                              + ") needs to be a positive integer smaller than the shape count ("
                                              + OUString::number(mpPage->GetObjCount()) + ")!");

    SdrObject* pObj = mpPage->GetObj( Index );
    if( pObj == nullptr )
        throw uno::RuntimeException("Runtime exception thrown while getting a ref to the SdrObject at index: "
                                    + OUString::number(Index));


    return Any(Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY ));
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
        SdrObject* pObj = SdrObject::getSdrObjectFromXShape( _rxShape );
        if ( !pObj )
            return;

        _rView.MarkObj( pObj, &_rPageView );
    }
}

// ATTENTION: SelectObjectsInView selects the css::drawing::Shapes
// only in the given SdrPageView. It hasn't to be the visible SdrPageView.
void SvxDrawPage::SelectObjectsInView( const Reference< drawing::XShapes > & aShapes, SdrPageView* pPageView ) noexcept
{
    SAL_WARN_IF(!pPageView, "svx", "SdrPageView is NULL!");
    SAL_WARN_IF(!mpView, "svx", "SdrView is NULL!");

    if(pPageView==nullptr || mpView==nullptr)
        return;

    mpView->UnmarkAllObj( pPageView );

    tools::Long nCount = aShapes->getCount();
    for( tools::Long i = 0; i < nCount; i++ )
    {
        uno::Any aAny( aShapes->getByIndex(i) );
        Reference< drawing::XShape > xShape;
        if( aAny >>= xShape )
            lcl_markSdrObjectOfShape( xShape, *mpView, *pPageView );
    }
}

// ATTENTION: SelectObjectInView selects the shape only in the given SdrPageView.
// It hasn't to be the visible SdrPageView.
void SvxDrawPage::SelectObjectInView( const Reference< drawing::XShape > & xShape, SdrPageView* pPageView ) noexcept
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

    SelectObjectInView( aGroup, pPageView );
    mpView->UnGroupMarked();

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();
}

rtl::Reference<SdrObject> SvxDrawPage::CreateSdrObject_(const Reference< drawing::XShape > & xShape)
{
    SdrObjKind nType = SdrObjKind::NONE;
    SdrInventor nInventor;

    GetTypeAndInventor( nType, nInventor, xShape->getShapeType() );
    if (nType == SdrObjKind::NONE)
        return nullptr;

    awt::Size aSize = xShape->getSize();
    aSize.Width += 1;
    aSize.Height += 1;
    awt::Point aPos = xShape->getPosition();
    tools::Rectangle aRect( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) );

    rtl::Reference<SdrObject> pNewObj = SdrObjFactory::MakeNewObject(
        *mpModel,
        nInventor,
        nType,
        &aRect);

    if (!pNewObj)
        return nullptr;

    if( nType == SdrObjKind::E3D_Scene )
    {
        auto pScene = static_cast<E3dScene* >(pNewObj.get());
        // initialise scene

        double fW = static_cast<double>(aSize.Width);
        double fH = static_cast<double>(aSize.Height);

        Camera3D aCam(pScene->GetCamera());
        aCam.SetAutoAdjustProjection(false);
        aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
        basegfx::B3DPoint aLookAt;
        basegfx::B3DPoint aCamPos(0.0, 0.0, 10000.0);
        aCam.SetPosAndLookAt(aCamPos, aLookAt);
        aCam.SetFocalLength(100.0);
        pScene->SetCamera(aCam);

        pScene->SetBoundAndSnapRectsDirty();
    }
    else if(nType == SdrObjKind::E3D_Extrusion)
    {
        auto pObj = static_cast<E3dExtrudeObj* >(pNewObj.get());
        basegfx::B2DPolygon aNewPolygon;
        aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
        aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
        aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
        aNewPolygon.setClosed(true);
        pObj->SetExtrudePolygon(basegfx::B2DPolyPolygon(aNewPolygon));

        // #107245# pObj->SetExtrudeCharacterMode(sal_True);
        pObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }
    else if(nType == SdrObjKind::E3D_Lathe)
    {
        auto pLatheObj = static_cast<E3dLatheObj* >(pNewObj.get());
        basegfx::B2DPolygon aNewPolygon;
        aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
        aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
        aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
        aNewPolygon.setClosed(true);
        pLatheObj->SetPolyPoly2D(basegfx::B2DPolyPolygon(aNewPolygon));

        // #107245# pObj->SetLatheCharacterMode(sal_True);
        pLatheObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    return pNewObj;
}

void SvxDrawPage::GetTypeAndInventor( SdrObjKind& rType, SdrInventor& rInventor, const OUString& aName ) noexcept
{
    std::optional<SdrObjKind> nTempType = UHashMap::getId( aName );

    if( !nTempType )
    {
        if( aName == "com.sun.star.drawing.TableShape" ||
            aName == "com.sun.star.presentation.TableShape" )
        {
            rInventor = SdrInventor::Default;
            rType = SdrObjKind::Table;
        }
#if HAVE_FEATURE_AVMEDIA
        else if ( aName == "com.sun.star.presentation.MediaShape" )
        {
            rInventor = SdrInventor::Default;
            rType = SdrObjKind::Media;
        }
#endif
    }
    else if( IsInventorE3D(*nTempType) )
    {
        rInventor = SdrInventor::E3d;
        rType = *nTempType;
    }
    else
    {
        rInventor = SdrInventor::Default;
        rType = *nTempType;

        switch( rType )
        {
            case SdrObjKind::OLEPluginFrame:
            case SdrObjKind::OLE2Plugin:
            case SdrObjKind::OLE2Applet:
                rType = SdrObjKind::OLE2;
                break;
            default:
                break;
        }
    }
}

rtl::Reference<SvxShape> SvxDrawPage::CreateShapeByTypeAndInventor( SdrObjKind nType, SdrInventor nInventor, SdrObject *pObj, SvxDrawPage *mpPage, OUString const & referer )
{
    rtl::Reference<SvxShape> pRet;

    switch( nInventor )
    {
        case SdrInventor::E3d:
        {
            switch( nType )
            {
                case SdrObjKind::E3D_Scene :
                    pRet = new Svx3DSceneObject( pObj, mpPage );
                    break;
                case SdrObjKind::E3D_Cube :
                    pRet = new Svx3DCubeObject( pObj );
                    break;
                case SdrObjKind::E3D_Sphere :
                    pRet = new Svx3DSphereObject( pObj );
                    break;
                case SdrObjKind::E3D_Lathe :
                    pRet = new Svx3DLatheObject( pObj );
                    break;
                case SdrObjKind::E3D_Extrusion :
                    pRet = new Svx3DExtrudeObject( pObj );
                    break;
                case SdrObjKind::E3D_Polygon :
                    pRet = new Svx3DPolygonObject( pObj );
                    break;
                default: // unknown 3D-object on page
                    assert(false && "the IsInventor3D function must be wrong");
                    pRet = new SvxShape( pObj );
                    break;
            }
            break;
        }
        case SdrInventor::Default:
        {
            switch( nType )
            {
                case SdrObjKind::Group:
                    pRet = new SvxShapeGroup( pObj, mpPage );
                    break;
                case SdrObjKind::Line:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::Rectangle:
                    pRet = new SvxShapeRect( pObj );
                    break;
                case SdrObjKind::CircleOrEllipse:
                case SdrObjKind::CircleSection:
                case SdrObjKind::CircleArc:
                case SdrObjKind::CircleCut:
                    pRet = new SvxShapeCircle( pObj );
                    break;
                case SdrObjKind::Polygon:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::PolyLine:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::PathLine:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::PathFill:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::FreehandLine:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::FreehandFill:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::Caption:
                    pRet = new SvxShapeCaption( pObj );
                    break;
                case SdrObjKind::TitleText:
                case SdrObjKind::OutlineText:
                case SdrObjKind::Text:
                    pRet = new SvxShapeText( pObj );
                    break;
                case SdrObjKind::Graphic:
                    pRet = new SvxGraphicObject( pObj );
                    break;
                case SdrObjKind::OLEPluginFrame:
                    pRet = new SvxFrameShape( pObj, referer );
                    break;
                case SdrObjKind::OLE2Applet:
                    pRet = new SvxAppletShape( pObj, referer );
                    break;
                case SdrObjKind::OLE2Plugin:
                    pRet = new SvxPluginShape( pObj, referer );
                    break;
                 case SdrObjKind::OLE2:
                     {
                        if( pObj && !pObj->IsEmptyPresObj() && mpPage )
                        {
                            SdrPage* pSdrPage = mpPage->GetSdrPage();
                            if( pSdrPage )
                            {
                                SdrModel& rSdrModel(pSdrPage->getSdrModelFromSdrPage());
                                ::comphelper::IEmbeddedHelper *pPersist = rSdrModel.GetPersist();

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
                                            pRet = new SvxPluginShape( pObj, referer );
                                            nType = SdrObjKind::OLE2Plugin;
                                        }
                                        else if( aAppletClassId == aClassId )
                                        {
                                            pRet = new SvxAppletShape( pObj, referer );
                                            nType = SdrObjKind::OLE2Applet;
                                        }
                                        else if( aIFrameClassId == aClassId )
                                        {
                                            pRet = new SvxFrameShape( pObj, referer );
                                            nType = SdrObjKind::OLEPluginFrame;
                                        }
                                    }
                                }
                            }
                        }
                        if( pRet == nullptr )
                        {
                            SvxUnoPropertyMapProvider& rSvxMapProvider = getSvxMapProvider();
                            pRet = new SvxOle2Shape( pObj, referer, rSvxMapProvider.GetMap(SVXMAP_OLE2),  rSvxMapProvider.GetPropertySet(SVXMAP_OLE2, SdrObject::GetGlobalDrawObjectItemPool()) );
                        }
                     }
                    break;
                case SdrObjKind::Edge:
                    pRet = new SvxShapeConnector( pObj );
                    break;
                case SdrObjKind::PathPoly:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::PathPolyLine:
                    pRet = new SvxShapePolyPolygon( pObj );
                    break;
                case SdrObjKind::Page:
                {
                    SvxUnoPropertyMapProvider& rSvxMapProvider = getSvxMapProvider();
                    pRet = new SvxShape( pObj, rSvxMapProvider.GetMap(SVXMAP_PAGE),  rSvxMapProvider.GetPropertySet(SVXMAP_PAGE, SdrObject::GetGlobalDrawObjectItemPool()) );
                }
                    break;
                case SdrObjKind::Measure:
                    pRet = new SvxShapeDimensioning( pObj );
                    break;
                case SdrObjKind::UNO:
                    pRet = new SvxShapeControl( pObj );
                    break;
                case SdrObjKind::CustomShape:
                    pRet = new SvxCustomShape( pObj );
                    break;
                case SdrObjKind::Media:
                    pRet = new SvxMediaShape( pObj, referer );
                    break;
                case SdrObjKind::Table:
                    pRet = new SvxTableShape( pObj );
                    break;
                default: // unknown 2D-object on page
                    assert(false && "Not implemented Starone-Shape created");
                    pRet = new SvxShapeText( pObj );
                    break;
            }
            break;
        }
        default: // unknown inventor
        {
            assert(false && "Unknown Inventor in SvxDrawPage::CreateShape()");
            break;
        }
    }

    if(pRet)
    {
        SdrObjKind nObjId = nType;

        switch(nObjId)
        {
        case SdrObjKind::CircleCut:          // segment of circle
        case SdrObjKind::CircleArc:          // arc of circle
        case SdrObjKind::CircleSection:          // sector
            nObjId = SdrObjKind::CircleOrEllipse;
            break;

        case SdrObjKind::TitleText:
        case SdrObjKind::OutlineText:
            nObjId = SdrObjKind::Text;
            break;
        default: ;
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

rtl::Reference<SdrObject> SvxDrawPage::CreateSdrObject( const Reference< drawing::XShape > & xShape, bool bBeginning ) noexcept
{
    rtl::Reference<SdrObject> pObj = CreateSdrObject_( xShape );
    if( pObj)
    {
        if ( !pObj->IsInserted() && !pObj->IsDoNotInsertIntoPageAutomatically() )
        {
            if(bBeginning)
                mpPage->InsertObject( pObj.get(), 0 );
            else
                mpPage->InsertObject( pObj.get() );
        }
    }

    return pObj;
}

// css::lang::XServiceInfo
OUString SAL_CALL SvxDrawPage::getImplementationName()
{
    return "SvxDrawPage";
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

rtl::Reference<SvxShape> CreateSvxShapeByTypeAndInventor(SdrObjKind nType, SdrInventor nInventor, OUString const & referer)
{
    return SvxDrawPage::CreateShapeByTypeAndInventor( nType, nInventor, nullptr, nullptr, referer );
}

/** returns a StarOffice API wrapper for the given SdrPage */
uno::Reference< drawing::XDrawPage > GetXDrawPageForSdrPage( SdrPage* pPage ) noexcept
{
    if(pPage)
    {
        uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );

        return xDrawPage;
    }

    return uno::Reference< drawing::XDrawPage >();
}

/** returns the SdrObject from the given StarOffice API wrapper */
SdrPage* GetSdrPageFromXDrawPage( const uno::Reference< drawing::XDrawPage >& xDrawPage ) noexcept
{
    if(xDrawPage.is())
    {
        SvxDrawPage* pDrawPage = comphelper::getFromUnoTunnel<SvxDrawPage>( xDrawPage );

        if(pDrawPage)
        {
            return pDrawPage->GetSdrPage();
        }
        assert(false && "non-SvxDrawPage?");
    }

    return nullptr;
}

// helper that returns true if the given XShape is member of the given
// XDrawPage or it's MasterPage (aka associated)
bool IsXShapeAssociatedWithXDrawPage(
    const css::uno::Reference<css::drawing::XShape>& rxShape,
    const css::uno::Reference< css::drawing::XDrawPage >& rxDrawPage) noexcept
{
    if (!rxShape)
        return false;

    if (!rxDrawPage)
        return false;

    const SdrObject* pSdrObject(SdrObject::getSdrObjectFromXShape(rxShape));
    if (nullptr == pSdrObject)
        return false;

    SdrPage* pSdrPage(GetSdrPageFromXDrawPage(rxDrawPage));
    if (nullptr == pSdrPage)
        return false;

    const SdrPage* pPageFromObj(pSdrObject->getSdrPageFromSdrObject());
    if (nullptr == pPageFromObj)
        return false;

    if (pSdrPage == pPageFromObj)
        // given XShape is member of given XDrawPage
        return true;

    if (pSdrPage->TRG_HasMasterPage())
        if (&pSdrPage->TRG_GetMasterPage() == pPageFromObj)
            // given XShape is member of MasterPage of given XDrawPage
            return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
