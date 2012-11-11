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

#ifndef __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_

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

#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class GlobalSettings
{
    public:
        GlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        ~GlobalSettings();

        enum UIElementType
        {
            UIELEMENT_TYPE_TOOLBAR,
            UIELEMENT_TYPE_DOCKWINDOW,
            UIELEMENT_TYPE_STATUSBAR
        };

        enum StateInfo
        {
            STATEINFO_LOCKED,
            STATEINFO_DOCKED
        };

        // settings access
        sal_Bool HasStatesInfo( UIElementType eElementType );
        sal_Bool GetStateInfo( UIElementType eElementType, StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue );

    private:
        GlobalSettings();
        GlobalSettings(const GlobalSettings&);
        GlobalSettings& operator=(const GlobalSettings& );

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_WINDOWSTATECONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
