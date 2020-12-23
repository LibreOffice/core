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

#include "storagetextstream.hxx"

#include <com/sun/star/io/TextOutputStream.hpp>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::io::TextOutputStream;
    using ::com::sun::star::io::XTextOutputStream2;

    // StorageTextOutputStream_Data
    struct StorageTextOutputStream_Data
    {
        Reference< XTextOutputStream2 >  xTextOutput;
    };

    const OUStringLiteral sLineFeed = u"\n";

    // StorageTextOutputStream
    StorageTextOutputStream::StorageTextOutputStream(   const Reference<XComponentContext>& i_rContext,
                                                        const Reference< XStorage >& i_rParentStorage,
                                                        const OUString& i_rStreamName
                                                    )
        :StorageOutputStream( i_rParentStorage, i_rStreamName )
        ,m_pData( new StorageTextOutputStream_Data )
    {
        m_pData->xTextOutput = TextOutputStream::create( i_rContext );
        m_pData->xTextOutput->setEncoding( "UTF-8" );
        m_pData->xTextOutput->setOutputStream( getOutputStream() );
    }

    StorageTextOutputStream::~StorageTextOutputStream()
    {
    }

    void StorageTextOutputStream::writeLine( const OUString& i_rLine )
    {
        m_pData->xTextOutput->writeString( i_rLine );
        m_pData->xTextOutput->writeString( sLineFeed );
    }

    void StorageTextOutputStream::writeLine()
    {
        m_pData->xTextOutput->writeString( sLineFeed );
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
