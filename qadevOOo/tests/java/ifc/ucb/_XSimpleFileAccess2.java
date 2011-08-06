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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
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
            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String fileURL = dirnameTo + "XSimpleFileAccess_new.txt";
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            XInputStream iStream = oObj.openFileRead(filename);
            oObj.writeFile(fileURL,iStream);
            shortWait();
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

    /**
    * Sleeps for 1 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

}  // finish class _XSimpleFileAccess

