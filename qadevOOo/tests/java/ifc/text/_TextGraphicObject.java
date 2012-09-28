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
package ifc.text;

import com.sun.star.awt.Point;
import com.sun.star.container.XIndexContainer;
import com.sun.star.uno.UnoRuntime;

import java.util.Random;

import lib.MultiPropertyTest;

import util.utils;


/**
 * Testing <code>com.sun.star.text.TextGraphicObject</code>
 * service properties :
 * <ul>
 *  <li><code> ImageMap</code></li>
 *  <li><code> ContentProtected</code></li>
 *  <li><code> SurroundContour</code></li>
 *  <li><code> ContourOutside</code></li>
 *  <li><code> ContourPolyPolygon</code></li>
 *  <li><code> GraphicCrop</code></li>
 *  <li><code> HoriMirroredOnEvenPages</code></li>
 *  <li><code> HoriMirroredOnOddPages</code></li>
 *  <li><code> VertMirrored</code></li>
 *  <li><code> GraphicURL</code></li>
 *  <li><code> GraphicFilter</code></li>
 *  <li><code> ActualSize</code></li>
 *  <li><code> AdjustLuminance</code></li>
 *  <li><code> AdjustContrast</code></li>
 *  <li><code> AdjustRed</code></li>
 *  <li><code> AdjustGreen</code></li>
 *  <li><code> AdjustBlue</code></li>
 *  <li><code> Gamma</code></li>
 *  <li><code> GraphicIsInverted</code></li>
 *  <li><code> Transparency</code></li>
 *  <li><code> GraphicColorMode</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ImageMap'</code> (an inmplementation of
 *  <code>com.sun.star.image.ImageMapObject</code>):
 *   is used to insert a new Map into collection
 *   from 'ImageMap' property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextGraphicObject
 */
public class _TextGraphicObject extends MultiPropertyTest {
    public Random rdm = new Random();

    /**
     * The tester which can change a sequence of <code>Point</code>'s
     * or create a new one if necessary.
     */
    protected PropertyTester PointTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                              throws java.lang.IllegalArgumentException {
            if (utils.isVoid(oldValue)) {
                return newPoint();
            } else {
                return changePoint((Point[][]) oldValue);
            }
        }
    };

    /**
     * Tested with custom <code>PointTester</code>.
     */
    public void _ContourPolyPolygon() {
        log.println("Testing with custom Property tester");
        testProperty("ContourPolyPolygon", PointTester);
    }

    /**
     * Retrieves an ImageMap from relation and inserts it to the collection
     * obtained as property value. Then this collection is set back.
     * After that property value is get again. The number of elements
     * in the old collection and in just gotten collection is checked.
     *
     * Has <b>OK</b> status if the number of elements in the new obtained
     * collection is greater than in old one.
     */
    public void _ImageMap() {
        boolean result = true;

        try {
            XIndexContainer imgMap = UnoRuntime.queryInterface(
                                             XIndexContainer.class,
                                             oObj.getPropertyValue("ImageMap"));
            int previous = imgMap.getCount();
            log.println("Count (previous) " + previous);

            Object im = tEnv.getObjRelation("IMGMAP");
            imgMap.insertByIndex(0, im);
            oObj.setPropertyValue("ImageMap", imgMap);
            imgMap = UnoRuntime.queryInterface(
                             XIndexContainer.class,
                             oObj.getPropertyValue("ImageMap"));

            int after = imgMap.getCount();
            log.println("Count (after) " + after);
            result = previous < after;
        } catch (Exception ex) {
            result = false;
        }

        tRes.tested("ImageMap", result);
    }

    /**
     * Creates a new random points sequence.
     */
    public Point[][] newPoint() {
        Point[][] res = new Point[1][185];

        for (int i = 0; i < res[0].length; i++) {
            res[0][i] = new Point();
            res[0][i].X = rd() * rd() * rd();
            res[0][i].Y = rd() * rd() * rd();
        }

        return res;
    }

    public int rd() {
        return rdm.nextInt(6);
    }

    /**
     * Changes the existing point sequence.
     */
    public Point[][] changePoint(Point[][] oldPoint) {
        Point[][] res = oldPoint;

        for (int i = 0; i < res[0].length; i++) {
            res[0][i].X += 1;
            res[0][i].Y += 1;
        }

        return res;
    }
} // finish class _TextGraphicObject
