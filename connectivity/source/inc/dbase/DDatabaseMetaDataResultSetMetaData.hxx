/*************************************************************************
 *
 *  $RCSfile: DDatabaseMetaDataResultSetMetaData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-28 11:28:46 $
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

#ifndef _CONNECTIVITY_FILE_ADATABASEMETADATARESULTSETMETADATA_HXX_
#define _CONNECTIVITY_FILE_ADATABASEMETADATARESULTSETMETADATA_HXX_

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_COLUMN_HXX_
#include "OColumn.hxx"
#endif

namespace connectivity
{
    namespace file
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************
                typedef ::cppu::WeakImplHelper1<        ::com::sun::star::sdbc::XResultSetMetaData>   ODatabaseMetaResultSetMetaData_BASE;

        class ODatabaseMetaDataResultSetMetaData :  public  ODatabaseMetaResultSetMetaData_BASE
        {
            friend class ODatabaseMetaDataResultSet;

            ::std::vector<sal_Int32> m_vMapping; // when not every column is needed
            ::std::map<sal_Int32,connectivity::OColumn> m_mColumns;
            ::std::map<sal_Int32,connectivity::OColumn>::const_iterator m_mColumnsIter;

            sal_Int32       m_nColCount;

        protected:
            void setColumnPrivilegesMap();
            void setColumnsMap();
            void setTablesMap();
            void setProcedureColumnsMap();
            void setPrimaryKeysMap();
            void setIndexInfoMap();
            void setTablePrivilegesMap();
            void setCrossReferenceMap();
            void setTypeInfoMap();
            void setProceduresMap();
            void setTableTypes();
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            ODatabaseMetaDataResultSetMetaData( ODatabaseMetaDataResultSet* _pRes)
                    :   m_nColCount(0)
            {
            }
            ~ODatabaseMetaDataResultSetMetaData();

            /// Avoid ambigous cast error from the compiler.
                        inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
            { return this; }

                        virtual sal_Int32 SAL_CALL getColumnCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getColumnName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getSchemaName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getTableName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getCatalogName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
                        virtual ::rtl::OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_FILE_ADATABASEMETARESULTSETMETADATA_HXX_


