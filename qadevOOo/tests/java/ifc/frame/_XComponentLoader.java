/*************************************************************************
 *
 *  $RCSfile: _XComponentLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:37:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            String url = util.utils.getFullTestURL("Writer_Link.sxw");
            oDoc = oObj.loadComponentFromURL(
                            url, frameName, 0, szArgs);

            try {
                Thread.sleep(500);
            }
            catch (InterruptedException ex) {
            }

            oDoc.dispose();

            url = util.utils.getFullTestURL("Calc_Link.sxc");
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
            log.println("Exception occured while loading");
            e.printStackTrace(log);
            result=false;
        }
        catch (com.sun.star.io.IOException e) {
            log.println("Exception occured while loading");
            e.printStackTrace(log);
            result=false;
        }

        tRes.tested("loadComponentFromURL()", result);
        return;
    }

}

