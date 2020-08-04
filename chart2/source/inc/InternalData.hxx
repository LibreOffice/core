/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

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

    void setData( const css::uno::Sequence< css::uno::Sequence< double > > & rDataInRows );
    css::uno::Sequence< css::uno::Sequence< double > > getData() const;

    css::uno::Sequence< double > getColumnValues( sal_Int32 nColumnIndex ) const;
    css::uno::Sequence< double > getRowValues( sal_Int32 nRowIndex ) const;

    void setColumnValues( sal_Int32 nColumnIndex, const std::vector< double > & rNewData );
    void setRowValues( sal_Int32 nRowIndex, const std::vector< double > & rNewData );

    void setComplexColumnLabel( sal_Int32 nColumnIndex, const std::vector< css::uno::Any >& rComplexLabel );
    void setComplexRowLabel( sal_Int32 nRowIndex, const std::vector< css::uno::Any >& rComplexLabel );

    std::vector< css::uno::Any > getComplexColumnLabel( sal_Int32 nColumnIndex ) const;
    std::vector< css::uno::Any > getComplexRowLabel( sal_Int32 nRowIndex ) const;

    void swapRowWithNext( sal_Int32 nRowIndex );
    void swapColumnWithNext( sal_Int32 nColumnIndex );

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

    typedef std::vector< std::vector< css::uno::Any > > tVecVecAny; //inner index is hierarchical level

    void setComplexRowLabels( const tVecVecAny& rNewRowLabels );
    const tVecVecAny& getComplexRowLabels() const;
    void setComplexColumnLabels( const tVecVecAny& rNewColumnLabels );
    const tVecVecAny& getComplexColumnLabels() const;

    void dump() const;

private: //methods
    /** resizes the data if at least one of the given dimensions is larger than
        before.  The data is never becoming smaller only larger.

        @return </true>, if the data was enlarged
    */
    bool enlargeData( sal_Int32 nColumnCount, sal_Int32 nRowCount );

private:
    sal_Int32   m_nColumnCount;
    sal_Int32   m_nRowCount;

    typedef std::valarray< double > tDataType;
    tDataType    m_aData;
    tVecVecAny   m_aRowLabels;//outer index is row index, inner index is category level
    tVecVecAny   m_aColumnLabels;//outer index is column index
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
