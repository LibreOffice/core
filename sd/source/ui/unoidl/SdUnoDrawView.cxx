/*************************************************************************
 *
 *  $RCSfile: SdUnoDrawView.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:58:02 $
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

#include <vector>

#include "SdUnoDrawView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_DOCUMENTZOOMTYPE_HPP_
#include <com/sun/star/view/DocumentZoomType.hpp>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>

#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#include "sdwindow.hxx"

#include "unohelp.hxx"
#include "unopage.hxx"
#include "unomodel.hxx"
#include "sdview.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "grviewsh.hxx"
#include "presvish.hxx"
#include "prvwshll.hxx"
#include "sdpage.hxx"
#include "unolayer.hxx"

using namespace ::std;
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//----------------------------------------------------------------------
//------ SdUnoDrawView--------------------------------------------------
//----------------------------------------------------------------------

SdUnoDrawView::SdUnoDrawView(SdView* pSdView, SdDrawViewShell* pSdViewSh) throw()
:   SdUnoDrawViewBase(),
    SfxBaseController((SfxViewShell*)pSdViewSh),
    OBroadcastHelper( ((SdUnoDrawViewBase *)this)->aMutex ),
    OPropertySetHelper( *static_cast<OBroadcastHelperVar<OMultiTypeInterfaceContainerHelper, OMultiTypeInterfaceContainerHelper::keyType> *>(this)),
    mpView(pSdView),
    mpViewSh(pSdViewSh),
    mbDisposing(sal_False),
    mbOldMasterPageMode(sal_False),
    mbOldLayerMode(sal_False),
    mpCurrentPage(NULL),
    meKind (SdUnoDrawView::unknown)
{
}

SdUnoDrawView::~SdUnoDrawView() throw()
{
}

sal_Bool SdUnoDrawView::getMasterPageMode(void) const throw()
{
    return mpViewSh && mpViewSh->GetEditMode() == EM_MASTERPAGE;
}

//----------------------------------------------------------------------

void SdUnoDrawView::setMasterPageMode(sal_Bool MasterPageMode_) throw()
{
    if(mpViewSh && ((mpViewSh->GetEditMode() == EM_MASTERPAGE) != MasterPageMode_))
        mpViewSh->ChangeEditMode(MasterPageMode_?EM_MASTERPAGE:EM_PAGE,mpViewSh->GetLayerMode());
}

//----------------------------------------------------------------------

sal_Bool SdUnoDrawView::getLayerMode(void) const throw()
{
    return mpViewSh && mpViewSh->GetLayerMode();
}

//----------------------------------------------------------------------

void SdUnoDrawView::setLayerMode(sal_Bool LayerMode_) throw()
{
    if(mpViewSh && (mpViewSh->GetLayerMode() != LayerMode_))
        mpViewSh->ChangeEditMode(mpViewSh->GetEditMode(),LayerMode_);
}


Reference<drawing::XLayer> SdUnoDrawView::getActiveLayer (void) throw ()
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference<drawing::XLayer> xCurrentLayer;

    // Retrieve the layer manager from the model.
    SdXImpressDocument* pModel = getModel();
    if (pModel != NULL)
    {
        SdDrawDocument* pSdModel = pModel->GetDoc();
        if (pSdModel != NULL)
        {
            // From the model get the current SdrLayer object via the layer admin.
            SdrLayerAdmin& rLayerAdmin = pSdModel->GetLayerAdmin ();
            SdrLayer* pLayer = rLayerAdmin.GetLayer (mpView->GetActiveLayer(), TRUE);
            if (pLayer != NULL)
            {
                // Get the corresponding XLayer object from the
                // implementation object of the layer manager.
                Reference<drawing::XLayerManager> xManager (pModel->getLayerManager(), uno::UNO_QUERY);
                SdLayerManager* pManager = SdLayerManager::getImplementation (xManager);
                if (pManager != NULL)
                    xCurrentLayer = pManager->GetLayer (pLayer);
            }
        }
    }

    return xCurrentLayer;
}




void SdUnoDrawView::setActiveLayer (const Reference<drawing::XLayer>& rxLayer) throw ()
{
    OGuard aGuard( Application::GetSolarMutex() );

    // Get the SdrLayer object corresponding to the given reference.
    if (rxLayer.is())
    {
        SdLayer* pLayer = SdLayer::getImplementation (rxLayer);
        if (pLayer != NULL)
        {
            SdrLayer* pSdrLayer = pLayer->GetSdrLayer();
            if (pSdrLayer != NULL)
            {
                // Set the new active layer and make the change visible.
                mpView->SetActiveLayer (pSdrLayer->GetName());
                mpViewSh->ResetActualLayer ();
            }
        }
    }
}



//----------------------------------------------------------------------

SdXImpressDocument* SdUnoDrawView::getModel() const throw()
{
    if(mpView && mpView->GetDocSh())
    {
        Reference< frame::XModel > xModel( mpView->GetDocSh()->GetModel() );
        return SdXImpressDocument::getImplementation( xModel );
    }
    else
        return NULL;
}

//----------------------------------------------------------------------
//------ XInterface ----------------------------------------------------
//----------------------------------------------------------------------

Any SAL_CALL SdUnoDrawView::queryInterface( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    QUERYINT( view::XSelectionSupplier );
    else QUERYINT( drawing::XDrawView );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( beans::XPropertySet );
    else if( rType == ITYPE(lang::XComponent) )
        aAny <<= uno::Reference< lang::XComponent >(static_cast<SfxBaseController*>(this));
    else QUERYINT( awt::XWindow );
    else
        return SfxBaseController::queryInterface(rType);

    return aAny;
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::acquire()
    throw ( )
{
    SfxBaseController::acquire();
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::release()
    throw ( )
{
    SfxBaseController::release();
}

//----------------------------------------------------------------------
//------ XTypeProvider -------------------------------------------------
//----------------------------------------------------------------------

Sequence< Type > SAL_CALL SdUnoDrawView::getTypes()
    throw(RuntimeException)
{
    static Sequence< Type > aTypeSequence;
    if( 0 == aTypeSequence.getLength() )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( 0 == aTypeSequence.getLength() )
        {
            const Sequence< Type > aBaseTypes( SfxBaseController::getTypes() );
            const sal_Int32 nBaseTypes = aBaseTypes.getLength();
            const Type* pBaseTypes = aBaseTypes.getConstArray();

            const sal_Int32 nOwnTypes = 6;      // !DANGER! Keep this updated!

            aTypeSequence.realloc(  nBaseTypes + nOwnTypes );
            Type* pTypes = aTypeSequence.getArray();

            *pTypes++ = ITYPE(view::XSelectionSupplier);
            *pTypes++ = ITYPE(drawing::XDrawView);
            *pTypes++ = ITYPE(lang::XServiceInfo);
            *pTypes++ = ITYPE(beans::XPropertySet);
            *pTypes++ = ITYPE(lang::XComponent);
            *pTypes++ = ITYPE(awt::XWindow);

            for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
                *pTypes++ = *pBaseTypes++;
        }
    }

    return aTypeSequence;
}

//----------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SdUnoDrawView::getImplementationId()
    throw(RuntimeException)
{
    static Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//----------------------------------------------------------------------
//------ XComponent ----------------------------------------------------
//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::dispose()
    throw( RuntimeException )
{
    if( !mbDisposing )
    {
        OGuard aGuard( Application::GetSolarMutex() );

        if( !mbDisposing )
        {
            mbDisposing = sal_True;
            mpView = NULL,
            mpViewSh = NULL;

            SfxBaseController::dispose();
        }
    }
}

void SAL_CALL SdUnoDrawView::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SfxBaseController::addEventListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SfxBaseController::removeEventListener( aListener );
}

//----------------------------------------------------------------------
//------ XServiceInfo --------------------------------------------------
//----------------------------------------------------------------------

OUString SAL_CALL SdUnoDrawView::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoDrawView" ) );
}

//----------------------------------------------------------------------

static sal_Char pImplSdUnoDrawViewService[sizeof("com.sun.star.drawing.DrawingDocumentDrawView")] = "com.sun.star.drawing.DrawingDocumentDrawView";
static sal_Char pImplSdUnoPresentationViewService[sizeof("com.sun.star.presentation.PresentationView")] = "com.sun.star.presentation.PresentationView";
static sal_Char pImplSdUnoSlideViewService[sizeof("com.sun.star.presentation.SlidesView")] = "com.sun.star.presentation.SlidesView";
static sal_Char pImplSdUnoPreviewViewService[sizeof("com.sun.star.presentation.PreviewView")] = "com.sun.star.presentation.PreviewView";
static sal_Char pImplSdUnoNotesViewService[sizeof("com.sun.star.presentation.NotesView")] = "com.sun.star.presentation.NotesView";
static sal_Char pImplSdUnoHandoutViewService[sizeof("com.sun.star.presentation.HandoutView")] = "com.sun.star.presentation.HandoutView";


sal_Bool SAL_CALL SdUnoDrawView::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    switch(GetDrawViewKind())
    {
    case slideshow:
        return ServiceName.equalsAscii( pImplSdUnoSlideViewService );
    case preview:
        return ServiceName.equalsAscii( pImplSdUnoPreviewViewService );
    case notes:
        return ServiceName.equalsAscii( pImplSdUnoNotesViewService ) || ServiceName.equalsAscii( pImplSdUnoDrawViewService );
    case handout:
        return ServiceName.equalsAscii( pImplSdUnoHandoutViewService ) || ServiceName.equalsAscii( pImplSdUnoDrawViewService );
    case presentation:
        if( ServiceName.equalsAscii( pImplSdUnoPresentationViewService ) )
            return sal_True;
//  case drawing:
    default:
        return ServiceName.equalsAscii( pImplSdUnoDrawViewService );

    }
}

//----------------------------------------------------------------------

Sequence< OUString > SAL_CALL SdUnoDrawView::getSupportedServiceNames(  ) throw(RuntimeException)
{
    SdUnoDrawViewKind eKind = GetDrawViewKind();
    Sequence< OUString > aSeq( ((eKind != notes) && (eKind != handout) && (eKind != presentation)) ? 1 : 2 );
    OUString* pServices = aSeq.getArray();

    switch( eKind )
    {
    case slideshow:
        pServices[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoSlideViewService ) );
        break;

    case preview:
        pServices[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoPreviewViewService ) );
        break;

    case notes:
        pServices[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoNotesViewService ) );
        pServices[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoDrawViewService ) );
        break;

    case handout:
        pServices[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoHandoutViewService ) );
        pServices[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoDrawViewService ) );
        break;

    case presentation:
        pServices[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoPresentationViewService ) );
//  case drawing:
    default:
        pServices[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( pImplSdUnoDrawViewService ) );
        break;

    }
    return aSeq;
}

//----------------------------------------------------------------------
//------ XSelectionSupplier --------------------------------------------
//----------------------------------------------------------------------

sal_Bool SAL_CALL SdUnoDrawView::select( const Any& aSelection )
    throw(lang::IllegalArgumentException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    vector<SdrObject*> aObjects;

    SdrPage* pSdrPage = NULL;

    Reference< drawing::XShape > xShape;
    aSelection >>= xShape;

    bool bOk = true;

    if(xShape.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if( pShape && (pShape->GetSdrObject() != NULL) )
        {
            SdrObject* pObj = pShape->GetSdrObject();
            pSdrPage = pObj->GetPage();
            aObjects.push_back( pObj );
        }
        else
        {
            bOk = false;
        }
    }
    else
    {
        Reference< drawing::XShapes > xShapes;
        aSelection >>= xShapes;
        if( xShapes.is() )
        {
            const sal_uInt32 nCount = xShapes->getCount();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                xShapes->getByIndex(i) >>= xShape;
                if( xShape.is() )
                {
                    SvxShape* pShape = SvxShape::getImplementation(xShape);
                    if( (pShape == NULL) || (pShape->GetSdrObject() == NULL) )
                    {
                        bOk = false;
                        break;
                    }

                    SdrObject* pObj = pShape->GetSdrObject();

                    if( pSdrPage == NULL )
                    {
                        pSdrPage = pObj->GetPage();
                    }
                    else if( pSdrPage != pObj->GetPage() )
                    {
                        bOk = false;
                        break;
                    }

                    aObjects.push_back( pObj );
                }
            }
        }
    }

    if( bOk )
    {
        if( pSdrPage )
        {
            setMasterPageMode( pSdrPage->IsMasterPage() );
            mpViewSh->SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
            mpViewSh->WriteFrameViewData();
        }

        SdrPageView *pPV = mpView->GetPageViewPvNum(0);

        if(pPV)
        {
            // first deselect all
            mpView->UnmarkAllObj( pPV );

            vector<SdrObject*>::iterator aIter( aObjects.begin() );
            const vector<SdrObject*>::iterator aEnd( aObjects.end() );
            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                mpView->MarkObj( pObj, pPV );
            }
        }
        else
        {
            bOk = false;
        }
    }

    return bOk;
}

//----------------------------------------------------------------------

Any SAL_CALL SdUnoDrawView::getSelection()
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdXImpressDocument* pModel = getModel();

    Reference< drawing::XShapes > xShapes( SvxShapeCollection_NewInstance(), UNO_QUERY );

    if(mpView)
    {
        const SdrMarkList& rMarkList = mpView->GetMarkList();
        sal_uInt32 nCount = rMarkList.GetMarkCount();
        for( sal_uInt32 nNum = 0; nNum < nCount; nNum++)
        {
            SdrMark *pMark = rMarkList.GetMark(nNum);
            if(pMark==NULL)
                continue;

            SdrObject *pObj = pMark->GetObj();
            if(pObj==NULL || pObj->GetPage() == NULL)
                continue;

            Reference< drawing::XDrawPage > xPage( pObj->GetPage()->getUnoPage(), UNO_QUERY);

            if(!xPage.is())
                continue;

            SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );

            if(pDrawPage==NULL)
                continue;

            Reference< drawing::XShape > xShape( pObj->getUnoShape(), UNO_QUERY );

            if(xShape.is())
                xShapes->add(xShape);
        }
    }

    Any aAny;
    if( 0 != xShapes->getCount() )
        aAny <<= xShapes;

    return aAny;
}

//----------------------------------------------------------------------

inline const ::com::sun::star::uno::Type & getSelectionTypeIdentifier()
{
    return ::getCppuType( (Reference< view::XSelectionChangeListener > *)0 );
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException)
{
    addListener( getSelectionTypeIdentifier(), xListener );
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener ) throw(RuntimeException)
{
    removeListener( getSelectionTypeIdentifier( ), xListener );
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireSelectionChangeListener() throw()
{
    OInterfaceContainerHelper * pLC = getContainer( getSelectionTypeIdentifier() );
    if( pLC )
    {
        Reference< XInterface > xSource( (XWeak*)this );
        const lang::EventObject aEvent( xSource );

        // Ueber alle Listener iterieren und Events senden
        OInterfaceIteratorHelper aIt( *pLC);
        while( aIt.hasMoreElements() )
        {
            view::XSelectionChangeListener * pL = (view::XSelectionChangeListener *)aIt.next();
            pL->selectionChanged( aEvent );
        }
    }
}

//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------

// Id must be the index into the array
enum properties
{
    PROPERTY_CURRENTPAGE = 0,
    PROPERTY_MASTERPAGEMODE,
    PROPERTY_LAYERMODE,
    PROPERTY_ACTIVE_LAYER,
    PROPERTY_WORKAREA,
    PROPERTY_ZOOMTYPE,
    PROPERTY_ZOOMVALUE,
    PROPERTY_VIEWOFFSET,
    PROPERTY_COUNT
};

/**
 * All Properties of this implementation. Must be sorted by name.
 */
static beans::Property * getBasicProps()
{
    static beans::Property *pTable = 0;

    if( ! pTable )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pTable )
        {

            static beans::Property aBasicProps[PROPERTY_COUNT] =
            {
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),        PROPERTY_CURRENTPAGE,   ::getCppuType((const Reference< drawing::XDrawPage > *)0), beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsLayerMode") ),        PROPERTY_LAYERMODE,     ::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode") ),   PROPERTY_MASTERPAGEMODE,::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ActiveLayer") ),        PROPERTY_ACTIVE_LAYER,  ::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") ),        PROPERTY_WORKAREA,      ::getCppuType((const ::com::sun::star::awt::Rectangle*)0), beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomValue") ),          PROPERTY_ZOOMVALUE,     ::getCppuType((const sal_Int16*)0), beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomType") ),           PROPERTY_ZOOMTYPE,      ::getCppuType((const sal_Int16*)0), beans::PropertyAttribute::BOUND ),

                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ViewOffset") ),         PROPERTY_VIEWOFFSET,    ::getCppuType((const ::com::sun::star::awt::Point*)0), beans::PropertyAttribute::BOUND ),
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}

//----------------------------------------------------------------------
//------ XPropertySet & OPropertySetHelper -----------------------------
//----------------------------------------------------------------------

/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & SdUnoDrawView::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo( getBasicProps(), PROPERTY_COUNT, sal_False );
    return aInfo;
}

//----------------------------------------------------------------------

Reference < beans::XPropertySetInfo >  SdUnoDrawView::getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//----------------------------------------------------------------------

// Return sal_True, value change
sal_Bool SdUnoDrawView::convertFastPropertyValue
(
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue
) throw ( com::sun::star::lang::IllegalArgumentException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
            {
                Reference< drawing::XDrawPage > xOldPage( getCurrentPage() );
                Reference< drawing::XDrawPage > xNewPage;
                convertPropertyValue( xNewPage, rValue );
                if( xOldPage != xNewPage )
                {
                    rConvertedValue <<= xNewPage;
                    rOldValue <<= xOldPage;
                    return sal_True;
                }
                else
                {
                    return sal_False;
                }
            }
        case PROPERTY_MASTERPAGEMODE:
            {
            sal_Bool bOldValue = getMasterPageMode();
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bOldValue )
            {

                rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                return sal_True;
            }
            else
                return sal_False;
            }

        case PROPERTY_LAYERMODE:
            {
            sal_Bool bOldValue = getLayerMode();
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bOldValue )
            {
                rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                return sal_True;
            }
            else
                return sal_False;
            }
        case PROPERTY_ACTIVE_LAYER:
            {
                Reference<drawing::XLayer> xOldLayer (getActiveLayer());
                Reference<drawing::XLayer> xNewLayer;
                convertPropertyValue (xNewLayer, rValue);
                if (xOldLayer != xNewLayer)
                {
                    rConvertedValue <<= xNewLayer;
                    rOldValue <<= xOldLayer;
                    return sal_True;
                }
                else
                {
                    return sal_False;
                }
            }
        case PROPERTY_ZOOMVALUE:
            {
                sal_Int16 nOldZoom = GetZoom();
                sal_Int16 nNewZoom;
                convertPropertyValue( nNewZoom, rValue );
                if( nNewZoom != nOldZoom )
                {
                    rConvertedValue <<= nNewZoom;
                    rOldValue <<= nOldZoom;
                    return sal_True;
                }
                else
                {
                    return sal_False;
                }
            }
        case PROPERTY_ZOOMTYPE:
            {
                sal_Int16 nOldType = com::sun::star::view::DocumentZoomType::BY_VALUE;
                sal_Int16 nNewType;
                convertPropertyValue( nNewType, rValue );
                if( nNewType != nOldType )
                {
                    rConvertedValue <<= nNewType;
                    rOldValue <<= nOldType;
                    return sal_True;
                }
                else
                {
                    return sal_False;
                }

            }
        case PROPERTY_VIEWOFFSET:
            {
                awt::Point aOld( GetViewOffset() );
                awt::Point aNew;
                convertPropertyValue( aNew, rValue );
                if( (aOld.X != aNew.X) && (aOld.Y != aNew.Y) )
                {
                    rConvertedValue <<= aNew;
                    rOldValue <<= aOld;
                    return sal_True;
                }
                else
                {
                    return sal_False;
                }
            }

        default:
            return sal_False;
    }
}

//----------------------------------------------------------------------

/**
 * only set the value.
 */
void SdUnoDrawView::setFastPropertyValue_NoBroadcast
(
    sal_Int32 nHandle,
    const Any& rValue
) throw ( com::sun::star::uno::Exception)
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
            {
                Reference< drawing::XDrawPage > xPage;
                rValue >>= xPage;
                setCurrentPage( xPage );
            }
            break;

        case PROPERTY_MASTERPAGEMODE:
            {
                sal_Bool bValue;
                rValue >>= bValue;
                setMasterPageMode( bValue );
            }
            break;

        case PROPERTY_LAYERMODE:
            {
                sal_Bool bValue;
                rValue >>= bValue;
                setLayerMode( bValue );
            }

        case PROPERTY_ACTIVE_LAYER:
            {
                Reference<drawing::XLayer> xLayer;
                rValue >>= xLayer;
                setActiveLayer (xLayer);
            }
            break;
        case PROPERTY_ZOOMVALUE:
            {
                sal_Int16 nZoom;
                rValue >>= nZoom;
                SetZoom( nZoom );
            }
            break;
        case PROPERTY_ZOOMTYPE:
            {
                sal_Int16 nType;
                rValue >>= nType;
                SetZoomType( nType );
            }
            break;
        case PROPERTY_VIEWOFFSET:
            {
                awt::Point aOffset;
                rValue >>= aOffset;
                SetViewOffset( aOffset );
            }
            break;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::getFastPropertyValue( Any & rRet, sal_Int32 nHandle ) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_CURRENTPAGE:
            rRet <<= (const_cast<SdUnoDrawView*>(this))->getCurrentPage();
            break;

        case PROPERTY_MASTERPAGEMODE:
            rRet <<= getMasterPageMode();
            break;

        case PROPERTY_LAYERMODE:
            rRet <<= getLayerMode();
            break;

        case PROPERTY_ACTIVE_LAYER:
            rRet <<= (const_cast<SdUnoDrawView*>(this))->getActiveLayer();
            break;

        case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle( maLastVisArea.Left(), maLastVisArea.Top(), maLastVisArea.GetWidth(), maLastVisArea.GetHeight() );
            break;

        case PROPERTY_ZOOMVALUE:
            rRet <<= GetZoom();
            break;
        case PROPERTY_ZOOMTYPE:
            rRet <<= (sal_Int16)com::sun::star::view::DocumentZoomType::BY_VALUE;
            break;
        case PROPERTY_VIEWOFFSET:
            rRet <<= GetViewOffset();
            break;

    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireChangeEditMode( sal_Bool bMasterPageMode ) throw()
{
    if( bMasterPageMode != mbOldMasterPageMode )
    {
        sal_Int32 nHandles = PROPERTY_MASTERPAGEMODE;
        Any aNewValue;
        aNewValue <<= bMasterPageMode;
        Any aOldValue;
        aOldValue <<= mbOldMasterPageMode;

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        mbOldMasterPageMode = bMasterPageMode;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireChangeLayerMode( sal_Bool bLayerMode ) throw()
{
    if( bLayerMode != mbOldLayerMode )
    {
        sal_Int32 nHandles = PROPERTY_LAYERMODE;
        Any aNewValue;
        aNewValue <<= bLayerMode;
        Any aOldValue;
        aOldValue <<= mbOldLayerMode;

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        mbOldLayerMode = bLayerMode;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireSwitchCurrentPage( SdPage* pCurrentPage ) throw()
{
    if( pCurrentPage != mpCurrentPage )
    {
        sal_Int32 nHandles = PROPERTY_CURRENTPAGE;
        Reference< drawing::XDrawPage > xNewPage( pCurrentPage->getUnoPage(), UNO_QUERY );
        Any aNewValue( makeAny( xNewPage ) );

        Any aOldValue;
        if( mpCurrentPage )
        {
            Reference< drawing::XDrawPage > xOldPage( mpCurrentPage->getUnoPage(), UNO_QUERY );
            aOldValue <<= xOldPage;
        }

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        mpCurrentPage = pCurrentPage;
    }
}

//----------------------------------------------------------------------
//------ XDrawView -----------------------------------------------------
//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpView)
    {
        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
        SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

        if(pSdrPage)
        {
            setMasterPageMode( pSdrPage->IsMasterPage() );
            mpViewSh->SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
            mpViewSh->WriteFrameViewData();
        }
    }
}

//----------------------------------------------------------------------

Reference< drawing::XDrawPage > SAL_CALL SdUnoDrawView::getCurrentPage()
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XDrawPage >  xPage;

    if(mpView)
    {
        SdXImpressDocument* pModel = getModel();
        SdrPageView *pPV = mpView->GetPageViewPvNum(0);
        SdrPage* pPage = pPV ? pPV->GetPage() : NULL;

        if(pPage)
            xPage = Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );
    }

    return xPage;
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireVisAreaChanged( const Rectangle& rVisArea ) throw()
{
    if( maLastVisArea != rVisArea )
    {
        sal_Int32 nHandles = PROPERTY_WORKAREA;
        Any aNewValue;
        aNewValue <<= awt::Rectangle( rVisArea.Left(), rVisArea.Top(), rVisArea.GetWidth(), rVisArea.GetHeight() );

        Any aOldValue;
        aOldValue <<= awt::Rectangle( maLastVisArea.Left(), maLastVisArea.Top(), maLastVisArea.GetWidth(), maLastVisArea.GetHeight() );

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        maLastVisArea = rVisArea;
    }
}

Reference< ::com::sun::star::awt::XWindow > SdUnoDrawView::getWindow()
{
    Reference< ::com::sun::star::awt::XWindow > xWindow;
    if( mpViewSh && mpViewSh->GetActiveWindow() )
        xWindow = VCLUnoHelper::GetInterface( mpViewSh->GetActiveWindow() );
    return xWindow;
}

void SAL_CALL SdUnoDrawView::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

::com::sun::star::awt::Rectangle SAL_CALL SdUnoDrawView::getPosSize(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Rectangle aRect;

    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        aRect = xWindow->getPosSize();

    return aRect;
}

void SAL_CALL SdUnoDrawView::setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setVisible( Visible );
}

void SAL_CALL SdUnoDrawView::setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setEnable( Enable );
}

void SAL_CALL SdUnoDrawView::setFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->setFocus();
}

void SAL_CALL SdUnoDrawView::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addWindowListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeWindowListener( xListener );
}

void SAL_CALL SdUnoDrawView::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addFocusListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeFocusListener( xListener );
}

void SAL_CALL SdUnoDrawView::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addKeyListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeKeyListener( xListener );
}

void SAL_CALL SdUnoDrawView::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addMouseListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeMouseListener( xListener );
}

void SAL_CALL SdUnoDrawView::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addMouseMotionListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removeMouseMotionListener( xListener );
}

void SAL_CALL SdUnoDrawView::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->addPaintListener( xListener );
}

void SAL_CALL SdUnoDrawView::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow > xWindow( getWindow() );
    if( xWindow.is() )
        xWindow->removePaintListener( xListener );
}



/** Detect the type of the view by first determining the view shell's
    implementation.  For a <type>SdDrawViewShell</type> the <member
    scope="SdDrawViewShell">GetPageKind</member> method is used as an
    additional indicator.
*/
SdUnoDrawView::SdUnoDrawViewKind SdUnoDrawView::GetDrawViewKind (void) const
{
    if (meKind == SdUnoDrawView::unknown)
    {
        if( mpViewSh->ISA( SdGraphicViewShell ) )
        {
            meKind = SdUnoDrawView::drawing;
        }
        else if( mpViewSh->ISA( SdPresViewShell ) )
        {
            meKind = SdUnoDrawView::slideshow;
        }
        else if( mpViewSh->ISA( SdPreviewViewShell ) )
        {
            meKind = SdUnoDrawView::preview;
        }
        else // Assuming SdDrawViewShell.
        {
            switch( mpViewSh->GetPageKind() )
            {
                case PK_NOTES:
                    meKind = SdUnoDrawView::notes;
                    break;
                case PK_HANDOUT:
                    meKind = SdUnoDrawView::handout;
                    break;
                default:
                    meKind = SdUnoDrawView::presentation;
                    break;
            }
        }
    }
    return meKind;
}

sal_Int16 SdUnoDrawView::GetZoom(void) const
{
    if( mpViewSh && mpViewSh->GetActiveWindow() )
    {
        return (sal_Int16)mpViewSh->GetActiveWindow()->GetZoom();
    }
    else
    {
        return 0;
    }
}

void SdUnoDrawView::SetZoom( sal_Int16 nZoom )
{
    SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, nZoom );

    if( mpViewSh )
    {
        SfxViewFrame* pViewFrame = mpViewSh->GetViewFrame();
        if( pViewFrame )
        {
            SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
            if( pDispatcher )
            {
                pDispatcher->Execute(SID_ATTR_ZOOM,SFX_CALLMODE_SYNCHRON,&aZoomItem, 0L);
            }
        }
    }
}


void SdUnoDrawView::SetViewOffset(const awt::Point& rWinPos )
{
    if( mpViewSh )
    {
        Point aWinPos( rWinPos.X, rWinPos.Y );
        aWinPos += mpViewSh->GetViewOrigin();
        mpViewSh->SetWinViewPos( aWinPos, true );
    }
}

awt::Point SdUnoDrawView::GetViewOffset() const
{
    Point aRet;
    if( mpViewSh )
    {
        aRet = mpViewSh->GetWinViewPos();
        aRet -= mpViewSh->GetViewOrigin();
    }
    return awt::Point( aRet.X(), aRet.Y() );
}

void SdUnoDrawView::SetZoomType( sal_Int16 nType )
{
    if( mpViewSh )
    {
        SfxViewFrame* pViewFrame = mpViewSh->GetViewFrame();
        if( pViewFrame )
        {
            SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
            if( pDispatcher )
            {
                SvxZoomType eZoomType;
                switch( nType )
                {
                case com::sun::star::view::DocumentZoomType::OPTIMAL:
                    eZoomType = SVX_ZOOM_OPTIMAL;
                    break;
                case com::sun::star::view::DocumentZoomType::PAGE_WIDTH:
                case com::sun::star::view::DocumentZoomType::PAGE_WIDTH_EXACT:
                    eZoomType = SVX_ZOOM_PAGEWIDTH;
                    break;
                case com::sun::star::view::DocumentZoomType::ENTIRE_PAGE:
                    eZoomType = SVX_ZOOM_WHOLEPAGE;
                    break;
//              case com::sun::star::view::DocumentZoomType::BY_VALUE:
                default:
                    return;
                }
                SvxZoomItem aZoomItem( eZoomType );
                pDispatcher->Execute(SID_ATTR_ZOOM,SFX_CALLMODE_SYNCHRON,&aZoomItem, 0L);
            }
        }
    }

}
