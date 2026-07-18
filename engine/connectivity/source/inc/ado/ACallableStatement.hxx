/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <ado/APreparedStatement.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XOutParameters.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity::ado
{

    //************ Class: java.sql.CallableStatement


    class OCallableStatement :  public OPreparedStatement,
                                public css::sdbc::XRow,
                                public css::sdbc::XOutParameters
    {
        OLEVariant          m_aValue;
    public:
        DECLARE_SERVICE_INFO();

        // a Constructor, that is needed for when Returning the Object is needed:
        OCallableStatement( OConnection* _pConnection, const OUString& sql );

        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XRow
        virtual bool wasNull(  ) override;
        virtual OUString getString( sal_Int32 columnIndex ) override;
        virtual bool getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) override;
        virtual float getFloat( sal_Int32 columnIndex ) override;
        virtual double getDouble( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) override;
        // XOutParameters
        virtual void registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
        virtual void registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) override;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
