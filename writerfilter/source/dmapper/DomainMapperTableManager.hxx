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
#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

#include "TablePropertiesHandler.hxx"
#include <TablePositionHandler.hxx>

#include <resourcemodel/TableManager.hxx>
#include <PropertyMap.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <vector>

namespace writerfilter {
namespace dmapper {

class DomainMapperTableManager : public DomainMapperTableManager_Base_t
{
    typedef boost::shared_ptr< std::vector<sal_Int32> > IntVectorPtr;

    sal_uInt32      m_nRow;
    ::std::vector< sal_uInt32 > m_nCell;
    sal_uInt32      m_nGridSpan;
    sal_uInt32      m_nGridBefore; ///< number of grid columns in the parent table's table grid which must be skipped before the contents of this table row are added to the parent table
    sal_uInt32      m_nGridAfter; ///< number of grid columns in the parent table's table grid which shall be left after the last cell in the table row
    sal_uInt32      m_nCellBorderIndex; //borders are provided for all cells and need counting
    sal_Int32       m_nHeaderRepeat; //counter of repeated headers - if == -1 then the repeating stops
    sal_Int32       m_nTableWidth; //might be set directly or has to be calculated from the column positions
    bool            m_bOOXML;
    OUString m_sTableStyleName;
    std::vector< TablePositionHandlerPtr > m_aTablePositions;
    std::vector< TablePositionHandlerPtr > m_aTmpPosition; ///< Temporarily stores the position to compare it later
    std::vector< TablePropertyMapPtr > m_aTmpTableProperties; ///< Temporarily stores the table properties until end of row
    PropertyMapPtr  m_pTableStyleTextProperies;

    ::std::vector< IntVectorPtr >  m_aTableGrid;
    ::std::vector< IntVectorPtr >  m_aGridSpans;
    /// If this is true, then we pushed a width before the next level started, and that should be carried over when starting the next level.
    bool            m_bPushCurrentWidth;
    /// Individual table cell width values, used only in case the number of cells doesn't match the table grid.
    ::std::vector< IntVectorPtr >  m_aCellWidths;
    /// Remember if a cell already set this, then it should not be set at a row level.
    bool m_bRowSizeTypeInserted;
    /// At least one cell in the current row has the btLr text direction.
    bool m_bHasBtlrCell;
    /// Remember if table width was already set, when we lack a w:tblW, it should be set manually at the end.
    bool m_bTableSizeTypeInserted;
    /// Table layout algorithm, IOW if we should consider fixed column width or not.
    sal_uInt32 m_nLayoutType;
    sal_Int32 m_nMaxFixedWidth;

    TablePropertiesHandler   *m_pTablePropsHandler;
    PropertyMapPtr            m_pStyleProps;

    virtual void clearData() SAL_OVERRIDE;

public:

    DomainMapperTableManager(bool bOOXML);
    virtual ~DomainMapperTableManager();

    // use this method to avoid adding the properties for the table
    // but in the provided properties map.
    inline void SetStyleProperties( PropertyMapPtr pProperties ) { m_pStyleProps = pProperties; };

    virtual bool sprm(Sprm & rSprm) SAL_OVERRIDE;

    virtual void startLevel( ) SAL_OVERRIDE;
    virtual void endLevel( ) SAL_OVERRIDE;

    virtual void endOfCellAction() SAL_OVERRIDE;
    virtual void endOfRowAction() SAL_OVERRIDE;

    IntVectorPtr getCurrentGrid( );
    IntVectorPtr getCurrentSpans( );
    IntVectorPtr getCurrentCellWidths( );

    const OUString& getTableStyleName() const { return m_sTableStyleName; }
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> getCurrentTablePosition();

    inline virtual void cellProps(TablePropertyMapPtr pProps) SAL_OVERRIDE
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->InsertProps(pProps);
        else
           DomainMapperTableManager_Base_t::cellProps( pProps );
    };

    inline virtual void cellPropsByCell(unsigned int i, TablePropertyMapPtr pProps) SAL_OVERRIDE
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->InsertProps(pProps);
        else
           DomainMapperTableManager_Base_t::cellPropsByCell( i, pProps );
    };

    inline virtual void insertRowProps(TablePropertyMapPtr pProps) SAL_OVERRIDE
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->InsertProps(pProps);
        else
           DomainMapperTableManager_Base_t::insertRowProps( pProps );
    };

    inline virtual void insertTableProps(TablePropertyMapPtr pProps) SAL_OVERRIDE
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->InsertProps(pProps);
        else
            m_aTmpTableProperties.back()->InsertProps(pProps);
    };

    bool IsRowSizeTypeInserted() const
    {
        return m_bRowSizeTypeInserted;
    }

    bool HasBtlrCell() const
    {
        return m_bHasBtlrCell;
    }

    bool IsTableSizeTypeInserted() const
    {
        return m_bTableSizeTypeInserted;
    }

    void SetLayoutType(sal_uInt32 nLayoutType)
    {
        m_nLayoutType = nLayoutType;
    }

};

}}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
