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
#include <o3tl/enumarray.hxx>
#include <vector>

namespace svt {

class SVL_DLLPUBLIC ShareControlFile : public LockFileCommon
{
    css::uno::Reference< css::io::XStream >       m_xStream;
    css::uno::Reference< css::io::XInputStream >  m_xInputStream;
    css::uno::Reference< css::io::XOutputStream > m_xOutputStream;
    css::uno::Reference< css::io::XSeekable >     m_xSeekable;
    css::uno::Reference< css::io::XTruncate >     m_xTruncate;

    std::vector< LockFileEntry >                  m_aUsersData;

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

    std::vector< LockFileEntry > GetUsersData();
    void SetUsersDataAndStore( const std::vector< LockFileEntry >& aUserNames );
    LockFileEntry InsertOwnEntry();
    bool HasOwnEntry();
    void RemoveEntry( const LockFileEntry& aOptionalSpecification );
    void RemoveEntry();
    void RemoveFile();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
