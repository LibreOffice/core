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

#include <list>
#include <algorithm>

#include <rtl/ustring.hxx>

#include "oox/core/namespaces.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "texttabstoplistcontext.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

        TextTabStopListContext::TextTabStopListContext( ContextHandler& rParent, std::list< TabStop >  & aTabList )
            : ContextHandler( rParent )
            , maTabList( aTabList )
        {
        }

        TextTabStopListContext::~TextTabStopListContext()
        {
        }

        void SAL_CALL TextTabStopListContext::endFastElement( ::sal_Int32 /*Element*/ )
            throw ( SAXException, RuntimeException)
        {
        }


    Reference< ::XFastContextHandler > TextTabStopListContext::createFastChildContext( ::sal_Int32 aElement,
                                                                                                                                                                             const Reference< XFastAttributeList >& xAttribs )
            throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;
            switch( aElement )
            {
            case NMSP_DRAWINGML|XML_tab:
            {
                OUString sValue;
                TabStop aTabStop;
                sValue = xAttribs->getOptionalValue( XML_pos );
                if( sValue.getLength() )
                {
                    aTabStop.Position = GetCoordinate( sValue );
                }
                sal_Int32 aToken = xAttribs->getOptionalValueToken( XML_algn, 0 );
                if( aToken != 0 )
                {
                    aTabStop.Alignment = GetTabAlign( aToken );
                }
                maTabList.push_back(aTabStop);
                break;
            }
            default:
                break;
            }
            if ( !xRet.is() )
                xRet.set( this );
            return xRet;
        }


} }


