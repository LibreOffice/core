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

#ifndef INCLUDED_PAGEBORDERSHANDLER_HXX
#define INCLUDED_PAGEBORDERSHANDLER_HXX

#include "BorderHandler.hxx"
#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

#include <com/sun/star/table/BorderLine.hpp>

#include <vector>


namespace writerfilter {
namespace dmapper {

class _PgBorder
{
public:
    com::sun::star::table::BorderLine m_rLine;
    sal_Int32   m_nDistance;
    BorderPosition m_ePos;

    _PgBorder( );
    ~_PgBorder( );
};

class WRITERFILTER_DLLPRIVATE PageBordersHandler : public LoggedProperties
{
private:

    // See implementation of SectionPropertyMap::ApplyBorderToPageStyles
    sal_Int32 m_nDisplay;
    sal_Int32 m_nOffset;
    vector<_PgBorder> m_aBorders;

    // Properties
    virtual void lcl_attribute( Id eName, Value& rVal );
    virtual void lcl_sprm( Sprm& rSprm );

public:
    PageBordersHandler( );
    ~PageBordersHandler( );

    inline sal_Int32 GetDisplayOffset( )
    {
        return ( m_nOffset << 5 ) + m_nDisplay;
    };
    void SetBorders( SectionPropertyMap* pSectContext );
};
typedef boost::shared_ptr< PageBordersHandler > PageBordersHandlerPtr;

} }

#endif
