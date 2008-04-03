/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:02:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//#include "precompiled_sdext.hxx"

#include "PresenterPaneContainer.hxx"
#include "PresenterPaneBase.hxx"
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterPaneContainer::PresenterPaneContainer (
    const Reference<XComponentContext>& rxContext)
    : PresenterPaneContainerInterfaceBase(m_aMutex),
      maPanes(),
      mxPresenterHelper()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }
}




PresenterPaneContainer::~PresenterPaneContainer (void)
{
}




void PresenterPaneContainer::PreparePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsTitle,
    const ViewInitializationFunction& rViewInitialization,
    const double nLeft,
    const double nTop,
    const double nRight,
    const double nBottom)
{
    if ( ! rxPaneId.is())
        return;

    SharedPaneDescriptor pPane (FindPaneURL(rxPaneId->getResourceURL()));
    if (pPane.get() == NULL)
    {
        // No entry found for the given pane id.  Create a new one.
        SharedPaneDescriptor pDescriptor (new PaneDescriptor());
        pDescriptor->mxPaneId = rxPaneId;
        pDescriptor->mxPane = NULL;
        pDescriptor->msTitle = rsTitle;
        pDescriptor->maViewInitialization = rViewInitialization;
        pDescriptor->mnLeft = nLeft;
        pDescriptor->mnTop = nTop;
        pDescriptor->mnRight = nRight;
        pDescriptor->mnBottom = nBottom;
        pDescriptor->mbIsActive = true;
        pDescriptor->maSpriteProvider = PaneDescriptor::SpriteProvider();
        pDescriptor->mbIsSprite = false;
        maPanes.push_back(pDescriptor);
    }
}




void SAL_CALL PresenterPaneContainer::disposing (void)
{
    while ( ! maPanes.empty())
        RemovePane(maPanes.back()->mxPaneId);
}




PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StorePane (const rtl::Reference<PresenterPaneBase>& rxPane)
{
    SharedPaneDescriptor pDescriptor;

    if (rxPane.is())
    {
        OUString sPaneURL;
        Reference<XResourceId> xPaneId (rxPane->getResourceId());
        if (xPaneId.is())
            sPaneURL = xPaneId->getResourceURL();

        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() == NULL)
            PreparePane(xPaneId, OUString(), ViewInitializationFunction(), 0,0,0,0);
        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() != NULL)
        {
            Reference<awt::XWindow> xWindow (rxPane->getWindow());
            pDescriptor->mxContentWindow = xWindow;
            pDescriptor->mxPaneId = xPaneId;
            pDescriptor->mxPane = rxPane;
            pDescriptor->mxPane->SetTitle(pDescriptor->msTitle);

            if (xWindow.is())
                xWindow->addEventListener(this);
        }
    }

    return pDescriptor;
}




PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StoreBorderWindow(
        const Reference<XResourceId>& rxPaneId,
        const Reference<awt::XWindow>& rxBorderWindow)
{
    // The content window may not be present.  Use the resource URL of the
    // pane id as key.
    OUString sPaneURL;
    if (rxPaneId.is())
        sPaneURL = rxPaneId->getResourceURL();

    SharedPaneDescriptor pDescriptor (FindPaneURL(sPaneURL));
    if (pDescriptor.get() != NULL)
    {
        pDescriptor->mxBorderWindow = rxBorderWindow;
        return pDescriptor;
    }
    else
        return SharedPaneDescriptor();
}




PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StoreView (
        const Reference<XView>& rxView,
        const util::Color aViewBackgroundColor,
        const Reference<rendering::XBitmap>& rxViewBackgroundBitmap)
{
    SharedPaneDescriptor pDescriptor;

    if (rxView.is())
    {
        OUString sPaneURL;
        Reference<XResourceId> xViewId (rxView->getResourceId());
        if (xViewId.is())
        {
            Reference<XResourceId> xPaneId (xViewId->getAnchor());
            if (xPaneId.is())
                sPaneURL = xPaneId->getResourceURL();
        }

        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() != NULL)
        {
            pDescriptor->mxView = rxView;
            pDescriptor->maViewBackgroundColor = aViewBackgroundColor;
            pDescriptor->mxViewBackgroundBitmap = rxViewBackgroundBitmap;
            pDescriptor->mxPane->SetBackground(aViewBackgroundColor, rxViewBackgroundBitmap);
            try
            {
                if ( ! pDescriptor->maViewInitialization.empty())
                    pDescriptor->maViewInitialization(rxView);

                // Activate or deactivate the pane/view.
                if ( ! pDescriptor->maActivator.empty())
                    pDescriptor->maActivator(pDescriptor->mbIsActive);
            }
            catch (RuntimeException&)
            {
                OSL_ASSERT(false);
            }
        }
    }

    return pDescriptor;
}




void PresenterPaneContainer::RemovePane (
    const Reference<drawing::framework::XResourceId>& rxPaneId)
{
    SharedPaneDescriptor pDescriptor (FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
    {
        if (pDescriptor->mxContentWindow.is())
            pDescriptor->mxContentWindow->removeEventListener(this);
        pDescriptor->mxContentWindow = NULL;
        pDescriptor->mxBorderWindow = NULL;
        pDescriptor->mxPane = NULL;
        pDescriptor->mxView = NULL;
        pDescriptor->mbIsActive = false;
    }
}





PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindBorderWindow (
    const Reference<awt::XWindow>& rxBorderWindow)
{
    PaneList::const_iterator iPane;
    PaneList::iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxBorderWindow == rxBorderWindow)
            return *iPane;
    }
    return SharedPaneDescriptor();
}




PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindContentWindow (
    const Reference<awt::XWindow>& rxContentWindow)
{
    PaneList::const_iterator iPane;
    PaneList::iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxContentWindow == rxContentWindow)
            return *iPane;
    }
    return SharedPaneDescriptor();
}




PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindPaneURL (
    const OUString& rsPaneURL)
{
    PaneList::const_iterator iPane;
    PaneList::const_iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxPaneId->getResourceURL() == rsPaneURL)
            return *iPane;
    }
    return SharedPaneDescriptor();
}




PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindPaneId (
    const Reference<XResourceId>& rxPaneId)
{
    PaneList::iterator iEnd (maPanes.end());

    if ( ! rxPaneId.is())
        return SharedPaneDescriptor();

    PaneList::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if (rxPaneId->compareTo((*iPane)->mxPaneId) == 0)
            return *iPane;
    }
    return SharedPaneDescriptor();
}




void PresenterPaneContainer::ToTop (const SharedPaneDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        // Find iterator for pDescriptor.
        PaneList::iterator iPane;
        PaneList::iterator iEnd (maPanes.end());
        for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
            if (iPane->get() == rpDescriptor.get())
                break;
        OSL_ASSERT(iPane!=iEnd);
        if (iPane == iEnd)
            return;

        if (mxPresenterHelper.is())
            mxPresenterHelper->toTop(rpDescriptor->mxBorderWindow);

        maPanes.erase(iPane);
        maPanes.push_back(rpDescriptor);
    }
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterPaneContainer::disposing (
    const com::sun::star::lang::EventObject& rEvent)
    throw (com::sun::star::uno::RuntimeException)
{
    SharedPaneDescriptor pDescriptor (
        FindContentWindow(Reference<awt::XWindow>(rEvent.Source, UNO_QUERY)));
    if (pDescriptor.get() != NULL)
    {
        RemovePane(pDescriptor->mxPaneId);
    }
}




//===== PresenterPaneContainer::PaneDescriptor ================================

void PresenterPaneContainer::PaneDescriptor::SetActivationState (const bool bIsActive)
{
    mbIsActive = bIsActive;
    if ( ! maActivator.empty())
        maActivator(mbIsActive);
}

} } // end of namespace ::sdext::presenter
