/*************************************************************************
 *
 *  $RCSfile: AccessibleDrawDocumentView.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: af $ $Date: 2002-06-07 14:48:28 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX
#include "AccessibleDrawDocumentView.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBLE_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBLE_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULSTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _SFXFRAME_HXX
#include<sfx2/viewfrm.hxx>
#endif

#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshcol.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "sdwindow.hxx"
#include <vcl/svapp.hxx>


#ifndef _SD_VIEWSHEL_HXX
#include "viewshel.hxx"
#endif
#ifndef _SD_SDVIEW_HXX
#include "sdview.hxx"
#endif
#include <memory>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

class SfxViewFrame;

namespace accessibility {


//=====  internal  ============================================================

AccessibleDrawDocumentView::AccessibleDrawDocumentView (
    SdWindow* pSdWindow,
    SdViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      mpChildrenManager (NULL)
{
}




AccessibleDrawDocumentView::~AccessibleDrawDocumentView (void)
{
    OSL_TRACE ("~AccessibleDrawDocumentView");

    // Unregister from the various event broadcasters.
    if (mpChildrenManager != NULL)
        delete mpChildrenManager;
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
        mpChildrenManager->AddAccessibleShape (
            std::auto_ptr<AccessibleShape>(CreateDrawPageShape()));
        mpChildrenManager->Update ();
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
                if (pShape != NULL)
                    pShape->Init();
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
    long mpChildCount = AccessibleDocumentViewBase::getAccessibleChildCount();

    // Forward request to children manager.
    if (mpChildrenManager != NULL)
        mpChildCount += mpChildrenManager->GetChildCount ();

    return mpChildCount;
}




uno::Reference<XAccessible> SAL_CALL
    AccessibleDrawDocumentView::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Take care of children of the base class.
    sal_Int32 nCount = AccessibleDocumentViewBase::getAccessibleChildCount();
    if (nCount > 0)
        if (nIndex < nCount)
            return AccessibleDocumentViewBase::getAccessibleChild(nIndex);
        else
            nIndex -= nCount;

    // Forward request to children manager.
    if (mpChildrenManager != NULL)
    {
        return mpChildrenManager->GetChild (nIndex);
    }
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("no accessible child with index ") + nIndex,
            static_cast<uno::XWeak*>(this));
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleDrawDocumentView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleDrawDocumentView"));
}




//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDrawDocumentView::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleDocumentViewBase::disposing (rEventObject);
    if (rEventObject.Source == mxModel)
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());
        // maShapeTreeInfo has been modified in base class.
        if (mpChildrenManager != NULL)
            mpChildrenManager->SetInfo (maShapeTreeInfo);
    }
}




//=====  XFrameActionListener  ================================================

void SAL_CALL
    AccessibleDrawDocumentView::frameAction (const frame::FrameActionEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleDocumentViewBase::frameAction (rEventObject);

    if (rEventObject.Action == frame::FrameAction_COMPONENT_REATTACHED)
    {
        if (mpChildrenManager != NULL)
        {
            // Clear the list of children to avoid unnecessary events and
            // update the children manager.
            mpChildrenManager->ClearAccessibleShapeList ();
            mpChildrenManager->SetInfo (maShapeTreeInfo);
            mpChildrenManager->ViewForwarderChanged (
                IAccessibleViewForwarderListener::TRANSFORMATION,
                &maViewForwarder);

            // To properly inform the registered listeners of this we just have
            // to call the children manager and update its shape list.
            uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
            if (xView.is())
                mpChildrenManager->SetShapeList (
                    uno::Reference<drawing::XShapes> (
                        xView->getCurrentPage(), uno::UNO_QUERY));
            mpChildrenManager->AddAccessibleShape (std::auto_ptr<AccessibleShape>(CreateDrawPageShape()));
            mpChildrenManager->Update (false);
            OSL_TRACE ("done handling frame event for AccessibleDrawDocumentView");
        }
    }
}




//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleDrawDocumentView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleDrawDocumentView::propertyChange");
    if (rEventObject.PropertyName == OUString (RTL_CONSTASCII_USTRINGPARAM("CurrentPage")))
    {
        OSL_TRACE ("    current page changed");

        // The current page changed.  Update the children manager accordingly.
        uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
        if (xView.is() && mpChildrenManager!=NULL)
        {
            // Inform the children manager to forget all children and give
            // him the new ones.
            mpChildrenManager->ClearAccessibleShapeList ();
            mpChildrenManager->SetShapeList (uno::Reference<drawing::XShapes> (
                xView->getCurrentPage(), uno::UNO_QUERY));
            mpChildrenManager->AddAccessibleShape (std::auto_ptr<AccessibleShape>(CreateDrawPageShape ()));
            mpChildrenManager->Update (false);
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




/// Create a name for this view.
::rtl::OUString
    AccessibleDrawDocumentView::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString (
        RTL_CONSTASCII_USTRINGPARAM("AccessibleDrawDocumentView"));
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
        OUString sFirstService = xInfo->getSupportedServiceNames()[0];
        if (sFirstService == OUString (
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentDrawView")))
        {
            sDescription = OUString (RTL_CONSTASCII_USTRINGPARAM("Draw Document"));
        }
        else
            sDescription = sFirstService;
    }
    else
        sDescription = OUString (
            RTL_CONSTASCII_USTRINGPARAM("Accessible Draw Document"));
    return sDescription;
}




/** Return selection state of specified child
*/
sal_Bool
    AccessibleDrawDocumentView::implIsSelected( sal_Int32 nAccessibleChildIndex )
    throw (uno::RuntimeException)
{
    const vos::OGuard                           aSolarGuard( Application::GetSolarMutex() );
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
    const vos::OGuard                           aSolarGuard( Application::GetSolarMutex() );
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
                    for(sal_Int32 i = 0, nCount = xShapes->getCount(); ( i < nCount ) && !bFound; ++i )
                        if( xShapes->getByIndex( i ) == xShape )
                            bFound = sal_True;

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
        mpChildrenManager->UpdateSelection();
}




void AccessibleDrawDocumentView::Deactivated (void)
{
    // Because this method may be called directly before the window is
    // deactivated the focused state has to be removed from the currently
    // focused shape by hand.
    // At the (a) next incompatible build the following implementation
    // should be moved to the children manager.

    // If we could be sure to track any change of the FOCUSED state we could
    // use the selection to directly find the currently focused shape.
    // But, alas, we can not. Therefore iterate over all children and remove
    // the FOCUSED state.
    for (sal_Int32 i=0, nCount=getAccessibleChildCount(); i < nCount; ++i)
    {
        AccessibleShape* pAccessibleShape = AccessibleShape::getImplementation(
            getAccessibleChild(i));
        if (pAccessibleShape != NULL)
            pAccessibleShape->ResetState (AccessibleStateType::FOCUSED);
    }
}


} // end of namespace accessibility
