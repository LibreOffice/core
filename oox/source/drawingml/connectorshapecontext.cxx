/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connectorshapecontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:17:43 $
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

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

ConnectorShapeContext::ConnectorShapeContext( ContextHandler& rParent,
        ShapePtr pMasterShapePtr, ShapePtr pGroupShapePtr )
: ShapeContext( rParent, pMasterShapePtr, pGroupShapePtr )
{
}

ConnectorShapeContext::~ConnectorShapeContext()
{
}

Reference< XFastContextHandler > ConnectorShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getToken( aElementToken ) )
    {
        case XML_nvCxnSpPr :
        break;

        default:
            xRet = ShapeContext::createFastChildContext( aElementToken, xAttribs );
    }
    if( !xRet.is() )
        xRet.set( this );


    return xRet;
}

} }
