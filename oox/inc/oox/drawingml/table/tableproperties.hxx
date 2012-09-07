/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    rtl::OUString&                      getStyleId(){ return maStyleId; };
    boost::shared_ptr< TableStyle >&    getTableStyle(){ return mpTableStyle; };
    sal_Bool&                           isRtl(){ return mbRtl; };
    sal_Bool&                           isFirstRow(){ return mbFirstRow; };
    sal_Bool&                           isFirstCol(){ return mbFirstCol; };
    sal_Bool&                           isLastRow(){ return mbLastRow; };
    sal_Bool&                           isLastCol(){ return mbLastCol; };
    sal_Bool&                           isBandRow(){ return mbBandRow; };
    sal_Bool&                           isBandCol(){ return mbBandCol; };

    void apply( const TablePropertiesPtr& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet, ::oox::drawingml::TextListStylePtr pMasterTextListStyle );

private:

    const TableStyle&                   getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase, sal_Bool &isCreateTabStyle);

    rtl::OUString                       maStyleId;              // either StyleId is available
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
