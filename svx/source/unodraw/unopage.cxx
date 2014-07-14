/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _SVX_USE_UNOGLOBALS_
#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <sfx2/dispatch.hxx>
#include <sot/clsids.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>
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
#include <svx/globl3d.hxx>
#include <svx/unoprov.hxx>
#include <svx/svdopath.hxx>
#include "svx/unoapi.hxx"
#include <svx/svdomeas.hxx>
#include <svx/extrud3d.hxx>
#include <svx/lathe3d.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/scene3d.hxx>
#include <svx/globaldrawitempool.hxx>
#include <tools/diagnose_ex.h>

using ::rtl::OUString;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const Reference< xint >*)0) ) \
        aAny <<= Reference< xint >(this)

DECLARE_LIST( SvxDrawPageList, SvxDrawPage * )


/**********************************************************************
* class SvxDrawPage                                                   *
**********************************************************************/

UNO3_GETIMPLEMENTATION_IMPL( SvxDrawPage );
SvxDrawPage::SvxDrawPage( SdrPage* pInPage ) throw()
: mrBHelper( getMutex() )
, mpPage( pInPage )
, mpModel( 0 )
{
    // Am Broadcaster anmelden
    if( mpPage )
        mpModel = &mpPage->getSdrModelFromSdrPage();

    if( mpModel )
        StartListening( *mpModel );


    // Erzeugen der (hidden) ::com::sun::star::sdbcx::View
    if(mpModel)
        mpView = new SdrView( *mpModel );

    if( mpView )
        mpView->SetDesignMode(true);
}

//----------------------------------------------------------------------
// Ctor fuer SvxDrawPage_NewInstance()
//----------------------------------------------------------------------
SvxDrawPage::SvxDrawPage() throw()
: mrBHelper( getMutex() )
, mpPage( NULL )
, mpModel( NULL )
, mpView( NULL )
{
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
}

//----------------------------------------------------------------------

// XInterface
void SvxDrawPage::release() throw()
{
/*
    uno::Reference< uno::XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_decrementInterlockedCount( &m_refCount ) == 0)
        {
            if (! mrBHelper.bDisposed)
            {
                uno::Reference< uno::XInterface > xHoldAlive( (uno::XWeak*)this );
                // First dispose
                try
                {
                    dispose();
                }
                catch(::com::sun::star::uno::Exception&)
                {
                    // release should not throw exceptions
                }

                // only the alive ref holds the object
                OSL_ASSERT( m_refCount == 1 );
                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_incrementInterlockedCount( &m_refCount );
    }
*/
    OWeakAggObject::release();
}

//----------------------------------------------------------------------

SvxDrawPage* SvxDrawPage::GetPageForSdrPage( SdrPage* mpPage ) throw()
{
    return getImplementation( mpPage->getUnoPage() );
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
    OGuard aSolarGuard( Application::GetSolarMutex() );

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
            // The listener container are automatically cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(::com::sun::star::uno::Exception& e)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw e;
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpModel == 0 )
        throw lang::DisposedException();

    mrBHelper.addListener( ::getCppuType( &aListener ) , aListener );
}

//----------------------------------------------------------------------

void SAL_CALL SvxDrawPage::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpModel == 0 )
        throw lang::DisposedException();

    mrBHelper.removeListener( ::getCppuType( &aListener ) , aListener );
}

//----------------------------------------------------------------------
// SfxListener
//----------------------------------------------------------------------

void SvxDrawPage::Notify( SfxBroadcaster&, const SfxHint& /*rHint*/ )
{
/*
    if( mpModel )
    {
        const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

        if( pSdrHint )
        {
            switch( pSdrHint->GetSdrHintKind() )
            {
            case HINT_MODELCLEARED:
                dispose();
                break;
            default:
                break;
            }
        }
    }
*/
}

//----------------------------------------------------------------------
// ::com::sun::star::drawing::XShapes
//----------------------------------------------------------------------

void SAL_CALL SvxDrawPage::add( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    else if ( !pObj->IsObjectInserted() )
    {
        mpPage->InsertObjectToSdrObjList(*pObj);
    }

    pShape->Create( pObj, this );
    OSL_ENSURE( pShape->GetSdrObject() == pObj, "SvxDrawPage::add: shape does not know about its newly created SdrObject!" );

    mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::remove( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

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
                    OSL_VERIFY( mpPage->RemoveObjectFromSdrObjList( nNum ) == pObj );
                    deleteSdrObjectSafeAndClearPointer( pObj );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    return( (sal_Int32) mpPage->GetObjCount() );
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxDrawPage::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    return mpPage && mpPage->GetObjCount()>0;
}

namespace
{
    void lcl_markSdrObjectOfShape( const Reference< drawing::XShape >& _rxShape, SdrView& _rView )
    {
        SvxShape* pShape = SvxShape::getImplementation( _rxShape );
        if ( !pShape )
            return;

        SdrObject* pObj = pShape->GetSdrObject();
        if ( !pObj )
            return;

        _rView.MarkObj( *pObj );
    }
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectsInView selektiert die ::com::sun::star::drawing::Shapes nur in der angegebennen
//         SdrPageView. Dies muﬂ nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectsInView( const Reference< drawing::XShapes > & aShapes ) throw ()
{
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(mpView!=NULL)
    {
        mpView->UnmarkAllObj();

        long nCount = aShapes->getCount();
        for( long i = 0; i < nCount; i++ )
        {
            uno::Any aAny( aShapes->getByIndex(i) );
            Reference< drawing::XShape > xShape;
            if( aAny >>= xShape )
                lcl_markSdrObjectOfShape( xShape, *mpView );
        }
    }
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectInView selektiert das Shape *nur* in der angegebennen
//         SdrPageView. Dies muﬂ nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectInView( const Reference< drawing::XShape > & xShape ) throw()
{
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(mpView != NULL)
    {
        mpView->UnmarkAllObj();
        lcl_markSdrObjectOfShape( xShape, *mpView );
    }
}

//----------------------------------------------------------------------
Reference< drawing::XShapeGroup > SAL_CALL SvxDrawPage::group( const Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    DBG_ASSERT(mpPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    Reference< ::com::sun::star::drawing::XShapeGroup >  xShapeGroup;
    if(mpPage==NULL||mpView==NULL||!xShapes.is())
        return xShapeGroup;

    mpView->ShowSdrPage( *mpPage );
    _SelectObjectsInView( xShapes );
    mpView->GroupMarked();
    mpView->RecreateAllMarkHandles();

    SdrObject* pObj = mpView->getSelectedIfSingle();
        if( pObj )
             xShapeGroup = Reference< drawing::XShapeGroup >::query( pObj->getUnoShape() );

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();

    return xShapeGroup;
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::ungroup( const Reference< drawing::XShapeGroup >& aGroup )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( (mpModel == 0) || (mpPage == 0) )
        throw lang::DisposedException();

    DBG_ASSERT(mpPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    if(mpPage==NULL||mpView==NULL||!aGroup.is())
        return;

    mpView->ShowSdrPage( *mpPage );
    Reference< drawing::XShape > xShape( aGroup, UNO_QUERY );
    _SelectObjectInView( xShape );
    mpView->UnGroupMarked();

    mpView->HideSdrPage();

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
SdrObject *SvxDrawPage::_CreateSdrObject( const Reference< drawing::XShape > & xShape ) throw()
{
    SdrObject* pNewObj = 0;

    if(mpModel)
    {
        const SvxShapeKind aSvxShapeKind(getSvxShapeKind(xShape->getShapeType()));

        if(SvxShapeKind_None != aSvxShapeKind)
        {
            const awt::Size aSize(xShape->getSize());
            const awt::Point aPos(xShape->getPosition());
            bool bSetTransform(true);

            // special cases
            if(SvxShapeKind_Measure == aSvxShapeKind)
            {
                // create measure direct point-based
                pNewObj = new SdrMeasureObj(
                    *mpModel,
                    Point(aPos.X, aPos.Y),
                    Point(aPos.X + aSize.Width, aPos.Y + aSize.Height));
                bSetTransform = false;
            }
            else if(SvxShapeKind_Path == aSvxShapeKind)
            {
                // create polygon object with some initial data to have a size
                basegfx::B2DPolygon aPoly;

                aPoly.append(basegfx::B2DPoint(aPos.X, aPos.Y));
                aPoly.append(basegfx::B2DPoint(aPos.X + 100.0, aPos.Y));
                pNewObj = new SdrPathObj(*mpModel, basegfx::B2DPolyPolygon(aPoly));
                bSetTransform = false;
            }

            if(!pNewObj)
            {
                sal_uInt16 nIdent(OBJ_NONE);
                sal_uInt32 nInvent(SdrInventor);

                SvxShapeKindToSdrObjectCreatorInventor(aSvxShapeKind, nIdent, nInvent);
                pNewObj = SdrObjFactory::MakeNewObject(*mpModel, SdrObjectCreationInfo(nIdent, nInvent));
            }

            if(pNewObj)
            {
                E3dScene* pE3dScene = dynamic_cast< E3dScene* >(pNewObj);
                E3dExtrudeObj* pE3dExtrudeObj = dynamic_cast< E3dExtrudeObj* >(pNewObj);
                E3dLatheObj* pE3dLatheObj = dynamic_cast< E3dLatheObj* >(pNewObj);

                if( pE3dScene )
                {
                    // init scene
                    const double fW(aSize.Width);
                    const double fH(aSize.Height);
                    Camera3D aCam(pE3dScene->GetCamera());

                    aCam.SetAutoAdjustProjection(sal_False);
                    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);

                    basegfx::B3DPoint aLookAt;
                    basegfx::B3DPoint aCamPos(0.0, 0.0, 10000.0);

                    aCam.SetPosAndLookAt(aCamPos, aLookAt);
                    aCam.SetFocalLength(100.0);
                    aCam.SetDefaults(aCamPos, aLookAt, 10000.0);
                    pE3dScene->SetCamera(aCam);
                    pE3dScene->ActionChanged();
                }
                else if(pE3dExtrudeObj)
                {
                    // init extrude to some defaults
                    basegfx::B2DPolygon aNewPolygon;

                    aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
                    aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
                    aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
                    aNewPolygon.setClosed(true);
                    pE3dExtrudeObj->SetExtrudePolygon(basegfx::B2DPolyPolygon(aNewPolygon));

                    pE3dExtrudeObj->SetMergedItem(Svx3DCharacterModeItem(sal_True));
                }
                else if(pE3dLatheObj)
                {
                    // init lathe to some defaults
                    basegfx::B2DPolygon aNewPolygon;

                    aNewPolygon.append(basegfx::B2DPoint(0.0, 0.0));
                    aNewPolygon.append(basegfx::B2DPoint(0.0, 1.0));
                    aNewPolygon.append(basegfx::B2DPoint(1.0, 0.0));
                    aNewPolygon.setClosed(true);
                    pE3dLatheObj->SetPolyPoly2D(basegfx::B2DPolyPolygon(aNewPolygon));

                    pE3dLatheObj->SetMergedItem(Svx3DCharacterModeItem(sal_True));
                }

                if(bSetTransform)
                {
                    pNewObj->setSdrObjectTransformation(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aSize.Width, aSize.Height,
                            aPos.X, aPos.Y));
                }
            }
        }
    }

    return pNewObj;
}

//----------------------------------------------------------------------
SvxShapeKind SvxDrawPage::getSvxShapeKind( const ::rtl::OUString& aName ) const throw()
{
    SvxShapeKind aSvxShapeKind(getSvxShapeKindFromTypeName(aName));

    if(SvxShapeKind_None != aSvxShapeKind)
    {
        switch(aSvxShapeKind)
        {
            case SvxShapeKind_Frame:
            case SvxShapeKind_Plugin:
            case SvxShapeKind_Applet:
            {
                aSvxShapeKind = SvxShapeKind_OLE2;
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        if(aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TableShape")) || aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.TableShape")))
        {
            aSvxShapeKind = SvxShapeKind_Table;
        }
        else if(aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.MediaShape")))
        {
            aSvxShapeKind = SvxShapeKind_Media;
        }
    }

    return aSvxShapeKind;
}

//----------------------------------------------------------------------
SvxShape* SvxDrawPage::CreateShapeBySvxShapeKind(
    SvxShapeKind aSvxShapeKind,
    SdrObject *pObj,
    SvxDrawPage *mpPage ) throw()
{
    SvxShape* pRet = 0;

    switch(aSvxShapeKind)
    {
        // 3D shapes
        case SvxShapeKind_3DScene:
        {
            pRet = new Svx3DSceneObject( pObj, mpPage );
            break;
        }
        case SvxShapeKind_3DCube:
        {
            pRet = new Svx3DCubeObject( pObj );
            break;
        }
        case SvxShapeKind_3DSphere:
        {
            pRet = new Svx3DSphereObject( pObj );
            break;
        }
        case SvxShapeKind_3DExtrude:
        {
            pRet = new Svx3DExtrudeObject( pObj );
            break;
        }
        case SvxShapeKind_3DLathe:
        {
            pRet = new Svx3DLatheObject( pObj );
            break;
        }
        case SvxShapeKind_3DPolygon:
        {
            pRet = new Svx3DPolygonObject( pObj );
            break;
        }

        // Svx shapes
        case SvxShapeKind_Group:
        {
            pRet = new SvxShapeGroup( pObj, mpPage );
            break;
        }
        case SvxShapeKind_Rectangle:
        {
            pRet = new SvxShapeRect( pObj );
            break;
        }
        case SvxShapeKind_Circle:
        {
            pRet = new SvxShapeCircle( pObj );
            break;
        }
        case SvxShapeKind_Path:
        {
            pRet = new SvxShapePolyPolygon( pObj );
            break;
        }
        case SvxShapeKind_Text:
        {
            pRet = new SvxShapeText( pObj );
            break;
        }
        case SvxShapeKind_Graphic:
        {
            pRet = new SvxGraphicObject( pObj );
            break;
        }
        case SvxShapeKind_OLE2:
        {
            if( pObj && !pObj->IsEmptyPresObj() && mpPage )
            {
                SdrPage* pSdrPage = mpPage->GetSdrPage();

                if( pSdrPage )
                {
                    ::comphelper::IEmbeddedHelper *pPersist = pSdrPage->getSdrModelFromSdrPage().GetPersist();
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
                                aSvxShapeKind = SvxShapeKind_Plugin;
                            }
                            else if( aAppletClassId == aClassId )
                            {
                                pRet = new SvxAppletShape( pObj );
                                aSvxShapeKind = SvxShapeKind_Applet;
                            }
                            else if( aIFrameClassId == aClassId )
                            {
                                pRet = new SvxFrameShape( pObj );
                                aSvxShapeKind = SvxShapeKind_Frame;
                            }
                        }
                    }
                }
            }

            if( !pRet )
            {
                pRet = new SvxOle2Shape( pObj, aSvxMapProvider.GetMap(SVXMAP_OLE2),  aSvxMapProvider.GetPropertySet(SVXMAP_OLE2, GetGlobalDrawObjectItemPool()) );
            }

            break;
        }
        case SvxShapeKind_Connector:
        {
            pRet = new SvxShapeConnector( pObj );
            break;
        }
        case SvxShapeKind_Caption:
        {
            pRet = new SvxShapeCaption( pObj );
            break;
        }
        case SvxShapeKind_Page:
        {
            pRet = new SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_PAGE),  aSvxMapProvider.GetPropertySet(SVXMAP_PAGE, GetGlobalDrawObjectItemPool()) );
            break;
        }
        case SvxShapeKind_Measure:
        {
            pRet = new SvxShapeDimensioning( pObj );
            break;
        }
        case SvxShapeKind_Frame:
        {
            pRet = new SvxFrameShape( pObj );
            break;
        }
        case SvxShapeKind_Control:
        {
            pRet = new SvxShapeControl( pObj );
            break;
        }
        case SvxShapeKind_Customshape:
        {
            pRet = new SvxCustomShape( pObj );
            break;
        }
        case SvxShapeKind_Media:
        {
            pRet = new SvxMediaShape( pObj );
            break;
        }
        case SvxShapeKind_Table:
        {
            pRet = new SvxTableShape( pObj );
            break;
        }

        // objects not directly associated with SdrObjKind
        case SvxShapeKind_Applet:
        {
            pRet = new SvxAppletShape( pObj );
            break;
        }
        case SvxShapeKind_Plugin:
        {
            pRet = new SvxPluginShape( pObj );
            break;
        }
        default: // unknown Object on Page
        {
            DBG_ERROR("Not implemented Shape created (!)");
            pRet = new SvxShapeText( pObj );
            break;
        }
    }

    if(pRet)
    {
        pRet->setSvxShapeKind(aSvxShapeKind);
    }

    return pRet;
}

//----------------------------------------------------------------------
Reference< drawing::XShape >  SvxDrawPage::_CreateShape( SdrObject *pObj ) const throw()
{
    Reference< drawing::XShape > xShape(
        CreateShapeBySvxShapeKind(
            SdrObjectCreatorInventorToSvxShapeKind(pObj->GetObjIdentifier(), pObj->GetObjInventor()),
            pObj,
            const_cast< SvxDrawPage* >(static_cast< const SvxDrawPage* >(this))));
    return xShape;
}

//----------------------------------------------------------------------
SdrObject *SvxDrawPage::CreateSdrObject( const Reference< drawing::XShape > & xShape ) throw()
{
    SdrObject* pObj = _CreateSdrObject( xShape );
    if( pObj && !pObj->IsObjectInserted() )
        mpPage->InsertObjectToSdrObjList(*pObj);

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

// eof
