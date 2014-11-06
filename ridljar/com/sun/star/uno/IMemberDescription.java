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
 * The <code>IMemberDescription</code> is the base interface
 * for the special subset of typedescriptions, which describe
 * members of IDL structs or interfeces.
 *
 * @deprecated This interface does not cover all the features supported by the
 * corresponding (unpublished) implementation.  But no client code should need
 * to access this functionality, anyway.
 */
public interface IMemberDescription {
    /**
     * Gives the name of this member.
     * @return  the name
     */
    String getName();

    /**
     * Indicates if this member is unsigned. (Not useful for IMethodDescription).
     * @return  the unsigned state
     */
    boolean isUnsigned();

    /**
     * Indicates if this member is an any.
     * @return  the any state
     */
    boolean isAny();

    /**
     * Indicates if this member is an interface.
     * @return  the interface state
     */
    boolean isInterface();

    /**
     * Gives the relative index of this member in the declaring
     * interface or struct (including superclasses).
     * @return  the relative index of this member
     */
    int getIndex();
}
