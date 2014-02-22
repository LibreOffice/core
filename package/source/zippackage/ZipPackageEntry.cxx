/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <ZipPackageEntry.hxx>
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

ZipPackageEntry::ZipPackageEntry ( bool bNewFolder )
: mbIsFolder ( bNewFolder )
, mbAllowRemoveOnInsert( true )
, pParent ( NULL )
{
}

ZipPackageEntry::~ZipPackageEntry()
{
    
    OSL_ENSURE( !pParent, "The parent must be disconnected already! Memory corruption is possible!\n" );
}


OUString SAL_CALL ZipPackageEntry::getName(  )
    throw(RuntimeException)
{
    return msName;
}
void SAL_CALL ZipPackageEntry::setName( const OUString& aName )
    throw(RuntimeException)
{
    if ( pParent && !msName.isEmpty() && pParent->hasByName ( msName ) )
        pParent->removeByName ( msName );

    
    
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( aName, true ) )
        throw RuntimeException(THROW_WHERE "Unexpected character is used in file name.", uno::Reference< XInterface >() );

    msName = aName;

    if ( pParent )
        pParent->doInsertByName ( this, sal_False );
}
uno::Reference< XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(RuntimeException)
{
    
    return uno::Reference< XInterface >( static_cast< ::cppu::OWeakObject* >( pParent ), UNO_QUERY );
}

void ZipPackageEntry::doSetParent ( ZipPackageFolder * pNewParent, sal_Bool bInsert )
{
    
    pParent = pNewParent;
    if ( bInsert && !msName.isEmpty() && !pNewParent->hasByName ( msName ) )
        pNewParent->doInsertByName ( this, sal_False );
}

void SAL_CALL ZipPackageEntry::setParent( const uno::Reference< XInterface >& xNewParent )
        throw(NoSupportException, RuntimeException)
{
    sal_Int64 nTest(0);
    uno::Reference < XUnoTunnel > xTunnel ( xNewParent, UNO_QUERY );
    if ( !xNewParent.is() || ( nTest = xTunnel->getSomething ( ZipPackageFolder::static_getImplementationId () ) ) == 0 )
        throw NoSupportException(THROW_WHERE, uno::Reference< uno::XInterface >() );

    ZipPackageFolder *pNewParent = reinterpret_cast < ZipPackageFolder * > ( nTest );

    if ( pNewParent != pParent )
    {
        if ( pParent && !msName.isEmpty() && pParent->hasByName ( msName ) && mbAllowRemoveOnInsert )
            pParent->removeByName( msName );
        doSetParent ( pNewParent, sal_True );
    }
}
    
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
