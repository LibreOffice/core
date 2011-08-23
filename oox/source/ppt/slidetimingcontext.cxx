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

#include "oox/ppt/slidetimingcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "oox/ppt/backgroundproperties.hxx"
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/ppt/timenodelistcontext.hxx"
#include "buildlistcontext.hxx"
#include "tokens.hxx"

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
    case NMSP_PPT|XML_bldLst:
        xRet.set( new BuildListContext( *this, xAttribs, maTimeNodeList ) );
        break;
    case NMSP_PPT|XML_extLst:
        return xRet;
    case NMSP_PPT|XML_tnLst:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
