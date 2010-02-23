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

#include "headerfootercontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"

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
