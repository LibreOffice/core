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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;

public class TestServiceProvider
{
    public static XSingleServiceFactory __getServiceFactory(
                                                String implName,
                                                XMultiServiceFactory multiFactory,
                                                XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals( TestComponentA.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                TestComponentA.class, TestComponentA.__serviceName,
                multiFactory, regKey);
        else if (implName.equals(TestComponentB.class.getName()))
            xSingleServiceFactory= FactoryHelper.getServiceFactory(
                TestComponentB.class, TestComponentB.__serviceName,
                multiFactory, regKey);
        return xSingleServiceFactory;
    }

    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey){
//         boolean bregA= FactoryHelper.writeRegistryServiceInfo(
//             TestComponentA.class.getName(),
//             TestComponentA.__serviceName, regKey);
//         boolean bregB= FactoryHelper.writeRegistryServiceInfo(
//             TestComponentB.class.getName(),
//             TestComponentB.__serviceName, regKey);
//         return bregA && bregB;
//     }
}


