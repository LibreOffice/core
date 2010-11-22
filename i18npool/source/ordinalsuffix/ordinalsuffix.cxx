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
#include "precompiled_i18npool.hxx"
#include <comphelper/processfactory.hxx>
#include <string.h>
#include "ordinalsuffix.hxx"


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {


OrdinalSuffix::OrdinalSuffix(
        const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) :
    _xServiceManager( rxMSF )
{
}

OrdinalSuffix::~OrdinalSuffix()
{
}


static OUString getOrdinalSuffixEn( sal_Int32 nNumber )
{
    OUString retValue;

    switch( labs( nNumber ) % 100 )
    {
        case 11: case 12: case 13:
            retValue = OUString(RTL_CONSTASCII_USTRINGPARAM("th"));
            break;
        default:
            switch( nNumber % 10 )
            {
                case 1:
                    retValue = OUString(RTL_CONSTASCII_USTRINGPARAM("st"));
                    break;
                case 2:
                    retValue = OUString(RTL_CONSTASCII_USTRINGPARAM("nd"));
                    break;
                case 3:
                    retValue = OUString(RTL_CONSTASCII_USTRINGPARAM("rd"));
                    break;
                default:
                    retValue = OUString(RTL_CONSTASCII_USTRINGPARAM("th"));
                    break;
            }
            break;
    }

    return retValue;
}


OUString SAL_CALL OrdinalSuffix::getOrdinalSuffix( sal_Int32 nNumber,
        const Locale &aLocale ) throw( RuntimeException )
{
    OUString retValue;

    if (aLocale.Language.equalsAsciiL("en",2))
        retValue = getOrdinalSuffixEn( nNumber );

    return retValue;
}


const sal_Char cOrdinalSuffix[] = "com.sun.star.i18n.OrdinalSuffix";

OUString SAL_CALL OrdinalSuffix::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(cOrdinalSuffix);
}

sal_Bool SAL_CALL OrdinalSuffix::supportsService( const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cOrdinalSuffix);
}

Sequence< OUString > SAL_CALL OrdinalSuffix::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cOrdinalSuffix);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
