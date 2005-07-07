/*************************************************************************
 *
 *  $RCSfile: SlsListener.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:35:39 $
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
#include "SlsListener.hxx"

#include "SlideSorterViewShell.hxx"
#include "ViewShellHint.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "model/SlideSorterModel.hxx"
#include "view/SlideSorterView.hxx"
#include "drawdoc.hxx"

#include "glob.hrc"

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_VIEW_SHELL_MANAGER_HXX
#include "ViewShellManager.hxx"
#endif
#include "FrameView.hxx"

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#include <sfx2/viewfrm.hxx>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace sd { namespace slidesorter { namespace controller {


Listener::Listener (SlideSorterController& rController)
    : ListenerInterfaceBase(maMutex),
      mrController (rController),
      mbListeningToDocument (false),
      mbListeningToUNODocument (false),
      mbListeningToController (false),
      mbListeningToFrame (false)
{
    StartListening (*mrController.GetModel().GetDocument());
    mbListeningToDocument = true;

    // Connect to the UNO document.
    Reference<document::XEventBroadcaster> xBroadcaster (
        mrController.GetModel().GetDocument()->getUnoModel(), uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        xBroadcaster->addEventListener (this);
        mbListeningToUNODocument = true;
    }

    // Listen for disposing events from the document.
    Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (
            Reference<lang::XEventListener>(
                static_cast<XWeak*>(this), UNO_QUERY));

    // Connect to the frame to listen for controllers being exchanged.
    if ( ! mrController.GetViewShell().IsMainViewShell())
    {
        // Listen to changes of certain properties.
        Reference<frame::XFrame> xFrame (
            mrController.GetViewShell().GetViewShellBase().GetFrame()
            ->GetTopFrame()->GetFrameInterface(),
            uno::UNO_QUERY);
        mxFrameWeak = xFrame;
        if (xFrame.is())
        {
            xFrame->addFrameActionListener (
                Reference<frame::XFrameActionListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
            mbListeningToFrame = true;
        }

        // Connect to the current controller.
        ConnectToController ();
    }
}




Listener::~Listener (void)
{
    ReleaseListeners();
}




void Listener::ReleaseListeners (void)
{
    if (mbListeningToDocument)
    {
        EndListening (*mrController.GetModel().GetDocument());
        mbListeningToDocument = false;
    }

    if (mbListeningToUNODocument)
    {
        Reference<document::XEventBroadcaster> xBroadcaster (
            mrController.GetModel().GetDocument()->getUnoModel(), UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener (this);

        // Remove the dispose listener.
        Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener (
                Reference<lang::XEventListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));

        mbListeningToUNODocument = false;
    }

    if (mbListeningToFrame)
    {
        // Listen to changes of certain properties.
        Reference<frame::XFrame> xFrame (mxFrameWeak);
        if (xFrame.is())
        {
            xFrame->removeFrameActionListener (
                Reference<frame::XFrameActionListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
            mbListeningToFrame = false;
        }
    }

    DisconnectFromController ();
}




void Listener::ConnectToController (void)
{
    ViewShell& rShell (mrController.GetViewShell());

    // Register at the controller of the main view shell (if we are that not
    // ourself).
    if ( ! rShell.IsMainViewShell())
    {
        // Listen to changes of certain properties.
        Reference<frame::XController> xController (rShell.GetViewShellBase().GetController());
        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        if (xSet.is())
        {
            try
            {
                xSet->addPropertyChangeListener(String::CreateFromAscii("CurrentPage"), this);
            }
            catch (beans::UnknownPropertyException aEvent)
            {
                OSL_TRACE ("caught exception in SlideSorterController::SetupListeners: %s",
                    ::rtl::OUStringToOString(aEvent.Message, RTL_TEXTENCODING_UTF8).getStr());
            }
            try
            {
                xSet->addPropertyChangeListener(String::CreateFromAscii("IsMasterPageMode"), this);
            }
            catch (beans::UnknownPropertyException aEvent)
            {
                OSL_TRACE ("caught exception in SlideSorterController::SetupListeners: %s",
                    ::rtl::OUStringToOString(aEvent.Message, RTL_TEXTENCODING_UTF8).getStr());
            }
        }

        // Listen for disposing events.
        Reference<XComponent> xComponent (xController, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->addEventListener (
                Reference<lang::XEventListener>(static_cast<XWeak*>(this), UNO_QUERY));

            mxControllerWeak = xController;
            mbListeningToController = true;
        }


        // Listen for hints at the view shell in the center pane as well.
        StartListening (*rShell.GetViewShellBase().GetMainViewShell());
    }
}




void Listener::DisconnectFromController (void)
{
    if (mbListeningToController)
    {
        Reference<frame::XController> xController = mxControllerWeak;
        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        try
        {
            // Remove the property listener.
            if (xSet.is())
            {
                xSet->removePropertyChangeListener (
                    String::CreateFromAscii("CurrentPage"),
                    this);
                xSet->removePropertyChangeListener (
                    String::CreateFromAscii("IsMasterPageMode"),
                    this);
            }

            // Remove the dispose listener.
            Reference<XComponent> xComponent (xController, UNO_QUERY);
            if (xComponent.is())
                xComponent->removeEventListener (
                    Reference<lang::XEventListener>(
                        static_cast<XWeak*>(this), UNO_QUERY));
        }
        catch (beans::UnknownPropertyException aEvent)
        {
            OSL_TRACE ("caught exception in destructor of SlideSorterController: %s",
                ::rtl::OUStringToOString(aEvent.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }

        // Stop listening for hints at the view shell in the center pane.
        EndListening (
            *mrController.GetViewShell().GetViewShellBase().GetMainViewShell());

        mbListeningToController = false;
        mxControllerWeak = Reference<frame::XController>();
    }
}




void Listener::Notify (
    SfxBroadcaster& rBroadcaster,
    const SfxHint& rHint)
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint& rSdrHint (*PTR_CAST(SdrHint,&rHint));
        switch (rSdrHint.GetKind())
        {
            case HINT_PAGEORDERCHG:
                mrController.HandleModelChange();
                break;
        }
    }
    else if (rHint.ISA(ViewShellHint))
    {
        ViewShellHint& rViewShellHint (*PTR_CAST(ViewShellHint,&rHint));
        switch (rViewShellHint.GetHintId())
        {
            case ViewShellHint::HINT_PAGE_RESIZE_START:
                // Initiate a model change but do nothing (well, not much)
                // until we are told that all slides have been resized.
                mrController.LockModelChange();
                mrController.HandleModelChange();
                break;

            case ViewShellHint::HINT_PAGE_RESIZE_END:
                // All slides have been resized.  The model has to be updated.
                mrController.UnlockModelChange();
                break;

            case ViewShellHint::HINT_CHANGE_EDIT_MODE_START:
                mrController.PrepareEditModeChange();
                break;

            case ViewShellHint::HINT_CHANGE_EDIT_MODE_END:
                mrController.FinishEditModeChange();
                break;

            case ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START:
                mrController.LockModelChange();
                break;

            case ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END:
                mrController.UnlockModelChange();
                break;
        }
    }
}



//=====  lang::XEventListener  ================================================

void SAL_CALL Listener::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if ((mbListeningToDocument || mbListeningToUNODocument)
        && mrController.GetModel().GetDocument()!=NULL
        && rEventObject.Source
           == mrController.GetModel().GetDocument()->getUnoModel())
    {
        mbListeningToDocument = false;
        mbListeningToUNODocument = false;
    }
    else if (mbListeningToController)
    {
        Reference<frame::XController> xController (mxControllerWeak);
        if (rEventObject.Source == xController)
        {
            mbListeningToController = false;
        }
    }
}




//=====  document::XEventListener  ============================================

void SAL_CALL Listener::notifyEvent (
    const document::EventObject& rEventObject)
    throw (RuntimeException)
{
}




//=====  beans::XPropertySetListener  =========================================

void SAL_CALL Listener::propertyChange (
    const PropertyChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    static const ::rtl::OUString sCurrentPagePropertyName (
        RTL_CONSTASCII_USTRINGPARAM("CurrentPage"));
    static const ::rtl::OUString sEditModePropertyName (
        RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode"));

    if (rEvent.PropertyName.equals (sCurrentPagePropertyName))
    {
        Any aCurrentPage = rEvent.NewValue;
        Reference<beans::XPropertySet> xPageSet (aCurrentPage, UNO_QUERY);
        if (xPageSet.is())
        {
            try
            {
                Any aPageNumber = xPageSet->getPropertyValue (
                    String(RTL_CONSTASCII_USTRINGPARAM("Number")));
                sal_Int32 nCurrentPage;
                aPageNumber >>= nCurrentPage;
                mrController.GetPageSelector().UpdateAllPages ();
                // The selection is already set but we call SelectPage()
                // nevertheless in order to make the new current page the
                // last recently selected page of the PageSelector.  This is
                // used when making the selection visible.
                mrController.GetPageSelector().SelectPage(nCurrentPage-1);
            }
            catch (beans::UnknownPropertyException aEvent)
            {
                OSL_TRACE ("caught exception while accessing the page number of a slide: %s",
                    ::rtl::OUStringToOString(aEvent.Message,
                        RTL_TEXTENCODING_UTF8).getStr());
            }
        }
    }
    else if (rEvent.PropertyName.equals (sEditModePropertyName))
    {
        sal_Bool bIsMasterPageMode;
        rEvent.NewValue >>= bIsMasterPageMode;
        mrController.ChangeEditMode (
            bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
    }
}




//===== frame::XFrameActionListener  ==========================================

void SAL_CALL Listener::frameAction (const frame::FrameActionEvent& rEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    switch (rEvent.Action)
    {
        case frame::FrameAction_COMPONENT_DETACHING:
            DisconnectFromController();
            break;

        case frame::FrameAction_COMPONENT_REATTACHED:
        {
            ConnectToController();
            mrController.GetPageSelector().UpdateAllPages();

            // When there is a new controller then the edit mode may have
            // changed at the same time.
            Reference<frame::XController> xController (mxControllerWeak);
            Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
            bool bIsMasterPageMode = false;
            if (xSet != NULL)
            {
                try
                {
                    Any aValue (xSet->getPropertyValue(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode"))));
                    aValue >>= bIsMasterPageMode;
                }
                catch (beans::UnknownPropertyException e)
                {
                    // When the property is not supported then the master
                    // page mode is not supported, too.
                    bIsMasterPageMode = false;
                }
            }
            mrController.ChangeEditMode (
                bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
        }
        break;

        default:
            break;
    }
}




//===== accessibility::XAccessibleEventListener  ==============================

void SAL_CALL Listener::notifyEvent (
    const AccessibleEventObject& rEvent)
    throw (RuntimeException)
{
}




void SAL_CALL Listener::disposing (void)
{
    SlideSorterViewShell& rShell (mrController.GetViewShell());
    ReleaseListeners();
}




void Listener::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "SlideSorterController object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}






} } } // end of namespace ::sd::slidesorter::controller
