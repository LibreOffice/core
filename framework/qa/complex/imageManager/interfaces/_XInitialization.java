/*************************************************************************
 *
 *  $RCSfile: _XInitialization.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-07-23 10:55:01 $
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

package imageManager.interfaces;

import share.LogWriter;


import com.sun.star.lang.XInitialization;
import lib.TestParameters;

/**
* Testing <code>com.sun.star.lang.XInitialization</code>
* interface methods. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInitialization.args'</code> (of type <code>Object[]</code>):
*   (<b>Optional</b>) : argument for <code>initialize</code>
*   method. If ommitet zero length array is used. </li>
* <ul> <p>
* Test is multithread compilant. <p>
* Till the present time there was no need to recreate environment
* after this test completion.
*/
public class _XInitialization {

    LogWriter log = null;
    TestParameters tEnv = null;
    public static XInitialization oObj = null;

    public _XInitialization(LogWriter log, TestParameters tEnv, XInitialization oObj) {
        this.log = log;
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    /**
    * Test calls the method with 0 length array and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public boolean _initialize() {
        boolean result = true ;

        try {
            Object[] args = (Object[]) tEnv.get("XInitialization.args");
            if (args==null) {
                oObj.initialize(new Object[0]);
            } else {
                oObj.initialize(args);
            }

        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while method calling.") ;
            e.printStackTrace((java.io.PrintWriter)log) ;
            result = false ;
        }

        return  result ;
    } // finished _initialize()

    /**
    * Disposes object environment.
    */
    public void after() {
//        disposeEnvironment() ;
    }

} // finished class _XInitialization


