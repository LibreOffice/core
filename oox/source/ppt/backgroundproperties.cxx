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



#include "oox/ppt/backgroundproperties.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {
// ---------------------------------------------------------------------

BackgroundPropertiesContext::BackgroundPropertiesContext( ContextHandler& rParent, ::oox::drawingml::FillProperties& rFillProperties ) throw()
: ContextHandler( rParent )
, mrFillProperties( rFillProperties )
{
}

Reference< XFastContextHandler > BackgroundPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case PPT_TOKEN( fill ): // a:CT_FillEffect
        break;
    }

    // FillPropertiesGroupContext
    if( !xRet.is() )
        xRet = ::oox::drawingml::FillPropertiesContext::createFillContext( *this, aElementToken, xAttribs, mrFillProperties );

    return xRet;
}

} }
