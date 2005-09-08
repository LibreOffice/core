/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TypeArgumentsAttribute.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:00:08 $
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
/** is used to mark a parameterized UNO entity(i.e. struct)
    to be an instantiation of a
    template with the specified type arguments.
    
    <p>Currently only UNO structs can have type parameters.</p>

    <pre>

    [TypeParameters(new string[]{"T"})]
    struct Foo {
      [ParameterizedType("T")]
      Object member;
    }

    public interface XFoo {
       [return:TypeArguments(new string[]{typeof(char)})]
       Foo func( [TypeArguments(new string[]{typeof(char)})] Foo f);
    }
    </pre>

    @see TypeParametersAttribute
    @see ParameterizedTypeAttribute
 */
[AttributeUsage(AttributeTargets.ReturnValue
                | AttributeTargets.Parameter
                | AttributeTargets.Field, Inherited=false)]
public sealed class TypeArgumentsAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameters
        arrayay of names representing the types.
        It must not be null.
     */
    public TypeArgumentsAttribute(Type[] arguments)
    {
        m_arguments = arguments;
    }

    public Type[] Arguments
    {
        get
        {
            return m_arguments;
        }
    }

    private Type[] m_arguments;
}

} 

