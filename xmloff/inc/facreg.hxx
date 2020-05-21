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

#ifndef INCLUDED_XMLOFF_INC_FACREG_HXX
#define INCLUDED_XMLOFF_INC_FACREG_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
}

// writer autotext event export
OUString XMLAutoTextEventExport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventExport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// writer autotext event import
OUString XMLAutoTextEventImport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventImport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// writer autotext event export OOo
OUString XMLAutoTextEventExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
