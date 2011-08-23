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

#define _SVX_USE_UNOGLOBALS_

#ifndef SVX_LIGHT
#ifndef _SOT_CLSIDS_HXX 
#include <sot/clsids.hxx>
#endif
#endif

/*
*/
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "svdoole2.hxx"
#include "svdview.hxx"
#include "unopage.hxx"
#include "shapeimpl.hxx"
#include "globl3d.hxx"
#include "polysc3d.hxx"
#include "svdopath.hxx"
#include "svdomeas.hxx"

#ifndef _E3D_EXTRUD3D_HXX
#include <extrud3d.hxx>
#endif

#ifndef _E3D_LATHE3D_HXX
#include <lathe3d.hxx>
#endif
#include <cube3d.hxx>
#include <sphere3d.hxx>
#include <polygn3d.hxx>

namespace binfilter {

using namespace ::vos;
using namespace ::rtl;
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

DECLARE_LIST( SvxDrawPageList, SvxDrawPage * )//STRIP008 ;

extern SfxItemPropertyMap* ImplGetSvxOle2PropertyMap();
extern SfxItemPropertyMap* ImplGetSvxPageShapePropertyMap();

/**********************************************************************
* class SvxDrawPage                                                   *
**********************************************************************/

UNO3_GETIMPLEMENTATION_IMPL( SvxDrawPage );

SvxDrawPage::SvxDrawPage( SdrPage* pInPage ) throw() :
        pPage	( pInPage ),
        pModel	( NULL )
{
    // Am Broadcaster anmelden
    pModel = pPage->GetModel();
    StartListening( *pModel );

    // Erzeugen der (hidden) ::com::sun::star::sdbcx::View
    pView = new SdrView( pModel );
    if( pView )
        pView->SetDesignMode(sal_True);
}

//----------------------------------------------------------------------
SvxDrawPage::~SvxDrawPage() throw()
{
    // Am Broadcaster abmelden
    if( pModel )
        EndListening( *pModel );

    delete pView;
}

// SfxListener

//----------------------------------------------------------------------
void SvxDrawPage::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( pModel )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

        sal_Bool bInvalid = sal_False;

        if( pSdrHint )
        {
            switch( pSdrHint->GetKind() )
            {
            case HINT_MODELCLEARED:
                bInvalid = sal_True;
                break;
            case HINT_PAGEORDERCHG:
                {
                    const SdrPage* pPg=pSdrHint->GetPage();
                    if( pPg == pPage ) // own page?
                    {
                        if(!pPg->IsInserted()) // page removed?
                            bInvalid = sal_True;
                    }
                }
                break;
            }
        }

        if( bInvalid )
        {
            pModel = NULL;
            delete pView;
            pView = NULL;
        }
    }
}

// ::com::sun::star::drawing::XShapes

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::add( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( NULL == pShape )
        return;

    SdrObject *pObj = pShape->GetSdrObject();

    if(!pObj)
    {
        pObj = CreateSdrObject( xShape );
    }
    else if ( !pObj->IsInserted() )
    {
        pObj->SetModel(pModel);
        pPage->InsertObject( pObj );
    }

    if(pObj == NULL)
        return;

    if(pShape)
        pShape->Create( pObj, this );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::remove( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(pShape)
    {
        SdrObject*	pObj = pShape->GetSdrObject();
        if(pObj)
        {
            // SdrObject aus der Page loeschen
            sal_uInt32 nCount = pPage->GetObjCount();
            for( sal_uInt32 nNum = 0; nNum < nCount; nNum++ )
            {
                if(pPage->GetObj(nNum) == pObj)
                {
                    delete pPage->RemoveObject(nNum);
                    pShape->InvalidateSdrObject();
                    break;
                }
            }
        }
    }

    if( pModel )
        pModel->SetChanged();
}

// ::com::sun::star::container::XIndexAccess

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxDrawPage::getCount()
    throw( uno::RuntimeException )
{
    return( (sal_Int32) pPage->GetObjCount() );
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxDrawPage::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPage == NULL)
        throw uno::RuntimeException();

    if ( Index < 0 || Index >= (sal_Int32)pPage->GetObjCount() )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pObj = pPage->GetObj( Index );
    if( pObj == NULL )
        throw uno::RuntimeException();

    
    return makeAny(Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY ));
}


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
    return pPage?(pPage->GetObjCount()>0):sal_False;
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectsInView selektiert die ::com::sun::star::drawing::Shapes nur in der angegebennen
//         SdrPageView. Dies muﬂ nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectsInView( const Reference< drawing::XShapes > & aShapes, SdrPageView* pPageView ) throw ()
{
    DBG_ASSERT(pPageView,"SdrPageView ist NULL! [CL]");
    DBG_ASSERT(pView, "SdrView ist NULL! [CL]");

    if(pPageView!=NULL && pView!=NULL)
    {
        pView->UnmarkAllObj( pPageView );

        long nCount = aShapes->getCount();
        for( long i = 0; i < nCount; i++ )
        {
            uno::Any aAny( aShapes->getByIndex(i) );
            Reference< drawing::XShape > xShape;
            if( aAny >>= xShape )
            {
                SvxShape* pShape = SvxShape::getImplementation( xShape );

                if( pShape )
                    pView->MarkObj( pShape->pObj, pPageView );
            }
        }
    }
}

//----------------------------------------------------------------------
// ACHTUNG: _SelectObjectInView selektiert das Shape *nur* in der angegebennen
//         SdrPageView. Dies muﬂ nicht die sichtbare SdrPageView sein.
//----------------------------------------------------------------------
void SvxDrawPage::_SelectObjectInView( const Reference< drawing::XShape > & xShape, SdrPageView* pPageView ) throw()
{
    DBG_ASSERT(pPageView,"SdrPageView ist NULL! [CL]");
    DBG_ASSERT(pView, "SdrView ist NULL! [CL]");

    if(pPageView!=NULL && pView != NULL)
    {
        pView->UnmarkAllObj( pPageView );

        SvxShape* pShape = SvxShape::getImplementation( xShape );

        if( pShape )
            pView->MarkObj( pShape->pObj, pPageView );
    }
}

//----------------------------------------------------------------------
Reference< drawing::XShapeGroup > SAL_CALL SvxDrawPage::group( const Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(pPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(pView, "SdrView ist NULL! [CL]");

    Reference< ::com::sun::star::drawing::XShapeGroup >  xShapeGroup;
    if(pPage==NULL||pView==NULL||!xShapes.is())
        return xShapeGroup;

    SdrPageView* pPageView = pView->ShowPage( pPage, Point() );

    _SelectObjectsInView( xShapes, pPageView );

    pView->GroupMarked();

    pView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        if( pObj )
             xShapeGroup = Reference< drawing::XShapeGroup >::query( pObj->getUnoShape() );
    }

    pView->HidePage(pPageView);

    if( pModel )
        pModel->SetChanged();

    return xShapeGroup;
}

//----------------------------------------------------------------------
void SAL_CALL SvxDrawPage::ungroup( const Reference< drawing::XShapeGroup >& aGroup )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(pPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(pView, "SdrView ist NULL! [CL]");

    if(pPage==NULL||pView==NULL||!aGroup.is())
        return;

    SdrPageView* pPageView = pView->ShowPage( pPage, Point() );

    Reference< drawing::XShape > xShape( aGroup, UNO_QUERY );
    _SelectObjectInView( xShape, pPageView );
    pView->UnGroupMarked();

    pView->HidePage(pPageView);

    if( pModel )
        pModel->SetChanged();
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
                pNewObj = new SdrMeasureObj( aRect.TopLeft(), aRect.BottomRight() );
                break;
            case OBJ_LINE:
                pNewObj = new SdrPathObj( aRect.TopLeft(), aRect.BottomRight() );
                break;
            }
        }

        if( pNewObj == NULL )
            pNewObj = SdrObjFactory::MakeNewObject( nInventor, nType, pPage );

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
                Vector3D aLookAt;
                Vector3D aCamPos(0.0, 0.0, 10000.0);
                aCam.SetPosAndLookAt(aCamPos, aLookAt);
                aCam.SetFocalLength(100.0);
                aCam.SetDefaults(aCamPos, aLookAt, 10000.0);
                pScene->SetCamera(aCam);

                pScene->SetRectsDirty();
                pScene->InitTransformationSet();
            }
            else if(pNewObj->ISA(E3dExtrudeObj))
            {
                E3dExtrudeObj* pObj = (E3dExtrudeObj*)pNewObj;
                Polygon3D aNewP(3);
                aNewP[0] = Vector3D(0,0,0);
                aNewP[1] = Vector3D(0,1,0);
                aNewP[2] = Vector3D(1,0,0);

                // #87922#
                // To avoid that CreateGeometry(...) sets the DoubleSided
                // item at once, use a closed poylgon.
                aNewP.SetClosed(TRUE);
                
                PolyPolygon3D aNewPP(aNewP);
                pObj->SetExtrudePolygon(aNewPP);

                // #107245# pObj->SetExtrudeCharacterMode(TRUE);
                pObj->SetItem(Svx3DCharacterModeItem(sal_True));
            }
            else if(pNewObj->ISA(E3dLatheObj))
            {
                E3dLatheObj* pObj = (E3dLatheObj*)pNewObj;
                Polygon3D aNewP(3);
                aNewP[0] = Vector3D(0,0,0);
                aNewP[1] = Vector3D(0,1,0);
                aNewP[2] = Vector3D(1,0,0);
                
                // #87922#
                // To avoid that CreateGeometry(...) sets the DoubleSided
                // item at once, use a closed poylgon.
                aNewP.SetClosed(TRUE);
                
                PolyPolygon3D aNewPP(aNewP);
                pObj->SetPolyPoly3D(aNewPP);

                // #107245# pObj->SetLatheCharacterMode(TRUE);
                pObj->SetItem(Svx3DCharacterModeItem(sal_True));
            }
        }
    }

    return pNewObj;
}

//----------------------------------------------------------------------
void SvxDrawPage::GetTypeAndInventor( sal_uInt16& rType, sal_uInt32& rInventor, const OUString& aName ) const throw()
{
    sal_uInt32 nTempType = aSdrShapeIdentifierMap.getId( aName );

    if(nTempType & E3D_INVENTOR_FLAG)
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
SvxShape* SvxDrawPage::CreateShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor, SdrObject *pObj, SvxDrawPage *pPage ) throw()
{
    SvxShape* pRet = NULL;
    switch( nInventor )
    {
        case E3dInventor:
        {
            if( nType == E3D_COMPOUNDOBJ_ID )
            {
                if( pObj->ISA( E3dSphereObj ) )
                    nType = E3D_SPHEREOBJ_ID;
                else if( pObj->ISA( E3dCubeObj ) )
                    nType = E3D_CUBEOBJ_ID;
                else if( pObj->ISA( E3dExtrudeObj ) )
                    nType = E3D_EXTRUDEOBJ_ID;
                else if( pObj->ISA( E3dLatheObj ) )
                    nType = E3D_LATHEOBJ_ID;
                else if( pObj->ISA( E3dPolygonObj ) )
                    nType = E3D_LATHEOBJ_ID;
            }

            switch( nType )
            {
                case E3D_SCENE_ID :
                case E3D_POLYSCENE_ID :
                    pRet = new Svx3DSceneObject( pObj, pPage );
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
                    DBG_ERROR( "svx::SvxDrawPage::CreateShapeByTypeAndInventor(), unkown 3d-object found!" );
                    pRet = new SvxShape( pObj );
                    break;
            }
            break;
        }
        case SdrInventor:
        {
            switch( nType )
            {
//				case OBJ_NONE:
//					break;
                case OBJ_GRUP:
                    pRet = new SvxShapeGroup( pObj, pPage );
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
#ifndef SVX_LIGHT
                    pRet = new SvxFrameShape( pObj );
                    break;
#endif
                case OBJ_OLE2_APPLET:
#ifndef SVX_LIGHT
                    pRet = new SvxAppletShape( pObj );
                    break;
#endif
                case OBJ_OLE2_PLUGIN:
#ifndef SVX_LIGHT
                    pRet = new SvxPluginShape( pObj );
                    break;
#endif
                 case OBJ_OLE2:
                     {
#ifndef SVX_LIGHT
                        if( pObj && !pObj->IsEmptyPresObj() )
                        {
                            SvPersist *pPersist = pPage->GetSdrPage()->GetModel()->GetPersist();

                            if( pPersist )
                            {
                                const SvInfoObject *pInfo = pPersist->Find( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() );

                                DBG_ASSERT( pInfo, "no info object for OLE object found" );

                                // CL->KA: Why is this not working anymore?
                                if( pInfo )
                                {

                                    const SvGlobalName aClassId( pInfo->GetClassName() );
                                    const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID ); //STRIP003 
                                    const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID ); //STRIP003 
                                    const SvGlobalName aIFrameClassId( BF_SO3_IFRAME_CLASSID );

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
#endif
                        if( pRet == NULL )
                        {
                            pRet = new SvxOle2Shape( pObj, ImplGetSvxOle2PropertyMap() );
                        }
                        break;
                     }
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
                    pRet = new SvxShape( pObj, ImplGetSvxPageShapePropertyMap() );
                    break;
                case OBJ_MEASURE:
                    pRet = new SvxShapeDimensioning( pObj );
                    break;
//				case OBJ_DUMMY:
//					break;
                case OBJ_UNO:
                    pRet = new SvxShapeControl( pObj );
                    break;
                default: // unbekanntes 2D-Objekt auf der Page
                    DBG_ERROR("Nicht implementierter Starone-Shape erzeugt! [CL]");
                    pRet = new SvxShapeText( pObj );
                    break;
            }
            break;
        }
        default: // Unbekannter Inventor
        {
            DBG_ERROR("AW: Unknown Inventor in SvxDrawPage::_CreateShape()");
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
        case OBJ_CCUT:			// Kreisabschnitt
        case OBJ_CARC:			// Kreisbogen
        case OBJ_SECT:			// Kreissektor
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
        pPage->InsertObject( pObj );

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
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
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


}
