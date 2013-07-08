/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#define __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_

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

#define WINDOWSTATE_PROPERTY_LOCKED         "Locked"
#define WINDOWSTATE_PROPERTY_DOCKED         "Docked"
#define WINDOWSTATE_PROPERTY_VISIBLE        "Visible"
#define WINDOWSTATE_PROPERTY_DOCKINGAREA    "DockingArea"
#define WINDOWSTATE_PROPERTY_DOCKPOS        "DockPos"
#define WINDOWSTATE_PROPERTY_DOCKSIZE       "DockSize"
#define WINDOWSTATE_PROPERTY_POS            "Pos"
#define WINDOWSTATE_PROPERTY_SIZE           "Size"
#define WINDOWSTATE_PROPERTY_UINAME         "UIName"
#define WINDOWSTATE_PROPERTY_INTERNALSTATE  "InternalState"
#define WINDOWSTATE_PROPERTY_STYLE          "Style"
#define WINDOWSTATE_PROPERTY_CONTEXT        "ContextSensitive"
#define WINDOWSTATE_PROPERTY_HIDEFROMENU    "HideFromToolbarMenu"
#define WINDOWSTATE_PROPERTY_NOCLOSE        "NoClose"
#define WINDOWSTATE_PROPERTY_SOFTCLOSE      "SoftClose"
#define WINDOWSTATE_PROPERTY_CONTEXTACTIVE  "ContextActive"

class WindowStateConfiguration :  private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                  public ::cppu::WeakImplHelper2< css::container::XNameAccess, css::lang::XServiceInfo>
{
    public:
        WindowStateConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~WindowStateConfiguration();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
            throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
            throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

        typedef ::boost::unordered_map< OUString,
                                 OUString,
                                 OUStringHash,
                                 ::std::equal_to< OUString > > ModuleToWindowStateFileMap;

        typedef ::boost::unordered_map< OUString,
                                 ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >,
                                 OUStringHash,
                                 ::std::equal_to< OUString > > ModuleToWindowStateConfigHashMap;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>         m_xContext;
        ModuleToWindowStateFileMap                                                          m_aModuleToFileHashMap;
        ModuleToWindowStateConfigHashMap                                                    m_aModuleToWindowStateHashMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >        m_xModuleManager;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_WINDOWSTATECONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
