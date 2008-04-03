/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:07:07 $
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

#ifndef SD_PRESENTER_PRESENTER_HELPER_HXX
#define SD_PRESENTER_PRESENTER_HELPER_HXX

#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <svtools/filter.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper2<
        css::lang::XInitialization,
        css::drawing::XPresenterHelper
    > PresenterHelperInterfaceBase;
}

/** Implementation of the XPresenterHelper interface: functionality that can
    not be implemented in an extension.
*/
    class PresenterHelper
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterHelperInterfaceBase
{
public:
    PresenterHelper (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterHelper (void);

    // XInitialize

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw(css::uno::Exception,css::uno::RuntimeException);


    // XPresenterHelper

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        sal_Bool bCreateSystemChildWindow,
        sal_Bool bInitiallyVisible,
        sal_Bool bEnableChildTransparentMode,
        sal_Bool bEnableParentClip)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::rendering::XCanvas> SAL_CALL createSharedCanvas (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxUpdateCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxUpdateWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxSharedCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
        const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::rendering::XCanvas> SAL_CALL createCanvas (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        sal_Int16 nRequestedCanvasFeatures,
        const ::rtl::OUString& rsOptionalCanvasServiceName)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL toTop (
        const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL loadBitmap (
        const ::rtl::OUString& rsURL,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL captureMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL releaseMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException);

    virtual css::awt::Rectangle SAL_CALL getWindowExtentsRelative (
        const css::uno::Reference<css::awt::XWindow>& rxChildWindow,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::boost::scoped_ptr<GraphicFilter> mpGraphicFilter;
};

} } // end of namespace ::sd::presenter

#endif
