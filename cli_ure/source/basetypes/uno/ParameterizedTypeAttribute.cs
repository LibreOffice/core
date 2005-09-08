/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParameterizedTypeAttribute.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:59:40 $
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
/** is used to mark a UNO entity to have a parameterized type.

    <p>Currently it is only applied to members of  polymorphic structs. That is structs,
    which have a type parameter list.
    </p>

    @see TypeParametersAttribute
 */
[AttributeUsage(AttributeTargets.Field, Inherited=false)]
public sealed class ParameterizedTypeAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameter
        the name of parameter from the parameter list from
        <type scope="uno.idl">TypeParametersAttribute</type>
        It must not be null.
     */
    public ParameterizedTypeAttribute(string parameter)
    {
        m_parameter = parameter;
    }

    public string Type
    {
        get
        {
            return m_parameter;
        }
    }

    private string m_parameter;
}

} 

