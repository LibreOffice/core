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

package com.sun.star.uno;

/**
 * The Enum class is the base class for all classes generated
 * as java binding for the IDL type enum.
 * <p>
 * Each java mapped enum class provides static member of this class
 * which represents the enum values.
 * You cannot create an object of this class or subclass direct, to
 * avoid enum values with integer values outside the defined range.
 * </p>
 */
public abstract class Enum {
    private final int m_value;

    /**
     * Constructs an enum value.
     * @param  value   the integer value of this enum value.
     */
    protected Enum(int value) {
        m_value = value;
    }

    /**
     * Get the integer value of an enum value.
     * @return   the integer value.
     */
    public final int getValue() {
        return m_value;
    }
}

