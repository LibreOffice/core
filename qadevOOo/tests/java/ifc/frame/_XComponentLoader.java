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
* Test is <b> NOT </b> multithread compliant. <p>
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
            Arg.Value = Short.valueOf(com.sun.star.document.UpdateDocMode.NO_UPDATE);
            szArgs[0]=Arg;
            String url = util.utils.getFullTestURL("Writer_link.sxw");
            log.println("try to load '" + url + "'");
            oDoc = oObj.loadComponentFromURL(
                            url, frameName, 0, szArgs);

            util.utils.shortWait();

            oDoc.dispose();

            url = util.utils.getFullTestURL("Calc_Link.sxc");
            log.println("try to load '" + url + "'");
            oDoc = oObj.loadComponentFromURL(
                            url, frameName, 0, szArgs);

            util.utils.shortWait();

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
    }

}

