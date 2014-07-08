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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_OLEPERSIST_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_OLEPERSIST_HXX

#include <sal/config.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
    namespace lang { class XMultiServiceFactory; }
} } }

OUString GetNewFilledTempFile_Impl(
    css::uno::Reference<css::io::XInputStream > const & xInStream,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & xFactory)
    throw (css::io::IOException, css::uno::RuntimeException);

bool KillFile_Impl( const OUString& aURL, const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
