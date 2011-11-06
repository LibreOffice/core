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
 * The Enum class is the base class for all classes generated
 * as java binding for the IDL type enum.
 * Each java mapped enum class provides static member of this class
 * which represents the enum values.
 * You cannot create a object of this class or subclass direct, to
 * avoid enum values with integer values outside the defined range.
 * <p>
 * @version     $Revision: 1.5 $ $ $Date: 2008-04-11 11:11:59 $
 */
public abstract class Enum {
    private int m_value;

    /**
     * Constructs a enum value.
     * <p>
     * @param  value   the integer value of this enum value.
     */
    protected Enum(int value) {
        m_value = value;
    }

    /**
     * Get the integer value of an enum value.
     * <p>
     * @return   the integer value.
     */
    public final int getValue() {
        return m_value;
    }
}

