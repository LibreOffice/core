/*************************************************************************
 *
 *  $RCSfile: RegComp.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2002-01-22 11:03:23 $
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

