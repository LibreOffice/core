/*************************************************************************
 *
 *  $RCSfile: ZipPackageEntry.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 10:17:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _ZIP_PACKAGE_ENTRY_HXX
#include <ZipPackageEntry.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGE_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _IMPL_VALID_CHARACTERS_HXX_
#include <ImplValidCharacters.hxx>
#endif
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include <ZipPackageFolder.hxx>
#endif
#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _CONTENT_INFO_HXX_
#include <ContentInfo.hxx>
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

ZipPackageEntry::ZipPackageEntry ( bool bNewFolder )
: pParent ( NULL )
, mbIsFolder ( bNewFolder )
{
}

ZipPackageEntry::~ZipPackageEntry()
{
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
    VOS_ENSURE ( Impl_IsValidChar (pChar, static_cast < sal_Int16 > ( aName.getLength() ), sal_False), "Invalid character in new zip package entry name!");

    aEntry.sName = aName;

    if ( pParent )
        pParent->doInsertByName ( this, sal_False );
}
Reference< XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(RuntimeException)
{
    return Reference< XInterface >( xParent, UNO_QUERY );
}

void ZipPackageEntry::doSetParent ( ZipPackageFolder * pNewParent, sal_Bool bInsert )
{
    xParent = pParent = pNewParent;
    if ( bInsert && !pNewParent->hasByName ( aEntry.sName ) )
        pNewParent->doInsertByName ( this, sal_False );
}

void SAL_CALL ZipPackageEntry::setParent( const Reference< XInterface >& xNewParent )
        throw(NoSupportException, RuntimeException)
{
    sal_Int64 nTest;
    Reference < XUnoTunnel > xTunnel ( xNewParent, UNO_QUERY );
    if ( !xNewParent.is() || ( nTest = xTunnel->getSomething ( ZipPackageFolder::static_getImplementationId () ) ) == 0 )
        throw NoSupportException();

    ZipPackageFolder *pNewParent = reinterpret_cast < ZipPackageFolder * > ( nTest );

    if ( pNewParent != pParent )
    {
        if ( pParent && pParent->hasByName ( aEntry.sName ) )
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
void SAL_CALL ZipPackageEntry::addPropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removePropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& aListener )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::addVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removeVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
