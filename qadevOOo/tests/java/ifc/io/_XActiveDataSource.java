/*************************************************************************
 *
 *  $RCSfile: _XActiveDataSource.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:42:28 $
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

package ifc.io;

import lib.MultiMethodTest;

import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XOutputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XActiveDataSource</code>
* interface methods:
* <ul>
*   <li><code>setOutputStream()</code></li>
*   <li><code>getOutputStream()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'OutputStream'</code>
*  (of type <code>com.sun.star.io.OutputStream</code>):
*   acceptable output stream which can be set by <code>setOutputStream</code> </li>
* <ul> <p>
*
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XOutputStream
*/
public class _XActiveDataSource extends MultiMethodTest {

    public XActiveDataSource oObj = null;

    private XOutputStream oStream = null;

    /**
    * Take the XOutputStream from the environment for setting and getting.
    */
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("OutputStream");
        oStream = (XOutputStream) UnoRuntime.queryInterface
                (XOutputStream.class, x) ;
    }

    /**
    * Test calls the method using interface <code>XOutputStream</code>
    * received in method <code>before()</code> as parameter. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setOutputStream() {
        oObj.setOutputStream(oStream);
        tRes.tested("setOutputStream()", true);
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set in the method <code>setOutputStream()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setOutputStream() </code></li>
    * </ul>
    */
    public void _getOutputStream() {
        requiredMethod("setOutputStream()");

        tRes.tested("getOutputStream()",
            oStream.equals(oObj.getOutputStream()));
    }

    /**
    * Forces object environment recreation.
    */
    public void after() {
        tEnv.dispose() ;
    }
}

