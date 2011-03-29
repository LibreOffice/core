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
#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#define __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <svl/svarray.hxx>
#include <vcl/bitmap.hxx>
#include <tools/string.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#ifndef _COM_SUN_STAR_IO_XOUPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace framework
{

class FWE_DLLPUBLIC ToolBoxConfiguration
{
    public:
        static sal_Bool LoadToolBox(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
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
