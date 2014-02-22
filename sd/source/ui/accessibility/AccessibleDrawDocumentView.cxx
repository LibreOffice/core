/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "AccessibleDrawDocumentView.hxx"
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.h>
#include<sfx2/viewfrm.hxx>

#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "Window.hxx"
#include <vcl/svapp.hxx>


#include "ViewShell.hxx"
#include "View.hxx"
#include "DrawDocShell.hxx"
#include <drawdoc.hxx>
#include <algorithm>
#include "sdpage.hxx"
#include "slideshow.hxx"
#include "anminfo.hxx"

#include "accessibility.hrc"
#include "sdresid.hxx"
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

namespace accessibility {


struct XShapePosCompareHelper
{
    bool operator() ( const uno::Reference<drawing::XShape>& xshape1,
        const uno::Reference<drawing::XShape>& xshape2 ) const
    {
        
        SdrObject* pObj1 = GetSdrObjectFromXShape(xshape1);
        SdrObject* pObj2 = GetSdrObjectFromXShape(xshape2);
        if(pObj1 && pObj2)
            return pObj1->GetOrdNum() < pObj2->GetOrdNum();
        else
            return false;
    }
};


AccessibleDrawDocumentView::AccessibleDrawDocumentView (
    ::sd::Window* pSdWindow,
    ::sd::ViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      mpSdViewSh( pViewShell ),
      mpChildrenManager (NULL)
{
    OSL_TRACE ("AccessibleDrawDocumentView");
    UpdateAccessibleName();
}




AccessibleDrawDocumentView::~AccessibleDrawDocumentView (void)
{
    OSL_TRACE ("~AccessibleDrawDocumentView");
    DBG_ASSERT (rBHelper.bDisposed || rBHelper.bInDispose,
        "~AccessibleDrawDocumentView: object has not been disposed");
}




void AccessibleDrawDocumentView::Init (void)
{
    AccessibleDocumentViewBase::Init ();

    
    uno::Reference<drawing::XShapes> xShapeList;
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
        xShapeList = uno::Reference<drawing::XShapes> (
            xView->getCurrentPage(), uno::UNO_QUERY);

    
    mpChildrenManager = new ChildrenManager(this, xShapeList, maShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
    {
        rtl::Reference<AccessiblePageShape> xPage(CreateDrawPageShape());
        if (xPage.is())
        {
            xPage->Init();
            mpChildrenManager->AddAccessibleShape (xPage.get());
            mpChildrenManager->Update ();
        }
        mpChildrenManager->UpdateSelection ();
    }
}




void AccessibleDrawDocumentView::ViewForwarderChanged (ChangeType aChangeType,
    const IAccessibleViewForwarder* pViewForwarder)
{
    AccessibleDocumentViewBase::ViewForwarderChanged (aChangeType, pViewForwarder);
    if (mpChildrenManager != NULL)
        mpChildrenManager->ViewForwarderChanged (aChangeType, pViewForwarder);
}




/**  The page shape is created on every call at the moment (provided that
     every thing goes well).
*/
rtl::Reference<AccessiblePageShape> AccessibleDrawDocumentView::CreateDrawPageShape (void)
{
    rtl::Reference<AccessiblePageShape> xShape;

    
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
    {
        uno::Reference<beans::XPropertySet> xSet (
            uno::Reference<beans::XPropertySet> (xView->getCurrentPage(), uno::UNO_QUERY));
        if (xSet.is())
        {
            
            uno::Reference<lang::XMultiServiceFactory> xFactory (mxModel, uno::UNO_QUERY);
            uno::Reference<drawing::XShape> xRectangle;
            if (xFactory.is())
                xRectangle = uno::Reference<drawing::XShape>(xFactory->createInstance ("com.sun.star.drawing.RectangleShape"),
                    uno::UNO_QUERY);

            
            if (xRectangle.is())
            {
                uno::Any aValue;
                awt::Point aPosition;
                awt::Size aSize;

                
                
                aValue = xSet->getPropertyValue ("BorderLeft");
                aValue >>= aPosition.X;
                aValue = xSet->getPropertyValue ("BorderTop");
                aValue >>= aPosition.Y;
                xRectangle->setPosition (aPosition);

                aValue = xSet->getPropertyValue ("Width");
                aValue >>= aSize.Width;
                aValue = xSet->getPropertyValue ("Height");
                aValue >>= aSize.Height;
                xRectangle->setSize (aSize);

                
                
                xShape = new AccessiblePageShape (
                    xView->getCurrentPage(), this, maShapeTreeInfo);
            }
        }
    }
    return xShape;
}






sal_Int32 SAL_CALL
    AccessibleDrawDocumentView::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    long mpChildCount = AccessibleDocumentViewBase::getAccessibleChildCount();

    
    if (mpChildrenManager != NULL)
        mpChildCount += mpChildrenManager->GetChildCount ();

    return mpChildCount;
}




uno::Reference<XAccessible> SAL_CALL
    AccessibleDrawDocumentView::getAccessibleChild (sal_Int32 nIndex)
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ThrowIfDisposed ();

    ::osl::ClearableMutexGuard aGuard (maMutex);

    
    sal_Int32 nCount = AccessibleDocumentViewBase::getAccessibleChildCount();
    if (nCount > 0)
    {
        if (nIndex < nCount)
            return AccessibleDocumentViewBase::getAccessibleChild(nIndex);
        else
            nIndex -= nCount;
    }

    
    
    ChildrenManager* pChildrenManager = mpChildrenManager;
    aGuard.clear();

    
    if (pChildrenManager != NULL)
    {
        return pChildrenManager->GetChild (nIndex);
    }
    else
        throw lang::IndexOutOfBoundsException (
            "no accessible child with index " + OUString::number(nIndex),
            static_cast<uno::XWeak*>(this));
}

OUString SAL_CALL
    AccessibleDrawDocumentView::getAccessibleName(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    OUString sName = SdResId(SID_SD_A11Y_D_PRESENTATION);
    ::sd::View* pSdView = static_cast< ::sd::View* >( maShapeTreeInfo.GetSdrView() );
    if ( pSdView )
    {
        SdDrawDocument& rDoc = pSdView->GetDoc();
        OUString sFileName = rDoc.getDocAccTitle();
        if ( !sFileName.getLength() )
        {
            ::sd::DrawDocShell* pDocSh = pSdView->GetDocSh();
            if ( pDocSh )
            {
                sFileName = pDocSh->GetTitle( SFX_TITLE_APINAME );
            }
        }

        OUString sReadOnly;
        if(rDoc.getDocReadOnly())
        {
            sReadOnly = SdResId(SID_SD_A11Y_D_PRESENTATION_READONLY);
        }

        if ( sFileName.getLength() )
        {
            sName = sFileName + sReadOnly + " - " + sName;
        }
    }

    return sName;
}



void SAL_CALL
    AccessibleDrawDocumentView::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::disposing (rEventObject);
    if (rEventObject.Source == mxModel)
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        
        if (mpChildrenManager != NULL)
            mpChildrenManager->SetInfo (maShapeTreeInfo);
    }
}






void SAL_CALL
    AccessibleDrawDocumentView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleDrawDocumentView::propertyChange");
    
    if (rEventObject.PropertyName == "CurrentPage" ||
        rEventObject.PropertyName == "PageChange")
    {
        OSL_TRACE ("    current page changed");

        
        UpdateAccessibleName();

        
        uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
        if (xView.is() && mpChildrenManager!=NULL)
        {
            
            
            mpChildrenManager->ClearAccessibleShapeList ();
            mpChildrenManager->SetShapeList (uno::Reference<drawing::XShapes> (
                xView->getCurrentPage(), uno::UNO_QUERY));

            rtl::Reference<AccessiblePageShape> xPage(CreateDrawPageShape ());
            if (xPage.is())
            {
                xPage->Init();
                mpChildrenManager->AddAccessibleShape (xPage.get());
                mpChildrenManager->Update (false);
            }
        }
        else
            OSL_TRACE ("View invalid");
        CommitChange(AccessibleEventId::PAGE_CHANGED,rEventObject.NewValue,rEventObject.OldValue);
    }
    else if ( rEventObject.PropertyName == "VisibleArea" )
    {
        OSL_TRACE ("    visible area changed");
        if (mpChildrenManager != NULL)
            mpChildrenManager->ViewForwarderChanged (
                IAccessibleViewForwarderListener::VISIBLE_AREA,
                &maViewForwarder);
    }
    else if (rEventObject.PropertyName == OUString (RTL_CONSTASCII_USTRINGPARAM("ActiveLayer")))
    {
        CommitChange(AccessibleEventId::PAGE_CHANGED,rEventObject.NewValue,rEventObject.OldValue);
    }
    else if (rEventObject.PropertyName == OUString (RTL_CONSTASCII_USTRINGPARAM("UpdateAcc")))
    {
        OSL_TRACE ("    acc on current page should be updated");

        
        uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
        if (xView.is() && mpChildrenManager!=NULL)
        {
            
            
            mpChildrenManager->ClearAccessibleShapeList ();
            
            
            
            rtl::Reference< sd::SlideShow > xSlideshow( sd::SlideShow::GetSlideShow( mpSdViewSh->GetViewShellBase() ) );
            if( xSlideshow.is() && xSlideshow->isRunning() && xSlideshow->isFullScreen() )
            {
                ::com::sun::star::uno::Reference< drawing::XDrawPage > xSlide;
                
                
                ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowController > mpSlideController = xSlideshow->getController();
                if( mpSlideController.is() )
                {
                    xSlide = mpSlideController->getCurrentSlide();
                    if (xSlide.is())
                    {
                        mpChildrenManager->SetShapeList (uno::Reference<drawing::XShapes> (
                                    xSlide, uno::UNO_QUERY));
                    }
                }
            }
            rtl::Reference<AccessiblePageShape> xPage(CreateDrawPageShape ());
            if (xPage.is())
            {
                xPage->Init();
                mpChildrenManager->AddAccessibleShape (xPage.get());
                mpChildrenManager->Update (false);
            }
        }
    }
    else
    {
        OSL_TRACE ("  unhandled");
    }
    OSL_TRACE ("  done");
}





OUString SAL_CALL
    AccessibleDrawDocumentView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleDrawDocumentView");
}




::com::sun::star::uno::Sequence< OUString> SAL_CALL
    AccessibleDrawDocumentView::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed();
    
    uno::Sequence<OUString> aServiceNames =
        AccessibleDocumentViewBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName ("com.sun.star.drawing.AccessibleDrawDocumentView");
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}



uno::Any SAL_CALL
    AccessibleDrawDocumentView::queryInterface (const uno::Type & rType)
    throw (uno::RuntimeException)
{
    uno::Any aReturn = AccessibleDocumentViewBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleGroupPosition*>(this)
            );
    return aReturn;
}

void SAL_CALL
    AccessibleDrawDocumentView::acquire (void)
    throw ()
{
    AccessibleDocumentViewBase::acquire ();
}
void SAL_CALL
    AccessibleDrawDocumentView::release (void)
    throw ()
{
    AccessibleDocumentViewBase::release ();
}

uno::Sequence< sal_Int32 > SAL_CALL
    AccessibleDrawDocumentView::getGroupPosition( const uno::Any& rAny )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard g;

    
    
    
    
    uno::Sequence< sal_Int32 > aRet( 3 );
    
    uno::Reference<XAccessibleContext> xAccContent;
    rAny >>= xAccContent;
    if ( !xAccContent.is() )
    {
        return aRet;
    }
    AccessibleShape* pAcc = AccessibleShape::getImplementation( xAccContent );
    if ( !pAcc )
    {
        return aRet;
    }
    uno::Reference< drawing::XShape > xCurShape = pAcc->GetXShape();
    if ( !xCurShape.is() )
    {
        return aRet;
    }
    
    if ( mpChildrenManager == NULL )
    {
        return aRet;
    }
    std::vector< uno::Reference<drawing::XShape> > vXShapes;
    sal_Int32 nCount = mpChildrenManager->GetChildCount();
    
    SdrPageView* pPV = NULL;
    ::sd::View* pSdView = NULL;
    if ( mpSdViewSh )
    {
        pSdView = mpSdViewSh->GetView();
        pPV = pSdView->GetSdrPageView();
    }
    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        uno::Reference< drawing::XShape > xShape = mpChildrenManager->GetChildShape(i);
        if ( xShape.is() )
        {
            
            SdrObject* pObj = GetSdrObjectFromXShape(xShape);
            if ( pObj && pPV && pSdView && pSdView->IsObjMarkable( pObj, pPV ) )
            {
                vXShapes.push_back( xShape );
            }
        }
    }
    std::sort( vXShapes.begin(), vXShapes.end(), XShapePosCompareHelper() );
    
    std::vector< uno::Reference<drawing::XShape> >::iterator aIter;
    
    sal_Int32 nPos = 1;
    for ( aIter = vXShapes.begin(); aIter != vXShapes.end(); ++aIter, nPos++ )
    {
        if ( (*aIter).get() == xCurShape.get() )
        {
            sal_Int32* pArray = aRet.getArray();
            pArray[0] = 1; 
            pArray[1] = vXShapes.size();
            pArray[2] = nPos;
            break;
        }
    }
    return aRet;
}

OUString AccessibleDrawDocumentView::getObjectLink( const uno::Any& rAny )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    OUString aRet;
    
    uno::Reference<XAccessibleContext> xAccContent;
    rAny >>= xAccContent;
    if ( !xAccContent.is() )
    {
        return aRet;
    }
    AccessibleShape* pAcc = AccessibleShape::getImplementation( xAccContent );
    if ( !pAcc )
    {
        return aRet;
    }
    uno::Reference< drawing::XShape > xCurShape = pAcc->GetXShape();
    if ( !xCurShape.is() )
    {
        return aRet;
    }
    SdrObject* pObj = GetSdrObjectFromXShape(xCurShape);
    if (pObj)
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj);
        if( pInfo && (pInfo->meClickAction == presentation::ClickAction_DOCUMENT) )
            aRet = pInfo->GetBookmark();
    }
    return aRet;
}


OUString AccessibleDrawDocumentView::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        uno::Sequence< OUString > aServices( xInfo->getSupportedServiceNames() );
        OUString sFirstService = aServices[0];
        if ( sFirstService == "com.sun.star.drawing.DrawingDocumentDrawView" )
        {
            if( aServices.getLength() >= 2 && aServices[1] == "com.sun.star.presentation.PresentationView")
            {
                SolarMutexGuard aGuard;

                sName = SD_RESSTR(SID_SD_A11Y_I_DRAWVIEW_N);
            }
            else
            {
                SolarMutexGuard aGuard;

                sName = SD_RESSTR(SID_SD_A11Y_D_DRAWVIEW_N);
            }
        }
        else if ( sFirstService == "com.sun.star.presentation.NotesView" )
        {
            SolarMutexGuard aGuard;

            sName = SD_RESSTR(SID_SD_A11Y_I_NOTESVIEW_N);
        }
        else if ( sFirstService == "com.sun.star.presentation.HandoutView" )
        {
            SolarMutexGuard aGuard;

            sName = SD_RESSTR(SID_SD_A11Y_I_HANDOUTVIEW_N);
        }
        else
        {
            sName = sFirstService;
        }
    }
    else
    {
        sName = "AccessibleDrawDocumentView";
    }
    return sName;
}




/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
OUString
    AccessibleDrawDocumentView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sDescription;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        uno::Sequence< OUString > aServices( xInfo->getSupportedServiceNames() );
        OUString sFirstService = aServices[0];
        if ( sFirstService == "com.sun.star.drawing.DrawingDocumentDrawView" )
        {
            if( aServices.getLength() >= 2 && aServices[1] == "com.sun.star.presentation.PresentationView")
            {
                SolarMutexGuard aGuard;

                sDescription = SD_RESSTR(SID_SD_A11Y_I_DRAWVIEW_D);
            }
            else
            {
                SolarMutexGuard aGuard;

                sDescription = SD_RESSTR(SID_SD_A11Y_D_DRAWVIEW_D);
            }
        }
        else if ( sFirstService == "com.sun.star.presentation.NotesView" )
        {
            SolarMutexGuard aGuard;

            sDescription = SD_RESSTR(SID_SD_A11Y_I_NOTESVIEW_D);
        }
        else if ( sFirstService == "com.sun.star.presentation.HandoutView" )
        {
            SolarMutexGuard aGuard;

            sDescription = SD_RESSTR(SID_SD_A11Y_I_HANDOUTVIEW_D);
        }
        else
        {
            sDescription = sFirstService;
        }
    }
    else
    {
        sDescription = "Accessible Draw Document";
    }
    return sDescription;
}




/** Return selection state of specified child
*/
bool
    AccessibleDrawDocumentView::implIsSelected( sal_Int32 nAccessibleChildIndex )
    throw (uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< view::XSelectionSupplier >  xSel( mxController, uno::UNO_QUERY );
    sal_Bool                                    bRet = sal_False;

    OSL_ENSURE( 0 <= nAccessibleChildIndex, "AccessibleDrawDocumentView::implIsSelected: invalid index!" );

    if( xSel.is() && ( 0 <= nAccessibleChildIndex ) )
    {
        uno::Any                            aAny( xSel->getSelection() );
        uno::Reference< drawing::XShapes >  xShapes;

        aAny >>= xShapes;

        if( xShapes.is() )
        {
            AccessibleShape* pAcc = AccessibleShape::getImplementation( getAccessibleChild( nAccessibleChildIndex ) );

            if( pAcc )
            {
                uno::Reference< drawing::XShape > xShape( pAcc->GetXShape() );

                if( xShape.is() )
                {
                    for( sal_Int32 i = 0, nCount = xShapes->getCount(); ( i < nCount ) && !bRet; ++i )
                        if( xShapes->getByIndex( i ) == xShape )
                            bRet = sal_True;
                }
            }
        }
    }

    return( bRet );
}




/** Select or delselect the specified shapes.  The corresponding accessible
    shapes are notified over the selection change listeners registered with
    the XSelectionSupplier of the controller.
*/
void
    AccessibleDrawDocumentView::implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< view::XSelectionSupplier >  xSel( mxController, uno::UNO_QUERY );

    if( xSel.is() )
    {
        uno::Any aAny;

        if( ACCESSIBLE_SELECTION_CHILD_ALL == nAccessibleChildIndex )
        {
            

            if( !bSelect )
                xSel->select( aAny );
            else
            {
                uno::Reference< drawing::XShapes > xShapes = drawing::ShapeCollection::create(
                        comphelper::getProcessComponentContext());

                for(sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
                {
                    AccessibleShape* pAcc = AccessibleShape::getImplementation( getAccessibleChild( i ) );

                    if( pAcc && pAcc->GetXShape().is() )
                        xShapes->add( pAcc->GetXShape() );
                }

                if( xShapes->getCount() )
                {
                    aAny <<= xShapes;
                    xSel->select( aAny );
                }
            }
        }
        else if( nAccessibleChildIndex >= 0 )
        {
            
            

            AccessibleShape* pAcc = AccessibleShape::getImplementation(
                getAccessibleChild( nAccessibleChildIndex ));

            
            
            if( pAcc )
            {
                uno::Reference< drawing::XShape > xShape( pAcc->GetXShape() );

                if( xShape.is() )
                {
                    uno::Reference< drawing::XShapes >  xShapes;
                    sal_Bool                            bFound = sal_False;

                    aAny = xSel->getSelection();
                    aAny >>= xShapes;

                    
                    if (xShapes.is())
                    {
                        sal_Int32 nCount = xShapes->getCount();
                        for (sal_Int32 i=0; ( i < nCount ) && !bFound; ++i )
                            if( xShapes->getByIndex( i ) == xShape )
                                bFound = sal_True;
                    }
                    else
                        
                        xShapes = drawing::ShapeCollection::create(
                                comphelper::getProcessComponentContext());

                    
                    if( !bFound && bSelect )
                        xShapes->add( xShape );
                    else if( bFound && !bSelect )
                        xShapes->remove( xShape );

                    aAny <<= xShapes;
                    xSel->select( aAny );
                }
            }
        }
    }
}




void AccessibleDrawDocumentView::Activated (void)
{
    if (mpChildrenManager != NULL)
    {
        sal_Bool bChange = sal_False;
        
        
        if ( ! mpChildrenManager->HasFocus())
        {
            SetState (AccessibleStateType::FOCUSED);
            bChange = sal_True;
        }
        else
            ResetState (AccessibleStateType::FOCUSED);
    mpChildrenManager->UpdateSelection();
    
    if (mpChildrenManager->HasFocus() && bChange)
        ResetState (AccessibleStateType::FOCUSED);
    }
}




void AccessibleDrawDocumentView::Deactivated (void)
{
    if (mpChildrenManager != NULL)
        mpChildrenManager->RemoveFocus();
    ResetState (AccessibleStateType::FOCUSED);
}




void AccessibleDrawDocumentView::impl_dispose (void)
{
    if (mpChildrenManager != NULL)
    {
        delete mpChildrenManager;
        mpChildrenManager = NULL;
    }

    AccessibleDocumentViewBase::impl_dispose();
}



/** This method is called from the component helper base class while
    disposing.
*/
void SAL_CALL AccessibleDrawDocumentView::disposing (void)
{

    
    if (mpChildrenManager != NULL)
    {
        delete mpChildrenManager;
        mpChildrenManager = NULL;
    }

    
    AccessibleDocumentViewBase::disposing ();
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL AccessibleDrawDocumentView::get_AccFlowTo(const ::com::sun::star::uno::Any& rAny, sal_Int32 nType)
        throw ( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard g;

    const sal_Int32 SPELLCHECKFLOWTO = 1;
    const sal_Int32 FINDREPLACEFLOWTO = 2;
    if ( nType == SPELLCHECKFLOWTO )
    {
        uno::Reference< ::com::sun::star::drawing::XShape > xShape;
        rAny >>= xShape;
        if ( mpChildrenManager && xShape.is() )
        {
            uno::Reference < XAccessible > xAcc = mpChildrenManager->GetChild(xShape);
            uno::Reference < XAccessibleSelection > xAccSelection( xAcc, uno::UNO_QUERY );
            if ( xAccSelection.is() )
            {
                if ( xAccSelection->getSelectedAccessibleChildCount() )
                {
                    uno::Reference < XAccessible > xSel = xAccSelection->getSelectedAccessibleChild( 0 );
                    if ( xSel.is() )
                    {
                        uno::Reference < XAccessibleContext > xSelContext( xSel->getAccessibleContext() );
                        if ( xSelContext.is() )
                        {
                            
                            if ( xSelContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                            {
                                uno::Sequence<uno::Any> aRet( 1 );
                                aRet[0] = uno::makeAny( xSel );
                                return aRet;
                            }
                        }
                    }
                }
            }
            uno::Reference<XAccessible> xPara = GetSelAccContextInTable();
            if ( xPara.is() )
            {
                uno::Sequence<uno::Any> aRet( 1 );
                aRet[0] = uno::makeAny( xPara );
                return aRet;
            }
        }
        else
        {
            goto Rt;
        }
    }
    else if ( nType == FINDREPLACEFLOWTO )
    {
        sal_Int32 nChildCount = getSelectedAccessibleChildCount();
        if ( nChildCount )
        {
            uno::Reference < XAccessible > xSel = getSelectedAccessibleChild( 0 );
            if ( xSel.is() )
            {
                uno::Reference < XAccessibleSelection > xAccChildSelection( xSel, uno::UNO_QUERY );
                if ( xAccChildSelection.is() )
                {
                    if ( xAccChildSelection->getSelectedAccessibleChildCount() )
                    {
                        uno::Reference < XAccessible > xChildSel = xAccChildSelection->getSelectedAccessibleChild( 0 );
                        if ( xChildSel.is() )
                        {
                            uno::Reference < XAccessibleContext > xChildSelContext( xChildSel->getAccessibleContext() );
                            if ( xChildSelContext.is() &&
                                xChildSelContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                            {
                                uno::Sequence<uno::Any> aRet( 1 );
                                aRet[0] = uno::makeAny( xChildSel );
                                return aRet;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            uno::Reference<XAccessible> xPara = GetSelAccContextInTable();
            if ( xPara.is() )
            {
                uno::Sequence<uno::Any> aRet( 1 );
                aRet[0] = uno::makeAny( xPara );
                return aRet;
            }
        }
    }

Rt:
    ::com::sun::star::uno::Sequence< uno::Any> aRet;
    return aRet;
}
uno::Reference<XAccessible> AccessibleDrawDocumentView::GetSelAccContextInTable()
{
    uno::Reference<XAccessible> xRet;
    sal_Int32 nCount = mpChildrenManager ? mpChildrenManager->GetChildCount() : 0;
    if ( nCount )
    {
        for ( sal_Int32 i = 0; i < nCount; i++ )
        {
            try
            {
                uno::Reference<XAccessible> xObj = mpChildrenManager->GetChild(i);
                if ( xObj.is() )
                {
                    uno::Reference<XAccessibleContext> xObjContext( xObj, uno::UNO_QUERY );
                    if ( xObjContext.is() && xObjContext->getAccessibleRole() == AccessibleRole::TABLE )
                    {
                        uno::Reference<XAccessibleSelection> xObjSelection( xObj, uno::UNO_QUERY );
                        if ( xObjSelection.is() && xObjSelection->getSelectedAccessibleChildCount() )
                        {
                            uno::Reference<XAccessible> xCell = xObjSelection->getSelectedAccessibleChild(0);
                            if ( xCell.is() )
                            {
                                uno::Reference<XAccessibleSelection> xCellSel( xCell, uno::UNO_QUERY );
                                if ( xCellSel.is() && xCellSel->getSelectedAccessibleChildCount() )
                                {
                                    uno::Reference<XAccessible> xPara = xCellSel->getSelectedAccessibleChild( 0 );
                                    if ( xPara.is() )
                                    {
                                        uno::Reference<XAccessibleContext> xParaContext( xPara, uno::UNO_QUERY );
                                        if ( xParaContext.is() &&
                                            xParaContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                                        {
                                            xRet = xPara;
                                            return xRet;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch ( lang::IndexOutOfBoundsException )
            {
                uno::Reference<XAccessible> xEmpty;
                return xEmpty;
            }
            catch ( uno::RuntimeException )
            {
                uno::Reference<XAccessible> xEmpty;
                return xEmpty;
            }
        }
    }

    return xRet;
}

void AccessibleDrawDocumentView::UpdateAccessibleName (void)
{
    OUString sNewName (CreateAccessibleName());
    sNewName += ": ";

    
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
    {
        uno::Reference<beans::XPropertySet> xProperties (xView->getCurrentPage(), UNO_QUERY);
        if (xProperties.is())
            try
            {
                sal_Int16 nPageNumber (0);
                if (xProperties->getPropertyValue("Number") >>= nPageNumber)
                {
                    sNewName += OUString::number(nPageNumber);
                }
            }
            catch (beans::UnknownPropertyException&)
            {
            }
    }

    
    Reference<drawing::XDrawPagesSupplier> xPagesSupplier (mxModel, UNO_QUERY);
    if (xPagesSupplier.is())
    {
        Reference<container::XIndexAccess> xPages (xPagesSupplier->getDrawPages(), UNO_QUERY);
        if (xPages.is())
        {
            sNewName += " / ";
            sNewName += OUString::number(xPages->getCount());
        }
    }

    SetAccessibleName (sNewName, AutomaticallyCreated);
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
