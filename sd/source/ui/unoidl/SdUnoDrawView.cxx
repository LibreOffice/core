/*************************************************************************
 *
 *  $RCSfile: SdUnoDrawView.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:48:44 $
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

#include "SdUnoDrawView.hxx"

#include <vector>

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
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "unohelp.hxx"
#include "unopage.hxx"
#include "unomodel.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#ifndef SD_PREVIEW_VIEW_SHELL_HXX
#include "PreviewViewShell.hxx"
#endif
#include "sdpage.hxx"
#include "unolayer.hxx"
#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sd {

SdUnoDrawView::SdUnoDrawView(
    ViewShellBase& rBase,
    ViewShell& rViewShell,
    View& rView) throw()
    :   DrawController (rBase, rViewShell, rView),
        mbOldMasterPageMode(sal_False),
        mbOldLayerMode(sal_False),
        mpCurrentPage(NULL)
{
}




SdUnoDrawView::~SdUnoDrawView() throw()
{
}




DrawViewShell& SdUnoDrawView::GetDrawViewShell (void) const
{
    ThrowIfDisposed();
    return static_cast<DrawViewShell&>(mrViewShell);
}




sal_Bool SdUnoDrawView::getMasterPageMode(void) const throw()
{
    ThrowIfDisposed();
    return GetDrawViewShell().GetEditMode() == EM_MASTERPAGE;
}




void SdUnoDrawView::setMasterPageMode (sal_Bool bMasterPageMode) throw()
{
    ThrowIfDisposed();
    if ((GetDrawViewShell().GetEditMode() == EM_MASTERPAGE) != bMasterPageMode)
        GetDrawViewShell().ChangeEditMode (
            bMasterPageMode ? EM_MASTERPAGE : EM_PAGE,
            GetDrawViewShell().IsLayerModeActive());
}




sal_Bool SdUnoDrawView::getLayerMode(void) const throw()
{
    ThrowIfDisposed();
    return GetDrawViewShell().IsLayerModeActive();
}




void SdUnoDrawView::setLayerMode (sal_Bool bLayerMode) throw()
{
    ThrowIfDisposed();
    if (GetDrawViewShell().IsLayerModeActive() != (bLayerMode==sal_True))
        GetDrawViewShell().ChangeEditMode (
            GetDrawViewShell().GetEditMode(),
            bLayerMode);
}




Reference<drawing::XLayer> SdUnoDrawView::getActiveLayer (void) throw ()
{
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    Reference<drawing::XLayer> xCurrentLayer;

    // Retrieve the layer manager from the model.
    SdXImpressDocument* pModel = GetModel();
    if (pModel != NULL)
    {
        SdDrawDocument* pSdModel = pModel->GetDoc();
        if (pSdModel != NULL)
        {
            // From the model get the current SdrLayer object via the layer admin.
            SdrLayerAdmin& rLayerAdmin = pSdModel->GetLayerAdmin ();
            SdrLayer* pLayer = rLayerAdmin.GetLayer (
                mrView.GetActiveLayer(), TRUE);
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
    ThrowIfDisposed();
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
                mrView.SetActiveLayer (pSdrLayer->GetName());
                GetDrawViewShell().ResetActualLayer ();
            }
        }
    }
}




void SdUnoDrawView::FireChangeEditMode (bool bMasterPageMode) throw()
{
    if( bMasterPageMode != mbOldMasterPageMode )
    {
        Any aNewValue;
        aNewValue <<= (sal_Bool)bMasterPageMode;
        Any aOldValue;
        aOldValue <<= (sal_Bool)mbOldMasterPageMode;

        FirePropertyChange(PROPERTY_MASTERPAGEMODE, aNewValue, aOldValue);

        mbOldMasterPageMode = bMasterPageMode;
    }
}




void SdUnoDrawView::FireChangeLayerMode (bool bLayerMode) throw()
{
    if( bLayerMode != mbOldLayerMode )
    {
        Any aNewValue;
        aNewValue <<= (sal_Bool)bLayerMode;
        Any aOldValue;
        aOldValue <<= (sal_Bool)mbOldLayerMode;

        FirePropertyChange (PROPERTY_LAYERMODE, aNewValue, aOldValue);

        mbOldLayerMode = bLayerMode;
    }
}




void SdUnoDrawView::FireSwitchCurrentPage (SdPage* pCurrentPage) throw()
{
    if( pCurrentPage != mpCurrentPage )
    {
        Reference< drawing::XDrawPage > xNewPage( pCurrentPage->getUnoPage(), UNO_QUERY );
        Any aNewValue( makeAny( xNewPage ) );

        Any aOldValue;
        if( mpCurrentPage )
        {
            Reference< drawing::XDrawPage > xOldPage( mpCurrentPage->getUnoPage(), UNO_QUERY );
            aOldValue <<= xOldPage;
        }

        FirePropertyChange (PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

        mpCurrentPage = pCurrentPage;
    }
}




// XTypeProvider

IMPLEMENT_GET_IMPLEMENTATION_ID(SdUnoDrawView);





// XServiceInfo


OUString SAL_CALL SdUnoDrawView::getImplementationName (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SdUnoDrawView"));
}



static sal_Char pImplSdUnoDrawViewService[sizeof("com.sun.star.drawing.DrawingDocumentDrawView")] = "com.sun.star.drawing.DrawingDocumentDrawView";
static sal_Char pImplSdUnoSlideViewService[sizeof("com.sun.star.presentation.SlidesView")] = "com.sun.star.presentation.SlidesView";
static sal_Char pImplSdUnoPreviewViewService[sizeof("com.sun.star.presentation.PreviewView")] = "com.sun.star.presentation.PreviewView";
static sal_Char pImplSdUnoNotesViewService[sizeof("com.sun.star.presentation.NotesView")] = "com.sun.star.presentation.NotesView";
static sal_Char pImplSdUnoHandoutViewService[sizeof("com.sun.star.presentation.HandoutView")] = "com.sun.star.presentation.HandoutView";


sal_Bool SAL_CALL SdUnoDrawView::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    ThrowIfDisposed();

    sal_Bool bServiceIsSupported = sal_False;

    switch (mrViewShell.GetShellType())
    {
        case ViewShell::ST_PREVIEW:
            bServiceIsSupported = ServiceName.equalsAscii(
                pImplSdUnoPreviewViewService);
            break;

        case ViewShell::ST_NOTES:
            bServiceIsSupported = ServiceName.equalsAscii(
                pImplSdUnoNotesViewService)
                || ServiceName.equalsAscii( pImplSdUnoDrawViewService );
            break;

        case ViewShell::ST_HANDOUT:
            bServiceIsSupported = ServiceName.equalsAscii(
                pImplSdUnoHandoutViewService )
                || ServiceName.equalsAscii( pImplSdUnoDrawViewService );
            break;

        case ViewShell::ST_IMPRESS:
            bServiceIsSupported = ServiceName.equalsAscii(
                pImplSdUnoDrawViewService );

        default:
            // Shell type is not handled by this object.
            return sal_False;
    }

    return bServiceIsSupported;
}



Sequence<OUString> SAL_CALL SdUnoDrawView::getSupportedServiceNames (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    ViewShell::ShellType eShellType = mrViewShell.GetShellType();
    Sequence<OUString> aServices (
           (eShellType == ViewShell::ST_NOTES)
        || (eShellType == ViewShell::ST_HANDOUT)
        || (eShellType == ViewShell::ST_PRESENTATION)
           ? 2 : 1);
    OUString* pServices = aServices.getArray();

    int nIndex = 0;
    switch (eShellType)
    {
        case ViewShell::ST_PREVIEW:
            pServices[nIndex++] = OUString(
                RTL_CONSTASCII_USTRINGPARAM(pImplSdUnoPreviewViewService));
        break;

        case ViewShell::ST_NOTES:
            pServices[nIndex++] = OUString(
                RTL_CONSTASCII_USTRINGPARAM(pImplSdUnoNotesViewService));
            break;

        case ViewShell::ST_HANDOUT:
            pServices[nIndex++] = OUString(
                RTL_CONSTASCII_USTRINGPARAM(pImplSdUnoHandoutViewService));
        break;
    }

    switch (eShellType)
    {
        case ViewShell::ST_NOTES:
        case ViewShell::ST_HANDOUT:
        case ViewShell::ST_IMPRESS:
            pServices[nIndex] = OUString(
                RTL_CONSTASCII_USTRINGPARAM(pImplSdUnoDrawViewService));
    }

    return aServices;
}





// XSelectionSupplier


sal_Bool SAL_CALL SdUnoDrawView::select( const Any& aSelection )
    throw(lang::IllegalArgumentException, RuntimeException)
{
    ThrowIfDisposed();
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
            GetDrawViewShell().SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
            GetDrawViewShell().WriteFrameViewData();
        }

        SdrPageView *pPV = mrView.GetPageViewPvNum(0);

        if(pPV)
        {
            // first deselect all
            mrView.UnmarkAllObj( pPV );

            vector<SdrObject*>::iterator aIter( aObjects.begin() );
            const vector<SdrObject*>::iterator aEnd( aObjects.end() );
            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                mrView.MarkObj( pObj, pPV );
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
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    SdXImpressDocument* pModel = GetModel();

    Reference< drawing::XShapes > xShapes( SvxShapeCollection_NewInstance(), UNO_QUERY );

    DBG_ASSERT (&mrView != NULL,
        "view is NULL in SdUnoDrawView::getSelection()");

    const SdrMarkList& rMarkList = mrView.GetMarkedObjectList();
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

    Any aAny;
    if( 0 != xShapes->getCount() )
        aAny <<= xShapes;

    return aAny;
}


//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------


/**
 * All Properties of this implementation. Must be sorted by name.
 */
void SdUnoDrawView::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    DrawController::FillPropertyTable (rProperties);

    static const beans::Property aBasicProps[
        PROPERTY__END - PROPERTY__BEGIN] = {
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),        PROPERTY_CURRENTPAGE,   ::getCppuType((const Reference< drawing::XDrawPage > *)0), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsLayerMode") ),        PROPERTY_LAYERMODE,     ::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode") ),   PROPERTY_MASTERPAGEMODE,::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ActiveLayer") ),        PROPERTY_ACTIVE_LAYER,  ::getCppuBooleanType(), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomValue") ),          PROPERTY_ZOOMVALUE,     ::getCppuType((const sal_Int16*)0), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomType") ),           PROPERTY_ZOOMTYPE,      ::getCppuType((const sal_Int16*)0), beans::PropertyAttribute::BOUND ),
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ViewOffset") ),         PROPERTY_VIEWOFFSET,    ::getCppuType((const ::com::sun::star::awt::Point*)0), beans::PropertyAttribute::BOUND ),
    };
    int n = PROPERTY__END - PROPERTY__BEGIN;
    for (int i=0; i<n; i++)
        rProperties.push_back (aBasicProps[i]);
}

//----------------------------------------------------------------------
//------ XPropertySet & OPropertySetHelper -----------------------------
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
    sal_Bool bResult = sal_False;

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
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_MASTERPAGEMODE:
            {
                sal_Bool bOldValue = getMasterPageMode();
                sal_Bool b;
                convertPropertyValue( b , rValue );
                if( b != bOldValue )
                {

                    rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                    rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_LAYERMODE:
            {
                sal_Bool bOldValue = getLayerMode();
                sal_Bool b;
                convertPropertyValue( b , rValue );
                if( b != bOldValue )
                {
                    rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                    rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_ACTIVE_LAYER:
            {
                Reference<drawing::XLayer> xOldLayer (getActiveLayer());
                Reference<drawing::XLayer> xNewLayer;
                convertPropertyValue (xNewLayer, rValue);
                if (xOldLayer != xNewLayer)
                {
                    rConvertedValue <<= xNewLayer;
                    rOldValue <<= xOldLayer;
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_ZOOMVALUE:
            {
                sal_Int16 nOldZoom = GetZoom();
                sal_Int16 nNewZoom;
                convertPropertyValue( nNewZoom, rValue );
                if( nNewZoom != nOldZoom )
                {
                    rConvertedValue <<= nNewZoom;
                    rOldValue <<= nOldZoom;
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_ZOOMTYPE:
            {
                sal_Int16 nOldType = com::sun::star::view::DocumentZoomType::BY_VALUE;
                sal_Int16 nNewType;
                convertPropertyValue( nNewType, rValue );
                if( nNewType != nOldType )
                {
                    rConvertedValue <<= nNewType;
                    rOldValue <<= nOldType;
                    bResult = sal_True;
                }
            }
            break;

        case PROPERTY_VIEWOFFSET:
            {
                awt::Point aOld( GetViewOffset() );
                awt::Point aNew;
                convertPropertyValue( aNew, rValue );
                if( (aOld.X != aNew.X) && (aOld.Y != aNew.Y) )
                {
                    rConvertedValue <<= aNew;
                    rOldValue <<= aOld;
                    bResult = sal_True;
                }
            }
            break;

        default:
            bResult = DrawController::convertFastPropertyValue
                (rConvertedValue, rOldValue, nHandle, rValue);
            break;
    }

    return bResult;
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

        default:
            DrawController::setFastPropertyValue_NoBroadcast (nHandle, rValue);
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

        case PROPERTY_ZOOMVALUE:
            rRet <<= GetZoom();
            break;
        case PROPERTY_ZOOMTYPE:
            rRet <<= (sal_Int16)com::sun::star::view::DocumentZoomType::BY_VALUE;
            break;
        case PROPERTY_VIEWOFFSET:
            rRet <<= GetViewOffset();
            break;

            /*      case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle(
                maLastVisArea.Left(),
                maLastVisArea.Top(),
                maLastVisArea.GetWidth(),
                maLastVisArea.GetHeight());
            break;
            */
        default:
            DrawController::getFastPropertyValue (rRet, nHandle);
    }
}




// XDrawView


void SAL_CALL SdUnoDrawView::setCurrentPage (
    const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT (&mrView!=NULL,
        "view is NULL in SdUnoDrawView::setCurrentPage");

    SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
    SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

    if(pSdrPage)
    {
        setMasterPageMode( pSdrPage->IsMasterPage() );
        GetDrawViewShell().SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
        GetDrawViewShell().WriteFrameViewData();
    }
}

//----------------------------------------------------------------------

Reference< drawing::XDrawPage > SAL_CALL SdUnoDrawView::getCurrentPage()
    throw(RuntimeException)
{
    ThrowIfDisposed();
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XDrawPage >  xPage;

    DBG_ASSERT (&mrView!=NULL,
        "view is NULL in SdUnoDrawView::getCurrentPage");

    SdXImpressDocument* pModel = GetModel();
    SdrPageView *pPV = mrView.GetPageViewPvNum(0);
    SdrPage* pPage = pPV ? pPV->GetPage() : NULL;

    if(pPage)
        xPage = Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );

    return xPage;
}


sal_Int16 SdUnoDrawView::GetZoom(void) const
{
    if (GetDrawViewShell().GetActiveWindow() )
    {
        return (sal_Int16)GetDrawViewShell().GetActiveWindow()->GetZoom();
    }
    else
    {
        return 0;
    }
}

void SdUnoDrawView::SetZoom( sal_Int16 nZoom )
{
    SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, nZoom );

    if (&mrViewShell != NULL)
    {
        SfxViewFrame* pViewFrame = mrViewShell.GetViewFrame();
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
    DBG_ASSERT (&mrViewShell != NULL,
        "view shell is NULL in SdUnoDrawView::SetViewOffset");

    Point aWinPos( rWinPos.X, rWinPos.Y );
    aWinPos += mrViewShell.GetViewOrigin();
    mrViewShell.SetWinViewPos( aWinPos, true );
}

awt::Point SdUnoDrawView::GetViewOffset() const
{
    Point aRet;

    DBG_ASSERT (&mrViewShell != NULL,
        "view shell is NULL in SdUnoDrawView::GetViewOffset");

    aRet = mrViewShell.GetWinViewPos();
    aRet -= mrViewShell.GetViewOrigin();

    return awt::Point( aRet.X(), aRet.Y() );
}

void SdUnoDrawView::SetZoomType ( sal_Int16 nType )
{
    DBG_ASSERT (&mrViewShell != NULL,
        "view shell is NULL in SdUnoDrawView::SetZoomType");

    SfxViewFrame* pViewFrame = mrViewShell.GetViewFrame();
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

                default:
                    return;
            }
            SvxZoomItem aZoomItem( eZoomType );
            pDispatcher->Execute(SID_ATTR_ZOOM,SFX_CALLMODE_SYNCHRON,&aZoomItem, 0L);
        }
    }
}

} // end of namespace sd
