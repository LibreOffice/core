/*************************************************************************
 *
 *  $RCSfile: CachedDynamicResultSetFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 11:35:39 $
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

package mod._cached;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.NumberedSortingInfo;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCachedDynamicResultSetStubFactory;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.CachedDynamicResultSetFactory</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ucb::XCachedDynamicResultSetFactory</code></li>
* </ul> <p>
* The following files used by this test :
* <ul>
*  <li><b> solibrary.jar </b> : this file is used for <code>
*   PackageContentProvider</code> from which a list of contents is
*   retrieved. </li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.ucb.XCachedDynamicResultSetFactory
* @see com.sun.star.ucb.CachedDynamicResultSetFactory
* @see ifc.ucb._XCachedDynamicResultSetFactory
*/
public class CachedDynamicResultSetFactory extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ucb.CachedDynamicResultSetFactory</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'CachedDynamicResultSetStub'</code> for
    *      {@link ifc.XCachedDynamicResultSetFactory} : the destination
    *   interface requires as its parameter an instance of
    *   <code>CachedDynamicResultSetStub</code> service. It is created
    *   using <code>UniversalContentBroker</code> and queriing it for
    *   <code>PackageContent</code> which represents JAR file mentioned
    *   above. Then the dynamic list of file contents (entries) is retrieved.
    *   Using <code>CachedDynamicResultSetStubFactory</code> service a stub of
    *   Cached result set is created which is relation required.
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
                ( "com.sun.star.ucb.CachedDynamicResultSetFactory" );

            // adding one child container
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // creating relation for XCachedDynamicResultSetFactory
        XDynamicResultSet resSetStub = null ;
        try {
            Object oUCB = xMSF.createInstanceWithArguments
                ("com.sun.star.ucb.UniversalContentBroker",
                new Object[] {"Local", "Office"}) ;

            XContentIdentifierFactory ciFac = (XContentIdentifierFactory)
                UnoRuntime.queryInterface(XContentIdentifierFactory.class, oUCB) ;

            String url = util.utils.getFullTestURL("solibrary.jar") ;
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

            XDynamicResultSet dynResSet = null;
            try {
                dynResSet = (XDynamicResultSet)
                    AnyConverter.toObject(new Type(XDynamicResultSet.class),
                                        cmdProc.execute(cmd, 0, null));
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                throw new StatusException("Couldn't convert Any ",iae);
            }

            Object oStubFactory = xMSF.createInstance
                ("com.sun.star.ucb.CachedDynamicResultSetStubFactory") ;

            XCachedDynamicResultSetStubFactory xStubFactory =
                (XCachedDynamicResultSetStubFactory) UnoRuntime.queryInterface
                (XCachedDynamicResultSetStubFactory.class, oStubFactory) ;

            resSetStub =
                xStubFactory.createCachedDynamicResultSetStub(dynResSet) ;

        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create relation." );
            e.printStackTrace(log) ;
            throw new StatusException( "Can't create relation", e );
        }

        tEnv.addObjRelation("CachedDynamicResultSetStub", resSetStub) ;

        return tEnv;
    } // finish method getTestEnvironment

}

