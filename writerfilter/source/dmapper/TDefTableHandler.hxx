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


#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#define INCLUDED_TDEFTABLEHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <vector>
namespace com{ namespace sun{ namespace star{namespace table {
    struct BorderLine;
}}}}

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE TDefTableHandler : public LoggedProperties
{
public:

private:
    ::std::vector<sal_Int32>                                m_aCellBorderPositions;
    ::std::vector<sal_Int32>                                m_aCellVertAlign;

    ::std::vector< ::com::sun::star::table::BorderLine >    m_aLeftBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aRightBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aTopBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aBottomBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aInsideHBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aInsideVBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aTl2brBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aTr2blBorderLines;

    //values of the current border
    sal_Int32                                           m_nLineWidth;
    sal_Int32                                           m_nLineType;
    sal_Int32                                           m_nLineColor;
    sal_Int32                                           m_nLineDistance;

    bool                                                m_bOOXML;

    void localResolve(Id Name, writerfilter::Reference<Properties>::Pointer_t pProperties);

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    TDefTableHandler( bool bOOXML );
    virtual ~TDefTableHandler();

    size_t                                      getCellCount() const;
    void                                        fillCellProperties( size_t nCell, ::boost::shared_ptr< TablePropertyMap > pCellProperties) const;
    ::boost::shared_ptr<PropertyMap>            getRowProperties() const;
    sal_Int32                                   getTableWidth() const;
};
typedef boost::shared_ptr< TDefTableHandler >          TDefTableHandlerPtr;
}}

#endif //
