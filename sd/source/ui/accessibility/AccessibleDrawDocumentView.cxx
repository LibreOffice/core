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
#include "precompiled_sd.hxx"
#include "AccessibleDrawDocumentView.hxx"
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
#include <rtl/ustring.h>
#include<sfx2/viewfrm.hxx>

#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshcol.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "Window.hxx"
#include <vcl/svapp.hxx>


#include "ViewShell.hxx"
#include "View.hxx"
#include <memory>

#include "accessibility.hrc"
#include "sdresid.hxx"
#include <vos/mutex.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

class SfxViewFrame;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace accessibility {


//=====  internal  ============================================================

AccessibleDrawDocumentView::AccessibleDrawDocumentView (
    ::sd::Window* pSdWindow,
    ::sd::ViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
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

    // Determine the list of shapes on the current page.
    uno::Reference<drawing::XShapes> xShapeList;
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
        xShapeList = uno::Reference<drawing::XShapes> (
            xView->getCurrentPage(), uno::UNO_QUERY);

    // Create the children manager.
    mpChildrenManager = new ChildrenManager(this, xShapeList, maShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
    {
        // Create the page shape and initialize it.  The shape is acquired
        // before initialization and released after transferring ownership
        // to the children manager to prevent premature disposing of the
        // shape.
        AccessiblePageShape* pPage = CreateDrawPageShape();
        if (pPage != NULL)
        {
            pPage->acquire();
            pPage->Init();
            mpChildrenManager->AddAccessibleShape (
                std::auto_ptr<AccessibleShape>(pPage));
            pPage->release();
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
AccessiblePageShape* AccessibleDrawDocumentView::CreateDrawPageShape (void)
{
    AccessiblePageShape* pShape = NULL;

    // Create a shape that represents the actual draw page.
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
    {
        uno::Reference<beans::XPropertySet> xSet (
            uno::Reference<beans::XPropertySet> (xView->getCurrentPage(), uno::UNO_QUERY));
        if (xSet.is())
        {
            // Create a rectangle shape that will represent the draw page.
            uno::Reference<lang::XMultiServiceFactory> xFactory (mxModel, uno::UNO_QUERY);
            uno::Reference<drawing::XShape> xRectangle;
            if (xFactory.is())
                xRectangle = uno::Reference<drawing::XShape>(xFactory->createInstance (
                    OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.RectangleShape"))),
                    uno::UNO_QUERY);

            // Set the shape's size and position.
            if (xRectangle.is())
            {
                uno::Any aValue;
                awt::Point aPosition;
                awt::Size aSize;

                // Set size and position of the shape to those of the draw
                // page.
                aValue = xSet->getPropertyValue (
                    OUString (RTL_CONSTASCII_USTRINGPARAM("BorderLeft")));
                aValue >>= aPosition.X;
                aValue = xSet->getPropertyValue (
                    OUString (RTL_CONSTASCII_USTRINGPARAM("BorderTop")));
                aValue >>= aPosition.Y;
                xRectangle->setPosition (aPosition);

                aValue = xSet->getPropertyValue (
                    OUString (RTL_CONSTASCII_USTRINGPARAM("Width")));
                aValue >>= aSize.Width;
                aValue = xSet->getPropertyValue (
                    OUString (RTL_CONSTASCII_USTRINGPARAM("Height")));
                aValue >>= aSize.Height;
                xRectangle->setSize (aSize);

                // Create the accessible object for the shape and
                // initialize it.
                pShape = new AccessiblePageShape (
                    xView->getCurrentPage(), this, maShapeTreeInfo);
            }
        }
    }
    return pShape;
}




//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
    AccessibleDrawDocumentView::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    long mpChildCount = AccessibleDocumentViewBase::getAccessibleChildCount();

    // Forward request to children manager.
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

    // Take care of children of the base class.
    sal_Int32 nCount = AccessibleDocumentViewBase::getAccessibleChildCount();
    if (nCount > 0)
    {
        if (nIndex < nCount)
            return AccessibleDocumentViewBase::getAccessibleChild(nIndex);
        else
            nIndex -= nCount;
    }

    // Create a copy of the pointer to the children manager and release the
    // mutex before calling any of its methods.
    ChildrenManager* pChildrenManager = mpChildrenManager;
    aGuard.clear();

    // Forward request to children manager.
    if (pChildrenManager != NULL)
    {
        return pChildrenManager->GetChild (nIndex);
    }
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("no accessible child with index ")
            + rtl::OUString::valueOf(nIndex),
            static_cast<uno::XWeak*>(this));
}




//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDrawDocumentView::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::disposing (rEventObject);
    if (rEventObject.Source == mxModel)
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        // maShapeTreeInfo has been modified in base class.
        if (mpChildrenManager != NULL)
            mpChildrenManager->SetInfo (maShapeTreeInfo);
    }
}




//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleDrawDocumentView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleDrawDocumentView::propertyChange");
    if (rEventObject.PropertyName == OUString (RTL_CONSTASCII_USTRINGPARAM("CurrentPage")))
    {
        OSL_TRACE ("    current page changed");

        // Update the accessible name to reflect the current slide.
        UpdateAccessibleName();

        // The current page changed.  Update the children manager accordingly.
        uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
        if (xView.is() && mpChildrenManager!=NULL)
        {
            // Inform the children manager to forget all children and give
            // him the new ones.
            mpChildrenManager->ClearAccessibleShapeList ();
            mpChildrenManager->SetShapeList (uno::Reference<drawing::XShapes> (
                xView->getCurrentPage(), uno::UNO_QUERY));

            // Create the page shape and initialize it.  The shape is
            // acquired before initialization and released after
            // transferring ownership to the children manager to prevent
            // premature disposing of the shape.
            AccessiblePageShape* pPage = CreateDrawPageShape ();
            if (pPage != NULL)
            {
                pPage->acquire();
                pPage->Init();
                mpChildrenManager->AddAccessibleShape (
                    std::auto_ptr<AccessibleShape>(pPage));
                mpChildrenManager->Update (false);
                pPage->release();
            }
        }
        else
            OSL_TRACE ("View invalid");
    }
    else if (rEventObject.PropertyName == OUString (RTL_CONSTASCII_USTRINGPARAM("VisibleArea")))
    {
        OSL_TRACE ("    visible area changed");
        if (mpChildrenManager != NULL)
            mpChildrenManager->ViewForwarderChanged (
                IAccessibleViewForwarderListener::VISIBLE_AREA,
                &maViewForwarder);
    }
    else
    {
        OSL_TRACE ("  unhandled");
    }
    OSL_TRACE ("  done");
}



//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleDrawDocumentView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "AccessibleDrawDocumentView"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleDrawDocumentView::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleDocumentViewBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.AccessibleDrawDocumentView"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}




/// Create a name for this view.
::rtl::OUString
    AccessibleDrawDocumentView::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString sName;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        uno::Sequence< ::rtl::OUString > aServices( xInfo->getSupportedServiceNames() );
        OUString sFirstService = aServices[0];
        if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentDrawView")))
        {
            if( aServices.getLength() >= 2 &&
                aServices[1] == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationView")))
            {
                SolarMutexGuard aGuard;

                sName = String( SdResId(SID_SD_A11Y_I_DRAWVIEW_N) );
            }
            else
            {
                SolarMutexGuard aGuard;

                sName = String( SdResId(SID_SD_A11Y_D_DRAWVIEW_N) );
            }
        }
        else if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.NotesView")))
        {
            SolarMutexGuard aGuard;

            sName = String( SdResId(SID_SD_A11Y_I_NOTESVIEW_N) );
        }
        else if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.HandoutView")))
        {
            SolarMutexGuard aGuard;

            sName = String( SdResId(SID_SD_A11Y_I_HANDOUTVIEW_N) );
        }
        else
        {
            sName = sFirstService;
        }
    }
    else
    {
        sName = OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleDrawDocumentView"));
    }
    return sName;
}




/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
::rtl::OUString
    AccessibleDrawDocumentView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString sDescription;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        uno::Sequence< ::rtl::OUString > aServices( xInfo->getSupportedServiceNames() );
        OUString sFirstService = aServices[0];
        if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentDrawView")))
        {
            if( aServices.getLength() >= 2 &&
                aServices[1] == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationView")))
            {
                SolarMutexGuard aGuard;

                sDescription = String( SdResId(SID_SD_A11Y_I_DRAWVIEW_D) );
            }
            else
            {
                SolarMutexGuard aGuard;

                sDescription = String( SdResId(SID_SD_A11Y_D_DRAWVIEW_D) );
            }
        }
        else if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.NotesView")))
        {
            SolarMutexGuard aGuard;

            sDescription = String( SdResId(SID_SD_A11Y_I_NOTESVIEW_D) );
        }
        else if (sFirstService == OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.HandoutView")))
        {
            SolarMutexGuard aGuard;

            sDescription = String( SdResId(SID_SD_A11Y_I_HANDOUTVIEW_D) );
        }
        else
        {
            sDescription = sFirstService;
        }
    }
    else
    {
        sDescription = OUString(RTL_CONSTASCII_USTRINGPARAM("Accessible Draw Document"));
    }
    return sDescription;
}




/** Return selection state of specified child
*/
sal_Bool
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
    AccessibleShape* pAccessibleChild;

    if( xSel.is() )
    {
        uno::Any aAny;

        if( ACCESSIBLE_SELECTION_CHILD_ALL == nAccessibleChildIndex )
        {
            // Select or deselect all children.

            if( !bSelect )
                xSel->select( aAny );
            else
            {
                uno::Reference< drawing::XShapes > xShapes( new SvxShapeCollection() );

                for(sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
                {
                    AccessibleShape* pAcc = AccessibleShape::getImplementation( getAccessibleChild( i ) );

                    if( pAcc && pAcc->GetXShape().is() )
                    {
                        xShapes->add( pAcc->GetXShape() );
                        pAccessibleChild = pAcc;
                    }
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
            // Select or deselect only the child with index
            // nAccessibleChildIndex.

            AccessibleShape* pAcc = AccessibleShape::getImplementation(
                getAccessibleChild( nAccessibleChildIndex ));
            pAccessibleChild = pAcc;

            // Add or remove the shape that is made accessible from the
            // selection of the controller.
            if( pAcc )
            {
                uno::Reference< drawing::XShape > xShape( pAcc->GetXShape() );

                if( xShape.is() )
                {
                    uno::Reference< drawing::XShapes >  xShapes;
                    sal_Bool                            bFound = sal_False;

                    aAny = xSel->getSelection();
                    aAny >>= xShapes;

                    // Search shape to be selected in current selection.
                    if (xShapes.is())
                    {
                        sal_Int32 nCount = xShapes->getCount();
                        for (sal_Int32 i=0; ( i < nCount ) && !bFound; ++i )
                            if( xShapes->getByIndex( i ) == xShape )
                                bFound = sal_True;
                    }
                    else
                        // Create an empty selection to add the shape to.
                        xShapes = new SvxShapeCollection();

                    // Update the selection.
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
        mpChildrenManager->UpdateSelection();
        // When none of the children has the focus then claim it for the
        // view.
        if ( ! mpChildrenManager->HasFocus())
            SetState (AccessibleStateType::FOCUSED);
        else
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

    // Release resources.
    if (mpChildrenManager != NULL)
    {
        delete mpChildrenManager;
        mpChildrenManager = NULL;
    }

    // Forward call to base classes.
    AccessibleDocumentViewBase::disposing ();
}




void AccessibleDrawDocumentView::UpdateAccessibleName (void)
{
    OUString sNewName (CreateAccessibleName());
    sNewName += A2S(": ");

    // Add the number of the current slide.
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
    {
        uno::Reference<beans::XPropertySet> xProperties (xView->getCurrentPage(), UNO_QUERY);
        if (xProperties.is())
            try
            {
                sal_Int16 nPageNumber (0);
                if (xProperties->getPropertyValue(A2S("Number")) >>= nPageNumber)
                {
                    sNewName += OUString::valueOf(sal_Int32(nPageNumber));
                }
            }
            catch (beans::UnknownPropertyException&)
            {
            }
    }

    // Add the number of pages/slides.
    Reference<drawing::XDrawPagesSupplier> xPagesSupplier (mxModel, UNO_QUERY);
    if (xPagesSupplier.is())
    {
        Reference<container::XIndexAccess> xPages (xPagesSupplier->getDrawPages(), UNO_QUERY);
        if (xPages.is())
        {
            sNewName += A2S(" / ");
            sNewName += OUString::valueOf(xPages->getCount());
        }
    }

    SetAccessibleName (sNewName, AutomaticallyCreated);
}




} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
