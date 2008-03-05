/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshowlistcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:46:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
