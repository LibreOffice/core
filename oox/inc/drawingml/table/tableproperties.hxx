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

#ifndef INCLUDED_OOX_DRAWINGML_TABLE_TABLEPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_TABLE_TABLEPROPERTIES_HXX

#include <drawingml/table/tablerow.hxx>
#include <drawingml/table/tablestyle.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/drawingml/color.hxx>

#include <map>
#include <memory>
#include <vector>

namespace oox { namespace drawingml { namespace table {

class TableProperties
{
public:

    TableProperties();

    std::vector< sal_Int32 >&           getTableGrid() { return mvTableGrid; };
    std::vector< TableRow >&            getTableRows() { return mvTableRows; };

    OUString&                           getStyleId()    { return maStyleId; };
    std::shared_ptr< TableStyle >&      getTableStyle() { return mpTableStyle; };
    bool                                isFirstRow() const  { return mbFirstRow; };
    void                                setFirstRow(bool b) { mbFirstRow = b; };
    bool                                isFirstCol() const  { return mbFirstCol; };
    void                                setFirstCol(bool b) { mbFirstCol = b; };
    bool                                isLastRow() const   { return mbLastRow; };
    void                                setLastRow(bool b)  { mbLastRow = b; };
    bool                                isLastCol() const   { return mbLastCol; };
    void                                setLastCol(bool b)  { mbLastCol = b; };
    bool                                isBandRow() const   { return mbBandRow; };
    void                                setBandRow(bool b)  { mbBandRow = b; };
    bool                                isBandCol() const   { return mbBandCol; };
    void                                setBandCol(bool b)  { mbBandCol = b; };

    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
        const css::uno::Reference < css::beans::XPropertySet > & xPropSet,
        const ::oox::drawingml::TextListStylePtr& pMasterTextListStyle );

private:

    const TableStyle&                   getUsedTableStyle(const ::oox::core::XmlFilterBase& rFilterBase, TableStyle*& rTableStyleToDelete);

    OUString                            maStyleId;              // either StyleId is available
    std::shared_ptr< TableStyle >       mpTableStyle;           // or the complete TableStyle
    std::vector< sal_Int32 >            mvTableGrid;
    std::vector< TableRow >             mvTableRows;

    bool                                mbFirstRow;
    bool                                mbFirstCol;
    bool                                mbLastRow;
    bool                                mbLastCol;
    bool                                mbBandRow;
    bool                                mbBandCol;
};

} } }

#endif // INCLUDED_OOX_DRAWINGML_TABLE_TABLEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
