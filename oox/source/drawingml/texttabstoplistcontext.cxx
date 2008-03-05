/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texttabstoplistcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:31:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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


