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



package com.sun.star.uno;

/**
 * The Union class is the base class for all classes generated
 * as java binding for the IDL type union.
 * <p>
 * Note: The idl type <code>union</code> is currently not fully
 * integrated into the UNO framework, so don't use it.
 *
 * @version     $Revision: 1.5 $ $ $Date: 2008-04-11 11:15:07 $
 */
public class Union {
    /**
     * Get the value in the union.
     * The representation of the value is an any.
     * <p>
     *  @return the any value.
     */
    public final Object getValue() {
        return m_value;
    }

    protected Object m_value;
}

