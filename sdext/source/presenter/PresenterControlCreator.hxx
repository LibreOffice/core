/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterControlCreator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:57:14 $
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

#ifndef SDEXT_PRESENTER_CONTROL_CREATOR_HXX
#define SDEXT_PRESENTER_CONTROL_CREATOR_HXX

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** This class creates an AWT container window and a set of child controls.
*/
class PresenterControlCreator
    : private ::boost::noncopyable
{
public:
    /** Create a container window with the given window as its parent.
    */
    explicit PresenterControlCreator (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const sal_Int32 nHeight);
    ~PresenterControlCreator (void);

    sal_Int32 GetToolBarHeight (void) const;

    class Property {public: sal_Char* msName; css::uno::Any maValue; };

    css::uno::Reference<css::awt::XControl> AddButton (
        const ::rtl::OUString& rsName,
        const ::rtl::OUString& rsImageName,
        const Property pProperties[]);

    css::uno::Reference<css::awt::XControl> AddLabel (
        const ::rtl::OUString& rsName,
        const ::rtl::OUString& rsDefaultText,
        const Property pProperties[]);

    css::uno::Reference<css::awt::XControl> AddEdit (
        const ::rtl::OUString& rsName,
        const Property pProperties[]);

    /** Return the container window.
    */
    css::uno::Reference<css::awt::XWindow> GetContainerWindow (void) const;
    css::uno::Reference<css::awt::XControlContainer> GetContainer (void) const;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<
        css::awt::XWindow> mxControlContainerWindow;
    css::uno::Reference<
        css::awt::XControlContainer> mxControlContainer;
    css::uno::Reference<
        css::container::XNameContainer> mxModelNameContainer;
    css::uno::Reference<
        css::lang::XMultiServiceFactory> mxControlFactory;
    sal_Int32 mnControlCount;
    const sal_Int32 mnToolBarHeight;

    bool CreateContainer (void);
    void CreateContainerWindow (
            const css::uno::Reference<
                css::awt::XWindow>& rxParentWindow);

    css::uno::Reference<css::awt::XControl> AddControl (
        const ::rtl::OUString& rsName,
        const ::rtl::OUString& rsModelServiceName,
        const ::rtl::OUString& rsControlServiceName,
        const Property pDefaultProperties[],
        const Property pUserProperties[]);

    void SetModelProperties (
        const css::uno::Reference<css::beans::XPropertySet>& rxSet,
        const Property pProperties[]);

};

} } // end of namespace ::sdext::presenter

#endif
