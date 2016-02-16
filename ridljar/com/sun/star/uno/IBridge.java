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

import java.io.IOException;

/**
 * This is abstract interface for bridges.
 *
 * <p>Bridges are able to map one object from one UNO environment to another and
 * vice versa.<p>
 *
 * @see com.sun.star.uno.IBridge
 * @see com.sun.star.uno.IQueryInterface
 * @see com.sun.star.uno.UnoRuntime
 *
 * @deprecated As of UDK 3.2, this interface is deprecated, without offering a
 * replacement.
 */
public interface IBridge {
    /**
     * Maps an object from the source environment to the destination
     * environment.
     *
     * @param object the object to map
     * @param type the type of the interface that shall be mapped
     * @return the object in the destination environment
     */
    Object mapInterfaceTo(Object object, Type type);

    /**
     * Maps an object from the destination environment to the source
     * environment.
     *
     * @param object the object to map
     * @param type the type of the interface that shall be mapped
     * @return the object in the source environment
     */
    Object mapInterfaceFrom(Object object, Type type);

    /**
     * Returns the source environment.
     *
     * @return the source environment of this bridge
     */
    IEnvironment getSourceEnvironment();

    /**
     * Returns the destination environment.
     *
     * @return the destination environment of this bridge
     */
    IEnvironment getTargetEnvironment();

    /**
     * Increases the life count.
     */
    void acquire();

    /**
     * Decreases the life count.
     *
     * <p>If the life count drops to zero, the bridge disposes itself.</p>
     */
    void release();

    /**
     * Disposes the bridge.
     *
     * <p>Sends involved threads an <code>InterruptedException</code>.  Releases
     * mapped objects.</p>
     *
     * @throws InterruptedException it's deprecated so who cares.
     * @throws IOException it's deprecated so who cares.
     */
    void dispose() throws InterruptedException, IOException;
}
