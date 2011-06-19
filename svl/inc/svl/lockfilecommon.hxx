/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVT_LOCKFILECOMMON_HXX
#define _SVT_LOCKFILECOMMON_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>

#define LOCKFILE_OOOUSERNAME_ID   0
#define LOCKFILE_SYSUSERNAME_ID   1
#define LOCKFILE_LOCALHOST_ID     2
#define LOCKFILE_EDITTIME_ID      3
#define LOCKFILE_USERURL_ID       4
#define LOCKFILE_ENTRYSIZE        5

namespace svt {

// This is a general implementation that is used in document lock file implementation and in sharing control file implementation
class SVL_DLLPUBLIC LockFileCommon
{
protected:
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::rtl::OUString m_aURL;


    INetURLObject ResolveLinks( const INetURLObject& aDocURL );

public:
    LockFileCommon( const ::rtl::OUString& aOrigURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory, const ::rtl::OUString& aPrefix );
    ~LockFileCommon();

    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > ParseList( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer );
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > ParseEntry( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static ::rtl::OUString ParseName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static ::rtl::OUString EscapeCharacters( const ::rtl::OUString& aSource );
    static ::rtl::OUString GetOOOUserName();
    static ::rtl::OUString GetCurrentLocalTime();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > GenerateOwnEntry();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
