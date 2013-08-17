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

#include "storagestream.hxx"

#include <com/sun/star/embed/ElementModes.hpp>

#include <tools/diagnose_ex.h>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::io::XStream;

    namespace ElementModes = ::com::sun::star::embed::ElementModes;

    // StorageOutputStream
    StorageOutputStream::StorageOutputStream(   const Reference<XComponentContext>& i_rContext,
                                                const Reference< XStorage >& i_rParentStorage,
                                                const OUString& i_rStreamName
                                             )
        :m_rContext( i_rContext )
    {
        ENSURE_OR_THROW( i_rParentStorage.is(), "illegal stream" );

        const Reference< XStream > xStream(
            i_rParentStorage->openStreamElement( i_rStreamName, ElementModes::READWRITE ), UNO_QUERY_THROW );
        m_xOutputStream.set( xStream->getOutputStream(), UNO_SET_THROW );
    }

    StorageOutputStream::~StorageOutputStream()
    {
    }

    void StorageOutputStream::close()
    {
        ENSURE_OR_RETURN_VOID( m_xOutputStream.is(), "already closed" );
        m_xOutputStream->closeOutput();
        m_xOutputStream.clear();

        // if you add additional functionality here, be aware that there are derived classes which
        // (legitimately) do not call this method here.
    }

    // StorageInputStream
    StorageInputStream::StorageInputStream( const Reference<XComponentContext>& i_rContext,
                                            const Reference< XStorage >& i_rParentStorage,
                                            const OUString& i_rStreamName
                                          )
        :m_rContext( i_rContext )
    {
        ENSURE_OR_THROW( i_rParentStorage.is(), "illegal stream" );

        const Reference< XStream > xStream(
            i_rParentStorage->openStreamElement( i_rStreamName, ElementModes::READ ), UNO_QUERY_THROW );
        m_xInputStream.set( xStream->getInputStream(), UNO_SET_THROW );
    }

    StorageInputStream::~StorageInputStream()
    {
    }

    void StorageInputStream::close()
    {
        ENSURE_OR_RETURN_VOID( m_xInputStream.is(), "already closed" );
        m_xInputStream->closeInput();
        m_xInputStream.clear();

        // if you add additional functionality here, be aware that there are derived classes which
        // (legitimately) do not call this method here.
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
