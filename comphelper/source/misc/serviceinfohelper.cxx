/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoprov.cxx,v $
 * $Revision: 1.72.92.1 $
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
