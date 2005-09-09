/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSimpleFileAccess2.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:33:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
            log.println("CommandAbortedException occured while testing "+
                "'writeFile()'");
            ex.printStackTrace(log);
            tRes.tested("writeFile()",false);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occured while testing 'writeFile()'");
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

