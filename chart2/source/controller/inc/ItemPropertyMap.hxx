/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
