/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    static OUString aServiceName( "com.sun.star.awt.StringMirror" );
    static Sequence< OUString > aServiceNames( &aServiceName, 1 );
    return aServiceNames;
}

OUString StringMirror_getImplementationName()
{
    return OUString( "vcl::StringMirror" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
