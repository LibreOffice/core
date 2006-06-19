/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubarwrapper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:05:11 $
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

#ifndef __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX_
#include <helper/uiconfigelementwrapperbase.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#include <uielement/menubarmanager.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

namespace framework
{

class MenuBarWrapper : public UIConfigElementWrapperBase
{
    public:
        // #110897#
        MenuBarWrapper(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~MenuBarWrapper();

        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

        MenuBarManager* GetMenuBarManager() const { return static_cast< MenuBarManager* >( m_xMenuBarManager.get() ); }

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException);

        // XUIElementSettings
        virtual void SAL_CALL updateSettings(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getSettings( sal_Bool bWriteable ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& UISettings ) throw (::com::sun::star::uno::RuntimeException);

private:
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >                      m_xMenuBarManager;
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_
