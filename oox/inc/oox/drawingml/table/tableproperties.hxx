/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    rtl::OUString&						getStyleId(){ return maStyleId; };
    boost::shared_ptr< TableStyle >&	getTableStyle(){ return mpTableStyle; };
    sal_Bool&							isRtl(){ return mbRtl; };
    sal_Bool&							isFirstRow(){ return mbFirstRow; };
    sal_Bool&							isFirstCol(){ return mbFirstCol; };
    sal_Bool&							isLastRow(){ return mbLastRow; };
    sal_Bool&							isLastCol(){ return mbLastCol; };
    sal_Bool&							isBandRow(){ return mbBandRow; };
    sal_Bool&							isBandCol(){ return mbBandCol; };

    void apply( const TablePropertiesPtr& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet, ::oox::drawingml::TextListStylePtr pMasterTextListStyle );

private:

    const TableStyle&					getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase );

    rtl::OUString						maStyleId;				// either StyleId is available
    boost::shared_ptr< TableStyle >		mpTableStyle;			// or the complete TableStyle
    std::vector< sal_Int32 >			mvTableGrid;
    std::vector< TableRow >				mvTableRows;

    sal_Bool							mbRtl;
    sal_Bool							mbFirstRow;
    sal_Bool							mbFirstCol;
    sal_Bool							mbLastRow;
    sal_Bool							mbLastCol;
    sal_Bool							mbBandRow;
    sal_Bool							mbBandCol;
};

} } }

#endif  //  OOX_DRAWINGML_TABLEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
