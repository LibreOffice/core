/*************************************************************************
 *
 *  $RCSfile: _XImporter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:27:05 $
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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XImporter;
import com.sun.star.lang.XComponent;

/**
* Testing <code>com.sun.star.document.XImporter</code>
* interface methods :
* <ul>
*  <li><code> setTargetDocument()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'TargetDocument'</code> (of type <code>XComponent</code>):
*   the target document to be passed to the method. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XImporter
*/
public class _XImporter extends MultiMethodTest {

    public XImporter oObj = null;

    /**
    * Retrieves relation and sets target document. <p>
    * Has <b> OK </b> status if no runtime exceptions occured,
    * really this method tested when the whole import result
    * checked.
    */
    public void _setTargetDocument() {
        boolean res = true;
        try {
            XComponent the_doc = (XComponent)
                tEnv.getObjRelation("TargetDocument");
            oObj.setTargetDocument(the_doc);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'setTargetDocument'");
            ex.printStackTrace(log);
            res = false;
        }

        tRes.tested("setTargetDocument()",res);

    }

}  // finish class _XImporter

