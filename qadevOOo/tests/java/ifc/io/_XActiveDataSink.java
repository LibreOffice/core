/*************************************************************************
 *
 *  $RCSfile: _XActiveDataSink.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:42:18 $
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

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XActiveDataSink</code>
* interface methods :
* <ul>
*  <li><code> setInputStream()</code></li>
*  <li><code> getInputStream()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'InputStream'</code>
*  (of type <code>com.sun.star.io.XInputStream</code>):
*   acceptable input stream which can be set by <code>setInputStream</code> </li>
* <ul> <p>
*
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XActiveDataSink
*/
public class _XActiveDataSink extends MultiMethodTest {

    public XActiveDataSink oObj = null;

    private XInputStream iStream = null;

        /**
        * Take the XInputStream from the environment for setting and getting
    */
    public void before() {
            XInterface x = (XInterface)tEnv.getObjRelation("InputStream");
            iStream = (XInputStream) UnoRuntime.queryInterface
                    (XInputStream.class, x) ;
    }

    /**
    * Just sets new input stream. <p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _setInputStream() {
        oObj.setInputStream(iStream) ;

        tRes.tested("setInputStream()", true) ;
    }

    /**
    * First retrieves current input stream, then sets to new
    * input stream (if old was <code>null</code>) or to null.
    * Then input stream retrieved again and checked to be not
    * equal to the old one. <p>
    * Has <b>OK</b> status if old and new streams retrieved are
    * not equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setInputStream() </code> : to be sure the method
    *   works without exceptions. </li>
    * </ul>
    */
    public void _getInputStream() {
        requiredMethod("setInputStream()") ;

        Object oldStream = oObj.getInputStream() ;
        XInputStream newStream = oldStream == null ? iStream : null ;

        oObj.setInputStream(newStream) ;
        Object getStream = oObj.getInputStream() ;

                tRes.tested("getInputStream()", getStream != oldStream) ;
    }

    public void after() {
        tEnv.dispose() ;
    }
}


