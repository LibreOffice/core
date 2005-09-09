/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaComponentLoader.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:31:10 $
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

package mod._javaloader;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for <code>com.sun.star.comp.stoc.Java2</code> service <p>
* Files which are used :
* <ul>
*  <li> <code>MyPersistObjectImpl.jar</code> : jar which will be loaded in
*    <code>XImplementationLoader</code> interface test. </li>
* <ul>
* Multithread testing compilant.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class JavaComponentLoader extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.comp.stoc.Java2</code>
    * service. <p>
    * The following object relations created :
    * <ul>
    *  <li> <code>'ImplementationLoader'</code> : service which is
    *    responsible for loading jar implementations. </li>
    *  <li> <code>'ImplementationUrl'</code> : jar file location. </li>
    *  <li> <code>'ImplementationName'</code> : Name of the implementation.</li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance(
                            "com.sun.star.comp.stoc.JavaComponentLoader" );
        }
        catch( Exception e ) {
            log.println("JavaComponentLoader Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for JavaComponentLoader object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding object relation for XImplementationLoader ifc test
        tEnv.addObjRelation("ImplementationLoader",
                                "com.sun.star.loader.JavaComponentLoader") ;

        String implURL = utils.getFullTestURL("qadevlibs/MyPersistObjectImpl.jar");
        tEnv.addObjRelation("ImplementationUrl", implURL) ;
        log.println("looking for shared lib: " + implURL);
        tEnv.addObjRelation("ImplementationName",
            "com.sun.star.cmp.MyPersistObject") ;

        return tEnv;
    } // finish method createTestEnvironment
}

