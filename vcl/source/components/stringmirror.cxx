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
#include "precompiled_vcl.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/util/XStringMapping.hpp"

#include "cppuhelper/implbase2.hxx"
#include "rtl/ustrbuf.hxx"
#include "vcl/svapp.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

// -----------------------------------------------------------------------

namespace vcl
{

class StringMirror : public ::cppu::WeakAggImplHelper2< XStringMapping, XServiceInfo >
{
public:
    StringMirror()
    {}

    virtual ~StringMirror()
    {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const OUString& ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    // XStringMapping
    virtual sal_Bool SAL_CALL mapStrings( Sequence< OUString >& io_rStrings ) throw (RuntimeException)
    {
        sal_Int32 nItems = io_rStrings.getLength();
        for( sal_Int32 n = 0; n < nItems; n++ )
        {
            rtl::OUString& rStr( io_rStrings.getArray()[n] );

            sal_Int32 nLen = rStr.getLength();
            rtl::OUStringBuffer aMirror( nLen );
            for(sal_Int32 i = nLen - 1; i >= 0; i--)
            {
                sal_Unicode cChar = rStr[ i ];
                aMirror.append(sal_Unicode(GetMirroredChar(cChar)));
            }
            rStr = aMirror.makeStringAndClear();
        }
        return sal_True;
    }
};

Sequence< OUString > StringMirror_getSupportedServiceNames()
{
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.StringMirror" ) );
    static Sequence< OUString > aServiceNames( &aServiceName, 1 );
    return aServiceNames;
}

OUString StringMirror_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "vcl::StringMirror" ) );
}

Reference< XInterface > SAL_CALL StringMirror_createInstance( const Reference< XMultiServiceFactory >&  )
{
    return static_cast< ::cppu::OWeakObject * >( new StringMirror );
}


// XServiceInfo
OUString SAL_CALL StringMirror::getImplementationName() throw (RuntimeException)
{
    return StringMirror_getImplementationName();
}

sal_Bool SAL_CALL StringMirror::supportsService( const OUString& i_rServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aSN( StringMirror_getSupportedServiceNames() );
    for( sal_Int32 nService = 0; nService < aSN.getLength(); nService++ )
    {
        if( aSN[nService] == i_rServiceName )
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL StringMirror::getSupportedServiceNames() throw (RuntimeException)
{
    return StringMirror_getSupportedServiceNames();
}

} // namespace vcl
