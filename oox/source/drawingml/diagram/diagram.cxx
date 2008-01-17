/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagram.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:57 $
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


#include <functional>
#include <boost/bind.hpp>

#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"


namespace oox { namespace drawingml {

namespace dgm {


void Connection::dump()
{
    OSL_TRACE("dgm: cnx modelId %s, srcId %s, dstId %s",
              OUSTRING_TO_CSTR( msModelId ),
              OUSTRING_TO_CSTR( msSourceId ),
              OUSTRING_TO_CSTR( msDestId ) );
}

Point::Point()
    : mpShape( new Shape( "com.sun.star.drawing.GroupShape" ) )
    , mnType( 0 )
{
}

void Point::dump()
{
    OSL_TRACE( "dgm: pt cnxId %s, modelId %s",
               OUSTRING_TO_CSTR( msCnxId ),
               OUSTRING_TO_CSTR( msModelId ) );
}


}


DiagramData::DiagramData()
    : mpFillProperties( new FillProperties( ) )
{
}

void DiagramData::dump()
{
    OSL_TRACE("Dgm: DiagramData # of cnx: %d", maConnections.size() );
    std::for_each( maConnections.begin(), maConnections.end(),
                  boost::bind( &dgm::Connection::dump, _1 ) );
    OSL_TRACE("Dgm: DiagramData # of pt: %d", maPoints.size() );
    std::for_each( maPoints.begin(), maPoints.end(),
                  boost::bind( &dgm::Point::dump, _1 ) );
}


void Diagram::setData( const DiagramDataPtr & pData)
{
    mpData = pData;
}


void Diagram::setLayout( const DiagramLayoutPtr & pLayout)
{
    mpLayout = pLayout;
}

void Diagram::setQStyles( const DiagramQStylesPtr & pStyles)
{
    mpQStyles = pStyles;
}


void Diagram::setColors( const DiagramColorsPtr & pColors)
{
    mpColors = pColors;
}


void Diagram::addTo( const ShapePtr & pShape )
{
    dgm::Points & aPoints( mpData->getPoints( ) );
    std::for_each( aPoints.begin(), aPoints.end(),
                   boost::bind( &Shape::addChild, boost::ref( pShape ),
                                boost::bind( &dgm::Point::getShape, _1 ) ) );

    OSL_TRACE( "Dgm: addTo() # of childs %d", pShape->getChilds().size() );
    for( std::vector< ShapePtr >::iterator iter = pShape->getChilds().begin();
         iter != pShape->getChilds().end(); ++iter)
    {
        OSL_TRACE( "Dgm: shape name %s", OUSTRING_TO_CSTR( (*iter)->getName() ) );
    }
}


} }
