/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExceptionAttribute.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:59:13 $
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

using System;


namespace uno
{
/** is used to mark a UNO method to throw exceptions.

    <p>A method can be an ordinary interface method, a get or set method from
    an interface attribute, or the constructor methods of service creator
    classes. If there are no exceptions specified for a method then this
    attribute should not be applied. Because a
    <type scope="com.sun.star.uno">RuntimeException</type>
    can always be thrown it is not specified. Hence no
    <code>ExceptionAttribute</code>
    should be applied in that case.</p>
 */
[AttributeUsage(AttributeTargets.Method, Inherited=false)]
public sealed class ExceptionAttribute: System.Attribute
{
    /** initializes an instance with the specified values.

        @param raises
        array of types of Exceptions which are declared in UNOIDL.
        It must not be null.
     */
    public ExceptionAttribute(Type[] raises)
    {
        m_raises = raises;
    }

    public Type[] Raises
    {
        get
        {
            return m_raises;
        }
    }

    private Type[] m_raises;
}

} 

