/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComponentContextEntry.java,v $
 * $Revision: 1.4 $
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
package com.sun.star.comp.helper;

/** Component context entry for constructing ComponentContext objects.
    <p>
    A ComponentContextEntry is separated into a late-init and direct-value
    purpose.
    The first one is commonly used for singleton objects of the component
    context, that are raised on first-time retrieval of the key.
    You have to pass a com.sun.star.lang.XSingleComponentFactory
    or string (=> service name) object for this.
    </p>
*/
public class ComponentContextEntry
{
    /** if late init of service instance, set service name (String) or
        component factory (XSingleComponentFactory), null otherwise
    */
    public Object m_lateInit;
    /** set entry value
    */
    public Object m_value;

    /** Creating a late-init singleton entry component context entry.
        The second parameter will be ignored and overwritten during
        instanciation of the singleton instance.

        @param lateInit
               object factory or service string
        @param value
               pass null (dummy separating from second ctor signature)
    */
    public ComponentContextEntry( Object lateInit, Object value )
    {
        this.m_lateInit = lateInit;
        this.m_value = value;
    }
    /** Creating a direct value component context entry.

        @param value
               pass null
    */
    public ComponentContextEntry( Object value )
    {
        this.m_lateInit = null;
        this.m_value = value;
    }
}
