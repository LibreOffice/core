/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Enum.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

