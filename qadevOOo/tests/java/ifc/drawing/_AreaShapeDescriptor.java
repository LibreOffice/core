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

public class _AreaShapeDescriptor extends MultiPropertyTest {

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

    protected PropertyTester StringTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(str1))
                return str2; else
                return str1;
        }
    } ;

    public void _FillBitmapURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("FillBitmapURL", URLTester) ;
    }

    public String str1 = "";
    public String str2 = "";

    public void _FillGradientName() {
        log.println("Testing with custom Property tester") ;
        str1 = "Gradient 1";
        str2 = "Gradient 3";
        testProperty("FillGradientName", StringTester) ;
    }

    public void _FillBitmapName() {
        log.println("Testing with custom Property tester") ;
        str1 = "Sky";
        str2 = "Blank";
        testProperty("FillBitmapName", StringTester) ;
    }

    public void _FillTransparenceGradientName() {
        log.println("Testing with custom Property tester") ;
        str1 = "Standard 1";
        str2 = "Standard 2";
        testProperty("FillTransparenceGradientName", StringTester) ;
    }

    public void _FillHatchName() {
        log.println("Testing with custom Property tester") ;
        str1 = "Black 0 degrees";
        str2 = "Black 45 degrees";
        testProperty("FillHatchName", StringTester) ;
    }

    public void _FillBitmapMode() {
        log.println("Testing with custom Property tester") ;
        try {
            Object getting = oObj.getPropertyValue("FillBitmapMode");
            if (! (getting instanceof com.sun.star.drawing.BitmapMode)) {
                log.println("getting the property 'FillBitmapMode'");
                log.println("return "+ oObj.getClass().getName());
                log.println("Expected was 'com.sun.star.drawing.BitmapMode'");
                tRes.tested("FillBitmapMode",false);
            } else testProperty("FillBitmapMode");
        }
        catch (Exception ex) {
            log.println("Exception while checking 'FillBitmapMode'");
            ex.printStackTrace(log);
            tRes.tested("FillBitmapMode",false);
        }

    }

}


