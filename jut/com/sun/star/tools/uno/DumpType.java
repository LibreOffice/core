/*************************************************************************
 *
 *  $RCSfile: DumpType.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2002-01-22 11:05:16 $
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

import com.sun.star.uno.Type;
/** A command-line tool. Prints information about com.sun.star.uno.Type to error stream.
 */
public class DumpType {
    static private final String[] __typeClassToTypeName = new String[]{
        "void",
        "char",
        "boolean",
        "byte",
        "short",
        "unsigned short",
        "long",
        "unsigned long",
        "hyper",
        "unsigned hyper",
        "float",
        "double",
        "string",
        "type",
        "any",
        "enum",
        "typedef",
        "struct",
        "union",
        "exception",
        "sequence",
        "array",
        "interface",
        "service",
        "module",
        "interface_method",
        "interface_attribute",
        "unknown"
    };

    /** Prints information about argument type to the standard error stream.
        The information consists of type name, type description, class name,
        as well as the type class.
        @param type the Type object whoose information are printed.
     */
    static public void dumpType(Type type) throws Exception {
        System.err.println("uno type name:" + type.getTypeName());
        System.err.println("description:" + type.getTypeDescription());
        System.err.println("java class:" + type.getZClass());

        System.err.println("type class:" + __typeClassToTypeName[type.getTypeClass().getValue()]);
    }

    /** Executes this tool.
        The command line arguments consist of an identifier followed by a type name.
        Identifiers can either be uno or java. Based on the type name, a com.sun.star.uno.Type
        object is constructed whose information are then printed out to the error stream.
        <br>
        usage: [uno &lt;type name&gt;]|[java &lt;class name&gt]* <br>
        Examples: <br>
        java com.sun.star.uno.tools.DumpType uno com.sun.star.lang.XMultiServiceFactory
        java com.sun.star.uno.tools.DumpType java java.lang.Byte java java.lang.String
        @param args command line arguments
    */
    static public void main(String args[]) throws Exception {
        if(args.length == 0)
            System.err.println("usage: [uno <type name>]|[java <class name>]*");

        else {
            int i = 0;
            while(i < args.length) {
                Type type = null;

                if(args[i].equals("uno"))
                    type = new Type(args[i + 1]);

                else
                    type = new Type(Class.forName(args[i + 1]));

                i += 2;


                dumpType(type);
            }
        }
    }
}


