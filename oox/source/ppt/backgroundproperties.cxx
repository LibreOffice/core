/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backgroundproperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:44:25 $
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

#include "oox/ppt/backgroundproperties.hxx"

#include <comphelper/propertysethelper.hxx>

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {
// ---------------------------------------------------------------------

BackgroundPropertiesContext::BackgroundPropertiesContext( ContextHandler& rParent, FillPropertiesPtr pFillPropertiesPtr ) throw()
: ContextHandler( rParent )
, mpFillPropertiesPtr( pFillPropertiesPtr )
{
}

Reference< XFastContextHandler > BackgroundPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_PPT|XML_fill: // a:CT_FillEffect
        break;
    }

    // FillPropertiesGroupContext
    if( !xRet.is() )
        xRet = FillPropertiesGroupContext::StaticCreateContext( *this, aElementToken, xAttribs, *mpFillPropertiesPtr );

    return xRet;
}

} }
