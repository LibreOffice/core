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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DTABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DTABLE_HXX

#include "file/FTable.hxx"
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/CommonTools.hxx>
#include <tools/urlobj.hxx>


namespace connectivity
{
    namespace dbase
    {
        typedef file::OFileTable ODbaseTable_BASE;
        class ODbaseConnection;

        class ODbaseTable : public ODbaseTable_BASE
        {
        // The first byte of a dBase file specifies its type
        public:
            enum DBFType  { dBaseIII         = 0x03,
                            dBaseIV          = 0x04,
                            dBaseV           = 0x05,
                            VisualFoxPro     = 0x30,
                            VisualFoxProAuto = 0x31, // Visual FoxPro with AutoIncrement field
                            dBaseFS          = 0x43,
                            dBaseFSMemo      = 0xB3,
                            dBaseIIIMemo     = 0x83,
                            dBaseIVMemo      = 0x8B,
                            dBaseIVMemoSQL   = 0x8E,
                            FoxProMemo       = 0xF5
                          };
            enum DBFMemoType {  MemodBaseIII = 0,
                                MemodBaseIV,
                                MemoFoxPro
                            };

        private:
            // sources: https://www.clicketyclick.dk/databases/xbase/format/dbf.html (dBASE III and 5)
            // http://www.dbase.com/KnowledgeBase/int/db7_file_fmt.htm (dBASE 7) which is similar at least for this part
            struct DBFHeader {                                                                   //   address/pos in trailer
                                DBFType     type;                      // dBASE/xBASE type, see DBFType 00h
                                sal_uInt8   dateElems[3];              // Date of last change (YYMMDD)  01h
                                sal_uInt32  nbRecords;                 // Number of records             04h
                                sal_uInt16  headerLength;              //                               08h
                                sal_uInt16  recordLength;              // Length of 1 record            10h
                                sal_uInt8   trailer[20];
                                // this last field contains these data:
                                // - reserved:2 bytes:should be filled with 0                           12h/0
                                // - incomplete transaction:1 byte:dBASE IV                             14h/2
                                                 // 00h Transaction ended (or rolled back)
                                                 // 01h Transaction started
                                // - encryptionFlag:1 byte: dBASE IV                                    15h/3
                                                 // 00h not encrypted
                                                 // 01h for encrypted
                                // - freeRecordThread:4 bytes:reserved for LAN only                     16h/4
                                // - multiUserdBASE:8 bytes:reserved for multi-user dBASE (dBASE III+)  20h/8
                                // - MDXFlag:1 byte:dBASE IV                                            28h/16
                                                 // 0x01 if a production .MDX file exists for this table
                                                 // 0x00 if no .MDX file exists
                                // - languageDriver:1 byte:codepage (from Foxpro)                       29h/17
                                // - reserved:2 bytes: should be filled with 0                          30h/18
                            };
            struct DBFColumn {                       /* Column descriptors */
                                sal_uInt8    db_fnm[11];                     /* Field name                  */
                                sal_uInt8    db_typ;                         /* Field type                  */
                                sal_uInt32   db_adr;                         /* Field address               */
                                sal_uInt8    db_flng;                        /* Field length                */
                                sal_uInt8    db_dez;                         /* Decimal places for N        */
                                sal_uInt8    db_frei2[14];                   /* Reserved                    */
                            };
            struct DBFMemoHeader
            {
                DBFMemoType db_typ;                         /* File type                    */
                sal_uInt32  db_next;                        /* Next free block              */
                sal_uInt16  db_size;                        /* Block size: dBase 3 fixed    */
                DBFMemoHeader()
                    : db_typ(MemodBaseIII)
                    , db_next(0)
                    , db_size(0)
                {
                }
            };

            std::vector<sal_Int32> m_aTypes;      // holds all types for columns just to avoid to ask the propertyset
            std::vector<sal_Int32> m_aPrecisions; // same as above
            std::vector<sal_Int32> m_aScales;
            std::vector<sal_Int32> m_aRealFieldLengths;
            DBFHeader       m_aHeader;
            DBFMemoHeader   m_aMemoHeader;
            SvStream*       m_pMemoStream;
            rtl_TextEncoding m_eEncoding;

            void alterColumn(sal_Int32 index,
                             const css::uno::Reference< css::beans::XPropertySet>& descriptor ,
                             const css::uno::Reference< css::sdbcx::XDataDescriptorFactory>& xOldColumn );
            void readHeader();
            void fillColumns();
            OUString createTempFile();
            void copyData(ODbaseTable* _pNewTable,sal_Int32 _nPos);
            bool CreateFile(const INetURLObject& aFile, bool& bCreateMemo);
            bool CreateMemoFile(const INetURLObject& aFile);
            bool HasMemoFields() const { return m_aHeader.type > dBaseIV;}
            bool ReadMemoHeader();
            bool ReadMemo(std::size_t nBlockNo, ORowSetValue& aVariable);

            bool WriteMemo(const ORowSetValue& aVariable, std::size_t& rBlockNr);
            bool WriteBuffer();
            bool UpdateBuffer(OValueRefVector& rRow, const OValueRefRow& pOrgRow, const css::uno::Reference< css::container::XIndexAccess>& _xCols, bool bForceAllFields);
            css::uno::Reference< css::beans::XPropertySet> isUniqueByColumnName(sal_Int32 _nColumnPos);
            bool AllocBuffer();

            void throwInvalidDbaseFormat();
            /// @throws css::sdbc::SQLException
            /// @throws css::container::ElementExistException
            /// @throws css::uno::RuntimeException
            void SAL_CALL renameImpl( const OUString& newName );
            void throwInvalidColumnType(const char* pErrorId, const OUString& _sColumnName);

        protected:
            virtual void FileClose() override;
//          using ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper;

        public:
            virtual void refreshColumns() override;
            virtual void refreshIndexes() override;

        public:
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection);
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description = OUString(),
                    const OUString& SchemaName = OUString(),
                    const OUString& CatalogName = OUString()
                );

            void construct() override; // can throw any exception

            virtual sal_Int32 getCurrentLastPos() const override;
            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) override;
            virtual bool fetchRow(OValueRefRow& _rRow,const OSQLColumns& _rCols, bool bRetrieveData) override;

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            virtual void SAL_CALL disposing() override;

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
            // XRename
            virtual void SAL_CALL rename( const OUString& newName ) override;

            bool    DropImpl();
            bool    CreateImpl();


            virtual bool InsertRow(OValueRefVector& rRow, const css::uno::Reference< css::container::XIndexAccess>& _xCols) override;
            virtual bool DeleteRow(const OSQLColumns& _rCols) override;
            virtual bool UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const css::uno::Reference< css::container::XIndexAccess>& _xCols) override;

            virtual void addColumn(const css::uno::Reference< css::beans::XPropertySet>& descriptor) override;
            virtual void dropColumn(sal_Int32 _nPos) override;

            static OUString   getEntry(file::OConnection* _pConnection,const OUString& _sURL );
            static bool     Drop_Static(const OUString& _sUrl, bool _bHasMemoFields, sdbcx::OCollection* _pIndexes );

            virtual void refreshHeader() override;

            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
