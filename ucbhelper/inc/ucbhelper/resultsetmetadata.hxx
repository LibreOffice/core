/*************************************************************************
 *
 *  $RCSfile: resultsetmetadata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_RESULTSETMETADATA_HXX
#define _UCBHELPER_RESULTSETMETADATA_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace lang  { class XMultiServiceFactory; }
    namespace beans { struct Property; }
} } }

namespace ucb
{

//=========================================================================

struct ResultSetMetaData_Impl;

/**
 * This is a simple implementation of the interface XResultSetMetaData.
 * Some methods will simply return standard values. Other calculating the
 * return values using the property sequence provided to the constructor of
 * this class.
 * One can derive from this class, if other implementations of any methods
 * is needed, but for many cases this implementation should do good guesses
 * for many use cases.
 */
class ResultSetMetaData :
                public ::cppu::OWeakObject,
                public ::com::sun::star::lang::XTypeProvider,
                public ::com::sun::star::sdbc::XResultSetMetaData
{
private:
    ResultSetMetaData_Impl* m_pImpl;

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::Property >            m_aProps;
    sal_Bool m_bReadOnly;

public:

    /**
      * Constructor.
      *
      * @param rxSMgr is a Servive Manager.
      * @param rProps is a sequence of properties (partially) describing the
      *        columns of a resultset.
      * @param bReadOnly is used to specify whether the whole(!) resultset
      *        is read-only.
      */
    ResultSetMetaData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::Property >& rProps,
            sal_Bool bReadOnly = sal_True );

    /**
      * Destructor.
      */
    virtual ~ResultSetMetaData();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XResultSetMetaData
    /**
      * @return the length of the property sequence.
      */
    virtual sal_Int32 SAL_CALL
    getColumnCount()
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the value of member m_bReadOnly.
      */
    virtual sal_Bool SAL_CALL
    isAutoIncrement( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always true.
      */
    virtual sal_Bool SAL_CALL
    isCaseSensitive( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always false.
      */
    virtual sal_Bool SAL_CALL
    isSearchable( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always false.
      */
    virtual sal_Bool SAL_CALL
    isCurrency( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always ColumnValue::NULLABLE.
      */
    virtual sal_Int32 SAL_CALL
    isNullable( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always false.
      */
    virtual sal_Bool SAL_CALL
    isSigned( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always 16.
      */
    virtual sal_Int32 SAL_CALL
    getColumnDisplaySize( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the name of the property that corresponds to column.
      */
    virtual ::rtl::OUString SAL_CALL
    getColumnLabel( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the name of the property that corresponds to column.
      */
    virtual ::rtl::OUString SAL_CALL
    getColumnName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always an empty string.
      */
    virtual ::rtl::OUString SAL_CALL
    getSchemaName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always -1.
      */
    virtual sal_Int32 SAL_CALL
    getPrecision( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always 0.
      */
    virtual sal_Int32 SAL_CALL
    getScale( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always an empty string.
      */
    virtual ::rtl::OUString SAL_CALL
    getTableName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always an empty string.
      */
    getCatalogName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the type of the property that corresponds to column - mapped
      *         from UNO-Type to SQL-Type.
      */
    virtual sal_Int32 SAL_CALL
    getColumnType( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always an empty string.
      */
    virtual ::rtl::OUString SAL_CALL
    getColumnTypeName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the value of member m_bReadOnly.
      */
    virtual sal_Bool SAL_CALL
    isReadOnly( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the inverted value of member m_bReadOnly.
      */
    virtual sal_Bool SAL_CALL
    isWritable( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the inverted value of member m_bReadOnly.
      */
    virtual sal_Bool SAL_CALL
    isDefinitelyWritable( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    /**
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return always an empty string.
      */
    virtual ::rtl::OUString SAL_CALL
    getColumnServiceName( sal_Int32 column )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
};

} // namespace ucb

#endif /* !_UCBHELPER_RESULTSETMETADATA_HXX */
