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
#include "precompiled_unotools.hxx"

#include <unotools/numberformatcodewrapper.hxx>
#include <tools/debug.hxx>
#include "instance.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


NumberFormatCodeWrapper::NumberFormatCodeWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF )
{
    setLocale( rLocale );
    xNFC = Reference< XNumberFormatCode > (
        intl_createInstance( xSMgr, "com.sun.star.i18n.NumberFormatMapper",
                             "NumberFormatCodeWrapper" ), uno::UNO_QUERY );
    DBG_ASSERT( xNFC.is(), "NumberFormatCodeWrapper: no NumberFormatMapper" );
}


NumberFormatCodeWrapper::~NumberFormatCodeWrapper()
{
}


void NumberFormatCodeWrapper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    aLocale = rLocale;
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getDefault( formatType, formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getDefault: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getFormatCode( sal_Int16 formatIndex ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getFormatCode( formatIndex, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getFormatCode: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCode( sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCode( formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCode: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCodes() const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCodes( aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCodes: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
