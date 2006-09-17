/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZipPackageFolderEnumeration.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:29:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include <ZipPackageFolderEnumeration.hxx>
#endif
#ifndef _CONTENT_INFO_HXX_
#include <ContentInfo.hxx>
#endif

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
        throw container::NoSuchElementException();
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
