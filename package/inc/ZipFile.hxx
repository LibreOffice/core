/*************************************************************************
 *
 *  $RCSfile: ZipFile.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-19 14:11:06 $
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
#ifndef _ZIP_FILE_HXX
#define _ZIP_FILE_HXX

#ifndef _COM_SUN_STAR_PACKAGE_XZIPFILE_HPP_
#include <com/sun/star/packages/XZipFile.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _BYTE_GRABBER_HXX_
#include <ByteGrabber.hxx>
#endif
#ifndef _ENTRY_HASH_HXX
#include <EntryHash.hxx>
#endif
#ifndef _INFLATER_HXX
#include <Inflater.hxx>
#endif

/*
 * We impose  arbitrary but reasonable limit on ZIP files.
 */
#define ZIP_MAXNAMELEN 512
#define ZIP_MAXEXTRA 256
#define ZIP_MAXENTRIES (0x10000 - 2)

class ZipFile : public cppu::WeakImplHelper1<
                        com::sun::star::packages::XZipFile>
{
private:
    ::rtl::OUString sName;          /* zip file name */
    ::rtl::OUString sComment;       /* zip file comment */
    EntryHash       aEntries;
    ByteGrabber     aGrabber;
    Inflater        aInflater;
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
public:
    ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput, sal_Bool bInitialise)
        throw(::com::sun::star::io::IOException, com::sun::star::packages::ZipException, com::sun::star::uno::RuntimeException);
    void setInputStream ( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewStream );
    sal_uInt32 SAL_CALL getHeader(const ::com::sun::star::packages::ZipEntry& rEntry)
        throw(::com::sun::star::io::IOException, ::com::sun::star::packages::ZipException, ::com::sun::star::uno::RuntimeException);
    virtual ~ZipFile();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawStream( const ::com::sun::star::packages::ZipEntry& rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::packages::ZipException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);
    // XZipFile
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( const ::com::sun::star::packages::ZipEntry& rEntry )
        throw(::com::sun::star::io::IOException, ::com::sun::star::packages::ZipException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSize(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL close(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess (except called with entries instead of
    // createEnumeration
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL entries(  )
        throw(::com::sun::star::uno::RuntimeException);
private:
    sal_Bool        readLOC(const com::sun::star::packages::ZipEntry &rEntry)
        throw(::com::sun::star::io::IOException, com::sun::star::packages::ZipException, com::sun::star::uno::RuntimeException);
    sal_Int32       readCEN()
        throw(::com::sun::star::io::IOException, com::sun::star::packages::ZipException, com::sun::star::uno::RuntimeException);
    sal_Int32       findEND()
        throw(::com::sun::star::io::IOException, com::sun::star::packages::ZipException, com::sun::star::uno::RuntimeException);
};

#endif
