/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uielementfactorymanager.hxx,v $
 * $Revision: 1.7 $
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

#ifndef __FRAMEWORK_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX_
#define __FRAMEWORK_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/ui/XUIElementFactoryRegistration.hpp>
#include "com/sun/star/frame/XModuleManager.hpp"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class ConfigurationAccess_UIElementFactoryManager;
class UIElementFactoryManager :  public com::sun::star::lang::XTypeProvider                         ,
                                 public com::sun::star::lang::XServiceInfo                          ,
                                 public ::com::sun::star::ui::XUIElementFactory               ,
                                 public ::com::sun::star::ui::XUIElementFactoryRegistration   ,
                                 private ThreadHelpBase                                             ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                 public ::cppu::OWeakObject
{
    public:
        UIElementFactoryManager( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~UIElementFactoryManager();

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XUIElementFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL createUIElement( const ::rtl::OUString& ResourceURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Args ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XUIElementFactoryRegistration
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getRegisteredFactories(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElementFactory > SAL_CALL getFactory( const ::rtl::OUString& ResourceURL, const ::rtl::OUString& ModuleIdentifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL registerFactory( const ::rtl::OUString& aType, const ::rtl::OUString& aName, const ::rtl::OUString& aModuleIdentifier, const ::rtl::OUString& aFactoryImplementationName ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deregisterFactory( const ::rtl::OUString& aType, const ::rtl::OUString& aName, const ::rtl::OUString& aModuleIdentifier ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    private:
        void RetrieveTypeNameFromResourceURL( const ::rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName );

        sal_Bool                                                                            m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > m_xModuleManager;
        ConfigurationAccess_UIElementFactoryManager*                                        m_pConfigAccess;
};

} // namespace framework

#endif // __FRAMEWORK_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX_
