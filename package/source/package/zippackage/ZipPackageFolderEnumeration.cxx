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
#include "precompiled_package.hxx"
#include <ZipPackageFolderEnumeration.hxx>
#include <ContentInfo.hxx>

using namespace com::sun::star;
using rtl::OUString;

ZipPackageFolderEnumeration::ZipPackageFolderEnumeration ( ContentHash &rInput)
: rContents (rInput)
, aIterator (rContents.begin())
{
}

ZipPackageFolderEnumeration::~ZipPackageFolderEnumeration( void )
{
}

sal_Bool SAL_CALL ZipPackageFolderEnumeration::hasMoreElements(  )
        throw(uno::RuntimeException)
{
    return (aIterator != rContents.end() );
}
uno::Any SAL_CALL ZipPackageFolderEnumeration::nextElement(  )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if (aIterator == rContents.end() )
        throw container::NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    aAny <<= (*aIterator).second->xTunnel;
    aIterator++;
    return aAny;
}

OUString ZipPackageFolderEnumeration::getImplementationName()
    throw (uno::RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageFolderEnumeration" ) );
}

uno::Sequence< OUString > ZipPackageFolderEnumeration::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames(1);
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageFolderEnumeration" ) );
    return aNames;
}
sal_Bool SAL_CALL ZipPackageFolderEnumeration::supportsService( OUString const & rServiceName )
    throw (uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}
