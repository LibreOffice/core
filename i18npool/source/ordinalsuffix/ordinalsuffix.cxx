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

#include <comphelper/processfactory.hxx>
#include <string.h>
#include "ordinalsuffix.hxx"

#include <unicode/rbnf.h>
#include <unicode/normlzr.h>

#define CSTR( ouStr ) rtl::OUStringToOString( ouStr, RTL_TEXTENCODING_UTF8 ).getStr( )

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
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


/*
 * For this method to properly return the ordinal suffix for other locales
 * than english ones, ICU 4.2+ has to be used.
 */
uno::Sequence< OUString > SAL_CALL OrdinalSuffix::getOrdinalSuffix( sal_Int32 nNumber,
        const lang::Locale &aLocale ) throw( RuntimeException )
{
    uno::Sequence< OUString > retValue;

    // Get the value from ICU
    UErrorCode nCode = U_ZERO_ERROR;
    const icu::Locale rIcuLocale(
            CSTR( aLocale.Language ),
            CSTR( aLocale.Country ),
            CSTR( aLocale.Variant ) );
    icu::RuleBasedNumberFormat formatter(
            icu::URBNF_ORDINAL, rIcuLocale, nCode );

    if ( U_SUCCESS( nCode ) )
    {
        int32_t nRuleSets = formatter.getNumberOfRuleSetNames( );
        for ( int32_t i = 0; i < nRuleSets; i++ )
        {
            icu::UnicodeString ruleSet = formatter.getRuleSetName( i );
            // format the string
            icu::UnicodeString icuRet;
            icu::FieldPosition icuPos;
            formatter.format( (int32_t)nNumber, ruleSet, icuRet, icuPos, nCode );

            if ( U_SUCCESS( nCode ) )
            {
                // Apply NFKC normalization to get normal letters
                icu::UnicodeString normalized;
                nCode = U_ZERO_ERROR;
                icu::Normalizer::normalize( icuRet, UNORM_NFKC, 0, normalized, nCode );
                if ( U_SUCCESS( nCode ) )
                {
                    // Convert the normalized UnicodeString to OUString
                    OUString sValue( reinterpret_cast<const sal_Unicode *>( normalized.getBuffer( ) ), normalized.length() );

                    // Remove the number to get the prefix
                    sal_Int32 len = OUString::valueOf( nNumber ).getLength( );

                    sal_Int32 newLength = retValue.getLength() + 1;
                    retValue.realloc( newLength );
                    retValue[ newLength - 1 ] = sValue.copy( len );
                }
            }
        }
    }

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
