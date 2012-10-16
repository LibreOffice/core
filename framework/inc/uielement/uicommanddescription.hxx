/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_UIELEMENT_UICOMMANDDESCRPTION_HXX_
#define __FRAMEWORK_UIELEMENT_UICOMMANDDESCRPTION_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
class UICommandDescription :  private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                              public ::cppu::WeakImplHelper2< com::sun::star::lang::XServiceInfo        ,
                                                              com::sun::star::container::XNameAccess >
{
    public:
        UICommandDescription( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~UICommandDescription();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
private:
        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
            throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

public:
        typedef ::boost::unordered_map< ::rtl::OUString,
                                 ::rtl::OUString,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > ModuleToCommandFileMap;

        typedef ::boost::unordered_map< ::rtl::OUString,
                                 ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > UICommandsHashMap;

    protected:
        UICommandDescription( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager,bool  );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > impl_createConfigAccess(const ::rtl::OUString& _sName);
        void impl_fillElements(const sal_Char* _pName);
        sal_Bool                                                                            m_bConfigRead;
        rtl::OUString                                                                       m_aPrivateResourceURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
        ModuleToCommandFileMap                                                              m_aModuleToCommandFileMap;
        UICommandsHashMap                                                                   m_aUICommandsHashMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xGenericUICommands;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >        m_xModuleManager;
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_UICOMMANDDESCRPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
