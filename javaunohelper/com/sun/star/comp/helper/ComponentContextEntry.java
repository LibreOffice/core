/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ComponentContextEntry.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:36:09 $
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
