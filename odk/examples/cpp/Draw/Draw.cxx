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

#include <iostream>

#include <cppuhelper/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/util/Color.hpp>

using namespace cppu;
using namespace rtl;
using namespace css::uno;
using namespace css::beans;
using namespace css::bridge;
using namespace css::frame;
using namespace css::lang;
using namespace css::text;
using namespace css::drawing;
using namespace css::awt;
using namespace css::container;

css::util::Color getCol(int r, int g, int b);
Reference<XComponent> openDraw(Reference<XComponentContext> xComponentContext);
Reference<XShape> createShape(Reference<XComponent> xDocComp, int height, int width, int x, int y,
                              OUString kind, css::util::Color col);
Reference<XShapeGroup> createSequence(Reference<XComponent> xDocComp, Reference<XDrawPage> xDP);

int main()
{
    Reference<XComponentContext> xContext = NULL;

    try
    {
        // get the remote office component context
        xContext = bootstrap();
    }
    catch (Exception& e)
    {
        std::cout << "Error: cannot do bootstraping." << std::endl << e.Message << std::endl;
        exit(1);
    }

    Reference<XComponent> xDrawDoc = NULL;
    Reference<XDrawPage> xDrawPage = NULL;

    xDrawDoc = openDraw(xContext);

    try
    {
        // getting the draw page
        Reference<XDrawPagesSupplier> xDPS(xDrawDoc, UNO_QUERY);
        Reference<XDrawPages> xDPn = xDPS->getDrawPages();
        Reference<XIndexAccess> xDPi(xDPn, UNO_QUERY);
        xDrawPage = Reference<XDrawPage>(xDPi->getByIndex(0), UNO_QUERY);
    }
    catch (Exception& e)
    {
        std::cout << "Error: Document creation was not possible" << std::endl;
        exit(1);
    }

    createSequence(xDrawDoc, xDrawPage);

    // Drawing some shapes
    Reference<XShapes> xShapes(xDrawPage, UNO_QUERY);
    xShapes->add(createShape(xDrawDoc, 1000, 1300, 2000, 2000, "Line", getCol(0, 0, 0)));
    xShapes->add(createShape(xDrawDoc, 2000, 4000, 15000, 2000, "Ellipse", getCol(0, 200, 200)));
    xShapes->add(createShape(xDrawDoc, 3000, 2500, 5500, 4000, "Rectangle", getCol(100, 100, 200)));

    exit(0);
}

Reference<XComponent> openDraw(Reference<XComponentContext> xContext)
{
    Reference<XComponent> xComp;

    try
    {
        // getting the remote LibreOffice service manager
        Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();

        Reference<XInterface> oDesktop
            = xMCF->createInstanceWithContext("com.sun.star.frame.Desktop", xContext);
        Reference<XComponentLoader> xCLoader(oDesktop, UNO_QUERY);
        Sequence<PropertyValue> szEmptyArgs(0);
        OUString strDoc("private:factory/sdraw");
        xComp = xCLoader->loadComponentFromURL(strDoc, "_blank", 0, szEmptyArgs);
    }
    catch (Exception e)
    {
        std::cout << "Error opening draw." << std::endl << e.Message << std::endl;
        exit(1);
    }

    return xComp;
}

Reference<XShape> createShape(Reference<XComponent> xDocComp, int height, int width, int x, int y,
                              OUString kind, css::util::Color col)
{
    // kind can be either 'Ellipse', 'Line' or 'Rectangle'
    Size size;
    Point position;
    Reference<XShape> xShape;

    // get the multiservice factory
    Reference<XMultiServiceFactory> xDocMSF(xDocComp, UNO_QUERY);

    try
    {
        Reference<XInterface> oInt
            = xDocMSF->createInstance("com.sun.star.drawing." + kind + "Shape");
        xShape = Reference<XShape>(oInt, UNO_QUERY);

        size.Height = height;
        size.Width = width;
        position.X = x;
        position.Y = y;
        xShape->setSize(size);
        xShape->setPosition(position);
    }
    catch (Exception e)
    {
        std::cout << "Could not create instance." << std::endl << e.Message << std::endl;
        exit(1);
    }

    Reference<XPropertySet> xSPS(xShape, UNO_QUERY);

    try
    {
        xSPS->setPropertyValue("FillColor", Any(col));
    }
    catch (Exception e)
    {
        std::cout << "Can not change the shape colors." << std::endl << e.Message << std::endl;
        exit(1);
    }

    return xShape;
}

Reference<XShapeGroup> createSequence(Reference<XComponent> xDocComp, Reference<XDrawPage> xDP)
{
    Size size;
    Point position;
    Reference<XShape> xShape;
    Reference<XShapes> xShapes(xDP, UNO_QUERY);
    int height = 2000;
    int width = 2500;
    int x = 1800;
    int y = 22000;
    Reference<XInterface> oInt;
    int r = 30;
    int g = 0;
    int b = 70;

    // getting the multiservice factory
    Reference<XMultiServiceFactory> xDocMSF(xDocComp, UNO_QUERY);

    for (int i = 0; i < 380; i = i + 30)
    {
        try
        {
            oInt = xDocMSF->createInstance("com.sun.star.drawing.EllipseShape");
            xShape = Reference<XShape>(oInt, UNO_QUERY);
            size.Height = height;
            size.Width = width;
            position.X = x + (i * 40);
            position.Y = y + (i * 40) % 4000;
            xShape->setSize(size);
            xShape->setPosition(position);
        }
        catch (Exception e)
        {
            // Some exception occurs.FAILED
            std::cout << "Could not get Shape." << std::endl << e.Message << std::endl;
            exit(1);
        }

        b = b + 8;

        Reference<XPropertySet> xSPS(xShape, UNO_QUERY);

        try
        {
            xSPS->setPropertyValue("FillColor", Any(getCol(r, g, b)));
            xSPS->setPropertyValue("Shadow", Any(true));
        }
        catch (Exception e)
        {
            std::cout << "Can not change shape colors." << std::endl << e.Message << std::endl;
            exit(1);
        }
        xShapes->add(xShape);
    }

    Reference<XShapeGrouper> xSGrouper(xDP, UNO_QUERY);

    return xSGrouper->group(xShapes);
}

css::util::Color getCol(int r, int g, int b) { return r * 65536 + g * 256 + b; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
