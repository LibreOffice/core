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

#include "uifactory/popupmenucontrollerfactory.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include "uifactory/factoryconfiguration.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   PopupMenuControllerFactory                      ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLERFACTORY          ,
                                            IMPLEMENTATIONNAME_POPUPMENUCONTROLLERFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   PopupMenuControllerFactory, {} )

PopupMenuControllerFactory::PopupMenuControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ToolbarControllerFactory(xServiceManager,true)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "PopupMenuControllerFactory::PopupMenuControllerFactory" );
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory( m_xServiceManager,rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Controller/Registered/PopupMenu" )) );
    m_pConfigAccess->acquire();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
