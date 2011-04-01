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
#include "precompiled_package.hxx"
#include <ZipPackageEntry.hxx>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <osl/diagnose.h>

#include <ZipPackageFolder.hxx>
#include <ZipPackageStream.hxx>
#include <ContentInfo.hxx>

#include <comphelper/storagehelper.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

ZipPackageEntry::ZipPackageEntry ( bool bNewFolder )
: mbIsFolder ( bNewFolder )
, mbAllowRemoveOnInsert( sal_True )
, pParent ( NULL )
{
}

ZipPackageEntry::~ZipPackageEntry()
{
    // When the entry is destroyed it must be already disconnected from the parent
    OSL_ENSURE( !pParent, "The parent must be disconnected already! Memory corruption is possible!\n" );
}

// XChild
OUString SAL_CALL ZipPackageEntry::getName(  )
    throw(RuntimeException)
{
    return msName;
}
void SAL_CALL ZipPackageEntry::setName( const OUString& aName )
    throw(RuntimeException)
{
    if ( pParent && msName.getLength() && pParent->hasByName ( msName ) )
        pParent->removeByName ( msName );

    // unfortunately no other exception than RuntimeException can be thrown here
    // usually the package is used through storage implementation, the problem should be detected there
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( aName, sal_True ) )
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Unexpected character is used in file name." ) ), uno::Reference< XInterface >() );

    msName = aName;

    if ( pParent )
        pParent->doInsertByName ( this, sal_False );
}
uno::Reference< XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(RuntimeException)
{
    // return uno::Reference< XInterface >( xParent, UNO_QUERY );
    return uno::Reference< XInterface >( static_cast< ::cppu::OWeakObject* >( pParent ), UNO_QUERY );
}

void ZipPackageEntry::doSetParent ( ZipPackageFolder * pNewParent, sal_Bool bInsert )
{
    // xParent = pParent = pNewParent;
    pParent = pNewParent;
    if ( bInsert && msName.getLength() && !pNewParent->hasByName ( msName ) )
        pNewParent->doInsertByName ( this, sal_False );
}

void SAL_CALL ZipPackageEntry::setParent( const uno::Reference< XInterface >& xNewParent )
        throw(NoSupportException, RuntimeException)
{
    sal_Int64 nTest(0);
    uno::Reference < XUnoTunnel > xTunnel ( xNewParent, UNO_QUERY );
    if ( !xNewParent.is() || ( nTest = xTunnel->getSomething ( ZipPackageFolder::static_getImplementationId () ) ) == 0 )
        throw NoSupportException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    ZipPackageFolder *pNewParent = reinterpret_cast < ZipPackageFolder * > ( nTest );

    if ( pNewParent != pParent )
    {
        if ( pParent && msName.getLength() && pParent->hasByName ( msName ) && mbAllowRemoveOnInsert )
            pParent->removeByName( msName );
        doSetParent ( pNewParent, sal_True );
    }
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageEntry::getPropertySetInfo(  )
        throw(RuntimeException)
{
    return uno::Reference < beans::XPropertySetInfo > ();
}
void SAL_CALL ZipPackageEntry::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
