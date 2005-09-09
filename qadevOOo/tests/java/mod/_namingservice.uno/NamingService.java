/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamingService.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:34:49 $
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

package mod._namingservice.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.uno.NamingService</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::uno::XNamingService</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.uno.XNamingService
* @see com.sun.star.uno.NamingService
* @see ifc.uno._XNamingService
*/
public class NamingService extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.uno.NamingService</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XNamingService.RegisterObject'</code> for
    *      {@link ifc.uno._XNamingService} : an object
    *      which can be registered. Here it is an instance
    *      of <code>com.sun.star.lang.ServiceManager</code>
    *      service.</li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.uno.NamingService" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // objRelation for XNamingService as object to be registered
        try {
            tEnv.addObjRelation("XNamingService.RegisterObject",
                xMSF.createInstance
                ("com.sun.star.lang.ServiceManager")) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create object relation") ;
            throw new StatusException("Can't create object relation", e) ;
        }

        return tEnv;
    } // finish method getTestEnvironment

}

