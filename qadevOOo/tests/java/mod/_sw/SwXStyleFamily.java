/*************************************************************************
 *
 *  $RCSfile: SwXStyleFamily.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:49:21 $
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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.StyleFamily</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNameContainer</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameReplace</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XNameReplace
 * @see ifc.container._XNameContainer
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XNameReplace
 */
public class SwXStyleFamily extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested. At first,
    * style families of text document are gotten using
    * <code>XStyleFamiliesSupplier</code> interface, then family indexed '0' is
    * gotten from previously obtained style families collection using
    * <code>XIndexAccess</code> interface. Finally, method creates an instance
    * of the service <code>com.sun.star.style.CharacterStyle</code> and inserts
    * it to a gotten style family as 'SwXStyleFamily' using
    * <code>XNameContainer</code> interface.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XNameReplaceINDEX'</code> for
    *      {@link ifc.container._XNameReplace} : number of last taken instance
    *      of <code>com.sun.star.style.CharacterStyle</code>, when multithread
    *      testing is going.</li>
    *  <li> <code>'NAMEREPLACE'</code> for
    *      {@link ifc.container._XNameReplace} : name of style family, inserted
    *      to style families of a text document.</li>
    *  <li> <code>'INSTANCEn'</code> for
    *      {@link ifc.container._XIndexContainer},
    *      {@link ifc.container._XIndexReplace},
    *      {@link ifc.container._XNameContainer},
    *      {@link ifc.container._XNameReplace} : several relations, which are
    *      represented by instances of service
    *      <code>com.sun.star.style.CharacterStyle</code>.</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XNameAccess oSFNA = null;

        log.println( "Creating Test Environment..." );

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        XComponent xComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xTextDoc);
        XInterface oInstance = (XInterface)
            SOF.createInstance(xComp, "com.sun.star.style.CharacterStyle");
        XStyleFamiliesSupplier oSFsS = (XStyleFamiliesSupplier)
                UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess oSF = oSFsS.getStyleFamilies();
        XIndexAccess oSFIA = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSF);

        try {
            oSFNA = (XNameAccess) AnyConverter.toObject(
                        new Type(XNameAccess.class),oSFIA.getByIndex(0));
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception. ", e);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception. ", e);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception. ", e);
        }

        XNameContainer oContainer = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, oSFNA);

        // insert a Style which can be replaced by name
        try {
            oContainer.insertByName("SwXStyleFamily",oInstance);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        } catch ( com.sun.star.container.ElementExistException e ) {
            log.println("Could not insert style family.");
            e.printStackTrace(log);
        }

        TestEnvironment tEnv = new TestEnvironment(oSFNA);

        int THRCNT = 1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }
        String nr = new Integer(THRCNT+1).toString();

        log.println( "adding NameReplaceIndex as mod relation to environment" );
        tEnv.addObjRelation("XNameReplaceINDEX", nr);

        for (int n=1; n<(THRCNT+3); n++ ) {
            log.println( "adding INSTANCE"+n+" as mod relation to environment");
            tEnv.addObjRelation("INSTANCE"+n,
                SOF.createInstance(xComp,"com.sun.star.style.CharacterStyle"));
        }

        log.println("adding NAMEREPLACE as mod relation to environment");
        tEnv.addObjRelation("NAMEREPLACE", "SwXStyleFamily");

        return tEnv;
    }

}    // finish class SwXStyleFamily
