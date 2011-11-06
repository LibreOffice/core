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



#include "oox/ppt/slidetimingcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "oox/ppt/backgroundproperties.hxx"
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/ppt/timenodelistcontext.hxx"
#include "buildlistcontext.hxx"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

SlideTimingContext::SlideTimingContext( ContextHandler& rParent, TimeNodePtrList & aTimeNodeList ) throw()
    : ContextHandler( rParent )
    , maTimeNodeList( aTimeNodeList )
{
}

SlideTimingContext::~SlideTimingContext() throw()
{

}

void SlideTimingContext::endFastElement( sal_Int32 /*aElement*/ ) throw ( SAXException, RuntimeException)
{
}


Reference< XFastContextHandler > SlideTimingContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case PPT_TOKEN( bldLst ):
        xRet.set( new BuildListContext( *this, xAttribs, maTimeNodeList ) );
        break;
    case PPT_TOKEN( extLst ):
        return xRet;
    case PPT_TOKEN( tnLst ):
        // timing nodes
    {
        xRet.set( new TimeNodeListContext( *this, maTimeNodeList ) );
    }
    break;

    default:
        break;
    }

    if( !xRet.is() )
        xRet.set(this);

    return xRet;
}

void SAL_CALL SlideTimingContext::endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{

}

} }

