/*************************************************************************
 *
 *  $RCSfile: DTable.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-01 10:52:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#define _CONNECTIVITY_DBASE_TABLE_HXX_

#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif


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
            enum DBFType  { dBaseIII = 0x03,
                            dBaseIV  = 0x04,
                            dBaseV   = 0x05,
                            dBaseFS  = 0x43,
                            dBaseFSMemo  = 0xB3,
                            dBaseIIIMemo = 0x83,
                            dBaseIVMemo = 0x8B,
                            dBaseIVMemoSQL = 0x8E,
                            FoxProMemo = 0xF5
                          };
            enum DBFMemoType {  MemodBaseIII = 0,
                                MemodBaseIV,
                                MemoFoxPro
                            };

        private:
            struct DBFHeader {                       /* Kopfsatz-Struktur            */
                                DBFType db_typ;                         /* Dateityp                     */
                                BYTE    db_aedat[3];                    /* Datum der letzen Aenderung   */
                                                                        /* JJ MM TT                     */
                                ULONG   db_anz;                         /* Anzahl der Saetze            */
                                USHORT  db_kopf;                        /* laenge Kopfsatz-Struktur     */
                                USHORT  db_slng;                        /* laenge der Daten-Saetze      */
                                BYTE    db_frei[20];                    /* reserviert                   */
                            };
            struct DBFColumn {                       /* Feldbezeichner               */
                                BYTE    db_fnm[11];                     /* Feldname                     */
                                BYTE    db_typ;                         /* Feldtyp                      */
                                ULONG   db_adr;                         /* Feldadresse                  */
                                BYTE    db_flng;                        /* Feldlaenge                   */
                                BYTE    db_dez;                         /* Dezimalstellen fuer N        */
                                BYTE    db_frei2[14];                   /* reserviert                   */
                            };
            struct DBFMemoHeader
                            {
                                DBFMemoType db_typ;                     /* Dateityp                     */
                                UINT32  db_next;                        /* nächster freier Block        */
                                USHORT  db_size;                        /* Blockgröße: dBase 3 fest     */
                            };

            ::std::vector<sal_Int32> m_aTypes;      // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32> m_aPrecisions; // same as aboth
            ::std::vector<sal_Int32> m_aScales;
            DBFHeader       m_aHeader;
            DBFMemoHeader   m_aMemoHeader;
            SvStream*       m_pMemoStream;
            sal_Bool        m_bWriteableMemo;

            void readHeader();
            void fillColumns();
            BOOL CreateFile(const INetURLObject& aFile, BOOL& bCreateMemo);
            BOOL CreateMemoFile(const INetURLObject& aFile);
            BOOL HasMemoFields() const { return m_aHeader.db_typ > dBaseIV;}
            BOOL ReadMemoHeader();
            BOOL ReadMemo(ULONG nBlockNo, file::ORowSetValue& aVariable);

            BOOL WriteMemo(file::ORowSetValue& aVariable, ULONG& rBlockNr);
            BOOL WriteBuffer();
            BOOL UpdateBuffer(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> isUniqueByColumnName(const ::rtl::OUString& _rColName);
            void AllocBuffer();

        protected:
            virtual void FileClose();
        public:
            virtual void refreshColumns();
            virtual void refreshIndexes();

        public:
            //  DECLARE_CTY_DEFAULTS( ODbaseTable_BASE);
            ODbaseTable( ODbaseConnection* _pConnection);
            ODbaseTable( ODbaseConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            void construct(); // can throw any exception

            virtual sal_Int32 getCurrentLastPos() const {return m_aHeader.db_anz;}
            virtual sal_Bool seekRow(FilePosition eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos);
            virtual sal_Bool fetchRow(file::OValueRow _rRow,const OSQLColumns& _rCols, sal_Bool _bUseTableDefs,sal_Bool bRetrieveData);

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing(void);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            BOOL DropImpl();
            BOOL CreateImpl();
            String      getEntry();
            String      getInvalidityMessage() const { return m_sInvalidityMessage; }

            virtual BOOL InsertRow(file::OValueVector& rRow, BOOL bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual BOOL DeleteRow(const OSQLColumns& _rCols);
            virtual BOOL UpdateRow(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
        };
    }
}
#endif // _CONNECTIVITY_DBASE_TABLE_HXX_

