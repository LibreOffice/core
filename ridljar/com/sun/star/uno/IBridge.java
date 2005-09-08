/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IBridge.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:22:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
