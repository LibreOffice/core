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



#ifndef SVTOOLS_URL_FILTER_HXX
#define SVTOOLS_URL_FILTER_HXX

#include <tools/string.hxx>
#include <tools/wldcrd.hxx>
#include <functional>
#include <vector>
/** filters allowed URLs
*/
class IUrlFilter
{
public:
    virtual bool isUrlAllowed( const String& _rURL ) const = 0;

protected:
    virtual inline ~IUrlFilter() = 0;
};

inline IUrlFilter::~IUrlFilter() {}

struct FilterMatch : public ::std::unary_function< bool, WildCard >
{
private:
    const String&   m_rCompareString;
public:
    FilterMatch( const String& _rCompareString ) : m_rCompareString( _rCompareString ) { }

    bool operator()( const WildCard& _rMatcher )
    {
        return _rMatcher.Matches( m_rCompareString ) ? true : false;
    }

    static void createWildCardFilterList(const String& _rFilterList,::std::vector< WildCard >& _rFilters);
};

#endif // SVTOOLS_URL_FILTER_HXX
