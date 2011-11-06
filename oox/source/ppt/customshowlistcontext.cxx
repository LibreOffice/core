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



#include "customshowlistcontext.hxx"

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
        case PPT_TOKEN( sld ) :
            mrCustomShow.maSldLst.push_back( xAttribs->getOptionalValue( R_TOKEN( id ) ) );
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
        case PPT_TOKEN( custShow ) :
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
