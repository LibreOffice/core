/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TypeParametersAttribute.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:00:22 $
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
/** is used to mark a UNO entity to have type parameters.

    <p>Currently it is only applied with polymorphic structs. That is structs,
    which have a type parameter list.
    </p>
 */
[AttributeUsage(AttributeTargets.Class, Inherited=false)]
public sealed class TypeParametersAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameters
        array of names representing the types.
        It must not be null.
     */
    public TypeParametersAttribute(string[] parameters)
    {
        m_parameters = parameters;
    }

    public string[] Parameters
    {
        get
        {
            return m_parameters;
        }
    }

    private string[] m_parameters;
}

} 

