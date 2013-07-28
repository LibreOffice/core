/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef CHART_ITEMPROPERTYMAP_HXX
#define CHART_ITEMPROPERTYMAP_HXX

#include <comphelper/InlineContainer.hxx>
#include "ItemConverter.hxx"

#define IPM_MAP_ENTRY(wid,uno,mid) (wid, ::std::make_pair< ::comphelper::ItemConverter::tPropertyNameType, ::comphelper::ItemConverter::tMemberIdType >(\
    uno, mid))

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
