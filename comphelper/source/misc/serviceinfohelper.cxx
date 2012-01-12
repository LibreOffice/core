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
#include "precompiled_comphelper.hxx"

#include "comphelper/serviceinfohelper.hxx"
#include <stdarg.h>

// #####################################################################

namespace comphelper
{

/** returns an empty UString(). most times sufficient */
::rtl::OUString SAL_CALL ServiceInfoHelper::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString();
}

/** the base implementation iterates over the service names from <code>getSupportedServiceNames</code> */
sal_Bool SAL_CALL ServiceInfoHelper::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    return supportsService( ServiceName, getSupportedServiceNames() );
}

sal_Bool SAL_CALL ServiceInfoHelper::supportsService( const ::rtl::OUString& ServiceName, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& SupportedServices ) throw()
{
    const ::rtl::OUString * pArray = SupportedServices.getConstArray();
    for( sal_Int32 i = 0; i < SupportedServices.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

/** the base implementation has no supported services */
::com::sun::star::uno::Sequence< ::rtl::OUString > ServiceInfoHelper::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq(0);
    return aSeq;
}

/** this method concatenates the given sequences and returns the result
 */
::com::sun::star::uno::Sequence< ::rtl::OUString > ServiceInfoHelper::concatSequences( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq1,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq2 ) throw()
{
    const sal_Int32 nLen1 = rSeq1.getLength();
    const sal_Int32 nLen2 = rSeq2.getLength();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq( nLen1 + nLen2 );

    ::rtl::OUString* pStrings = aSeq.getArray();

    sal_Int32 nIdx;
    const ::rtl::OUString* pStringSrc = rSeq1.getConstArray();
    for( nIdx = 0; nIdx < nLen1; nIdx++ )
        *pStrings++ = *pStringSrc++;

    pStringSrc = rSeq2.getConstArray();
    for( nIdx = 0; nIdx < nLen2; nIdx++ )
        *pStrings++ = *pStringSrc++;

    return aSeq;
}

/** this method adds a variable number of char pointer to a given Sequence
 */
void ServiceInfoHelper::addToSequence( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq, sal_uInt16 nServices, /* char * */ ... ) throw()
{
    sal_uInt32 nCount = rSeq.getLength();

    rSeq.realloc( nCount + nServices );
    rtl::OUString* pStrings = rSeq.getArray();

    va_list marker;
    va_start( marker, nServices );
    for( sal_uInt16 i = 0 ; i < nServices; i++ )
        pStrings[nCount++] = rtl::OUString::createFromAscii(va_arg( marker, char*));
    va_end( marker );
}

}


