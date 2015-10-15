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
import com.sun.star.container.XIndexContainer;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.GraphicObjectShape</code>
* service properties :
* <ul>
*  <li><code> GraphicURL</code></li>
*  <li><code> GraphicStreamURL</code></li>
*  <li><code> GraphicObjectFillBitmap</code></li>
*  <li><code> AdjustLuminance</code></li>
*  <li><code> AdjustContrast</code></li>
*  <li><code> AdjustRed</code></li>
*  <li><code> AdjustGreen</code></li>
*  <li><code> AdjustBlue</code></li>
*  <li><code> Gamma</code></li>
*  <li><code> Transparency</code></li>
*  <li><code> GraphicColorMode</code></li>
*  <li><code> ImageMap</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Bitmap1', 'Bitmap2'</code> (of type <code>XBitmap</code>):
*   values to be set for property GraphicObjectFillBitmap </li>
*  <li> <code>'IMAP'</code>
*   (<code>com.sun.star.image.ImageMapRectangleObject</code> service instance):
*   is used to be added to ImageMap container.</li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.GraphicObjectShape
*/
public class _GraphicObjectShape extends MultiPropertyTest {

    /**
     * Property tester which changes URL.
     */
    protected PropertyTester URLTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    /**
     * Property tester which switches two XBitmap objects.
     * It also uses two object relations with bitmaps.
     */
    protected PropertyTester BitmapTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue){
            XBitmap aBitmap1=(XBitmap) tEnv.getObjRelation("Bitmap1");
            XBitmap aBitmap2=(XBitmap) tEnv.getObjRelation("Bitmap2");
            if (oldValue.equals(aBitmap1)) return aBitmap2;
            else return aBitmap1;
        }
    } ;

    /**
     * Property tester which returns new <code>XIndexAccess</code> object.
     */
    protected PropertyTester ImapTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            return null;
        }
    } ;


    /**
     * This property must have URL format
     */
    public void _GraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicURL", URLTester) ;
    }

    public void _GraphicStreamURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicStreamURL", URLTester) ;
    }

    public void _GraphicObjectFillBitmap() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicObjectFillBitmap", BitmapTester) ;
    }

    /**
     * The test first retrieves ImageMap relation, then inserts it
     * to the current container.
     */
    public void _ImageMap() {
        if (! util.utils.hasPropertyByName(oObj,"ImageMap")) {
            log.println("optional property 'ImageMap' isn't available");
            tRes.tested("ImageMap",true);
            return;
        }
        try {
            boolean result = true;
            Object imapObject = tEnv.getObjRelation("ImapObject");

            if ( imapObject == null){
                System.out.println("ERROR: object relation 'ImapObject' isn't available");
                tRes.tested("ImageMap", false);
                return;
            }

            Object o = oObj.getPropertyValue("ImageMap");
            XIndexContainer xIndexContainer = UnoRuntime.queryInterface(XIndexContainer.class, o);
            util.dbg.printInterfaces(xIndexContainer);
            int elementCountFirst = xIndexContainer.getCount();
            xIndexContainer.insertByIndex(elementCountFirst, imapObject);

            // this does not really change the property: the implementation
            // behind "ImageMap" stays the same, but for a real change a C++
            // implementation is needed. See css.lang.XUnoTunnel
            oObj.setPropertyValue("ImageMap", xIndexContainer);
            Object newObject = oObj.getPropertyValue("ImageMap");
            xIndexContainer = UnoRuntime.queryInterface(XIndexContainer.class, newObject);

            int elementCountSecond = xIndexContainer.getCount();
            result = (elementCountFirst + 1 == elementCountSecond);

            tRes.tested("ImageMap", result);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        }
        catch(Exception e) {
            e.printStackTrace(log);
        }
    }

}

