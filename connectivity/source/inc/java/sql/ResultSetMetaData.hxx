/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResultSetMetaData.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 12:04:25 $
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
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSETMETADATA_HXX_
#define _CONNECTIVITY_JAVA_SQL_RESULTSETMETADATA_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#include "java/sql/ConnectionLog.hxx"

namespace connectivity
{
    //**************************************************************
    //************ Class: java.sql.ResultSetMetaData
    //**************************************************************
    class java_sql_Connection;
    class java_sql_ResultSetMetaData :  public ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XResultSetMetaData>,
                                        public java_lang_Object
    {
    protected:
        java::sql::ConnectionLog    m_aLogger;
        java_sql_Connection*        m_pConnection;

        // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
        virtual ~java_sql_ResultSetMetaData();
    public:
        static jclass getMyClass();

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_ResultSetMetaData( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rResultSetLogger, java_sql_Connection& _rCon  );

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
#endif // _CONNECTIVITY_JAVA_SQL_RESULTSETMETADATA_HXX_

