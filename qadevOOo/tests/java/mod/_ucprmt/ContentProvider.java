/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContentProvider.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:53:55 $
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

package mod._ucprmt;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.RemoteAccessContentProvider</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ucb::XContentProvider</code></li>
*  <li> <code>com::sun::star::ucb::XFileIdentifierConverter</code></li>
*  <li> <code>com::sun::star::ucb::XContentIdentifierFactory</code></li>
*  <li> <code>com::sun::star::ucb::XParameterizedContentProvider</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.ucb.XContentProvider
* @see com.sun.star.ucb.XFileIdentifierConverter
* @see com.sun.star.ucb.XContentIdentifierFactory
* @see com.sun.star.ucb.XParameterizedContentProvider
* @see ifc.ucb._XContentProvider
* @see ifc.ucb._XFileIdentifierConverter
* @see ifc.ucb._XContentIdentifierFactory
* @see ifc.ucb._XParameterizedContentProvider
*/
public class ContentProvider extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ucprmt.ContentProvider</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'FACTORY'</code> for
    *      {@link ifc.ucb._XContentProvider} factory for
    *   content identifiers. Here it is
    *   <code>UniversalContentBroker</code> service.</li>
    *  <li> <code>'CONTENT1'</code> for
    *     {@link ifc.ucb._XContentProvider}: suitable
    *   content URL for this provider </li>
    *  <li> <code>'CONTENT2'</code> for
    *     {@link ifc.ucb._XContentProvider}: another suitable
    *   content URL for this provider </li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log ) {
        XInterface oObj = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XContentIdentifierFactory cntIDFac = null ;

        try {
            oObj = (XInterface) xMSF.createInstance
                ("com.sun.star.ucb.RemoteAccessContentProvider");
            cntIDFac = (XContentIdentifierFactory) UnoRuntime.queryInterface
                    (XContentIdentifierFactory.class, oObj) ;
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        if (oObj == null) {
            log.println("!!! Object wasn't created !!!") ;
            throw new StatusException(Status.failed
                    ("!!! Object wasn't created !!!")) ;
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XContentProvider
        tEnv.addObjRelation("FACTORY", cntIDFac) ;
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.wfs:///user/work");
        tEnv.addObjRelation("CONTENT2","vnd.sun.star.wfs:///user/config");

        return tEnv;
    } // finish method getTestEnvironment

}

