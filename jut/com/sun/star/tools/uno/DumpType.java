/*************************************************************************
 *
 *  $RCSfile: DumpType.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2001-04-05 10:27:35 $
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


import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.lib.uno.typedesc.MethodDescription;

public class DumpType {
    static public void dumpType(String typeName) throws Exception {
        TypeDescription typeDescription = TypeDescription.getTypeDescription(typeName);

        System.err.println("TypeName:" + typeDescription.getTypeName());
        System.err.println("ArrayTypeName:" + typeDescription.getArrayTypeName());
        System.err.println("SuperType:" + typeDescription.getSuperType());
        System.err.println("TypeClass:" + typeDescription.getTypeClass());
        System.err.println("ComponentType:" + typeDescription.getComponentType());
        System.err.println("Class:" + typeDescription.getZClass());

        System.err.println("Methods:");
        MethodDescription methodDescriptions[] = typeDescription.getMethodDescriptions();
        if(methodDescriptions != null)
            for(int i = 0; i < methodDescriptions.length; ++ i) {
                System.err.print("Name: " + methodDescriptions[i].getName());
                System.err.print(" index: " + methodDescriptions[i].getIndex());
                System.err.print(" isOneyWay: " + methodDescriptions[i].isOneway());
                System.err.print(" isConst: " + methodDescriptions[i].isConst());
                System.err.print(" isUnsigned: " + methodDescriptions[i].isUnsigned());
                System.err.print(" isAny: " + methodDescriptions[i].isAny());
                System.err.println("\tisInterface: " + methodDescriptions[i].isInterface());

                System.err.print("\tgetInSignature: ");
                TypeDescription in_sig[] = methodDescriptions[i].getInSignature();
                for(int j = 0; j < in_sig.length; ++ j)
                    System.err.print("\t\t" + in_sig[j]);
                System.err.println();

                System.err.print("\tgetOutSignature: ");
                TypeDescription out_sig[] = methodDescriptions[i].getOutSignature();
                for(int j = 0; j < out_sig.length; ++ j)
                    System.err.print("\t\t" + out_sig[j]);
                System.err.println();

                System.err.println("\tgetReturnSig: " + methodDescriptions[i].getReturnSig());
                System.err.println("\tgetMethod:" + methodDescriptions[i].getMethod());
            }
        System.err.println();

        System.err.println("Members:");
        MemberTypeInfo memberTypeInfos[] = typeDescription.getMemberTypeInfos();
        if(memberTypeInfos != null)
            for(int i = 0; i < memberTypeInfos.length; ++ i)
                System.err.println("\tMember: " + memberTypeInfos[i].getName()
                                   + " unsigned: " + memberTypeInfos[i].isUnsigned()
                                   + " any: " + memberTypeInfos[i].isAny()
                                   + " interface: " + memberTypeInfos[i].isInterface());




    }

    static public void main(String args[]) throws Exception {
        if(args.length == 0)
            System.err.println("usage: [<type name>]*");
        else
            for(int i = 0; i < args.length; ++ i)
                dumpType(args[i]);
    }
}


