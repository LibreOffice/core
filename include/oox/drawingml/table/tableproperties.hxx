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

#ifndef OOX_DRAWINGML_TABLEPROPERTIES_HXX
#define OOX_DRAWINGML_TABLEPROPERTIES_HXX

#include "oox/drawingml/table/tablerow.hxx"
#include "oox/drawingml/table/tablestyle.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <map>

namespace oox { namespace drawingml { namespace table {

class TableProperties
{
public:

    TableProperties();
    ~TableProperties();

    std::vector< sal_Int32 >& getTableGrid() { return mvTableGrid; };
    std::vector< TableRow >& getTableRows() { return mvTableRows; };

    OUString&                      getStyleId(){ return maStyleId; };
    boost::shared_ptr< TableStyle >&    getTableStyle(){ return mpTableStyle; };
    sal_Bool&                           isRtl(){ return mbRtl; };
    sal_Bool&                           isFirstRow(){ return mbFirstRow; };
    sal_Bool&                           isFirstCol(){ return mbFirstCol; };
    sal_Bool&                           isLastRow(){ return mbLastRow; };
    sal_Bool&                           isLastCol(){ return mbLastCol; };
    sal_Bool&                           isBandRow(){ return mbBandRow; };
    sal_Bool&                           isBandCol(){ return mbBandCol; };

    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet, ::oox::drawingml::TextListStylePtr pMasterTextListStyle );

private:

    const TableStyle&                   getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase );

    OUString                       maStyleId;              // either StyleId is available
    boost::shared_ptr< TableStyle >     mpTableStyle;           // or the complete TableStyle
    std::vector< sal_Int32 >            mvTableGrid;
    std::vector< TableRow >             mvTableRows;

    sal_Bool                            mbRtl;
    sal_Bool                            mbFirstRow;
    sal_Bool                            mbFirstCol;
    sal_Bool                            mbLastRow;
    sal_Bool                            mbLastCol;
    sal_Bool                            mbBandRow;
    sal_Bool                            mbBandCol;
};

} } }

#endif  //  OOX_DRAWINGML_TABLEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
