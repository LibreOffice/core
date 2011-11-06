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



package com.sun.star.script.framework.provider.java;

/**
 * The Resolver interface is an interface common to all classes which
 * want to implement an algorithm for obtaining a ScriptProxy object
 * for a particular ScriptDescriptor and Class
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 */

public interface Resolver {
    /**
     * Returns a ScriptProxy object for the given ScriptDescriptor and Class
     *
     * @param  sd  A script Descriptor
     * @param  c   A Class
     * @return     The ScriptProxy value
     */
    public ScriptProxy getProxy( ScriptDescriptor sd, Class c )
    throws NoSuchMethodException;
}

