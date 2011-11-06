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


#ifndef INCLUDED_BORDERHANDLER_HXX
#define INCLUDED_BORDERHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/table/BorderLine.hpp>

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class WRITERFILTER_DLLPRIVATE BorderHandler : public LoggedProperties
{
public:
    //todo: order is a guess
    enum BorderPosition
    {
        BORDER_TOP,
        BORDER_LEFT,
        BORDER_BOTTOM,
        BORDER_RIGHT,
        BORDER_HORIZONTAL,
        BORDER_VERTICAL,
        BORDER_COUNT
    };

private:
    sal_Int8        m_nCurrentBorderPosition;
    //values of the current border
    sal_Int32       m_nLineWidth;
    sal_Int32       m_nLineType;
    sal_Int32       m_nLineColor;
    sal_Int32       m_nLineDistance;
    bool            m_bOOXML;

    bool                                        m_aFilledLines[BORDER_COUNT];
    ::com::sun::star::table::BorderLine         m_aBorderLines[BORDER_COUNT];

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    BorderHandler( bool bOOXML );
    virtual ~BorderHandler();

    ::boost::shared_ptr<PropertyMap>            getProperties();
    ::com::sun::star::table::BorderLine         getBorderLine();
    sal_Int32                                   getLineDistance() const { return m_nLineDistance;}
};
typedef boost::shared_ptr< BorderHandler >          BorderHandlerPtr;
}}

#endif //
