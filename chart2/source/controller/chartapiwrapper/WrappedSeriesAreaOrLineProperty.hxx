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


#ifndef CHART_WRAPPED_SERIES_AREA_OR_LINE_PROPERTY_HXX
#define CHART_WRAPPED_SERIES_AREA_OR_LINE_PROPERTY_HXX

#include "WrappedProperty.hxx"

//.............................................................................
namespace chart
{
namespace wrapper
{

class DataSeriesPointWrapper;
class WrappedSeriesAreaOrLineProperty : public WrappedProperty
{
public:
    WrappedSeriesAreaOrLineProperty( const ::rtl::OUString& rOuterName
        , const ::rtl::OUString& rInnerAreaTypeName, const ::rtl::OUString& rInnerLineTypeName
        , DataSeriesPointWrapper* pDataSeriesPointWrapper );
    virtual ~WrappedSeriesAreaOrLineProperty();

    virtual ::rtl::OUString getInnerName() const;

protected:
    DataSeriesPointWrapper* m_pDataSeriesPointWrapper;
    ::rtl::OUString m_aInnerAreaTypeName;
    ::rtl::OUString m_aInnerLineTypeName;
};

} //namespace wrapper
} //namespace chart
//.............................................................................

// CHART_WRAPPED_SERIES_AREA_OR_LINE_PROPERTY_HXX
#endif
