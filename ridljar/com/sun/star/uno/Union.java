/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 * The Union class is the base class for all classes generated
 * as java binding for the IDL type union.
 * <p>
 * Note: The idl type <code>union<code> is currently not fully
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

