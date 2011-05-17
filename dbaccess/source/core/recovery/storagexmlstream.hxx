/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef STORAGEXMLSTREAM_HXX
#define STORAGEXMLSTREAM_HXX

#include "storagestream.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
/** === end UNO includes === **/

#include <memory>

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= StorageXMLOutputStream
    //====================================================================
    struct StorageXMLOutputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLOutputStream : public StorageOutputStream
    {
    public:
        StorageXMLOutputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        ~StorageXMLOutputStream();

        // StorageOutputStream overridables
        virtual void close();

        void    addAttribute( const ::rtl::OUString& i_rName, const ::rtl::OUString& i_rValue ) const;

        void    startElement( const ::rtl::OUString& i_rElementName ) const;
        void    endElement() const;

        void    ignorableWhitespace( const ::rtl::OUString& i_rWhitespace ) const;
        void    characters( const ::rtl::OUString& i_rCharacters ) const;

    private:
        StorageXMLOutputStream();                                           // never implemented
        StorageXMLOutputStream( const StorageXMLOutputStream& );            // never implemented
        StorageXMLOutputStream& operator=( const StorageXMLOutputStream& ); // never implemented

    private:
        ::std::auto_ptr< StorageXMLOutputStream_Data >   m_pData;
    };

    //====================================================================
    //= StorageXMLInputStream
    //====================================================================
    struct StorageXMLInputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLInputStream : public StorageInputStream
    {
    public:
        StorageXMLInputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
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

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // STORAGEXMLSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
