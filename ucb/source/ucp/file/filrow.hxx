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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILROW_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILROW_HXX

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <cppuhelper/implbase.hxx>

namespace fileaccess {

    class shell;

    class XRow_impl: public cppu::WeakImplHelper<
        css::sdbc::XRow >
    {
    public:
        XRow_impl( shell* pShell,const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& __m_aValueMap );
        virtual ~XRow_impl();

        virtual sal_Bool SAL_CALL
        wasNull(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual OUString SAL_CALL
        getString(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL
        getBoolean(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Int8 SAL_CALL
        getByte(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Int16 SAL_CALL
        getShort(
            sal_Int32 columnIndex )
            throw(
                com::sun::star::sdbc::SQLException,
                com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual sal_Int32 SAL_CALL
        getInt(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual sal_Int64 SAL_CALL
        getLong(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual float SAL_CALL
        getFloat(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual double SAL_CALL
        getDouble(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getBytes(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::util::Date SAL_CALL
        getDate(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::util::Time SAL_CALL
        getTime(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::util::DateTime SAL_CALL
        getTimestamp(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getBinaryStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getCharacterStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Any SAL_CALL
        getObject(
            sal_Int32 columnIndex,
            const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& typeMap )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRef > SAL_CALL
        getRef(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XBlob > SAL_CALL
        getBlob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XClob > SAL_CALL
        getClob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XArray > SAL_CALL
        getArray(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        osl::Mutex                                                              m_aMutex;
        com::sun::star::uno::Sequence< com::sun::star::uno::Any >                m_aValueMap;
        bool                                                                 m_nWasNull;
        shell*                                                                   m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >  m_xProvider;
        com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > m_xTypeConverter;
    };

} // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
