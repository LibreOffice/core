/*************************************************************************
 *
 *  $RCSfile: pq_sequenceresultset.hxx,v $
 *
 *  $Revision: 1.1.2.3 $
 *
 *  last change: $Author: jbu $ $Date: 2006/01/22 15:14:32 $
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
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *
 ************************************************************************/
#ifndef _PG_SEQUENCERESULTSET_HXX_
#define _PG_SEQUENCERESULTSET_HXX_

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include "pq_connection.hxx"
#include "pq_baseresultset.hxx"
#include "pq_statics.hxx"

namespace pq_sdbc_driver
{

// static const sal_Int32 RESULTSET_CURSOR_NAME = 0;
// static const sal_Int32 RESULTSET_ESCAPE_PROCESSING = 1;
// static const sal_Int32 RESULTSET_FETCH_DIRECTION = 2;
// static const sal_Int32 RESULTSET_FETCH_SIZE = 3;
// static const sal_Int32 RESULTSET_RESULT_SET_CONCURRENCY = 4;
// static const sal_Int32 RESULTSET_RESULT_SET_TYPE = 5;

//#define RESULTSET_SIZE 6
class ResultSetGuard;

class SequenceResultSet : public BaseResultSet
{
protected:
    ::com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > m_data;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_columnNames;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSetMetaData > m_meta;

protected:
    /** mutex should be locked before called
     */
    virtual void checkClosed()
        throw ( com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException );

    /** unchecked, acquire mutex before calling
     */
    virtual ::com::sun::star::uno::Any getValue( sal_Int32 columnIndex );

public:
    SequenceResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        const com::sun::star::uno::Sequence< rtl::OUString > &colNames,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > &data,
        const com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > &tc,
        const ColumnMetaDataVector *pVec = 0);
    ~SequenceResultSet();

public: // XCloseable
    virtual void SAL_CALL close(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XResultSetMetaDataSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XColumnLocate
    virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
};

}
#endif
