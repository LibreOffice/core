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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.io.XInputStream;
import com.sun.star.ucb.XSimpleFileAccess2;

/**
* Testing <code>com.sun.star.ucb.XSimpleFileAccess2</code>
* interface methods. <p>
* @see com.sun.star.ucb.XSimpleFileAccess2
*/
public class _XSimpleFileAccess2 extends MultiMethodTest {

    public static XSimpleFileAccess2 oObj = null;

    /**
    * Writes <b>XSimpleFileAccess_new.txt</b> to disk, checks
    * if it was successfully created and then deletes it. <p>
    * Has <b> OK </b> status if after method call the file
    * exists and no exceptions were thrown. <p>
    */
    public void _writeFile() {
        boolean result = true;
        try {
            String dirnameTo = util.utils.getOfficeTemp(tParam.getMSF()) ;
            String fileURL = dirnameTo + "XSimpleFileAccess_new.txt";
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            XInputStream iStream = oObj.openFileRead(filename);
            oObj.writeFile(fileURL,iStream);
            util.utils.pause(1000);
            result = oObj.exists(fileURL);
            oObj.kill(fileURL);
            tRes.tested("writeFile()",result);
        }
        catch (com.sun.star.ucb.CommandAbortedException ex) {
            log.println("CommandAbortedException occurred while testing "+
                "'writeFile()'");
            ex.printStackTrace(log);
            tRes.tested("writeFile()",false);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'writeFile()'");
            ex.printStackTrace(log);
            tRes.tested("writeFile()",false);
        }

    } //EOF writeFile()

}  // finish class _XSimpleFileAccess

