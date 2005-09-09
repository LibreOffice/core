/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OInteractionHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:02:26 $
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

package mod._dbaccess;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdb.InteractionHandler</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::task::XInteractionHandler</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.task.XInteractionHandler
* @see com.sun.star.sdb.InteractionHandler
* @see ifc.task._XInteractionHandler
*/
public class OInteractionHandler extends TestCase {


    private static class TestRequest implements XInteractionRequest {
        public Object getRequest() {
            return new SQLException("Test exception") ;
        }

        public XInteractionContinuation[] getContinuations() {
            return new XInteractionContinuation[0] ;
        }
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sdb.InteractionHandler</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XInteractionHandler.Request'</code> for
    *      {@link ifc.task._XInteractionHandler} : this realtion
    *    is <code>com.sun.star.task.XInteractionRequest</code>
    *    interface implementation which depends on the component
    *    tested. In this case it emulates SQL error by returning
    *    <code>SQLException</code> object. </li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                            "com.sun.star.sdb.InteractionHandler" );

        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

          oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XInteractionHandler.Request", new TestRequest()) ;

        return tEnv;
    } // finish method getTestEnvironment

}

