/*************************************************************************
 *
 *  $RCSfile: moduleuicfgsupplier.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:35:53 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_
#define __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_


/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <hash_map>

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

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


namespace framework
{
    class ModuleUIConfigurationManagerSupplier : public com::sun::star::lang::XTypeProvider ,
                                                 public com::sun::star::lang::XServiceInfo  ,
                                                 public com::sun::star::lang::XComponent    ,
                                                 public drafts::com::sun::star::ui::XModuleUIConfigurationManagerSupplier      ,
                                                 private ThreadHelpBase                     ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                                 public ::cppu::OWeakObject
    {
        public:
            //  XInterface, XTypeProvider, XServiceInfo
            DECLARE_XINTERFACE
            DECLARE_XTYPEPROVIDER
            DECLARE_XSERVICEINFO

            ModuleUIConfigurationManagerSupplier( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
            virtual ~ModuleUIConfigurationManagerSupplier();

            // XComponent
            virtual void SAL_CALL dispose()
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
                throw (::com::sun::star::uno::RuntimeException);

            // XModuleUIConfigurationManagerSupplier
            virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager( const ::rtl::OUString& ModuleIdentifier )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        private:
            typedef ::std::hash_map< rtl::OUString, com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIConfigurationManager >, OUStringHashCode, ::std::equal_to< rtl::OUString > > ModuleToModuleCfgMgr;

            void impl_initStorages();

            // private methods
            ModuleToModuleCfgMgr                                                                m_aModuleToModuleUICfgMgrMap;
            bool                                                                                m_bDisposed;
            bool                                                                                m_bInit;
            rtl::OUString                                                                       m_aDefaultConfigURL;
            rtl::OUString                                                                       m_aUserConfigURL;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xDefaultCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xUserCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XTransactedObject >          m_xUserRootCommit;
            com::sun::star::uno::Reference< ::drafts::com::sun::star::frame::XModuleManager >   m_xModuleMgr;
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >        m_xServiceManager;
            ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
   };
}

#endif // __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_
