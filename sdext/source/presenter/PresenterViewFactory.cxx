/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterViewFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:06:49 $
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

#include "PresenterViewFactory.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterHelper.hxx"
#include "PresenterHelpView.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterSlidePreview.hxx"
#include "PresenterSlideSorter.hxx"
#include "PresenterToolBar.hxx"
#include "PresenterClock.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XSlideSorterBase.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <cppuhelper/compbase1.hxx>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

const ::rtl::OUString PresenterViewFactory::msCurrentSlidePreviewViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/CurrentSlidePreview"));
const ::rtl::OUString PresenterViewFactory::msNextSlidePreviewViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/NextSlidePreview"));
const ::rtl::OUString PresenterViewFactory::msNotesViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/Notes"));
const ::rtl::OUString PresenterViewFactory::msToolBarViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/ToolBar"));
const ::rtl::OUString PresenterViewFactory::msSlideSorterURL(
    OUString::createFromAscii("private:resource/view/Presenter/SlideSorter"));
const ::rtl::OUString PresenterViewFactory::msClockViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/Clock"));
const ::rtl::OUString PresenterViewFactory::msHelpViewURL(
    OUString::createFromAscii("private:resource/view/Presenter/Help"));



//===== SimpleView ============================================================

namespace {

typedef ::cppu::WeakComponentImplHelper1 <XView> SimpleViewInterfaceBase;

class SimpleView
    : private ::cppu::BaseMutex,
      public SimpleViewInterfaceBase
{
public:
    SimpleView (const Reference<XResourceId>& rxViewId)
        : SimpleViewInterfaceBase(m_aMutex),mxResourceId(rxViewId) {};
    virtual ~SimpleView (void) {};

    // XView

    virtual Reference<XResourceId> SAL_CALL getResourceId (void) throw (RuntimeException)
    { return mxResourceId; };

private:
    Reference<XResourceId> mxResourceId;
};



/** By default the PresenterSlidePreview shows the preview of the current
    slide.  This adapter class makes it display the preview of the next
    slide.
*/
class NextSlidePreview : public PresenterSlidePreview
{
public:
    NextSlidePreview (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxAnchorPane,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
        : PresenterSlidePreview(rxContext, rxViewId, rxAnchorPane, rpPresenterController)
    {
    }
    virtual ~NextSlidePreview (void) {}
    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException)
    {
        Reference<presentation::XSlideShowController> xSlideShowController (
            mpPresenterController->GetSlideShowController());
        Reference<drawing::XDrawPage> xSlide;
        if (xSlideShowController.is())
        {
            const sal_Int32 nCount (xSlideShowController->getSlideCount());
            sal_Int32 nNextSlideIndex (-1);
            if (xSlideShowController->getCurrentSlide() == rxSlide)
            {
                nNextSlideIndex = xSlideShowController->getNextSlideIndex();
            }
            else
            {
                for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
                {
                    if (rxSlide == Reference<drawing::XDrawPage>(
                        xSlideShowController->getSlideByIndex(nIndex), UNO_QUERY))
                    {
                        nNextSlideIndex = nIndex + 1;
                    }
                }
            }
            if (nNextSlideIndex >= 0)
            {
                if (nNextSlideIndex < nCount)
                {
                    xSlide = Reference<drawing::XDrawPage>(
                        xSlideShowController->getSlideByIndex(nNextSlideIndex),
                         UNO_QUERY);
                }
            }
        }
        PresenterSlidePreview::setCurrentPage(xSlide);
    }
};

} // end of anonymous namespace




//===== PresenterViewFactory ==============================================

PresenterViewFactory::PresenterViewFactory (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterViewFactoryInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxConfigurationController(),
      mxControllerWeak(rxController),
      mpPresenterController(rpPresenterController)
{
}




Reference<drawing::framework::XResourceFactory> PresenterViewFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterViewFactory> pFactory (
        new PresenterViewFactory(rxContext,rxController,rpPresenterController));
    pFactory->Register(rxController);
    return Reference<drawing::framework::XResourceFactory>(
        static_cast<XWeak*>(pFactory.get()), UNO_QUERY);
}




void PresenterViewFactory::Register (const Reference<frame::XController>& rxController)
{
    try
    {
        // Get the configuration controller.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        mxConfigurationController = xCM->getConfigurationController();
        if ( ! mxConfigurationController.is())
        {
            throw RuntimeException();
        }
        else
        {
            mxConfigurationController->addResourceFactory(msCurrentSlidePreviewViewURL, this);
            mxConfigurationController->addResourceFactory(msNextSlidePreviewViewURL, this);
            mxConfigurationController->addResourceFactory(msNotesViewURL, this);
            mxConfigurationController->addResourceFactory(msToolBarViewURL, this);
            mxConfigurationController->addResourceFactory(msSlideSorterURL, this);
            mxConfigurationController->addResourceFactory(msClockViewURL, this);
            mxConfigurationController->addResourceFactory(msHelpViewURL, this);
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (mxConfigurationController.is())
            mxConfigurationController->removeResourceFactoryForReference(this);
        mxConfigurationController = NULL;

        throw;
    }
}




PresenterViewFactory::~PresenterViewFactory (void)
{
}




void SAL_CALL PresenterViewFactory::disposing (void)
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeResourceFactoryForReference(this);
    mxConfigurationController = NULL;
}




//----- XViewFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresenterViewFactory::createResource (
    const Reference<XResourceId>& rxViewId)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    Reference<XResource> xView;

    if (rxViewId.is())
    {
        try
        {
            const OUString sResourceURL (rxViewId->getResourceURL());
            if (sResourceURL.equals(msCurrentSlidePreviewViewURL))
            {
                xView = Reference<XResource>(CreateSlideShowView(rxViewId), UNO_QUERY);
            }
            else if (sResourceURL.equals(msNotesViewURL))
            {
                xView = Reference<XResource>(CreateNotesView(rxViewId), UNO_QUERY);
            }
            else if (sResourceURL.equals(msNextSlidePreviewViewURL))
            {
                xView = Reference<XResource>(CreateSlidePreviewView(rxViewId), UNO_QUERY);
            }
            else if (sResourceURL.equals(msToolBarViewURL))
            {
                xView = new PresenterToolBar(
                    mxComponentContext,
                    rxViewId,
                    Reference<frame::XController>(mxControllerWeak),
                    mpPresenterController);
            }
            else if (sResourceURL.equals(msSlideSorterURL))
            {
                xView = Reference<XResource>(CreateSlideSorterView(rxViewId), UNO_QUERY);
            }
            else if (sResourceURL.equals(msClockViewURL))
            {
                xView = Reference<XResource>(
                    static_cast<XWeak*>(PresenterClock::Create(
                        mxComponentContext,
                        rxViewId,
                        Reference<frame::XController>(mxControllerWeak),
                        mpPresenterController).get()),
                    UNO_QUERY);
            }
            else if (sResourceURL.equals(msHelpViewURL))
            {
                xView = Reference<XResource>(CreateHelpView(rxViewId), UNO_QUERY);
            }
        }
        catch (RuntimeException&)
        {
            xView = NULL;
        }
    }

    return xView;
}




void SAL_CALL PresenterViewFactory::releaseResource (const Reference<XResource>& rxView)
    throw (RuntimeException)
{
    Reference<lang::XComponent> xComponent (rxView, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}




//-----------------------------------------------------------------------------

Reference<XView> PresenterViewFactory::CreateSlideShowView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        Reference<XPane> xAnchorPane (
            mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY_THROW);
        Reference<awt::XWindow> xWindow (xAnchorPane->getWindow());
        const double nSlideAspectRatio (GetSlideAspectRatio());
        Reference<presentation::XSlideShowController> xSlideShowController(
            PresenterHelper::GetSlideShowController(
                Reference<frame::XController>(mxControllerWeak)));
        Sequence<Any> aArguments(5);
        aArguments[0] <<= rxViewId;
        aArguments[1] <<= Reference<frame::XController>(mxControllerWeak);
        aArguments[2] <<= xSlideShowController;
        aArguments[3] <<= nSlideAspectRatio;
        aArguments[4] <<= mpPresenterController->GetViewBackgroundColor(rxViewId->getResourceURL());

        xView = Reference<XView>(
            xFactory->createInstanceWithArgumentsAndContext(
                OUString::createFromAscii("com.sun.star.drawing.presenter.SlideShowView"),
                aArguments,
                mxComponentContext),
            UNO_QUERY_THROW);

        if( xSlideShowController.is() )
        {
            Reference<presentation::XSlideShow> xSlideShow (
                xSlideShowController->getSlideShow(), UNO_QUERY_THROW);
            xSlideShow->addView(Reference<presentation::XSlideShowView>(xView, UNO_QUERY_THROW));
        }
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}




Reference<XView> PresenterViewFactory::CreateSlidePreviewView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        Reference<XPane> xAnchorPane (
            mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY_THROW);
        xView = Reference<XView>(
            static_cast<XWeak*>(new NextSlidePreview(
                mxComponentContext,
                rxViewId,
                xAnchorPane,
                mpPresenterController)),
            UNO_QUERY_THROW);
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}




Reference<XView> PresenterViewFactory::CreateNotesView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        Reference<XPane> xAnchorPane (
            mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY_THROW);
        xView = Reference<XView>(static_cast<XWeak*>(
            new PresenterNotesView(
                mxComponentContext,
                rxViewId,
                Reference<frame::XController>(mxControllerWeak),
                mpPresenterController)),
            UNO_QUERY_THROW);
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}




Reference<XView> PresenterViewFactory::CreateSlideSorterView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        rtl::Reference<PresenterSlideSorter> pView (
            new PresenterSlideSorter(
                mxComponentContext,
                rxViewId,
                Reference<frame::XController>(mxControllerWeak),
                mpPresenterController));
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindPaneId(rxViewId->getAnchor()));
        if (pDescriptor.get() != NULL)
            pDescriptor->maActivator = ::boost::bind(
                &PresenterSlideSorter::SetActiveState, pView.get(), _1);
        xView = pView.get();
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}




Reference<XView> PresenterViewFactory::CreateHelpView(
    const Reference<XResourceId>& rxViewId) const
{
    return Reference<XView>(new PresenterHelpView(
        mxComponentContext,
        rxViewId,
        Reference<frame::XController>(mxControllerWeak),
        mpPresenterController));
}





double PresenterViewFactory::GetSlideAspectRatio (void) const
{
    double nSlideAspectRatio (28.0/21.0);

    try
    {
        Reference<frame::XController> xController (mxControllerWeak);
        if (xController.is())
        {
            Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
                xController->getModel(), UNO_QUERY_THROW);
            Reference<drawing::XDrawPages> xSlides (xSlideSupplier->getDrawPages());
            if (xSlides.is() && xSlides->getCount()>0)
            {
                Reference<beans::XPropertySet> xProperties(xSlides->getByIndex(0),UNO_QUERY_THROW);
                sal_Int32 nWidth (28000);
                sal_Int32 nHeight (21000);
                if ((xProperties->getPropertyValue(OUString::createFromAscii("Width")) >>= nWidth)
                    && (xProperties->getPropertyValue(OUString::createFromAscii("Height")) >>= nHeight)
                    && nHeight > 0)
                {
                    nSlideAspectRatio = double(nWidth) / double(nHeight);
                }
            }
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }

    return nSlideAspectRatio;
}




void PresenterViewFactory::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterViewFactory object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} }
