/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#define _CONNECTIVITY_DBASE_TABLE_HXX_

#include "file/FTable.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/CommonTools.hxx"
#include <tools/urlobj.hxx>


namespace connectivity
{
    namespace dbase
    {
        typedef file::OFileTable ODbaseTable_BASE;
        class ODbaseConnection;

        typedef ::std::map< ::rtl::OUString,
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed>, comphelper::UStringMixLess > OContainer;

        class ODbaseTable : public ODbaseTable_BASE
        {
            // der Typ einer dBase datei wird mit dem ersten Byte bestimmt
        public:
            enum DBFType  { dBaseIII         = 0x03,
                            dBaseIV          = 0x04,
                            dBaseV           = 0x05,
                            VisualFoxPro     = 0x30,
                            VisualFoxProAuto = 0x31, // Visual FoxPro w. AutoIncrement field
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
            struct DBFHeader {                       /* Kopfsatz-Struktur            */
                                DBFType db_typ;                         /* Dateityp                     */
                                sal_uInt8    db_aedat[3];                    /* Datum der letzen Aenderung   */
                                                                        /* JJ MM TT                     */
                                sal_uInt32   db_anz;                         /* Anzahl der Saetze            */
                                sal_uInt16  db_kopf;                        /* laenge Kopfsatz-Struktur     */
                                sal_uInt16  db_slng;                        /* laenge der Daten-Saetze      */
                                sal_uInt8    db_frei[20];                    /* reserviert                   */
                            };
            struct DBFColumn {                       /* Feldbezeichner               */
                                sal_uInt8    db_fnm[11];                     /* Feldname                     */
                                sal_uInt8    db_typ;                         /* Feldtyp                      */
                                sal_uInt32  db_adr;                         /* Feldadresse                  */
                                sal_uInt8    db_flng;                        /* Feldlaenge                   */
                                sal_uInt8    db_dez;                         /* Dezimalstellen fuer N        */
                                sal_uInt8    db_frei2[14];                   /* reserviert                   */
                            };
            struct DBFMemoHeader
                            {
                                DBFMemoType db_typ;                     /* Dateityp                     */
                                sal_uInt32  db_next;                        /* naechster freier Block       */
                                sal_uInt16  db_size;                        /* Blockgroesse: dBase 3 fest   */
                            };

            ::std::vector<sal_Int32> m_aTypes;      // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32> m_aPrecisions; // same as aboth
            ::std::vector<sal_Int32> m_aScales;
            ::std::vector<sal_Int32> m_aRealFieldLengths;
            DBFHeader       m_aHeader;
            DBFMemoHeader   m_aMemoHeader;
            SvStream*       m_pMemoStream;
            rtl_TextEncoding m_eEncoding;
            sal_Bool        m_bWriteableMemo;

            void alterColumn(sal_Int32 index,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor ,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XDataDescriptorFactory>& xOldColumn );
            void readHeader();
            void fillColumns();
            String createTempFile();
            void copyData(ODbaseTable* _pNewTable,sal_Int32 _nPos);
            sal_Bool CreateFile(const INetURLObject& aFile, sal_Bool& bCreateMemo);
            sal_Bool CreateMemoFile(const INetURLObject& aFile);
            sal_Bool HasMemoFields() const { return m_aHeader.db_typ > dBaseIV;}
            sal_Bool ReadMemoHeader();
            sal_Bool ReadMemo(sal_uIntPtr nBlockNo, ORowSetValue& aVariable);

            sal_Bool WriteMemo(ORowSetValue& aVariable, sal_uIntPtr& rBlockNr);
            sal_Bool WriteBuffer();
            sal_Bool UpdateBuffer(OValueRefVector& rRow, OValueRefRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> isUniqueByColumnName(sal_Int32 _nColumnPos);
            void AllocBuffer();

            void throwInvalidDbaseFormat();
            void SAL_CALL renameImpl( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
            void throwInvalidColumnType(const sal_uInt16 _nErrorId,const ::rtl::OUString& _sColumnName);

        protected:
            virtual void FileClose();
//          using ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper;

        public:
            virtual void refreshColumns();
            virtual void refreshIndexes();

        public:
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection);
            ODbaseTable( sdbcx::OCollection* _pTables,ODbaseConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            void construct(); // can throw any exception

            virtual sal_Int32 getCurrentLastPos() const;
            virtual sal_Bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos);
            virtual sal_Bool fetchRow(OValueRefRow& _rRow,const OSQLColumns& _rCols, sal_Bool _bUseTableDefs,sal_Bool bRetrieveData);

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing(void);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
            // XRename
            virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

            sal_Bool    DropImpl();
            sal_Bool    CreateImpl();


            virtual sal_Bool InsertRow(OValueRefVector& rRow, sal_Bool bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual sal_Bool DeleteRow(const OSQLColumns& _rCols);
            virtual sal_Bool UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);

            virtual void addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor);
            virtual void dropColumn(sal_Int32 _nPos);

            static String   getEntry(file::OConnection* _pConnection,const ::rtl::OUString& _sURL );
            static sal_Bool     Drop_Static(const ::rtl::OUString& _sUrl,sal_Bool _bHasMemoFields,sdbcx::OCollection* _pIndexes );

            virtual void refreshHeader();

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const;
        };
    }
}
#endif // _CONNECTIVITY_DBASE_TABLE_HXX_

