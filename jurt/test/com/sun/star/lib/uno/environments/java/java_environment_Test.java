/*************************************************************************
 *
 *  $RCSfile: java_environment_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 11:34:31 $
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
package com.sun.star.lib.uno.environments.java;


import java.util.Vector;


import com.sun.star.uno.XInterface;


public class java_environment_Test {
    static public boolean test(Vector notpassed) throws Exception {
        boolean passed = true;

        System.err.println("java_environment - doing tests...");

        java_environment env = new java_environment(null);

        Object obj = new Integer(3);
        String oid[] = new String[1];

        System.err.println("\tregistering ordinary interface twice...");
        Object obj2 = env.registerInterface(obj, oid, XInterface.class);
        Object obj3 = env.registerInterface(obj, oid, XInterface.class);

        passed = passed && (obj == obj2) && (obj == obj3);

//          env.list();

        System.err.println("\tasking for registered interface...");
        passed = passed && (obj == env.getRegisteredInterface(oid[0], XInterface.class));


        System.err.println("\trevoking interface...");
        env.revokeInterface(oid[0], XInterface.class);
        env.revokeInterface(oid[0], XInterface.class);

        passed = passed && (null == env.getRegisteredInterface(oid[0], XInterface.class));

        System.err.println("java_environment - tests passed? " + passed);

        if(!passed && notpassed != null)
            notpassed.addElement("java_environment - tests passed? " + passed);


        return passed;
    }

    static public void main(String args[]) throws Exception{
        test(null);
    }
}
