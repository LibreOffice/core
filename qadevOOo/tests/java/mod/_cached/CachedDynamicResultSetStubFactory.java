/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ucb.CachedDynamicResultSetStubFactory</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'DynamicResultSet'</code> for
    *      {@link ifc.ucb._XCachedDynamicResultSetStubFactory} : the destination
    *   interface requires as its parameter an instance of
    *   <code>DynamicResultSet</code> service. It is created
    *   using <code>UniversalContentBroker</code> and querying it for
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
                new Object[0]) ;

            XContentIdentifierFactory ciFac = UnoRuntime.queryInterface(XContentIdentifierFactory.class, oUCB) ;

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

            XContentProvider cntProv = UnoRuntime.queryInterface(XContentProvider.class, oUCB) ;

            XContent cnt = cntProv.queryContent(CI) ;

            XCommandProcessor cmdProc = UnoRuntime.queryInterface(XCommandProcessor.class, cnt) ;

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
