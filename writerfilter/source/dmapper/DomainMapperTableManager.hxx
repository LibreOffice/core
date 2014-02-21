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


#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

#include "TablePropertiesHandler.hxx"

#include <resourcemodel/TableManager.hxx>
#include <PropertyMap.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <vector>
#include <stack>

namespace writerfilter {
namespace dmapper {

class DomainMapperTableManager : public DomainMapperTableManager_Base_t
{
    typedef boost::shared_ptr< std::vector<sal_Int32> > IntVectorPtr;

    ::std::stack< sal_uInt32 > m_nCellCounterForCurrentRow;
    sal_uInt32 m_nGridSpanOfCurrentCell;
    ::std::stack< sal_uInt32 > m_nCurrentCellBorderIndex; //borders are provided for all cells and need counting
    ::std::stack< sal_Int32 > m_nCurrentHeaderRepeatCount; //counter of repeated headers - if == -1 then the repeating stops
    ::std::stack< sal_Int32 > m_nTableWidthOfCurrentTable; //might be set directly or has to be calculated from the column positions
    bool            m_bOOXML;

    ::std::stack< IntVectorPtr >  m_aTableGrid;
    ::std::stack< IntVectorPtr >  m_aGridSpans;

    TablePropertiesHandler   *m_pTablePropsHandler;
    PropertyMapPtr            m_pStyleProps;

    void pushStackOfMembers();
    void popStackOfMembers();

    IntVectorPtr getCurrentGrid();
    IntVectorPtr getCurrentSpans( );

public:

    DomainMapperTableManager(bool bOOXML);
    virtual ~DomainMapperTableManager();

    // use this method to avoid adding the properties for the table
    // but in the provided properties map.
    void SetStyleProperties( PropertyMapPtr pProperties );

    virtual bool sprm(Sprm & rSprm);

    virtual void startLevel();
    virtual void endLevel();

    virtual void endOfCellAction();
    virtual void endOfRowAction();

    inline virtual void cellProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::cellProps( pProps );
    };

    inline virtual void cellPropsByCell(unsigned int i, TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::cellPropsByCell( i, pProps );
    };

    inline virtual void insertRowProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::insertRowProps( pProps );
    };

    inline virtual void insertTableProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::insertTableProps( pProps );
    };

};

}}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
