/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
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
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

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


