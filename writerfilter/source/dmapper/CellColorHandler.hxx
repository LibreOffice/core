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


#ifndef INCLUDED_CELLCOLORHANDLER_HXX
#define INCLUDED_CELLCOLORHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
//#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine.hpp>

namespace writerfilter {
namespace dmapper
{
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE CellColorHandler : public LoggedProperties
{
public:
    sal_Int32 m_nShadowType;
    sal_Int32 m_nColor;
    sal_Int32 m_nFillColor;
    enum Type {P, C, Others};
    Type m_eType;

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    CellColorHandler( );
    virtual ~CellColorHandler();

    ::boost::shared_ptr<TablePropertyMap>            getProperties();

    void setType(Type type) { m_eType = type; }
};
typedef boost::shared_ptr< CellColorHandler >          CellColorHandlerPtr;
}}

#endif //
