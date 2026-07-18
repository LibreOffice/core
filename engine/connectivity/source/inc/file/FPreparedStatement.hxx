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

#include <file/filedllapi.hxx>
#include <file/FStatement.hxx>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <file/FResultSet.hxx>

namespace connectivity::file
{
    class OResultSetMetaData;

    class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE OPreparedStatement : public  OStatement_BASE2,
                                public  css::sdbc::XPreparedStatement,
                                public  css::sdbc::XParameters,
                                public  css::sdbc::XResultSetMetaDataSupplier,
                                public  css::lang::XServiceInfo

    {
    protected:

        // Data attributes

        OValueRefRow                                          m_aParameterRow;
        rtl::Reference<OResultSetMetaData>                    m_xMetaData;

        ::rtl::Reference<connectivity::OSQLColumns>           m_xParamColumns;    // the parameter columns

        // factory method for resultset's
        virtual rtl::Reference<OResultSet> createResultSet() override;
        ::rtl::Reference< OResultSet > makeResultSet();
        void initResultSet(OResultSet*);

        void checkAndResizeParameters(sal_Int32 parameterIndex);
        void setParameter(sal_Int32 parameterIndex, const ORowSetValue& x);

        sal_uInt32 AddParameter(connectivity::OSQLParseNode const * pParameter,
                            const css::uno::Reference< css::beans::XPropertySet>& _xCol);
        void scanParameter(OSQLParseNode* pParseNode,std::vector< OSQLParseNode*>& _rParaNodes);
        void describeColumn(OSQLParseNode const * _pParameter, OSQLParseNode const * _pNode, const OSQLTable& _xTable);
        void describeParameter();

        virtual void parseParamterElem(const OUString& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem) override;
        virtual void initializeResultSet(OResultSet* _pResult) override;

        virtual ~OPreparedStatement() override;
    public:
        DECLARE_SERVICE_INFO();
        // a Constructor, that is needed for when Returning the Object is needed:
        OPreparedStatement( OConnection* _pConnection);

        virtual void construct(const OUString& sql) override;

        // OComponentHelper
        virtual void disposing() override;
        //XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

        // XPreparedStatement
        virtual css::uno::Reference< css::sdbc::XResultSet > executeQuery(  ) override;
        virtual sal_Int32 executeUpdate(  ) override;
        virtual bool execute(  ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;
        // XParameters
        virtual void setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
        virtual void setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
        virtual void setBoolean( sal_Int32 parameterIndex, bool x ) override;
        virtual void setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
        virtual void setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
        virtual void setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
        virtual void setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
        virtual void setFloat( sal_Int32 parameterIndex, float x ) override;
        virtual void setDouble( sal_Int32 parameterIndex, double x ) override;
        virtual void setString( sal_Int32 parameterIndex, const OUString& x ) override;
        virtual void setBytes( sal_Int32 parameterIndex, const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
        virtual void setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
        virtual void setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
        virtual void setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setObject( sal_Int32 parameterIndex, const cpo::uno::Any& x ) override;
        virtual void setObjectWithInfo( sal_Int32 parameterIndex, const cpo::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) override;
        virtual void setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) override;
        virtual void setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) override;
        virtual void setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) override;
        virtual void setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) override;
        virtual void clearParameters(  ) override;
        // XCloseable
                    virtual void close(  ) override;
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;

        const rtl::Reference<OResultSetMetaData> & getMetaDataImpl();
    };

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
