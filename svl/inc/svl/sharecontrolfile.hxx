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

#ifndef _SVT_SHARECONTROLFILE_HXX
#define _SVT_SHARECONTROLFILE_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svl/lockfilecommon.hxx>

#define SHARED_OOOUSERNAME_ID   LOCKFILE_OOOUSERNAME_ID
#define SHARED_SYSUSERNAME_ID   LOCKFILE_SYSUSERNAME_ID
#define SHARED_LOCALHOST_ID     LOCKFILE_LOCALHOST_ID
#define SHARED_EDITTIME_ID      LOCKFILE_EDITTIME_ID
#define SHARED_USERURL_ID       LOCKFILE_USERURL_ID
#define SHARED_ENTRYSIZE        LOCKFILE_ENTRYSIZE

namespace svt {

class SVL_DLLPUBLIC ShareControlFile : public LockFileCommon
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > m_xOutputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable > m_xSeekable;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XTruncate > m_xTruncate;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > m_aUsersData;

    void OpenStream();
    void Close();
    sal_Bool IsValid()
    {
        return ( m_xFactory.is() && m_xStream.is() && m_xInputStream.is() && m_xOutputStream.is() && m_xSeekable.is() && m_xTruncate.is() );
    }

public:

    // The constructor will throw exception in case the stream can not be opened
    ShareControlFile( const ::rtl::OUString& aOrigURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() );
    ~ShareControlFile();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > > GetUsersData();
    void SetUsersDataAndStore( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >& aUserNames );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > InsertOwnEntry();
    bool HasOwnEntry();
    void RemoveEntry( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aOptionalSpecification = ::com::sun::star::uno::Sequence< ::rtl::OUString >() );
    void RemoveFile();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
