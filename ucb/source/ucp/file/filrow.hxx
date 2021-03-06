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
#pragma once

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase.hxx>

namespace fileaccess {

    class TaskManager;

    class XRow_impl: public cppu::WeakImplHelper<
        css::sdbc::XRow >
    {
    public:
        XRow_impl( TaskManager* pShell,const css::uno::Sequence< css::uno::Any >& aValueMap );
        virtual ~XRow_impl() override;

        virtual sal_Bool SAL_CALL
        wasNull() override;

        virtual OUString SAL_CALL
        getString( sal_Int32 columnIndex ) override;

        virtual sal_Bool SAL_CALL
        getBoolean( sal_Int32 columnIndex ) override;

        virtual sal_Int8 SAL_CALL
        getByte( sal_Int32 columnIndex ) override;

        virtual sal_Int16 SAL_CALL
        getShort( sal_Int32 columnIndex ) override;

        virtual sal_Int32 SAL_CALL
        getInt( sal_Int32 columnIndex ) override;

        virtual sal_Int64 SAL_CALL
        getLong( sal_Int32 columnIndex ) override;

        virtual float SAL_CALL
        getFloat( sal_Int32 columnIndex ) override;

        virtual double SAL_CALL
        getDouble(
            sal_Int32 columnIndex ) override;

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getBytes( sal_Int32 columnIndex ) override;

        virtual css::util::Date SAL_CALL
        getDate( sal_Int32 columnIndex ) override;

        virtual css::util::Time SAL_CALL
        getTime( sal_Int32 columnIndex ) override;

        virtual css::util::DateTime SAL_CALL
        getTimestamp( sal_Int32 columnIndex ) override;

        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
        getBinaryStream( sal_Int32 columnIndex ) override;

        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
        getCharacterStream( sal_Int32 columnIndex ) override;

        virtual css::uno::Any SAL_CALL
        getObject(
            sal_Int32 columnIndex,
            const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL
        getRef( sal_Int32 columnIndex ) override;

        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL
        getBlob( sal_Int32 columnIndex ) override;

        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL
        getClob( sal_Int32 columnIndex ) override;

        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL
        getArray( sal_Int32 columnIndex ) override;

    private:
        osl::Mutex                                         m_aMutex;
        css::uno::Sequence< css::uno::Any >                m_aValueMap;
        bool                                               m_nWasNull;
        TaskManager*                                       m_pMyShell;
        css::uno::Reference< css::script::XTypeConverter > m_xTypeConverter;

        bool isIndexOutOfBounds( sal_Int32 nIndex );
        template<typename T>
        T getValue(sal_Int32 columnIndex);
    };

} // end namespace fileaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
