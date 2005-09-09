/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleRegistry.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:21:54 $
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

package mod._simplereg.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.registry.SimpleRegistry</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::registry::XSimpleRegistry</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> XSimpleRegistry.rdb </b> : a registry file with
 *   some key set. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.registry.XSimpleRegistry
 * @see ifc.registry._XSimpleRegistry
 */
public class SimpleRegistry extends TestCase {

    /**
    * Creates a temporary copy of file, which is deleted when VM exits.
    * @param src Source file path.
    * @param dst Destination file path.
    * @param log The log writer.
    * @throws java.io.IOException If any problems occur during copiing.
    */
    protected void copyFile(String src, String dst, PrintWriter log)
            throws java.io.IOException {
        File srcF = new File(src) ;
        File dstF = new File(dst) ;
        System.out.println("H1");

        if (dstF.exists()) dstF.delete() ;
        System.out.println("H2");
        dstF.createNewFile() ;

        dstF.deleteOnExit() ;
        System.out.println("H3");

        FileInputStream fIn = new FileInputStream(srcF) ;
        System.out.println("H4");

        FileOutputStream fOut = new FileOutputStream(dstF) ;

        byte[] buf = new byte[1024] ;
        int bytesRead = 0 ;
        while ((bytesRead = fIn.read(buf)) > 0)
            fOut.write(buf, 0, bytesRead) ;

        fIn.close() ;
        fOut.close() ;
    }
    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.registry.SimpleRegistry</code>. Then
     * makes three copies of a predefined registry file with different
     * names in a temporary SOffice directory and passes their URLs as
     * relations. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'XSimpleRegistry.open'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     *  <li> <code>'XSimpleRegistry.merge'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     *  <li> <code>'XSimpleRegistry.destroy'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        final String tmpDir = utils.getOfficeTempDirSys(
                                (XMultiServiceFactory)Param.getMSF()) ;
        final String openF = "XSimpleRegistry_open.rdb" ;
        final String destroyF = "XSimpleRegistry_destroy.rdb" ;
        final String mergeF = "XSimpleRegistry_merge.rdb" ;


        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.registry.SimpleRegistry" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        if (oInterface == null)
            log.println("Service wasn't created") ;

        oObj = (XInterface) oInterface;

        log.println("creating copies of the registry for XSimpleRegistry");
       try {
            String source = utils.getFullTestDocName("XSimpleRegistry.rdb");
            copyFile(source, tmpDir + openF, log);
            copyFile(source, tmpDir + destroyF, log);
            copyFile(source, tmpDir + mergeF, log);
        } catch (java.io.IOException e) {
            log.println("Exception occured while copying files");
            e.printStackTrace(log);
        }

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XSimpleRegistry.open", tmpDir + openF);
        tEnv.addObjRelation("XSimpleRegistry.destroy", tmpDir + destroyF);
        tEnv.addObjRelation("XSimpleRegistry.merge", tmpDir + mergeF);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SimpleRegistry

