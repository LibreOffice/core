/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ZipPackageEntry.cxx,v $
 * $Revision: 1.30 $
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
#ifndef _COM_SUN_STAR_PACKAGE_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#include <vos/diagnose.hxx>

#if defined( OSL_DEBUG )
#if OSL_DEBUG > 0
#include <ImplValidCharacters.hxx>
#endif
#endif
#include <ZipPackageFolder.hxx>
#include <ZipPackageStream.hxx>
#include <ContentInfo.hxx>

#if defined( OSL_DEBUG_LEVEL )
#if OSL_DEBUG_LEVEL > 0
#include <ImplValidCharacters.hxx>
#endif
#endif

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
    return aEntry.sName;
}
void SAL_CALL ZipPackageEntry::setName( const OUString& aName )
    throw(RuntimeException)
{
    if ( pParent && pParent->hasByName ( aEntry.sName ) )
        pParent->removeByName ( aEntry.sName );

    const sal_Unicode *pChar = aName.getStr();
    if(pChar == 0 || pChar != 0) {
        VOS_ENSURE ( Impl_IsValidChar (pChar, static_cast < sal_Int16 > ( aName.getLength() ), sal_False), "Invalid character in new zip package entry name!");
    }
    aEntry.sName = aName;

    if ( pParent )
        pParent->doInsertByName ( this, sal_False );
}
Reference< XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(RuntimeException)
{
    // return Reference< XInterface >( xParent, UNO_QUERY );
    return Reference< XInterface >( static_cast< ::cppu::OWeakObject* >( pParent ), UNO_QUERY );
}

void ZipPackageEntry::doSetParent ( ZipPackageFolder * pNewParent, sal_Bool bInsert )
{
    // xParent = pParent = pNewParent;
    pParent = pNewParent;
    if ( bInsert && !pNewParent->hasByName ( aEntry.sName ) )
        pNewParent->doInsertByName ( this, sal_False );
}

void SAL_CALL ZipPackageEntry::setParent( const Reference< XInterface >& xNewParent )
        throw(NoSupportException, RuntimeException)
{
    sal_Int64 nTest(0);
    Reference < XUnoTunnel > xTunnel ( xNewParent, UNO_QUERY );
    if ( !xNewParent.is() || ( nTest = xTunnel->getSomething ( ZipPackageFolder::static_getImplementationId () ) ) == 0 )
        throw NoSupportException();

    ZipPackageFolder *pNewParent = reinterpret_cast < ZipPackageFolder * > ( nTest );

    if ( pNewParent != pParent )
    {
        if ( pParent && pParent->hasByName ( aEntry.sName ) && mbAllowRemoveOnInsert )
            pParent->removeByName( aEntry.sName );
        doSetParent ( pNewParent, sal_True );
    }
}
    //XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageEntry::getPropertySetInfo(  )
        throw(RuntimeException)
{
    return Reference < beans::XPropertySetInfo > ();
}
void SAL_CALL ZipPackageEntry::addPropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removePropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::addVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removeVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
