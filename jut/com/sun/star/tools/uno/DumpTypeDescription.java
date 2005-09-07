/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DumpTypeDescription.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:20:11 $
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


import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.Type;

/** A command-line tool. Prints information of com.sun.star.lib.uno.typedesc.TypeDescription objects to the
    error stream.
 */
public class DumpTypeDescription {

    /** Prints information about a TypeDescription object that has been constructed based
        on the argument <i>typeName</i>. The output is written to the error stream.
        The information consists of general as well as method and member information.
        These are:<br>
        <ul>
        <li>type name</li>
        <li>array type name</li>
        <li>type of the super class</li>
        <li>type class</li>
        <li>component type</li>
        <li>java.lang.Class object</li>
        </ul>
        Method information:
        <ul>
        <li>name</li>
        <li>index</li>
        <li>is one way</li>
        <li>is unsigned</li>
        <li>is Any</li>
        <li>is interface</li>
        <li>in signature</li>
        <li>out signature</li>
        <li>return signature</li>
        <li>java.lang.reflect.Method object</li>
        </ul>

        Field information:
        <ul>
        <li>name</li>
        <li>index</li>
        <li>is one way</li>
        <li>is unsigned</li>
        <li>is Any</li>
        <li>is interface</li>
        <li>com.sun.star.lib.uno.typedesc.TypeDescription object</li>
        </ul>

        @param typeName name of a type for which an TypeDescription  object
        is constructed whoose information are printed.
     */
    static public void dumpTypeDescription(String typeName) throws Exception {
        ITypeDescription iTypeDescription = TypeDescription.getTypeDescription(typeName);

        System.err.println("TypeName:" + iTypeDescription.getTypeName());
        System.err.println("ArrayTypeName:" + iTypeDescription.getArrayTypeName());
        System.err.println("SuperType:" + iTypeDescription.getSuperType());
        System.err.println("TypeClass:" + iTypeDescription.getTypeClass());
        System.err.println("ComponentType:" + iTypeDescription.getComponentType());
        System.err.println("Class:" + iTypeDescription.getZClass());

        System.err.println("Methods:");
        IMethodDescription iMethodDescriptions[] = iTypeDescription.getMethodDescriptions();
        if(iMethodDescriptions != null)
            for(int i = 0; i < iMethodDescriptions.length; ++ i) {
                System.err.print("Name: " + iMethodDescriptions[i].getName());
                System.err.print(" index: " + iMethodDescriptions[i].getIndex());
                System.err.print(" isOneyWay: " + iMethodDescriptions[i].isOneway());
//                  System.err.print(" isConst: " + iMethodDescriptions[i].isConst());
                System.err.print(" isUnsigned: " + iMethodDescriptions[i].isUnsigned());
                System.err.print(" isAny: " + iMethodDescriptions[i].isAny());
                System.err.println("\tisInterface: " + iMethodDescriptions[i].isInterface());

                System.err.print("\tgetInSignature: ");
                ITypeDescription in_sig[] = iMethodDescriptions[i].getInSignature();
                for(int j = 0; j < in_sig.length; ++ j)
                    System.err.print("\t\t" + in_sig[j]);
                System.err.println();

                System.err.print("\tgetOutSignature: ");
                ITypeDescription out_sig[] = iMethodDescriptions[i].getOutSignature();
                for(int j = 0; j < out_sig.length; ++ j)
                    System.err.print("\t\t" + out_sig[j]);
                System.err.println();

                System.err.println("\tgetReturnSig: " + iMethodDescriptions[i].getReturnSignature());
                System.err.println("\tgetMethod:" + iMethodDescriptions[i].getMethod());
            }
        System.err.println();

        System.err.println("Members:");
        IFieldDescription iFieldDescriptions[] = iTypeDescription.getFieldDescriptions();
        if(iFieldDescriptions != null)
            for(int i = 0; i < iFieldDescriptions.length; ++ i) {
                System.err.print("\tMember: " + iFieldDescriptions[i].getName());
//                  System.err.print(" isConst: " + iMethodDescriptions[i].isConst());
                System.err.print(" isUnsigned: " + iFieldDescriptions[i].isUnsigned());
                System.err.print(" isAny: " + iFieldDescriptions[i].isAny());
                System.err.print("\tisInterface: " + iFieldDescriptions[i].isInterface());
                System.err.println("\tclass: " + iFieldDescriptions[i].getTypeDescription());

            }
    }
    /** Executes this tool.
        The command line arguments consist of an identifier followed by a type name.
        Identifiers can either be uno or java. Based on the type name, a
        com.sun.star.lib.uno.typedesc.TypeDescription
        object is constructed whose information are then printed out to the error stream.
        <br>
        usage: [uno &lt;type name&gt;]|[java &lt;class name&gt]* <br>
        Examples: <br>
        java com.sun.star.uno.tools.DumpTypeDescription uno com.sun.star.lang.XMultiServiceFactory
        java com.sun.star.uno.tools.DumpTypeDescription java java.lang.Byte java java.lang.String
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


                dumpTypeDescription(type.getTypeName());
            }
        }
    }
}


