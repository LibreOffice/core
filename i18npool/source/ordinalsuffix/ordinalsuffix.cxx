/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            retValue = OUString::createFromAscii( "th" );
            break;
        default:
            switch( nNumber % 10 )
            {
                case 1:
                    retValue = OUString::createFromAscii( "st" );
                    break;
                case 2:
                    retValue = OUString::createFromAscii( "nd" );
                    break;
                case 3:
                    retValue = OUString::createFromAscii( "rd" );
                    break;
                default:
                    retValue = OUString::createFromAscii( "th" );
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
