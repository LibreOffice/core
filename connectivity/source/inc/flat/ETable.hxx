/*************************************************************************
 *
 *  $RCSfile: ETable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-27 07:59:34 $
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

#ifndef _CONNECTIVITY_FLAT_TABLE_HXX_
#define _CONNECTIVITY_FLAT_TABLE_HXX_

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
    namespace flat
    {
        //==================================================================
        // Ableitung von String mit ueberladenen GetToken/GetTokenCount-Methoden
        // Speziell fuer FLAT FILE-Format: Strings koennen gequotet sein
        //==================================================================
        class OFlatString : public ByteString
        {
        public:
            OFlatString(){}

            xub_StrLen  GetTokenCount( sal_uInt8 cTok = ';', sal_uInt8 cStrDel = '\0' ) const;
            ByteString  GetToken( xub_StrLen nToken, sal_uInt8 cTok = ';', sal_uInt8 cStrDel = '\0' ) const;
        };


        typedef file::OFileTable OFlatTable_BASE;
        class OFlatConnection;

        typedef ::std::map< ::rtl::OUString,
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed>, comphelper::UStringMixLess > OContainer;

        class OFlatTable :  public OFlatTable_BASE
        {
            sal_Int32 m_nRowPos;
            sal_Int32 m_nMaxRowCount; // will be set if stream is once eof
            // maps a row postion to a file position
            ::std::map<sal_Int32,sal_Int32> m_aRowToFilePos;
            OFlatString m_aCurrentLine;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > m_xNumberFormatter;
        public:

        private:
            void fillColumns();
            BOOL CreateFile(const INetURLObject& aFile, BOOL& bCreateMemo);

            BOOL WriteBuffer();
            BOOL UpdateBuffer(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet> isUniqueByColumnName(const ::rtl::OUString& _rColName);
            void AllocBuffer();

            sal_Bool checkHeaderLine();
        public:
            virtual void refreshColumns();
            virtual void refreshIndexes();

        public:
            //  DECLARE_CTY_DEFAULTS( OFlatTable_BASE);
            OFlatTable( OFlatConnection* _pConnection);
            OFlatTable( OFlatConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            virtual sal_Bool seekRow(FilePosition eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos);
            virtual sal_Bool fetchRow(file::OValueRow _rRow,const OSQLColumns& _rCols, sal_Bool bIsTable,sal_Bool bRetrieveData);

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing(void);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            BOOL DropImpl();
            BOOL CreateImpl();
            String getEntry();

            virtual BOOL InsertRow(file::OValueVector& rRow, BOOL bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual BOOL DeleteRow(const OSQLColumns& _rCols);
            virtual BOOL UpdateRow(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
        };
    }
}
#endif // _CONNECTIVITY_FLAT_TABLE_HXX_

