/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OResultSetMetaData.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:55:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_
#define _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif

namespace connectivity
{
    namespace odbc
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************
        typedef ::cppu::WeakImplHelper1<        ::com::sun::star::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

        class OResultSetMetaData :  public  OResultSetMetaData_BASE
        {
        protected:
            ::std::vector<sal_Int32> m_vMapping; // when not every column is needed

            SQLHANDLE       m_aStatementHandle;
            OConnection*    m_pConnection;
            sal_Int32       m_nColCount;
            sal_Bool        m_bUseODBC2Types;

            ::rtl::OUString getCharColAttrib(sal_Int32 column,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getNumColAttrib(sal_Int32 column,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OResultSetMetaData(OConnection* _pConnection, SQLHANDLE _pStmt )
                :m_aStatementHandle( _pStmt )
                ,m_pConnection(_pConnection)
                ,m_nColCount(-1)
                ,m_bUseODBC2Types(sal_False)
            {}
            OResultSetMetaData(OConnection* _pConnection, SQLHANDLE _pStmt ,const ::std::vector<sal_Int32> & _vMapping)
                    :m_vMapping(_vMapping)
                    ,m_aStatementHandle( _pStmt )
                    ,m_pConnection(_pConnection)
                    ,m_nColCount(_vMapping.size()-1)
                    ,m_bUseODBC2Types(sal_False)
            {}
            virtual ~OResultSetMetaData();


            static  SWORD getNumColAttrib(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 _column
                                              ,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static SWORD getColumnODBCType(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 column)
                                               throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            inline oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }
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
#endif // _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_

