/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentlockfile.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef _SVT_DOCUMENTLOCKFILE_HXX
#define _SVT_DOCUMENTLOCKFILE_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <osl/mutex.hxx>

// TODO/LATER: should be combined with sharecontrolfile
#define LOCKFILE_OOOUSERNAME_ID   0
#define LOCKFILE_SYSUSERNAME_ID   1
#define LOCKFILE_LOCALHOST_ID     2
#define LOCKFILE_EDITTIME_ID      3
#define LOCKFILE_USERURL_ID       4
#define LOCKFILE_ENTRYSIZE        5

namespace svt {

class SVT_DLLPUBLIC DocumentLockFile
{
    // the workaround for automated testing!
    static sal_Bool m_bAllowInteraction;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::rtl::OUString m_aURL;


    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenStream();

    void WriteEntryToStream( ::com::sun::star::uno::Sequence< ::rtl::OUString > aEntry, ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xStream );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > ParseEntry( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer );
    ::rtl::OUString ParseName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    ::rtl::OUString EscapeCharacters( const ::rtl::OUString& aSource );
    ::rtl::OUString GetOOOUserName();
    ::rtl::OUString GetCurrentLocalTime();

public:
    DocumentLockFile( const ::rtl::OUString& aOrigURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() );
    ~DocumentLockFile();

    sal_Bool CreateOwnLockFile();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetLockData();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GenerateOwnEntry();
    sal_Bool OverwriteOwnLockFile();
    void RemoveFile();

    // the methods allow to control whether UI interaction regarding the locked document file is allowed
    // this is a workaround for automated tests
    static void AllowInteraction( sal_Bool bAllow ) { m_bAllowInteraction = bAllow; }
    static sal_Bool IsInteractionAllowed() { return m_bAllowInteraction; }
};

}

#endif

