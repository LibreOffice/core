/*************************************************************************
 *
 *  $RCSfile: ZipPackageEntry.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-16 17:11:42 $
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
#include "ZipPackageEntry.hxx"
#endif

using namespace com::sun::star;
using namespace com::sun::star::packages::ZipConstants;
using namespace rtl;

ZipPackageEntry::ZipPackageEntry (void)
: bPackageMember  ( sal_False )
, bToBeCompressed ( sal_True )
{
}

ZipPackageEntry::~ZipPackageEntry( void )
{
}
/* I made these pure virtual to bypass a couple of virtual calls...
 * acquire/release/queryInterface are called several thousand times in a single
 * ZipPackage instance
uno::Any SAL_CALL ZipPackageEntry::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    return ( ::cppu::queryInterface (   rType                                       ,
                                                // OWeakObject interfaces
                                                static_cast< uno::XWeak*            > ( this )  ,
                                                static_cast< uno::XInterface*       > ( this )  ,
                                                // my own interfaces
                                                static_cast< container::XNamed*     > ( this )  ,
                                                static_cast< lang::XUnoTunnel*      > ( this )  ,
                                                static_cast< container::XChild*     > ( this )  ) );
}
void SAL_CALL ZipPackageEntry::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL ZipPackageEntry::release(  )
    throw()
{
    OWeakObject::release();
}
*/
    // XChild
::rtl::OUString SAL_CALL ZipPackageEntry::getName(  )
    throw(uno::RuntimeException)
{
    return aEntry.sName;
}
void SAL_CALL ZipPackageEntry::setName( const ::rtl::OUString& aName )
    throw(uno::RuntimeException)
{
    aEntry.sName = aName;
}
uno::Reference< uno::XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(uno::RuntimeException)
{
    return xParent;
}
void SAL_CALL ZipPackageEntry::setParent( const uno::Reference< uno::XInterface >& Parent )
        throw(lang::NoSupportException, uno::RuntimeException)
{
    uno::Reference < container::XNameContainer > xOldParent (xParent, uno::UNO_QUERY);
    uno::Reference < container::XNameContainer > xNewParent (Parent, uno::UNO_QUERY);

    if ( !xNewParent.is())
        throw lang::NoSupportException();

    if ( xOldParent.is() && xOldParent->hasByName(getName()))
        xOldParent->removeByName(getName());
    uno::Any aAny;
    uno::Reference < lang::XUnoTunnel > xTunnel  = this;
    aAny <<= xTunnel;
    if (!xNewParent->hasByName(getName()))
        xNewParent->insertByName(getName(), aAny);
    xParent = Parent;
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageEntry::getPropertySetInfo(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < beans::XPropertySetInfo > (NULL);
}
void SAL_CALL ZipPackageEntry::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
    {
        aValue >>= sMediaType;

        if (sMediaType.getLength() > 0)
        {
            if ( sMediaType.indexOf (OUString( RTL_CONSTASCII_USTRINGPARAM ( "text" ) ) ) != -1)
                bToBeCompressed = sal_True;
            else
                bToBeCompressed = sal_False;
        }
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
        aValue >>= aEntry.nSize;
#if SUPD>617
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Compressed") ) )
#else
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Compress") ) )
#endif
        aValue >>= bToBeCompressed;
    else
        throw beans::UnknownPropertyException();
}
uno::Any SAL_CALL ZipPackageEntry::getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
#if SUPD>617
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
#else
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compress" ) ) )
#endif
    {
        aAny <<= bToBeCompressed;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}
void SAL_CALL ZipPackageEntry::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageEntry::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
