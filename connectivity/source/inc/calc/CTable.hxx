/*************************************************************************
 *
 *  $RCSfile: CTable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-21 11:34:47 $
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

#ifndef _CONNECTIVITY_CALC_TABLE_HXX_
#define _CONNECTIVITY_CALC_TABLE_HXX_

#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

namespace com { namespace sun { namespace star { namespace sheet {
    class XSpreadsheet;
} } } }

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormats;
} } } }


namespace connectivity
{
    namespace calc
    {
        typedef file::OFileTable OCalcTable_BASE;
        class OCalcConnection;

        typedef ::std::map< ::rtl::OUString,
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed>,
                        comphelper::UStringMixLess > OContainer;

        class OCalcTable :  public OCalcTable_BASE
        {
        private:
            ::std::vector<sal_Int32> m_aTypes;      // holds all type for columns just to avoid to ask the propertyset
            ::std::vector<sal_Int32> m_aPrecisions; // same as aboth
            ::std::vector<sal_Int32> m_aScales;
            ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > m_xSheet;
            sal_Int32 m_nStartCol;
            sal_Int32 m_nStartRow;
            sal_Int32 m_nDataCols;
            sal_Int32 m_nDataRows;
            sal_Bool m_bHasHeaders;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > m_xFormats;
            ::Date m_aNullDate;

            void fillColumns();

            BOOL WriteBuffer();
            BOOL UpdateBuffer(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> isUniqueByColumnName(const ::rtl::OUString& _rColName);

        protected:
            virtual void FileClose();
        public:
            virtual void refreshColumns();
            virtual void refreshIndexes();

        public:
            //  DECLARE_CTY_DEFAULTS( OCalcTable_BASE);
            OCalcTable( OCalcConnection* _pConnection);
            OCalcTable( OCalcConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            virtual sal_Int32 getCurrentLastPos() const;
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

            virtual BOOL InsertRow(file::OValueVector& rRow, BOOL bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
            virtual BOOL DeleteRow(const OSQLColumns& _rCols);
            virtual BOOL UpdateRow(file::OValueVector& rRow, file::OValueRow pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols);
        };
    }
}

#endif // _CONNECTIVITY_CALC_TABLE_HXX_

