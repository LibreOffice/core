/*************************************************************************
 *
 *  $RCSfile: SwXStyle.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:48:59 $
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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.Style</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::style::Style</code></li>
 *  <li> <code>com::sun::star::style::XStyle</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.style.Style
 * @see com.sun.star.style.XStyle
 * @see ifc.container._XNamed
 * @see ifc.style._Style
 * @see ifc.style._XStyle
 */
public class SwXStyle extends TestCase {
    XTextDocument xTextDoc;
    SOfficeFactory SOF = null;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
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
    * Creating a Testenvironment for the interfaces to be tested.
    * At first style families are gotten from a text document using
    * <code>XStyleFamiliesSupplier</code> interface, then family indexed '0' is
    * gotten from this style family using <code>XIndexAccess</code> interface.
    * Next, method creates an instance of the service
    * <code>com.sun.star.style.CharacterStyle</code> and inserts it to a
    * previously obtained style family using <code>XNameContainer</code>
    * interface. Finally, method creates a cursor of a major text of text
    * document and sets it's property 'CharStyleName' value to the name of
    * previously created style's name.
    *     Object relations created :
    * <ul>
    *  <li> <code>'PoolStyle'</code> for
    *      {@link ifc.style._XStyle} : slyle indexed '10' obtained from
    *  StyleFamily indexed '0' from text document using
    *  <code>XIndexAccess</code> interface.</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        TestEnvironment tEnv = null;
        XNameAccess oSFNA = null;
        XStyle oStyle = null;
        XStyle oMyStyle = null;

        log.println("creating a test environment");

        try {
            log.println("getting style");
            XStyleFamiliesSupplier oSFS = (XStyleFamiliesSupplier)
                UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
                xTextDoc);
            XNameAccess oSF = oSFS.getStyleFamilies();
            XIndexAccess oSFsIA = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSF);
            oSFNA = (XNameAccess) AnyConverter.toObject(
                        new Type(XNameAccess.class),oSFsIA.getByIndex(0));
            XIndexAccess oSFIA = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, oSFNA);
            oStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),oSFIA.getByIndex(10));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error: exception occured.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            log.println("Error: exception occured.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error: exception occured.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }

        try {
            log.print("Creating a user-defined style... ");
            XMultiServiceFactory oMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
            XInterface oInt = (XInterface)
                oMSF.createInstance("com.sun.star.style.CharacterStyle");
            oMyStyle = (XStyle) UnoRuntime.queryInterface(XStyle.class, oInt);
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Error: exception occured.");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }


        if (oMyStyle == null)
            log.println("FAILED");
        else
            log.println("OK");
            XNameContainer oSFNC = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, oSFNA);

        try {
            if ( oSFNC.hasByName("My Style") )
                oSFNC.removeByName("My Style");
            oSFNC.insertByName("My Style", oMyStyle);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch     ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.container.ElementExistException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create environment ", e );
        }

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        XPropertySet xProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oCursor);

        try {
            xProp.setPropertyValue("CharStyleName", oMyStyle.getName());
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create environment ", e );
        }

        log.println("creating a new environment for object");
        tEnv = new TestEnvironment(oMyStyle);
        tEnv.addObjRelation("PoolStyle", oStyle);

        XPropertySet xStyleProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oMyStyle);
        tEnv.addObjRelation("PropertyNames",getPropertyNames(xStyleProp));

        return tEnv;
    }

    public String[] getPropertyNames(XPropertySet props) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        String[] names = new String[the_props.length];
        String placebo = "";
        for (int i=0;i<the_props.length;i++) {
            boolean isWritable =
                ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);
            if (isWritable) placebo=the_props[i].Name;
        }
        for (int i=0;i<the_props.length;i++) {
            boolean isWritable =
                ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);
            if (isWritable) {
                names[i]=the_props[i].Name;
            } else {
                names[i] = placebo;
            }
        }
        return names;
    }

}    // finish class SwXStyle
