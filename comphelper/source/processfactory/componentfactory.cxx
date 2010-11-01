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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/componentfactory.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/shlib.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::rtl;


namespace comphelper
{

Reference< XInterface > getComponentInstance(
            const OUString & rLibraryName,
            const OUString & rImplementationName
            )
{
    Reference< XInterface > xI;
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    if ( xMSF.is() )
        xI = xMSF->createInstance( rImplementationName );
    if( !xI.is() )
    {
        Reference< XSingleServiceFactory > xSSF =
            loadLibComponentFactory( rLibraryName, rImplementationName,
            Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() );
        if (xSSF.is())
            xI = xSSF->createInstance();
    }
    return xI;
}


Reference< XSingleServiceFactory > loadLibComponentFactory(
            const OUString & rLibName,
            const OUString & rImplName,
            const Reference< XMultiServiceFactory > & xSF,
            const Reference< XRegistryKey > & xKey
            )
{
    return Reference< XSingleServiceFactory >( ::cppu::loadSharedLibComponentFactory(
        rLibName, OUString(), rImplName, xSF, xKey ), UNO_QUERY );
}

}   // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
