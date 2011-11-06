/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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
