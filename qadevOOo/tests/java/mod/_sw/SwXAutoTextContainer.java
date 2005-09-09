/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXAutoTextContainer.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:43:16 $
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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XAutoTextContainer;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.AutoTextContainer</code>.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::text::XAutoTextContainer</code></li>
* </ul>
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.text.XAutoTextContainer
* @see com.sun.star.text.AutoTextContainer
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.text._XAutoTextContainer
*/
public class SwXAutoTextContainer extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.AutoTextContainer</code>.<p>
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XAutoTextContainer  oContainer = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a AutoTextContainer");
        try {
            XMultiServiceFactory myMSF = (XMultiServiceFactory)Param.getMSF();
            Object oInst = myMSF.createInstance
                ("com.sun.star.text.AutoTextContainer");
            oContainer = (XAutoTextContainer)UnoRuntime.queryInterface
                (XAutoTextContainer.class,oInst);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create AutoTextContainer", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oContainer);

        return tEnv;
    }

}    // finish class SwXAutoTextcontainer

