/*************************************************************************
 *
 *  $RCSfile: ScAreaLinkObj.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2003-11-18 16:29:15 $
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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.CellAreaLink</code>.
* This object reflects some cell range (this range
* can also be from another saved document) in
* any range (of the same size) of the current
* document.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XAreaLink</code></li>
*  <li> <code>com::sun::star::util::XRefreshable</code></li>
*  <li> <code>com::sun::star::sheet::CellAreaLink</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.XAreaLink
* @see com.sun.star.util.XRefreshable
* @see com.sun.star.sheet.CellAreaLink
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._XAreaLink
* @see ifc.util._XRefreshable
* @see ifc.sheet._CellAreaLink
* @see ifc.beans._XPropertySet
*/
public class ScAreaLinkObj extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

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
            throw new StatusException( "Couldn³t create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        oComp.dispose();
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of Area Links using the 'AreaLinks'
    * property of the Spreadsheet document. Adds a new link to this
    * collection, which has a source in the same document. This
    * link is passed as a tested object.
    */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        try {

            // creation of testobject here
            XPropertySet props = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class, xSheetDoc);
            XAreaLinks links = (XAreaLinks) AnyConverter.toObject(
                new Type(XAreaLinks.class),props.getPropertyValue("AreaLinks")) ;
            CellAddress addr = new CellAddress ((short) 1,2,3) ;
            String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
            links.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

            oObj = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class), links.getByIndex(0)) ;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
            throw new StatusException("Couldn't create test object", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }
}
