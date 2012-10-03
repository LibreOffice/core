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
#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#define __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <vcl/bitmap.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#ifndef _COM_SUN_STAR_IO_XOUPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework
{

class FWE_DLLPUBLIC ToolBoxConfiguration
{
    public:
        static sal_Bool LoadToolBox(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rToolbarConfiguration );

        static sal_Bool StoreToolBox(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rToolbarConfiguration );
};

} // namespace framework

#endif // __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
