/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IMemberDescription.java,v $
 * $Revision: 1.7 $
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
 * The <code>IMemberDescription</code> is the base interface
 * for for the special subset of typedescriptions, which describe
 * members of IDL structs or interfeces.
 *
 * @deprecated This interface does not cover all the features supported by the
 * corresponding (unpublished) implementation.  But no client code should need
 * to access this functionality, anyway.
 */
public interface IMemberDescription {
    /**
     * Gives the name of this member.
     * <p>
     * @return  the name
     */
    String getName();

    /**
     * Indicates if this member is unsigned. (Not useful for IMethodDescription).
     * <p>
     * @return  the unsigned state
     */
    boolean isUnsigned();

    /**
     * Indicates if this member is an any.
     * <p>
     * @return  the any state
     */
    boolean isAny();

    /**
     * Indicates if this member is an interface.
     * <p>
     * @return  the interface state
     */
    boolean isInterface();

    /**
     * Gives the relative index of this member in the declaring
     * interface or struct (including superclasses).
     * <p>
     * @return  the relative index of this member
     */
    int getIndex();
}
