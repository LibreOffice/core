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



#include "headerfootercontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    HeaderFooterContext::HeaderFooterContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& xAttribs, HeaderFooter& rHeaderFooter )
        : ContextHandler( rParent )
    {
        AttributeList aAttribs( xAttribs );
        if ( xAttribs->hasAttribute( XML_sldNum ) )
        {
            rHeaderFooter.mbSlideNumber = aAttribs.getBool( XML_sldNum, sal_True );
        }
        if ( xAttribs->hasAttribute( XML_hdr ) )
        {
            rHeaderFooter.mbHeader = aAttribs.getBool( XML_hdr, sal_True );
        }
        if ( xAttribs->hasAttribute( XML_ftr ) )
        {
            rHeaderFooter.mbFooter = aAttribs.getBool( XML_ftr, sal_True );
        }
        if ( xAttribs->hasAttribute( XML_dt ) )
        {
            rHeaderFooter.mbDateTime = aAttribs.getBool( XML_dt, sal_True );
        }
    }

    HeaderFooterContext::~HeaderFooterContext( )
    {
    }

} }
