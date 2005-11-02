/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScIndexEnumeration_CellAreaLinksEnumeration.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:02:04 $
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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_CellAreaLinksEnumeration extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
   protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    public synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log){

      XInterface oObj = null;
      TestEnvironment tEnv = null ;

      try {

        // creation of testobject here
        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface
            (XPropertySet.class, xSheetDoc);
        oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),props.getPropertyValue("AreaLinks")) ;
        XAreaLinks links = null ;

        // adding one link into collection (for best testing)
        links = (XAreaLinks) UnoRuntime.queryInterface(XAreaLinks.class, oObj) ;
        CellAddress addr = new CellAddress ((short) 1,2,3) ;
        String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
        links.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

        XEnumerationAccess ea = (XEnumerationAccess)
                    UnoRuntime.queryInterface(XEnumerationAccess.class,oObj);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: "+util.utils.getImplName(oObj));
        // creating test environment
        tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("ENUM",ea);

      } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
      }

       return tEnv ;
    }

}

