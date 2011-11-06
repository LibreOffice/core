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



package com.sun.star.lib.uno.environments.remote;


import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.XInterface;

public interface TestIWorkAt extends XInterface {
    void syncCall() throws Throwable ;
    void asyncCall() throws Throwable ;

    void increment() throws Throwable;

    void notifyme();

    public static final TypeInfo UNOTYPEINFO[] = {
        new MethodTypeInfo("increment", 0, 0),
        new MethodTypeInfo("notifyme", 1, 0),
        new MethodTypeInfo("syncCall", 2, 0),
        new MethodTypeInfo("asyncCall", 3, 0)
            };
}
