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

#include "customshowlistcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

class CustomShowContext : public ::oox::core::ContextHandler
{
    CustomShow mrCustomShow;

public:
    CustomShowContext( ::oox::core::ContextHandler& rParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
            CustomShow& rCustomShow );
    ~CustomShowContext( );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& /*xAttribs*/ )
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
};

CustomShowContext::CustomShowContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& rxAttribs,
        CustomShow& rCustomShow )
: ContextHandler( rParent )
, mrCustomShow( rCustomShow )
{
    mrCustomShow.maName = rxAttribs->getOptionalValue( XML_name );
    mrCustomShow.mnId = rxAttribs->getOptionalValue( XML_id );
}

CustomShowContext::~CustomShowContext( )
{
}

Reference< XFastContextHandler > SAL_CALL CustomShowContext::createFastChildContext( sal_Int32 aElementToken,
    const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_PPT|XML_sld :
            mrCustomShow.maSldLst.push_back( xAttribs->getOptionalValue( NMSP_RELATIONSHIPS | XML_id ) );
        default:
        break;
    }
    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

//---------------------------------------------------------------------------

CustomShowListContext::CustomShowListContext( ContextHandler& rParent,
    std::vector< CustomShow >& rCustomShowList )
: ContextHandler( rParent )
, mrCustomShowList( rCustomShowList )
{
}

CustomShowListContext::~CustomShowListContext( )
{
}

Reference< XFastContextHandler > SAL_CALL CustomShowListContext::createFastChildContext( sal_Int32 aElementToken,
    const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_PPT|XML_custShow :
        {
            CustomShow aCustomShow;
            mrCustomShowList.push_back( aCustomShow );
            xRet = new CustomShowContext( *this, xAttribs, mrCustomShowList.back() );
        }
        default:
        break;
    }
    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}


} }
