/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresentationFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:03:56 $
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

#ifndef SD_FRAMEWORK_PRESENTATION_FACTORY_HXX
#define SD_FRAMEWORK_PRESENTATION_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase2.hxx>


#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > PresentationFactoryInterfaceBase;

} // end of anonymous namespace.



namespace sd { namespace framework {

/** This factory creates a marker view whose existence in a configuration
    indicates that a slideshow is running (in another but associated
    application window).
*/
class PresentationFactory
    : private sd::MutexOwner,
      public PresentationFactoryInterfaceBase
{
public:
    static const ::rtl::OUString msPresentationViewURL;

    PresentationFactory (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~PresentationFactory (void);

    virtual void SAL_CALL disposing (void);


    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxViewId)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView)
        throw(css::uno::RuntimeException);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;

    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
