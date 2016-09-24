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

import lib.MultiPropertyTest;

import com.sun.star.awt.XBitmap;

public class _GraphicObjectShapeDescriptor extends MultiPropertyTest {

    @Override
    protected boolean compare(Object ob1, Object ob2) {

        return super.compare(ob1, ob2);

    }

    protected PropertyTester URLTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    protected PropertyTester BitmapTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            XBitmap aBitmap1=(XBitmap) tEnv.getObjRelation("Bitmap1");
            XBitmap aBitmap2=(XBitmap) tEnv.getObjRelation("Bitmap2");
            if (oldValue.equals(aBitmap1)) return aBitmap2; else return aBitmap1;
        }
    } ;

    public void _GraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicURL", URLTester) ;
    }

    public void _GraphicObjectFillBitmap() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicObjectFillBitmap", BitmapTester) ;
    }

}

