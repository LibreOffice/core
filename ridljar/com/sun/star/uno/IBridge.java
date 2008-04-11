/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IBridge.java,v $
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
     */
    void dispose() throws InterruptedException, IOException;
}
