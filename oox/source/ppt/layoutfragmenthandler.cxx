/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layoutfragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:47:25 $
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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/ppt/layoutfragmenthandler.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

// CT_SlideLayout

LayoutFragmentHandler::LayoutFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, SlidePersistPtr pMasterPersistPtr )
    throw()
: SlideFragmentHandler( rFilter, rFragmentPath, pMasterPersistPtr, Layout )
{
}

LayoutFragmentHandler::~LayoutFragmentHandler()
    throw()
{

}

Reference< XFastContextHandler > LayoutFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet = getFastContextHandler();
    switch( aElementToken )
    {
        case NMSP_PPT|XML_sldLayout:        // CT_SlideLayout
            mpSlidePersistPtr->setLayoutValueToken( xAttribs->getOptionalValueToken( XML_type, 0 ) );   // CT_SlideLayoutType
        break;
        default:
            xRet.set( SlideFragmentHandler::createFastChildContext( aElementToken, xAttribs ) );
    }
    return xRet;
}

void SAL_CALL LayoutFragmentHandler::endDocument()
    throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
}

} }

