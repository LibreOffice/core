/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uielementfactorymanager.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:08:13 $
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

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUIELEMENTFACTORY_HPP_
#include <com/sun/star/ui/XUIElementFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUIELEMENTFACTORYREGISTRATION_HPP_
#include <com/sun/star/ui/XUIElementFactoryRegistration.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include "com/sun/star/frame/XModuleManager.hpp"
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
