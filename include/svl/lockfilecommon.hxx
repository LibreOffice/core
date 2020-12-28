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

#ifndef INCLUDED_SVL_LOCKFILECOMMON_HXX
#define INCLUDED_SVL_LOCKFILECOMMON_HXX

#include <svl/svldllapi.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <o3tl/enumarray.hxx>

#include <string_view>
#include <vector>

enum class LockFileComponent
{
    OOOUSERNAME, SYSUSERNAME, LOCALHOST, EDITTIME, USERURL, LAST=USERURL
};

typedef o3tl::enumarray<LockFileComponent,OUString> LockFileEntry;

namespace svt {

/// This is a general implementation that is used in document lock file implementation and in sharing control file implementation
class SVL_DLLPUBLIC LockFileCommon
{
private:
    OUString m_aURL;

protected:
    ::osl::Mutex m_aMutex;

    /// This method generates the URL of the lock file based on the document URL and the specified prefix.
    static OUString GenerateOwnLockFileURL(const OUString& aOrigURL, std::u16string_view aPrefix);

public:
    LockFileCommon(const OUString& aLockFileURL);
    virtual ~LockFileCommon();

    const OUString& GetURL() const;
    void SetURL(const OUString& aURL);

    static void ParseList( const css::uno::Sequence< sal_Int8 >& aBuffer, std::vector< LockFileEntry > &rOutput );
    static LockFileEntry ParseEntry( const css::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static OUString ParseName( const css::uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos );
    static OUString EscapeCharacters( const OUString& aSource );
    static OUString GetOOOUserName();
    static OUString GetCurrentLocalTime();
    static LockFileEntry GenerateOwnEntry();

    static INetURLObject ResolveLinks( const INetURLObject& aDocURL );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
