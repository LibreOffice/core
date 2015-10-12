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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGEXMLSTREAM_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGEXMLSTREAM_HXX

#include "storagestream.hxx"

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <memory>

namespace dbaccess
{

    // StorageXMLOutputStream
    struct StorageXMLOutputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLOutputStream : public StorageOutputStream
    {
    public:
        StorageXMLOutputStream(
            const css::uno::Reference< css::uno::XComponentContext >& i_rContext,
            const css::uno::Reference< css::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        virtual ~StorageXMLOutputStream();

        // StorageOutputStream overridables
        virtual void close() SAL_OVERRIDE;

        void    addAttribute( const OUString& i_rName, const OUString& i_rValue ) const;

        void    startElement( const OUString& i_rElementName ) const;
        void    endElement() const;

        void    ignorableWhitespace( const OUString& i_rWhitespace ) const;
        void    characters( const OUString& i_rCharacters ) const;

    private:
        StorageXMLOutputStream( const StorageXMLOutputStream& ) = delete;
        StorageXMLOutputStream& operator=( const StorageXMLOutputStream& ) = delete;

    private:
        ::std::unique_ptr< StorageXMLOutputStream_Data >   m_pData;
    };

    // StorageXMLInputStream
    struct StorageXMLInputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLInputStream : public StorageInputStream
    {
    public:
        StorageXMLInputStream(
            const css::uno::Reference< css::uno::XComponentContext >& i_rContext,
            const css::uno::Reference< css::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        virtual ~StorageXMLInputStream();

        void    import(
                    const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_rHandler
                );

    private:
        StorageXMLInputStream( const StorageXMLInputStream& ) = delete;
        StorageXMLInputStream& operator=( const StorageXMLInputStream& ) = delete;

    private:
        ::std::unique_ptr< StorageXMLInputStream_Data >   m_pData;
    };

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGEXMLSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
