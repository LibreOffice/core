/*************************************************************************
 *
 *  $RCSfile: IBridge.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:51:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
