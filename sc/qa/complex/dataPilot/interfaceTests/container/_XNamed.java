/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XNamed.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:06:39 $
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

package complex.dataPilot.interfaceTests.container;

import com.sun.star.container.XNamed;
import lib.TestParameters;
import share.LogWriter;
import util.utils;

/**
* Testing <code>com.sun.star.container.XNamed</code>
* interface methods :
* <ul>
*  <li><code> getName()</code></li>
*  <li><code> setName()</code></li>
* </ul>
* This test need the following object relations :
* <ul>
*  <li> <code>'setName'</code> : of <code>Boolean</code>
*    type. If it exists then <code>setName</code> method
*    isn't to be tested and result of this test will be
*    equal to relation value.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XNamed
*/
public class _XNamed {

    /**
     * The object that is testsed.
     */
    public XNamed oObj = null;

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     * The log writer
     */
    private LogWriter log = null;

    /**
     * Constructor: gets the object to test, a logger and the test parameters
     * @param xObj The test object
     * @param log A log writer
     * @param param The test parameters
     */
    public _XNamed(XNamed xObj, LogWriter log, TestParameters param) {
        oObj = xObj;
        this.log = log;
        this.param = param;
    }

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public boolean _getName() {

        // write to log what we try next
        log.println( "test for getName()" );

        boolean result = true;
        boolean loc_result = true;
        String name = null;
        String NewName = null;

        loc_result = ((name = oObj.getName()) != null);
        log.println("getting the name \"" + name + "\"");

        if (loc_result) log.println("... getName() - OK");
        else log.println("... getName() - FAILED");
        result &= loc_result;
        return result;
    }

    /**
    * Sets a new name for object and checks if it was properly
    * set. Special cases for the following objects :
    * <ul>
    *  <li><code>ScSheetLinkObj</code> : name must be in form of URL.</li>
    *  <li><code>ScDDELinkObj</code> : name must contain link to cell in
    *     some external Sheet.</li>
    * </ul>
    * Has <b> OK </b> status if new name was successfully set, or if
    * object environment contains relation <code>'setName'</code> with
    * value <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getName() </code> : to be sure the method works</li>
    * </ul>
    */
    public boolean _setName(){
//        requiredMethod("getName()");
        log.println("testing setName() ... ");

        String oldName = oObj.getName();
        String NewName = oldName == null ? "XNamed" : oldName + "X" ;

        boolean result = true;
        boolean loc_result = true;
        log.println("set the name of object to \"" + NewName + "\"");
        oObj.setName(NewName);
        log.println("check that container has element with this name");

        String name = oObj.getName();
        log.println("getting the name \"" + name + "\"");
        loc_result = name.equals(NewName);

        if (loc_result) log.println("... setName() - OK");
                        else log.println("... setName() - FAILED");
        result &= loc_result;
        oObj.setName(oldName);
        return result;
    }
}


