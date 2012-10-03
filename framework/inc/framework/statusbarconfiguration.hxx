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
#ifndef __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <tools/stream.hxx>
#include <vcl/status.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework
{

class FWE_DLLPUBLIC StatusBarConfiguration
{
    public:
        static sal_Bool LoadStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rStatusbarConfiguration );

        static sal_Bool StoreStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusbarConfiguration );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
