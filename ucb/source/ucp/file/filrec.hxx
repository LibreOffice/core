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

#ifndef _FILREC_HXX_
#define _FILREC_HXX_

#include <osl/file.hxx>

namespace fileaccess {

class ReconnectingFile
{
    ::osl::File     m_aFile;

    sal_uInt32      m_nFlags;
    sal_Bool        m_bFlagsSet;

    sal_Bool        m_bDisconnect;

    ReconnectingFile( ReconnectingFile& );

    ReconnectingFile& operator=( ReconnectingFile& );

public:

    ReconnectingFile( const OUString& aFileURL )
        : m_aFile( aFileURL )
        , m_nFlags( 0 )
        , m_bFlagsSet( sal_False )
        , m_bDisconnect( sal_False )
    {}

    ~ReconnectingFile()
    {
        close();
    }

    void disconnect();
    sal_Bool reconnect();

    ::osl::FileBase::RC open( sal_uInt32 uFlags );

    ::osl::FileBase::RC close();

    ::osl::FileBase::RC setPos( sal_uInt32 uHow, sal_Int64 uPos );

    ::osl::FileBase::RC getPos( sal_uInt64& uPos );

    ::osl::FileBase::RC setSize( sal_uInt64 uSize );

    ::osl::FileBase::RC getSize( sal_uInt64 &rSize );

    ::osl::FileBase::RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead );

    ::osl::FileBase::RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten);

    ::osl::FileBase::RC sync() const;
};

} // namespace fileaccess
#endif  // _FILREC_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
