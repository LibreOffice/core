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

package ifc.frame;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;


/**
* Testing <code>com.sun.star.frame.XComponentLoader</code>
* interface methods:
* <ul>
*  <li><code> loadComponentFromURL() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XComponentLoader
*/
public class _XComponentLoader extends MultiMethodTest {
    public XComponentLoader oObj = null; // oObj filled by MultiMethodTest

    /**
    * Method which tests the objects ability to load a
    * component from URL. All available components are loaded by turns. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * @see XComponent
    */
    public void _loadComponentFromURL() {
        boolean result = true;

        log.println("testing loadComponentFromURL() ... ");
        try {
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String frameName = "_blank";
            XComponent oDoc = null;

            log.println("load writer doc that contains links");
            PropertyValue [] szArgs = new PropertyValue [1];
            PropertyValue Arg = new PropertyValue();
            Arg.Name = "UpdateDocMode";
            Arg.Value = new Short(com.sun.star.document.UpdateDocMode.NO_UPDATE);
            szArgs[0]=Arg;
            String url = util.utils.getFullTestURL("Writer_link.sxw");
            log.println("try to load '" + url + "'");
            oDoc = oObj.loadComponentFromURL(
                            url, frameName, 0, szArgs);

            try {
                Thread.sleep(500);
            }
            catch (InterruptedException ex) {
            }

            oDoc.dispose();

            url = util.utils.getFullTestURL("Calc_Link.sxc");
            log.println("try to load '" + url + "'");
            oDoc = oObj.loadComponentFromURL(
                            url, frameName, 0, szArgs);

            try {
                Thread.sleep(500);
            }
            catch (InterruptedException ex) {
            }

            oDoc.dispose();



            log.println("load a blank impress doc");
            Arg.Name = "OpenFlags";
            Arg.Value = "S";
            Arg.Handle = -1;
            Arg.State = PropertyState.DEFAULT_VALUE;
            szArgs[0]=Arg;
            oDoc = oObj.loadComponentFromURL(
                            "private:factory/simpress", frameName, 0, szArgs );

            log.println("disposing impress doc");
            oDoc.dispose();

            log.println("load a blank writer doc");
            oDoc = oObj.loadComponentFromURL(
                        "private:factory/swriter", frameName, 0, szEmptyArgs );

            log.println("disposing writer doc");
            oDoc.dispose();

            log.println("load a blank calc doc");
            oDoc = oObj.loadComponentFromURL(
                        "private:factory/scalc", frameName, 0, szEmptyArgs );

            log.println("disposing calc doc");
            oDoc.dispose();

            log.println("load a blank draw doc");
            oDoc = oObj.loadComponentFromURL(
                        "private:factory/sdraw", frameName, 0, szEmptyArgs );

            log.println("disposing draw doc");
            oDoc.dispose();

            log.println("load a blank math doc");
            oDoc = oObj.loadComponentFromURL(
                        "private:factory/smath", frameName, 0, szEmptyArgs );

            log.println("disposing math doc");
            oDoc.dispose();

        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occurred while loading");
            e.printStackTrace(log);
            result=false;
        }
        catch (com.sun.star.io.IOException e) {
            log.println("Exception occurred while loading");
            e.printStackTrace(log);
            result=false;
        }

        tRes.tested("loadComponentFromURL()", result);
        return;
    }

}

