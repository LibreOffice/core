/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegComp.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:20:26 $
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

package com.sun.star.tools.uno;


import com.sun.star.comp.helper.RegistryServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.registry.XImplementationRegistration;

/** A command-line tool. It registers an UNO component with a registry database.
    During registration the component will be instantiated and be asked
    to write the necessary information to the database.
    Command line arguments:<br>
    database register|unregister locationURL loader
     <br>
    For example:
    <pre>
    java com.sun.star.tools.RegComp applicat.rdb register
    file:///e:/demos/InstanceInspector.jar com.sun.star.loader.Java2
    </pre>

    In order to examine the results of the registration use the regview.exe
    tool. For example:
    <pre>
    regview nameOfDatatbase /SERVICES/serviceName
    regview nameOfDataBase /IMPLEMENTATIONS/implementationName
    </pre>
    @see <a href="http://udk.openoffice.org/java/man/register_java_components.html">Registering Java Components</a>
 */
public class RegComp {

    static public void main(String args[]) throws Exception {
        if(args.length != 0) {
            Object serviceManager = RegistryServiceFactory.create(args[0]);
            XMultiServiceFactory serviceManager_xMultiServiceFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, serviceManager);

            Object implementationRegistration = serviceManager_xMultiServiceFactory.createInstance("com.sun.star.registry.ImplementationRegistration");
            XImplementationRegistration implementationRegistration_xImplementationRegistration =
                (XImplementationRegistration)UnoRuntime.queryInterface(XImplementationRegistration.class, implementationRegistration);

            if(args[1].equals("register")) {
                System.err.println("------ registering " + args[2] + " with " + args[3] + " in " + args[0]);

                implementationRegistration_xImplementationRegistration.registerImplementation(args[3], args[2], null);
            }
            else {
                System.err.println("------ revoke " + args[2] + " of " + args[0]);

                implementationRegistration_xImplementationRegistration.revokeImplementation(args[2], null);
            }
        }
        else {
            System.err.println("usage: <applicat> ");
            System.err.println("\tregister <url> <loader>");
            System.err.println("\trevoke <url>");
        }

        System.exit(0);

    }

}

