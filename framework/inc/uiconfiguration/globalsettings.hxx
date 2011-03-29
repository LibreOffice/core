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

#ifndef __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

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
#include <com/sun/star/container/XNameAccess.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class GlobalSettings
{
    public:
        GlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr );
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

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xSrvMgr;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_WINDOWSTATECONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
