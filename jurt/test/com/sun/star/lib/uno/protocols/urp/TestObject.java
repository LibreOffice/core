/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.lib.uno.protocols.urp;



class TestObject implements TestXInterface {
    public void method1( /*IN*/java.lang.Object itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void method2( /*OUT*/java.lang.Object[] itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void method3( /*INOUT*/java.lang.Object[] itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public Object method4(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return null;
    }

    public Object returnAny(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return null;
    }


    public void method() throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithInParameter( /*IN*/String text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithOutParameter( /*OUT*/String[] text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithInOutParameter( /*INOUT*/String[] text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public String methodWithResult(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return "TestObject_resultString";
    }

    public String MethodWithIn_Out_InOut_Paramters_and_result( /*IN*/String text, /*OUT*/String[] outtext, /*INOUT*/String[] inouttext ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return "TestObject_resultString";
    }
}

