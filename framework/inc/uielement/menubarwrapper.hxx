/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubarwrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:05:05 $
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
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef __COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <stl/hash_map>

namespace framework
{

class MenuBarWrapper : public UIConfigElementWrapperBase,
                       public ::com::sun::star::container::XNameAccess

{
    public:
        // #110897#
        MenuBarWrapper(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~MenuBarWrapper();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider
        //---------------------------------------------------------------------------------------------------------
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

        MenuBarManager* GetMenuBarManager() const { return static_cast< MenuBarManager* >( m_xMenuBarManager.get() ); }
        void            InvalidatePopupControllerCache() { m_bRefreshPopupControllerCache = sal_True; }

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

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    private:
        void fillPopupControllerCache();

        sal_Bool                                                                                m_bRefreshPopupControllerCache : 1;
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >                      m_xMenuBarManager;
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
        PopupControllerCache                                                                    m_aPopupControllerCache;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_
