/*************************************************************************
 *
 *  $RCSfile: DLLComponentLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:41:08 $
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

package mod._cpld;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for <code>com.sun.star.comp.stoc.DLLComponentLoader</code> service <p>
* Files which are used :
* <ul>
*  <li> <code>XImpReg.dll</code> : DLL which will be loaded in
*    <code>XImplementationLoader</code> interface test. </li>
* <ul>
* Multithread testing compilant.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class DLLComponentLoader extends TestCase {

    /**
    * Does nothing.
    */
    protected void initialize ( TestParameters Param, PrintWriter log) {

    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.comp.stoc.DLLComponentLoader</code>
    * service. <p>
    * The following object relations created :
    * <ul>
    *  <li> <code>'ImplementationLoader'</code> : service which is
    *    responsible for loading DLL implementations. </li>
    *  <li> <code>'ImplementationUrl'</code> : DLL file location. </li>
    *  <li> <code>'ImplementationName'</code> : Name of the implementation.</li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.comp.stoc.DLLComponentLoader" );
        }
        catch( Exception e ) {
            log.println("DLLComponentLoader Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for DLLComponentLoader object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding object relation for XImplementationLoader ifc test
        tEnv.addObjRelation("ImplementationLoader",
            "com.sun.star.loader.SharedLibrary") ;
        String os = (String) Param.get("OS");
        if (os == null || os == "")
            throw new StatusException(
                "Couldn't determine operating system for naming convention.",
                new NullPointerException());
        String name = "javaloader";
        if (!os.equals("wntmsci"))
            name = "lib" + name + ".so";
        else
            name += ".dll";
        String implURL = utils.getFullTestURL(os + "/" + name);
        tEnv.addObjRelation("ImplementationUrl", implURL) ;
        log.println("looking for shared lib: " + implURL);
        tEnv.addObjRelation("ImplementationName",
            "com.sun.star.comp.stoc.JavaComponentLoader") ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class DLLComponentLoader

