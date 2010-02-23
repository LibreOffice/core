/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.drawing;

import lib.MultiPropertyTest;

public class _AreaShapeDescriptor extends MultiPropertyTest {

    protected boolean compare(Object ob1, Object ob2) {

        return super.compare(ob1, ob2);

    }

    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    protected PropertyTester StringTester = new PropertyTester() {
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


