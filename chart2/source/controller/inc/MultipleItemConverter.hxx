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


#ifndef CHART_ALLAXISITEMCONVERTER_HXX
#define CHART_ALLAXISITEMCONVERTER_HXX

#include <com/sun/star/frame/XModel.hpp>

#include "ItemConverter.hxx"

#include <vector>

namespace comphelper
{

/** Note: virtual const sal_uInt16 * GetWhichPairs() const; is still pure virtual
 */
class MultipleItemConverter : public ItemConverter
{
public:
    virtual ~MultipleItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

    /// implemented empty (returns always false)
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

protected:
    MultipleItemConverter( SfxItemPool& rItemPool );

    ::std::vector< ItemConverter * >            m_aConverters;
};

} //  namespace comphelper

#endif
