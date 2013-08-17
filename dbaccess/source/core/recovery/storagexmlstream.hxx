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

#ifndef STORAGEXMLSTREAM_HXX
#define STORAGEXMLSTREAM_HXX

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        ~StorageXMLOutputStream();

        // StorageOutputStream overridables
        virtual void close();

        void    addAttribute( const OUString& i_rName, const OUString& i_rValue ) const;

        void    startElement( const OUString& i_rElementName ) const;
        void    endElement() const;

        void    ignorableWhitespace( const OUString& i_rWhitespace ) const;
        void    characters( const OUString& i_rCharacters ) const;

    private:
        StorageXMLOutputStream();                                           // never implemented
        StorageXMLOutputStream( const StorageXMLOutputStream& );            // never implemented
        StorageXMLOutputStream& operator=( const StorageXMLOutputStream& ); // never implemented

    private:
        ::std::auto_ptr< StorageXMLOutputStream_Data >   m_pData;
    };

    // StorageXMLInputStream
    struct StorageXMLInputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLInputStream : public StorageInputStream
    {
    public:
        StorageXMLInputStream(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        ~StorageXMLInputStream();

        void    import(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& i_rHandler
                );

    private:
        StorageXMLInputStream();                                            // never implemented
        StorageXMLInputStream( const StorageXMLInputStream& );              // never implemented
        StorageXMLInputStream& operator=( const StorageXMLInputStream& );   // never implemented

    private:
        ::std::auto_ptr< StorageXMLInputStream_Data >   m_pData;
    };

} // namespace dbaccess

#endif // STORAGEXMLSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
