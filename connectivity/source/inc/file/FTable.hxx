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

#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#define _CONNECTIVITY_FILE_TABLE_HXX_

#include "connectivity/sdbcx/VTable.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "file/FConnection.hxx"
#include "file/filedllapi.hxx"
#include <tools/stream.hxx>
#include "connectivity/FValue.hxx"
#include "TResultSetHelper.hxx"

namespace connectivity
{
    namespace file
    {
        typedef connectivity::sdbcx::OTable OTable_TYPEDEF;

        class OOO_DLLPUBLIC_FILE OFileTable :   public OTable_TYPEDEF
        {
        protected:
            OConnection*                                        m_pConnection;
            SvStream*                                           m_pFileStream;
            ::rtl::Reference<OSQLColumns>                           m_aColumns;
            sal_Int32                                           m_nFilePos;                 // current IResultSetHelper::Movement
            sal_uInt8*                                          m_pBuffer;
            sal_uInt16                                          m_nBufferSize;  // size of the ReadBuffer, if pBuffer != NULL
            sal_Bool                                            m_bWriteable;   // svstream cann't say if we are writeable
                                                                                // so we have to

            virtual void FileClose();
            virtual ~OFileTable( );
        public:
            virtual void refreshColumns();
            virtual void refreshKeys();
            virtual void refreshIndexes();
        public:
            OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection);
            OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName = OUString(),
                    const OUString& _CatalogName = OUString()
                );

            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);

            OConnection* getConnection() const { return m_pConnection;}
            virtual sal_Int32 getCurrentLastPos() const {return -1;}

            virtual sal_Bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) = 0;
            virtual sal_Bool fetchRow(OValueRefRow& _rRow,const OSQLColumns& _rCols, sal_Bool _bUseTableDefs,sal_Bool bRetrieveData) = 0;

            ::rtl::Reference<OSQLColumns> getTableColumns() const {return m_aColumns;}
            virtual sal_Bool InsertRow(OValueRefVector& rRow, sal_Bool bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual sal_Bool DeleteRow(const OSQLColumns& _rCols);
            virtual sal_Bool UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual void addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor);
            virtual void dropColumn(sal_Int32 _nPos);
            // refresh the header of file based tables to see changes done by someone
            virtual void refreshHeader();

            OUString SAL_CALL getName() throw() { return m_Name; }

            OUString getSchema() { return m_SchemaName; }
            sal_Bool isReadOnly() const { return !m_bWriteable; }
                // m_pFileStream && !m_pFileStream->IsWritable(); }
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();


            sal_Int32 getFilePos() const { return m_nFilePos; }

        public:
            // helper

            // creates a stream using ::utl::UcbStreamHelper::CreateStream, but the error is simplified
            // (NULL or non-NULL is returned)
            static SvStream* createStream_simpleError( const OUString& _rFileName, StreamMode _eOpenMode);
        };
    }
}
#endif // _CONNECTIVITY_FILE_TABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
