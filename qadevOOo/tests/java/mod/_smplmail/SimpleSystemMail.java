/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleSystemMail.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:24:46 $
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

package mod._smplmail;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.system.SimpleSystemMail</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::system::XSimpleMailClientSupplier</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.system.XSimpleMailClientSupplier
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.lang.XTypeProvider
 * @see ifc.system._XSimpleMailClientSupplier
 * @see ifc.lang._XServiceInfo
 * @see ifc.lang._XTypeProvider
 */
public class SimpleSystemMail extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * In case if OS is Win or not specified tries to create an instance
     * of the service <code>com.sun.star.system.SimpleSystemMail</code>. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'SystemMailExist'</code> for
     *      {@link ifc.system._XSimpleMailClientSupplier} :
     *      the value depends on tested platform and
     *      <code>test.system.mail.isExist</code> INI
     *      parameter.</li>
     * </ul>
     */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        boolean isWinOS = false ;
        boolean isSystemMailExist = false ;

        // if 'OS' parameter is not defined then by default
        // Win platform is used. If really not the error will occurs.
        isWinOS = Param.get("OS") == null ||
            "wntmsci".equals(Param.get("OS")) ;
        isSystemMailExist = "true".equalsIgnoreCase
            ((String) Param.get("test.system.mail.isExist")) ;

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        try {
            if (isWinOS) {
                log.println("The OS is Win : trying to create service");
                oInterface = xMSF.createInstance
                    ( "com.sun.star.system.SimpleSystemMail" );
            } else {
                log.println("The OS is NOT Win, SKIPPING this component");
                log.println("Creating Dummy object ...");
                oInterface = xMSF.createInstance( "com.sun.star.io.Pipe" );
            }
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("SystemMailExist", new Boolean(isSystemMailExist));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SimpleSystemMail

