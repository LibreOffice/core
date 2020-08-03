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

#include <connectivity/sdbcx/VTable.hxx>
#include <file/FConnection.hxx>
#include <file/filedllapi.hxx>
#include <tools/stream.hxx>
#include <connectivity/FValue.hxx>
#include <TResultSetHelper.hxx>

namespace connectivity::file
    {
        typedef connectivity::sdbcx::OTable OTable_TYPEDEF;

        class OOO_DLLPUBLIC_FILE OFileTable :   public OTable_TYPEDEF
        {
        protected:
            OConnection*                                        m_pConnection;
            std::unique_ptr<SvStream>                           m_pFileStream;
            ::rtl::Reference<OSQLColumns>                           m_aColumns;
            sal_Int32                                           m_nFilePos;                 // current IResultSetHelper::Movement
            std::unique_ptr<sal_uInt8[]>                        m_pBuffer;
            sal_uInt16                                          m_nBufferSize;  // size of the ReadBuffer, if pBuffer != NULL
            bool                                            m_bWriteable;   // svstream can't say if we are writeable
                                                                                // so we have to

            virtual void FileClose();
            virtual ~OFileTable( ) override;
        public:
            virtual void refreshColumns() override;
            virtual void refreshKeys() override;
            virtual void refreshIndexes() override;
        public:
            OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection);
            OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description,
                    const OUString& SchemaName,
                    const OUString& CatalogName
                );

            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;

            OConnection* getConnection() const { return m_pConnection;}
            virtual sal_Int32 getCurrentLastPos() const {return -1;}

            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) = 0;
            virtual bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, bool bRetrieveData) = 0;

            const ::rtl::Reference<OSQLColumns>& getTableColumns() const {return m_aColumns;}
            virtual bool InsertRow(OValueRefVector& rRow, const css::uno::Reference< css::container::XIndexAccess>& _xCols);
            virtual bool DeleteRow(const OSQLColumns& _rCols);
            virtual bool UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const css::uno::Reference< css::container::XIndexAccess>& _xCols);
            virtual void addColumn(const css::uno::Reference< css::beans::XPropertySet>& descriptor);
            virtual void dropColumn(sal_Int32 _nPos);
            // refresh the header of file based tables to see changes done by someone
            virtual void refreshHeader();

            OUString SAL_CALL getName() override { return m_Name; }

            const OUString& getSchema() const { return m_SchemaName; }
            bool isReadOnly() const { return !m_bWriteable; }
                // m_pFileStream && !m_pFileStream->IsWritable(); }
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();


            sal_Int32 getFilePos() const { return m_nFilePos; }

        public:
            // helper

            // creates a stream using ::utl::UcbStreamHelper::CreateStream, but the error is simplified
            // (NULL or non-NULL is returned)
            static std::unique_ptr<SvStream> createStream_simpleError( const OUString& _rFileName, StreamMode _eOpenMode);
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
