/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.test.XSomethingB;
import com.sun.star.uno.Type;

// TestComponentB implements all necessary interfaces self, this is only
// for demonstration. More convenient is to use the implementation WeakBase or
// ComponentBase, see implementation of TestComponentA.
public class TestComponentB implements XTypeProvider, XServiceInfo, XSomethingB {
    static final String __serviceName= "com.sun.star.test.SomethingB";

    private final Object[] args;

    public TestComponentB(XComponentContext context, Object[] args) {
        this.args= args;
    }

    // XSomethingB
    public String methodTwo(String val) {
        if (args.length > 0 && args[0] instanceof String )
            return (String) args[0];
        return val;
    }

    //XTypeProvider
    public com.sun.star.uno.Type[] getTypes(  ) {
        Type[] retValue= new Type[3];
        retValue[0]= new Type( XServiceInfo.class);
        retValue[1]= new Type( XTypeProvider.class);
        retValue[2]= new Type( XSomethingB.class);
        return retValue;
    }
    //XTypeProvider
    public byte[] getImplementationId(  ) {
        return new byte[0];
    }

    //XServiceInfo
    public String getImplementationName(  ) {
        return getClass().getName();
    }

    // XServiceInfo
    public boolean supportsService( /*IN*/String serviceName ) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }
    //XServiceInfo
    public String[] getSupportedServiceNames(  ) {
        String[] retValue= new String[0];
        retValue[0]= __serviceName;
        return retValue;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
