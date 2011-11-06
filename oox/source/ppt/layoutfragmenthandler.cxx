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



#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "headerfootercontext.hxx"
#include "oox/ppt/layoutfragmenthandler.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

// CT_SlideLayout

LayoutFragmentHandler::LayoutFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, SlidePersistPtr pMasterPersistPtr )
    throw()
: SlideFragmentHandler( rFilter, rFragmentPath, pMasterPersistPtr, Layout )
{
}

LayoutFragmentHandler::~LayoutFragmentHandler()
    throw()
{

}

Reference< XFastContextHandler > LayoutFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet = getFastContextHandler();
    switch( aElementToken )
    {
        case PPT_TOKEN( sldLayout ):        // CT_SlideLayout
            mpSlidePersistPtr->setLayoutValueToken( xAttribs->getOptionalValueToken( XML_type, 0 ) );   // CT_SlideLayoutType
        break;
        case PPT_TOKEN( hf ):               // CT_HeaderFooter
            xRet.set( new HeaderFooterContext( *this, xAttribs, mpSlidePersistPtr->getHeaderFooter() ) );
        break;
        default:
            xRet.set( SlideFragmentHandler::createFastChildContext( aElementToken, xAttribs ) );
    }
    return xRet;
}

void SAL_CALL LayoutFragmentHandler::endDocument()
    throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
}

} }

