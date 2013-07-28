/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "chartview/DataPointSymbolSupplier.hxx"
#include "ShapeFactory.hxx"
#include "macros.hxx"

namespace chart
{
using namespace ::com::sun::star;

uno::Reference< drawing::XShapes > DataPointSymbolSupplier::create2DSymbolList(
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory
            , const uno::Reference< drawing::XShapes >& xTarget
            , const drawing::Direction3D& rSize )
{
    uno::Reference< drawing::XShape > xGroup(
                xShapeFactory->createInstance(
                "com.sun.star.drawing.GroupShape" ), uno::UNO_QUERY );
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

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
