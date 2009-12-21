/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalData.hxx,v $
 * $Revision: 1.0 $
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
#ifndef CHART2_INTERNALDATA_HXX
#define CHART2_INTERNALDATA_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <valarray>

namespace chart
{

class InternalData
{
public:
    InternalData();

    void createDefaultData();

    void setData( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< double > > & rDataInRows );
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< double > > getData() const;

    ::com::sun::star::uno::Sequence< double > getColumnValues( sal_Int32 nColumnIndex ) const;
    ::com::sun::star::uno::Sequence< double > getRowValues( sal_Int32 nRowIndex ) const;

    void setColumnValues( sal_Int32 nColumnIndex, const ::std::vector< double > & rNewData );
    void setRowValues( sal_Int32 nRowIndex, const ::std::vector< double > & rNewData );

    void swapRowWithNext( sal_Int32 nRowIndex );
    void swapColumnWithNext( sal_Int32 nColumnIndex );

    /** resizes the data if at least one of the given dimensions is larger than
        before.  The data is never becoming smaller only larger.

        @return </TRUE>, if the data was enlarged
    */
    bool enlargeData( sal_Int32 nColumnCount, sal_Int32 nRowCount );

    void insertColumn( sal_Int32 nAfterIndex );
    void insertRow( sal_Int32 nAfterIndex );
    void deleteColumn( sal_Int32 nAtIndex );
    void deleteRow( sal_Int32 nAtIndex );

    /// @return the index of the newly appended column
    sal_Int32 appendColumn();
    /// @return the index of the newly appended row
    sal_Int32 appendRow();

    sal_Int32 getRowCount() const;
    sal_Int32 getColumnCount() const;

    void setRowLabels( const ::std::vector< ::rtl::OUString > & rNewRowLabels );
    ::std::vector< ::rtl::OUString > getRowLabels() const;
    void setColumnLabels( const ::std::vector< ::rtl::OUString > & rNewColumnLabels );
    ::std::vector< ::rtl::OUString > getColumnLabels() const;

#if OSL_DEBUG_LEVEL > 2
    void traceData() const;
#endif

private:
    sal_Int32   m_nColumnCount;
    sal_Int32   m_nRowCount;

    typedef ::std::valarray< double > tDataType;
    typedef ::std::vector< ::rtl::OUString > tLabelType;

    tDataType    m_aData;
    tLabelType   m_aRowLabels;
    tLabelType   m_aColumnLabels;
};

#endif

} //  namespace chart
