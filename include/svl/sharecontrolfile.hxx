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

#ifndef INCLUDED_SVL_SHARECONTROLFILE_HXX
#define INCLUDED_SVL_SHARECONTROLFILE_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>

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

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > m_aUsersData;

    void OpenStream();
    void Close();
    bool IsValid()
    {
        return ( m_xStream.is() && m_xInputStream.is() && m_xOutputStream.is() && m_xSeekable.is() && m_xTruncate.is() );
    }

public:

    // The constructor will throw exception in case the stream can not be opened
    ShareControlFile( const OUString& aOrigURL );
    ~ShareControlFile();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > GetUsersData();
    void SetUsersDataAndStore( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > >& aUserNames );
    ::com::sun::star::uno::Sequence< OUString > InsertOwnEntry();
    bool HasOwnEntry();
    void RemoveEntry( const ::com::sun::star::uno::Sequence< OUString >& aOptionalSpecification = ::com::sun::star::uno::Sequence< OUString >() );
    void RemoveFile();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
