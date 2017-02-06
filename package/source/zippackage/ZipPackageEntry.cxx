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

#include <ZipPackageEntry.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <osl/diagnose.h>

#include <ZipPackageFolder.hxx>
#include <ZipPackageStream.hxx>
#include <ContentInfo.hxx>

#include <comphelper/storagehelper.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

ZipPackageEntry::ZipPackageEntry()
: mbIsFolder( false )
, mbAllowRemoveOnInsert(false)
, mpParent ( nullptr )
, m_nFormat(0)
{
}

ZipPackageEntry::~ZipPackageEntry()
{
    // When the entry is destroyed it must be already disconnected from the parent
    OSL_ENSURE( !mpParent, "The parent must be disconnected already! Memory corruption is possible!\n" );
}

// XChild
OUString SAL_CALL ZipPackageEntry::getName(  )
{
    return msName;
}
void SAL_CALL ZipPackageEntry::setName( const OUString& aName )
{
    if ( mpParent && !msName.isEmpty() && mpParent->hasByName ( msName ) )
        mpParent->removeByName ( msName );

    // unfortunately no other exception than RuntimeException can be thrown here
    // usually the package is used through storage implementation, the problem should be detected there
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( aName, true ) )
        throw RuntimeException(THROW_WHERE "Unexpected character is used in file name." );

    msName = aName;

    if ( mpParent )
        mpParent->doInsertByName ( this, false );
}
uno::Reference< XInterface > SAL_CALL ZipPackageEntry::getParent(  )
{
    // return uno::Reference< XInterface >( xParent, UNO_QUERY );
    return uno::Reference< XInterface >( static_cast< ::cppu::OWeakObject* >( mpParent ), UNO_QUERY );
}

void ZipPackageEntry::doSetParent ( ZipPackageFolder * pNewParent )
{
    // xParent = mpParent = pNewParent;
    mpParent = pNewParent;
    if ( !msName.isEmpty() && !pNewParent->hasByName ( msName ) )
        pNewParent->doInsertByName ( this, false );
}

void SAL_CALL ZipPackageEntry::setParent( const uno::Reference< XInterface >& xNewParent )
{
    sal_Int64 nTest(0);
    uno::Reference < XUnoTunnel > xTunnel ( xNewParent, UNO_QUERY );
    if ( !xNewParent.is() || ( nTest = xTunnel->getSomething ( ZipPackageFolder::static_getImplementationId () ) ) == 0 )
        throw NoSupportException(THROW_WHERE );

    ZipPackageFolder *pNewParent = reinterpret_cast < ZipPackageFolder * > ( nTest );

    if ( pNewParent != mpParent )
    {
        if ( mpParent && !msName.isEmpty() && mpParent->hasByName ( msName ) && mbAllowRemoveOnInsert )
            mpParent->removeByName( msName );
        doSetParent ( pNewParent );
    }
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageEntry::getPropertySetInfo(  )
{
    return uno::Reference < beans::XPropertySetInfo > ();
}
void SAL_CALL ZipPackageEntry::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
}
void SAL_CALL ZipPackageEntry::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackageEntry::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackageEntry::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
