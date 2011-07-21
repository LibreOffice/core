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

#include "oox/ppt/customshowlistcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

class CustomShowContext : public ::oox::core::FragmentHandler2
{
    CustomShow mrCustomShow;

public:
    CustomShowContext( ::oox::core::FragmentHandler2& rParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
            CustomShow& rCustomShow );
    ~CustomShowContext( );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs );
};

CustomShowContext::CustomShowContext( FragmentHandler2& rParent,
    const Reference< XFastAttributeList >& rxAttribs,
        CustomShow& rCustomShow )
: FragmentHandler2( rParent )
, mrCustomShow( rCustomShow )
{
    mrCustomShow.maName = rxAttribs->getOptionalValue( XML_name );
    mrCustomShow.mnId = rxAttribs->getOptionalValue( XML_id );
}

CustomShowContext::~CustomShowContext( )
{
}

::oox::core::ContextHandlerRef CustomShowContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case PPT_TOKEN( sld ) :
            mrCustomShow.maSldLst.push_back( rAttribs.getString( R_TOKEN( id ), rtl::OUString() ) );
            return this;
        default:
        break;
    }

    return this;
}

//---------------------------------------------------------------------------

CustomShowListContext::CustomShowListContext( FragmentHandler2& rParent,
    std::vector< CustomShow >& rCustomShowList )
: FragmentHandler2( rParent )
, mrCustomShowList( rCustomShowList )
{
}

CustomShowListContext::~CustomShowListContext( )
{
}

::oox::core::ContextHandlerRef CustomShowListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case PPT_TOKEN( custShow ) :
        {
            CustomShow aCustomShow;
            mrCustomShowList.push_back( aCustomShow );
            return new CustomShowContext( *this, rAttribs.getFastAttributeList(), mrCustomShowList.back() );
        }
        default:
        break;
    }

    return this;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
