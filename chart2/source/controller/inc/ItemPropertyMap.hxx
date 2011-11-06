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


#ifndef CHART_ITEMPROPERTYMAP_HXX
#define CHART_ITEMPROPERTYMAP_HXX


#include <comphelper/InlineContainer.hxx>
#include "ItemConverter.hxx"


#define IPM_MAP_ENTRY(wid,uno,mid) (wid, ::std::make_pair< ::comphelper::ItemConverter::tPropertyNameType, ::comphelper::ItemConverter::tMemberIdType >(\
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(uno)), mid))

namespace comphelper
{

typedef ::std::map< ItemConverter::tWhichIdType, ::std::pair< ItemConverter::tPropertyNameType, ItemConverter::tMemberIdType > >
    ItemPropertyMapType;
typedef ::comphelper::MakeMap< ItemConverter::tWhichIdType, ::std::pair< ItemConverter::tPropertyNameType, ItemConverter::tMemberIdType > >
    MakeItemPropertyMap;

class FillItemSetFunc : public ::std::unary_function< ItemConverter *, void >
{
public:
    explicit FillItemSetFunc( SfxItemSet & rOutItemSet ) :
            m_rOutItemSet( rOutItemSet )
    {}

    void operator() ( ItemConverter * pConv )
    {
        pConv->FillItemSet( m_rOutItemSet );
    }

private:
    SfxItemSet & m_rOutItemSet;
};

class ApplyItemSetFunc : public ::std::unary_function< ItemConverter *, void >
{
public:
    explicit ApplyItemSetFunc( const SfxItemSet & rItemSet,
                               bool & rOutResult ) :
            m_rItemSet( rItemSet ),
            m_rOutResult( rOutResult )
    {}

    void operator() ( ItemConverter * pConv )
    {
        m_rOutResult = pConv->ApplyItemSet( m_rItemSet ) || m_rOutResult;
    }

private:
    const SfxItemSet & m_rItemSet;
    bool & m_rOutResult;
};

struct DeleteItemConverterPtr : public ::std::unary_function< ItemConverter *, void >
{
    void operator() ( ItemConverter * pConv )
    { delete pConv; }
};

} //  namespace comphelper

// CHART_ITEMPROPERTYMAP_HXX
#endif
