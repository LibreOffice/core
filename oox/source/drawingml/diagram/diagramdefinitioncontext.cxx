/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagramdefinitioncontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:38:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include "oox/core/skipcontext.hxx"
#include "layoutnodecontext.hxx"
#include "diagramdefinitioncontext.hxx"
#include "oox/drawingml/diagram/datamodelcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml {


// CT_DiagramDefinition
DiagramDefinitionContext::DiagramDefinitionContext( ContextHandler& rParent,
                                                    const Reference< XFastAttributeList >& xAttributes,
                                                    const DiagramLayoutPtr &pLayout )
    : ContextHandler( rParent )
    , mpLayout( pLayout )
{
    OSL_TRACE( "OOX: DiagramDefinitionContext::DiagramDefinitionContext()" );
    mpLayout->setDefStyle( xAttributes->getOptionalValue( XML_defStyle ) );
    OUString sValue = xAttributes->getOptionalValue( XML_minVer );
    if( sValue.getLength() == 0 )
    {
        sValue = CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/diagram" );
    }
    mpLayout->setMinVer( sValue );
    mpLayout->setUniqueId( xAttributes->getOptionalValue( XML_uniqueId ) );
}


DiagramDefinitionContext::~DiagramDefinitionContext()
{
    mpLayout->getNode()->dump(0);
}

void SAL_CALL DiagramDefinitionContext::endFastElement( ::sal_Int32 )
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramDefinitionContext::createFastChildContext( ::sal_Int32 aElement,
                                                  const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case NMSP_DIAGRAM|XML_title:
        mpLayout->setTitle( xAttribs->getOptionalValue( XML_val ) );
        break;
    case NMSP_DIAGRAM|XML_desc:
        mpLayout->setDesc( xAttribs->getOptionalValue( XML_val ) );
        break;
    case NMSP_DIAGRAM|XML_layoutNode:
        mpLayout->getNode().reset( new LayoutNode() );
        xRet.set( new LayoutNodeContext( *this, xAttribs, mpLayout->getNode() ) );
        break;
     case NMSP_DIAGRAM|XML_clrData:
        // TODO, does not matter for the UI. skip.
        xRet.set( new SkipContext( *this ) );
        break;
    case NMSP_DIAGRAM|XML_sampData:
        mpLayout->getSampData().reset( new DiagramData );
        xRet.set( new DataModelContext( *this, mpLayout->getSampData() ) );
        break;
    case NMSP_DIAGRAM|XML_styleData:
        mpLayout->getStyleData().reset( new DiagramData );
        xRet.set( new DataModelContext( *this, mpLayout->getStyleData() ) );
        break;
    case NMSP_DIAGRAM|XML_cat:
    case NMSP_DIAGRAM|XML_catLst:
        // TODO, does not matter for the UI
    default:
        break;
    }
    if( !xRet.is() )
        xRet.set(this);

    return xRet;
}


} }
