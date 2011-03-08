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

    ReconnectingFile( const ::rtl::OUString& aFileURL )
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
