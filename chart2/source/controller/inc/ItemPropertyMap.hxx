/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ItemPropertyMap.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:23:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART_ITEMPROPERTYMAP_HXX
#define CHART_ITEMPROPERTYMAP_HXX

#include "InlineContainer.hxx"

namespace comphelper
{

typedef ::std::map< USHORT, ::rtl::OUString > ItemPropertyMapType;
typedef ::comphelper::MakeMap< USHORT, ::rtl::OUString > MakeItemPropertyMap;

class FillItemSetFunc : public ::std::unary_function< void, ItemConverter * >
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

class ApplyItemSetFunc : public ::std::unary_function< void, ItemConverter * >
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

struct DeleteItemConverterPtr : public ::std::unary_function< void, ItemConverter * >
{
    void operator() ( ItemConverter * pConv )
    { delete pConv; }
};

} //  namespace comphelper

// CHART_ITEMPROPERTYMAP_HXX
#endif
