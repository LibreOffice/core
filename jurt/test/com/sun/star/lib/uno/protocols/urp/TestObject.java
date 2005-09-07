/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestObject.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:15:48 $
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

