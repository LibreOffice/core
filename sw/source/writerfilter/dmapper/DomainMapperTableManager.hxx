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

#include "TablePropertiesHandler.hxx"
#include "TablePositionHandler.hxx"

#include "TableManager.hxx"
#include "PropertyMap.hxx"
#include <vector>
#include <memory>
#include <comphelper/sequenceashashmap.hxx>

namespace writerfilter::dmapper {

class DomainMapper;

class DomainMapperTableManager : public TableManager
{
    typedef std::shared_ptr< std::vector<sal_Int32> > IntVectorPtr;

    sal_uInt32      m_nRow;
    ::std::vector< sal_uInt32 > m_nCell;
    sal_uInt32      m_nGridSpan;
    sal_Int32       m_nHeaderRepeat; //counter of repeated headers - if == -1 then the repeating stops
    sal_Int32       m_nTableWidth; //might be set directly or has to be calculated from the column positions
    /// Are we in a shape (text append stack is not empty) or in the body document?
    bool m_bIsInShape;
    std::vector< OUString > m_aTableStyleNames;
    /// Moved table (in moveRangeFromStart...moveRangeFromEnd or moveRangeToStart...moveRangeToEnd)
    std::vector< OUString > m_aMoved;
    /// Grab-bag of table look attributes for preserving.
    comphelper::SequenceAsHashMap m_aTableLook;
    std::vector< TablePositionHandlerPtr > m_aTablePositions;
    std::vector< TablePositionHandlerPtr > m_aTmpPosition; ///< Temporarily stores the position to compare it later
    std::vector< TablePropertyMapPtr > m_aTmpTableProperties; ///< Temporarily stores the table properties until end of row

    ::std::vector< IntVectorPtr >  m_aTableGrid;
    /// If this is true, then we pushed a width before the next level started, and that should be carried over when starting the next level.
    bool            m_bPushCurrentWidth;
    /// Individual table cell width values, used only in case the number of cells doesn't match the table grid.
    ::std::vector< IntVectorPtr >  m_aCellWidths;
    /// Remember if table width was already set, when we lack a w:tblW, it should be set manually at the end.
    bool m_bTableSizeTypeInserted;
    /// Table layout algorithm, IOW if we should consider fixed column width or not.
    sal_uInt32 m_nLayoutType;
    /// Collected table paragraphs for table style handling
    std::stack< TableParagraphVectorPtr > m_aParagraphsToEndTable;

    std::unique_ptr<TablePropertiesHandler> m_pTablePropsHandler;
    PropertyMapPtr            m_pStyleProps;

    bool shouldInsertRow(IntVectorPtr pCellWidths, IntVectorPtr pTableGrid, size_t nGrids, bool& rIsIncompleteGrid);

    virtual void clearData() override;

public:

    DomainMapperTableManager();
    virtual ~DomainMapperTableManager() override;

    // use this method to avoid adding the properties for the table
    // but in the provided properties map.
    void SetStyleProperties(const PropertyMapPtr& pProperties) { m_pStyleProps = pProperties; };

    virtual bool sprm(Sprm & rSprm) override;
    bool attribute(Id nName, Value const & val);

    virtual void startLevel( ) override;
    virtual void endLevel( ) override;

    virtual void endOfCellAction() override;
    virtual void endOfRowAction() override;

    IntVectorPtr const & getCurrentGrid( );
    IntVectorPtr const & getCurrentCellWidths( );
    const TableParagraphVectorPtr & getCurrentParagraphs( );

    /// Turn the attributes collected so far in m_aTableLook into a property and clear the container.
    void finishTableLook();
    css::uno::Sequence<css::beans::PropertyValue> getCurrentTablePosition();
    TablePositionHandler* getCurrentTableRealPosition();

    virtual void cellProps(const TablePropertyMapPtr& pProps) override
    {
        if ( m_pStyleProps )
            m_pStyleProps->InsertProps(pProps.get());
        else
           TableManager::cellProps( pProps );
    };

    virtual void insertRowProps(const TablePropertyMapPtr& pProps) override
    {
        if ( m_pStyleProps )
            m_pStyleProps->InsertProps(pProps.get());
        else
           TableManager::insertRowProps( pProps );
    };

    virtual void insertTableProps(const TablePropertyMapPtr& pProps) override
    {
        if ( m_pStyleProps )
            m_pStyleProps->InsertProps(pProps.get());
        else
            m_aTmpTableProperties.back()->InsertProps(pProps.get());
    };

    void SetLayoutType(sal_uInt32 nLayoutType)
    {
        m_nLayoutType = nLayoutType;
    }

    using TableManager::isInCell;

    void setIsInShape(bool bIsInShape);

    // moveFromRangeStart and moveToRangeStart are there
    // in the first paragraph in the first cell of the
    // table moved by drag & drop with track changes, but
    // moveFromRangeEnd and moveToRangeEnd follow the
    // table element w:tbl in the same level (not in paragraph).
    // (Special indexing is related to the load of the tables:
    // first-level tables handled by two levels during the
    // import, to support table join etc. In the first cell,
    // setMoved() writes the first level from these two levels
    // i.e. second startLevel() hasn't been called, yet.)
    // TODO: check drag & drop of only a part of the tables.
    void setMoved(OUString sMoved)
    {
        if ( m_aMoved.empty() )
            return;

        if ( !sMoved.isEmpty() )
            m_aMoved[m_aMoved.size() - 1] = sMoved;
        else if ( m_aMoved.size() >= 2 )
            // next table rows weren't moved
            m_aMoved[m_aMoved.size() - 2] = "";
        else
            m_aMoved[m_aMoved.size() - 1] = "";
    }

    OUString getMoved() const
    {
        if ( m_aMoved.size() >= 2 && !m_aMoved[m_aMoved.size() - 2].isEmpty() )
           return m_aMoved[m_aMoved.size() - 2];
        else if ( !m_aMoved.empty() )
           return m_aMoved[m_aMoved.size() -1 ];

        return OUString();
    }

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
