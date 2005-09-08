/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataPointSymbolSupplier.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:47:01 $
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

#include "chartview/DataPointSymbolSupplier.hxx"
#include "ShapeFactory.hxx"
#include "macros.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::com::sun::star::chart2;

uno::Reference< drawing::XShapes > DataPointSymbolSupplier::create2DSymbolList(
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory
            , const uno::Reference< drawing::XShapes >& xTarget
            , const drawing::Direction3D& rSize )
{
    uno::Reference< drawing::XShape > xGroup(
                xShapeFactory->createInstance( C2U(
                "com.sun.star.drawing.GroupShape" ) ), uno::UNO_QUERY );
    if(xTarget.is())
        xTarget->add(xGroup);
    uno::Reference< drawing::XShapes > xGroupShapes =
        uno::Reference<drawing::XShapes>( xGroup, uno::UNO_QUERY );

    ShapeFactory aShapeFactory(xShapeFactory);
    drawing::Position3D  aPos(0,0,0);
    for(sal_Int32 nS=0;nS<ShapeFactory::getSymbolCount();nS++)
    {
        aShapeFactory.createSymbol2D( xGroupShapes, aPos, rSize, nS );
    }
    return xGroupShapes;
}

//.............................................................................
} //namespace chart
//.............................................................................
