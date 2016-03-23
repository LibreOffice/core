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
            struct DBFHeader {                       /* Header struct */
                                DBFType      db_typ;                        /* File type                    */
                                sal_uInt8    db_aedat[3];                   /* Date of last change          */
                                                                            /* YY MM DD                     */
                                sal_uInt32  db_anz;                         /* Data set count               */
                                sal_uInt16  db_kopf;                        /* Header length                */
                                sal_uInt16  db_slng;                        /* Data set length              */
                                sal_uInt8   db_frei[20];                    /* Reserved                     */
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

            ::std::vector<sal_Int32> m_aTypes;      // holds all types for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32> m_aPrecisions; // same as above
            ::std::vector<sal_Int32> m_aScales;
            ::std::vector<sal_Int32> m_aRealFieldLengths;
            DBFHeader       m_aHeader;
            DBFMemoHeader   m_aMemoHeader;
            SvStream*       m_pMemoStream;
            rtl_TextEncoding m_eEncoding;
            bool        m_bWriteableMemo;

            void alterColumn(sal_Int32 index,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor ,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XDataDescriptorFactory>& xOldColumn );
            void readHeader();
            void fillColumns();
            OUString createTempFile();
            void copyData(ODbaseTable* _pNewTable,sal_Int32 _nPos);
            bool CreateFile(const INetURLObject& aFile, bool& bCreateMemo);
            bool CreateMemoFile(const INetURLObject& aFile);
            bool HasMemoFields() const { return m_aHeader.db_typ > dBaseIV;}
            bool ReadMemoHeader();
            bool ReadMemo(sal_Size nBlockNo, ORowSetValue& aVariable);

            bool WriteMemo(const ORowSetValue& aVariable, sal_Size& rBlockNr);
            bool WriteBuffer();
            bool UpdateBuffer(OValueRefVector& rRow, OValueRefRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols, bool bForceAllFields);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> isUniqueByColumnName(sal_Int32 _nColumnPos);
            bool AllocBuffer();

            void throwInvalidDbaseFormat();
            void SAL_CALL renameImpl( const OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception);
            void throwInvalidColumnType(const sal_uInt16 _nErrorId,const OUString& _sColumnName);

        protected:
            virtual void FileClose() override;
//          using ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper;

        public:
            virtual void refreshColumns() override;
            virtual void refreshIndexes() override;

        public:
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection);
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName = OUString(),
                    const OUString& _CatalogName = OUString()
                );

            void construct() override; // can throw any exception

            virtual sal_Int32 getCurrentLastPos() const override;
            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) override;
            virtual bool fetchRow(OValueRefRow& _rRow,const OSQLColumns& _rCols, bool _bUseTableDefs, bool bRetrieveData) override;

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL disposing() override;

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XRename
            virtual void SAL_CALL rename( const OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            bool    DropImpl();
            bool    CreateImpl();


            virtual bool InsertRow(OValueRefVector& rRow, bool bFlush, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols) override;
            virtual bool DeleteRow(const OSQLColumns& _rCols) override;
            virtual bool UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols) override;

            virtual void addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor) override;
            virtual void dropColumn(sal_Int32 _nPos) override;

            static OUString   getEntry(file::OConnection* _pConnection,const OUString& _sURL );
            static bool     Drop_Static(const OUString& _sUrl, bool _bHasMemoFields, sdbcx::OCollection* _pIndexes );

            virtual void refreshHeader() override;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
