/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._cached;

import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.NumberedSortingInfo;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.CachedDynamicResultSetStubFactory</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ucb::XCachedDynamicResultSetStubFactory</code></li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.ucb.XCachedDynamicResultSetStubFactory
* @see com.sun.star.ucb.CachedDynamicResultSetStubFactory
* @see ifc.ucb._XCachedDynamicResultSetStubFactory
*/
public class CachedDynamicResultSetStubFactory extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ucb.CachedDynamicResultSetStubFactory</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'DynamicResultSet'</code> for
    *      {@link ifc.XCachedDynamicResultSetStubFactory} : the destination
    *   interface requires as its parameter an instance of
    *   <code>DynamicResultSet</code> service. It is created
    *   using <code>UniversalContentBroker</code> and queriing it for
    *   <code>PackageContent</code> which represents JAR file mentioned
    *   above. Then the dynamic list of file contents (entries) is retrieved.
    *  </li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        Object relationContainer = null ;


        try {
            oInterface = xMSF.createInstance
                ( "com.sun.star.ucb.CachedDynamicResultSetStubFactory" );

            // adding one child container
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // creating relation for XCachedDynamicResultSetStubFactory
        XDynamicResultSet dynResSet = null ;
        try {
            Object oUCB = xMSF.createInstanceWithArguments
                ("com.sun.star.ucb.UniversalContentBroker",
                new Object[] {"Local", "Office"}) ;

            XContentIdentifierFactory ciFac = (XContentIdentifierFactory)
                UnoRuntime.queryInterface(XContentIdentifierFactory.class, oUCB) ;

            String url = util.utils.getFullTestURL("SwXTextEmbeddedObject.sxw") ;
            String escUrl = "" ;

            // In base URL of a JAR file in content URL all directory
            // separators ('/') must be replaced with escape symbol '%2F'.
            int idx = url.indexOf("/") ;
            int lastIdx = -1 ;
            while (idx >= 0) {
                escUrl += url.substring(lastIdx + 1, idx) + "%2F" ;
                lastIdx = idx ;
                idx = url.indexOf("/", idx + 1) ;
            }
            escUrl += url.substring(lastIdx + 1) ;
            String cntUrl = "vnd.sun.star.pkg://" + escUrl + "/" ;

            XContentIdentifier CI = ciFac.createContentIdentifier(cntUrl) ;

            XContentProvider cntProv = (XContentProvider)
                UnoRuntime.queryInterface(XContentProvider.class, oUCB) ;

            XContent cnt = cntProv.queryContent(CI) ;

            XCommandProcessor cmdProc = (XCommandProcessor)
                UnoRuntime.queryInterface(XCommandProcessor.class, cnt) ;

            Property prop = new Property() ;
            prop.Name = "Title" ;

            Command cmd = new Command("open", -1, new OpenCommandArgument2
                (OpenMode.ALL, 10000, null, new Property[] {prop},
                 new NumberedSortingInfo[0])) ;

            dynResSet = null;
            try {
                dynResSet = (XDynamicResultSet)
                    AnyConverter.toObject(new Type(XDynamicResultSet.class),
                                        cmdProc.execute(cmd, 0, null));
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                throw new StatusException("Couldn't convert Any ",iae);
            }

        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create relation." );
            e.printStackTrace(log) ;
            throw new StatusException( "Can't create relation", e );
        }

        tEnv.addObjRelation("DynamicResultSet", dynResSet) ;

        return tEnv;
    } // finish method getTestEnvironment

}
