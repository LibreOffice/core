/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
