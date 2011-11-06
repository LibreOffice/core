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


#ifndef INCLUDED_CELLMARGINHANDLER_HXX
#define INCLUDED_CELLMARGINHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE CellMarginHandler : public LoggedProperties
{
private:
    sal_Int32   m_nValue;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    sal_Int32   m_nLeftMargin;
    bool        m_bLeftMarginValid;
    sal_Int32   m_nRightMargin;
    bool        m_bRightMarginValid;
    sal_Int32   m_nTopMargin;
    bool        m_bTopMarginValid;
    sal_Int32   m_nBottomMargin;
    bool        m_bBottomMarginValid;

public:
    CellMarginHandler( );
    virtual ~CellMarginHandler();

    ::boost::shared_ptr<TablePropertyMap>            getProperties();

};
typedef boost::shared_ptr< CellMarginHandler >          CellMarginHandlerPtr;
}}

#endif //
