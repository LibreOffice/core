/*************************************************************************
 *
 *  $RCSfile: ScAutoFormatFieldObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:04:20 $
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
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.TableAutoFormatField</code>.
* In StarCalc application there is a collection of autoformats
* for tables (you can select a predefined format for a
* table or create your own). Each autoformat has a number
* of fields with definitions of font parameters, number
* formats etc. for different parts of a table (column and
* row names, footers, data). This object represents the
* field of the same kind. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::TableAutoFormatField</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.TableAutoFormatField
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._TableAutoFormatField
* @see ifc.beans._XPropertySet
*/
public class ScAutoFormatFieldObj extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Using SOffice ServiceManager an instance of
    * <code>com.sun.star.sheet.TableAutoFormatField</code> service
    * is created. From this collection one Format is retrieved
    * and then from this format one of its compound fields is
    * retrieved.
    */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        try {

                  log.println ("create Object ...") ;
            // creation of testobject here
            XInterface formats = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sheet.TableAutoFormats");
            XIndexAccess formatsIndex = (XIndexAccess)UnoRuntime.queryInterface
                (XIndexAccess.class, formats);
            XInterface format = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatsIndex.getByIndex(0));
            XIndexAccess formatIndex = (XIndexAccess) UnoRuntime.queryInterface
                (XIndexAccess.class, format);
            oObj = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatIndex.getByIndex(0));

        } catch (com.sun.star.uno.Exception e) {
            log.println ("Exception occured while creating test Object.") ;
            e.printStackTrace(log) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }

}    // finish class ScAutoFormatFieldObj

