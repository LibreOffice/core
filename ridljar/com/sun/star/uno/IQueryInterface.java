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
 * This is the delegator interface for Java objects implementing interfaces of
 * an underlying UNO object.
 *
 * <p>Calls are delegated through the <code>UnoRuntime</code> to this
 * interface.  Implement this interface in case you want to customize the
 * behaviour of <code>UnoRuntime.queryInterface</code>.</p>
 *
 * @see com.sun.star.uno.UnoRuntime
 */
public interface IQueryInterface {
    /**
     * Returns the unique object identifier (OID) of the underlying UNO object.
     *
     * @return the OID of the underlying object
     */
    String getOid();

    /**
     * Returns an object implementing the requested interface type.
     *
     * @param type the requested UNO interface type; must be a <code>Type</code>
     * object representing a UNO interface type
     * @return a reference to the requested UNO interface type if available,
     * otherwise <code>null</code>
     * @see com.sun.star.uno.UnoRuntime
     */
    Object queryInterface(Type type);

    /**
     * Tests if the given reference represents a facet of the underlying UNO
     * object.
     *
     * @param object a reference to any Java object representing (a facet of) a
     * UNO object; may be <code>null</code>
     * @return <code>true</code> if and only if <code>object</code> is not
     * <code>null</code> and represents the same UNO object as this object
     */
    boolean isSame(Object object);
}
