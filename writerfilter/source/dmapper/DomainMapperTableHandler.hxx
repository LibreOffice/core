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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPERTABLEHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPERTABLEHANDLER_HXX

#include <TableManager.hxx>
#include <PropertyMap.hxx>
#include <vector>

#include <com/sun/star/text/XTextAppendAndConvert.hpp>

namespace writerfilter {
namespace dmapper {

typedef css::uno::Reference< css::text::XTextRange > Handle_t;
typedef css::uno::Sequence<Handle_t> CellSequence_t;
typedef std::shared_ptr<CellSequence_t> CellSequencePointer_t;
typedef css::uno::Sequence<CellSequence_t> RowSequence_t;
typedef std::shared_ptr<RowSequence_t> RowSequencePointer_t;
typedef css::uno::Sequence<RowSequence_t> TableSequence_t;
typedef std::shared_ptr<TableSequence_t> TableSequencePointer_t;
typedef css::text::XTextAppendAndConvert Text_t;
typedef css::uno::Reference<Text_t> TextReference_t;

typedef css::beans::PropertyValues TablePropertyValues_t;
typedef css::uno::Sequence<TablePropertyValues_t> RowPropertyValuesSeq_t;
typedef css::uno::Sequence<RowPropertyValuesSeq_t> CellPropertyValuesSeq_t;

typedef std::vector<PropertyMapPtr>     PropertyMapVector1;
typedef std::vector<PropertyMapVector1> PropertyMapVector2;

class DomainMapper_Impl;
class TableStyleSheetEntry;
struct TableInfo;

/// A horizontally merged cell is in fact a range of cells till its merge is performed.
struct HorizontallyMergedCell
{
    sal_Int32 m_nFirstRow;
    sal_Int32 m_nFirstCol;
    sal_Int32 m_nLastRow;
    sal_Int32 m_nLastCol;
    HorizontallyMergedCell(sal_Int32 nFirstRow, sal_Int32 nFirstCol)
        : m_nFirstRow(nFirstRow)
        , m_nFirstCol(nFirstCol)
        , m_nLastRow(0)
        , m_nLastCol(0)
    {
    }
};

class DomainMapperTableHandler : public TableDataHandler
{
    TextReference_t         m_xText;
    DomainMapper_Impl&      m_rDMapper_Impl;
    CellSequencePointer_t   m_pCellSeq;
    RowSequencePointer_t    m_pRowSeq;
    TableSequencePointer_t  m_pTableSeq;

    Handle_t               m_xTableRange;

    // properties
    PropertyMapVector2      m_aCellProperties;
    PropertyMapVector1      m_aRowProperties;
    TablePropertyMapPtr     m_aTableProperties;

    sal_Int32 m_nCellIndex;
    sal_Int32 m_nRowIndex;

    TableStyleSheetEntry * endTableGetTableStyle(TableInfo & rInfo, std::vector<css::beans::PropertyValue>& rFrameProperties);
    CellPropertyValuesSeq_t endTableGetCellProperties(TableInfo & rInfo, std::vector<HorizontallyMergedCell>& rMerges);
    RowPropertyValuesSeq_t endTableGetRowProperties();

public:
    typedef std::shared_ptr<DomainMapperTableHandler> Pointer_t;

    DomainMapperTableHandler(TextReference_t const& xText,
                             DomainMapper_Impl& rDMapper_Impl);
    virtual ~DomainMapperTableHandler();

    virtual void startTable(unsigned int nRows, unsigned int nDepth,
                            TablePropertyMapPtr pProps) SAL_OVERRIDE;
    virtual void endTable(unsigned int nestedTableLevel) SAL_OVERRIDE;
    virtual void startRow(unsigned int nCells, TablePropertyMapPtr pProps) SAL_OVERRIDE;
    virtual void endRow() SAL_OVERRIDE;
    virtual void startCell(const Handle_t & start, TablePropertyMapPtr pProps) SAL_OVERRIDE;
    virtual void endCell(const Handle_t & end) SAL_OVERRIDE;

    Handle_t& getTable() { return m_xTableRange; };
};

}}

#endif // INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPERTABLEHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
