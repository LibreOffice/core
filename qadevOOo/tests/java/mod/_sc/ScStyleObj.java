/*************************************************************************
 *
 *  $RCSfile: ScStyleObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:16:40 $
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

package mod._sc;

import java.io.PrintWriter;
import java.util.Vector;

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
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.table.XCell;
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
* </ul>
* @see com.sun.star.style.Style
* @see com.sun.star.container.XNamed
* @see com.sun.star.style.Style
* @see com.sun.star.style.XStyle
* @see ifc.container._XNamed
* @see ifc.style._Style
* @see ifc.style._XStyle
*/
public class ScStyleObj extends TestCase {
    XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of style families from the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains style family with index 0 from the collection and obtains
    * style with index 0 from this style family. Creates the instance of the
    * service <code>com.sun.star.style.CellStyle</code>. Inserts the created
    * style to the obtained style family. Retrieves a collection of spreadsheets
    * from the document and takes one of them. Gets a cell from the spreadsheet
    * and sets the value of property <code>'CellStyle'</code> to the created
    * style. The created style is the instance of the service
    * <code>com.sun.star.style.Style</code> also.
    * Object relations created :
    * <ul>
    *  <li> <code>'PoolStyle'</code> for
    *      {@link ifc.style._XStyle}(the style with index 0 that was obtained
    *      from the collection)</li>
    * </ul>
    * @see com.sun.star.style.CellStyle
    * @see com.sun.star.style.Style
    * @see com.sun.star.style.XStyleFamiliesSupplier
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {


        // creation of the testobject here
        // first we write what we are intend to do to log file

        log.println("creating a test environment");

        log.println("getting style");
        XStyleFamiliesSupplier oStyleFamiliesSupplier = (XStyleFamiliesSupplier)
            UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class, xSpreadsheetDoc);
        XNameAccess oStyleFamilies = oStyleFamiliesSupplier.getStyleFamilies();
        XIndexAccess oStyleFamiliesIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oStyleFamilies);
        XNameAccess oStyleFamilyNameAccess = null;
        XStyle oStyle = null;
        try {
            oStyleFamilyNameAccess = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class),
                    oStyleFamiliesIndexAccess.getByIndex(0));

            XIndexAccess oStyleFamilyIndexAccess = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class,
                oStyleFamilyNameAccess);
            oStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oStyleFamilyIndexAccess.getByIndex(0));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        }

        log.println("Creating a user-defined style");
        XMultiServiceFactory oMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xSpreadsheetDoc);

        XInterface oInt = null;
        try {
            oInt = (XInterface)
                oMSF.createInstance("com.sun.star.style.CellStyle");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }
        XStyle oMyStyle = (XStyle)UnoRuntime.queryInterface(XStyle.class, oInt);

        XNameContainer oStyleFamilyNameContainer = (XNameContainer)UnoRuntime.
            queryInterface(XNameContainer.class, oStyleFamilyNameAccess);

        try {
            if (oStyleFamilyNameContainer.hasByName("My Style")) {
                oStyleFamilyNameContainer.removeByName("My Style");
            }

            oStyleFamilyNameContainer.insertByName("My Style", oMyStyle);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test environment", e);
        } catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test environment", e);
        } catch(com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test environment", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test environment", e);
        }


        //using the style
        log.println("Getting spreadsheet") ;
        XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
        XIndexAccess oIndexSheets = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);

        XCell aCell = null;
        try {
            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));
            log.println("Getting a cell from sheet") ;
            aCell = oSheet.getCellByPosition(2,3) ;
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet by index", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet by index", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get spreadsheet by index", e);
        }

        XPropertySet xProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, aCell);

        try {
            xProp.setPropertyValue("CellStyle", oMyStyle.getName());
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property CellStyle", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property CellStyle", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property CellStyle", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property CellStyle", e);
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oMyStyle);

        tEnv.addObjRelation("PoolStyle", oStyle);

        tEnv.addObjRelation("PropertyNames", getPropertyNames
            ((XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, oMyStyle)));

        return tEnv;
    }

    public String[] getPropertyNames(XPropertySet props) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        Vector names = new Vector() ;

        for (int i=0;i<the_props.length;i++) {
            boolean isWritable =
                ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);
            if (isWritable) {
                names.add(the_props[i].Name);
            }
        }
        return (String[]) names.toArray(new String[names.size()]) ;
    }
}    // finish class ScStyleObj
