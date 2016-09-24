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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_PRIVATEROW_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_PRIVATEROW_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include "RowSetRow.hxx"

namespace dbaccess
{
    class OPrivateRow : public ::cppu::WeakImplHelper< css::sdbc::XRow>
    {
        ORowSetValueVector::Vector m_aRow;
        sal_Int32 m_nPos;
    public:
        explicit OPrivateRow(const ORowSetValueVector::Vector& i_aRow) : m_aRow(i_aRow),m_nPos(0)
        {
        }
        virtual sal_Bool SAL_CALL wasNull(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getString( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getBoolean( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int8 SAL_CALL getByte( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getShort( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getInt( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int64 SAL_CALL getLong( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual float SAL_CALL getFloat( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getDouble( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getBytes( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Date SAL_CALL getDate( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Time SAL_CALL getTime( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::DateTime SAL_CALL getTimestamp( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getObject( ::sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( ::sal_Int32 columnIndex ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    };
} // dbaccess
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_PRIVATEROW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
