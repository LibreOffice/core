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

#ifndef _SVT_LOCKFILECOMMON_HXX
#define _SVT_LOCKFILECOMMON_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>

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
    OUString m_aURL;

    INetURLObject ResolveLinks( const INetURLObject& aDocURL );

public:
    LockFileCommon( const OUString& aOrigURL, const OUString& aPrefix );
    ~LockFileCommon();

    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > ParseList( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer );
    static ::com::sun::star::uno::Sequence< OUString > ParseEntry( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static OUString ParseName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static OUString EscapeCharacters( const OUString& aSource );
    static OUString GetOOOUserName();
    static OUString GetCurrentLocalTime();
    static ::com::sun::star::uno::Sequence< OUString > GenerateOwnEntry();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
