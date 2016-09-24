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

package ifc.drawing;

import lib.MultiMethodTest;
import util.ValueComparer;
import util.XInstCreator;

import com.sun.star.drawing.XLayer;
import com.sun.star.drawing.XLayerManager;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.drawing.XLayerManager</code>
* interface methods :
* <ul>
*  <li><code> insertNewByIndex()</code></li>
*  <li><code> remove()</code></li>
*  <li><code> attachShapeToLayer()</code></li>
*  <li><code> getLayerForShape()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Shape'</code> (of type <code>util.XInstCreator</code>):
*   instance creator which can create shapes.</li>
*  <li> <code>'Shapes'</code>
*   (of type <code>com.sun.star.drawing.XShapes</code>):
*   The collection of shapes in the document. Is used
*   to add new created shapes.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XLayerManager
*/
public class _XLayerManager extends MultiMethodTest {

    public XLayerManager oObj = null;                // oObj filled by MultiMethodTest
    XInstCreator shape = null;
    public XInterface oShape = null;
    public XLayer oL = null;

    /**
     * Inserts a new layer into collection. <p>
     * Has <b> OK </b> status if the value returned is not null. <p>
     */
    public void _insertNewByIndex(){
        log.println("insertNewByName() ... ");
        oL = oObj.insertNewByIndex(0);
        tRes.tested("insertNewByIndex()", oL != null);
    }

    /**
    * First a shape created and inserted into the document using
    * relations retrieved. Attaches this shape to layer created before. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertNewByIndex </code> : to have a layer attach to.</li>
    * </ul>
    */
    public void _attachShapeToLayer() {
        requiredMethod("insertNewByIndex()");
        shape = (XInstCreator)tEnv.getObjRelation("Shape");
        oShape = shape.createInstance();
        XShape oSh = (XShape) oShape;
        XShapes oShapes = (XShapes) tEnv.getObjRelation("Shapes");
        oShapes.add(oSh);
        boolean result = false;

        log.println("attachShapeToLayer() ... ");

        oObj.attachShapeToLayer((XShape) oShape,oL);
        result = true;

        tRes.tested("attachShapeToLayer()", result);
    }

    /**
    * Gets the layer for shape which was attached before. <p>
    * Has <b> OK </b> status if the names of layer get and
    * the layer attached before are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> attachShapeToLayer() </code>  </li>
    * </ul>
    */
    public void _getLayerForShape() {
        requiredMethod("attachShapeToLayer()");
        log.println("getLayerForShape() ... ");
        XLayer Lay1 = oL;
        XLayer Lay2 = oObj.getLayerForShape((XShape)oShape);
        Object Obj1 = null;
        Object Obj2 = null;

        try {
            Obj1 = Lay1.getPropertyValue("Name");
            Obj2 = Lay2.getPropertyValue("Name");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        }

        tRes.tested("getLayerForShape()",ValueComparer.equalValue(Obj1,Obj2));
    }

    /**
    * Test removes the layer added before. Number of layers are get before
    * and after removing.<p>
    * Has <b> OK </b> status if number of layers decreases by one. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLayerForShape() </code>  </li>
    * </ul>
    */
    public void _remove () {
        requiredMethod("getLayerForShape()");
        boolean result = true ;
        // get the current thread's holder
        log.println("removing the Layer...");

        int cntBefore = oObj.getCount();

        try {
            oObj.remove(oL);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            result = false;
        }

        int cntAfter = oObj.getCount();

        result = cntBefore == cntAfter + 1;

        tRes.tested("remove()", result);
    }
}


