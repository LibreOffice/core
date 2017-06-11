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

#include <uielement/footermenucontroller.hxx>

#include "services.h"
#include <strings.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>

//  Defines

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::style;
using namespace com::sun::star::container;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   FooterMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_FOOTERMENUCONTROLLER
                                        )

FooterMenuController::FooterMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    HeaderMenuController( xContext,true )
{
}

FooterMenuController::~FooterMenuController()
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
