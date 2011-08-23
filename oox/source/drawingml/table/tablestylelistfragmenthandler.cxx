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

#include "oox/drawingml/table/tablestylelistfragmenthandler.hxx"
#include "oox/drawingml/table/tablestylecontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {
namespace table {

// ============================================================================

TableStyleListFragmentHandler::TableStyleListFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, TableStyleList& rTableStyleList ): 
FragmentHandler2( rFilter, rFragmentPath ),
mrTableStyleList( rTableStyleList )
{
}

TableStyleListFragmentHandler::~TableStyleListFragmentHandler()
{
}

// CT_TableStyleList
Reference< XFastContextHandler > TableStyleListFragmentHandler::createFastChildContext(
    sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_tblStyleLst:	// CT_TableStyleList
            mrTableStyleList.getDefaultStyleId() = xAttribs->getOptionalValue( XML_def );
            break;
        case NMSP_DRAWINGML|XML_tblStyle:		// CT_TableStyle
            std::vector< TableStyle >& rTableStyles = mrTableStyleList.getTableStyles();
            rTableStyles.resize( rTableStyles.size() + 1 );
            xRet = new TableStyleContext( *this, xAttribs, rTableStyles.back() );
            break;
    }
    if ( !xRet.is() )
        xRet = getFastContextHandler();
    return xRet;
}

// ============================================================================

} // namespace table
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
