/*************************************************************************
 *
 *  $RCSfile: ZipPackageStream.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-24 10:34:27 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ZIP_PACKAGE_STREAM_HXX
#include "ZipPackageStream.hxx"
#endif

using namespace com::sun::star;

ZipPackageStream::ZipPackageStream (ZipFile &rInFile)
: rZipFile(rInFile)
, bPackageMember( sal_False )
{
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = -1;
    aEntry.nTime        = -1;
    aEntry.nCrc         = -1;
    aEntry.nCompressedSize  = -1;
    aEntry.nSize        = -1;
    aEntry.nOffset      = -1;
}

ZipPackageStream::~ZipPackageStream( void )
{
}

void ZipPackageStream::setZipEntry( const package::ZipEntry &rInEntry)
{
    aEntry.nVersion = rInEntry.nVersion;
    aEntry.nFlag = rInEntry.nFlag;
    aEntry.nMethod = rInEntry.nMethod;
    aEntry.nTime = rInEntry.nTime;
    aEntry.nCrc = rInEntry.nCrc;
    aEntry.nCompressedSize = rInEntry.nCompressedSize;
    aEntry.nSize = rInEntry.nSize;
    aEntry.nOffset = rInEntry.nOffset;
    aEntry.sName = rInEntry.sName;
    aEntry.extra = rInEntry.extra;
    aEntry.sComment = rInEntry.sComment;
}
    //XInterface
uno::Any SAL_CALL ZipPackageStream::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    // Ask for my own supported interfaces ...
    uno::Any aReturn    ( ::cppu::queryInterface    (   rType                                       ,
                                                static_cast< container::XNamed*     > ( this )  ,
                                                static_cast< container::XChild*     > ( this )  ,
                                                static_cast< io::XActiveDataSink*       > ( this )  ,
                                                static_cast< lang::XUnoTunnel*      > ( this )  ,
                                                static_cast< beans::XPropertySet*       > ( this ) ) );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue () == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return ZipPackageEntry::queryInterface ( rType ) ;
    }
}
void SAL_CALL ZipPackageStream::acquire(  )
    throw()
{
    ZipPackageEntry::acquire();
}
void SAL_CALL ZipPackageStream::release(  )
    throw()
{
    ZipPackageEntry::release();
}
    // XActiveDataSink
void SAL_CALL ZipPackageStream::setInputStream( const uno::Reference< io::XInputStream >& aStream )
        throw(uno::RuntimeException)
{
    xStream = aStream;
    bPackageMember = sal_False;
}
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream(  )
        throw(uno::RuntimeException)
{
    if (bPackageMember)
        return rZipFile.getInputStream(aEntry);
    else
        return xStream;
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageStream::getPropertySetInfo(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < beans::XPropertySetInfo > ();
}
void SAL_CALL ZipPackageStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aPropertyName == ::rtl::OUString::createFromAscii("MediaType"))
        aValue >>= aEntry.sComment;
    else if (aPropertyName == ::rtl::OUString::createFromAscii("Size"))
        aValue >>= aEntry.nSize;
    else
        throw beans::UnknownPropertyException();
}
uno::Any SAL_CALL ZipPackageStream::getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (PropertyName == ::rtl::OUString::createFromAscii("MediaType"))
    {
        uno::Any aAny;
        aAny <<= aEntry.sComment;
        return aAny;
    }
    else if (PropertyName == ::rtl::OUString::createFromAscii("Size"))
    {
        uno::Any aAny;
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}
void SAL_CALL ZipPackageStream::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageStream::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageStream::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageStream::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
uno::Sequence< sal_Int8 > ZipPackageStream::getUnoTunnelImplementationId( void )
    throw (uno::RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

sal_Int64 SAL_CALL ZipPackageStream::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    throw uno::RuntimeException();
}
