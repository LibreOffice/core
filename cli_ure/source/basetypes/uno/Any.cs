/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Any.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:58:44 $
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
using System.Text;

namespace uno
{

/** This class can be used as a base class for UNO objects.
    It implements the capability to be kept weakly
    (unoidl.com.sun.star.uno.XWeak) and it implements
    unoidl.com.sun.star.lang.XTypeProvider which is necessary for
    using the object from StarBasic.
*/
public struct Any
{
    private object _value;
    private Type _type;

    public static Any VOID = new Any(typeof(void), null);
    
    private static void checkArgs(Type type, Object value)
    {
        //value can only be null if type == void
        if (type == null 
			|| (value == null 
                && type != typeof(void) 
                && type != typeof(object) 
                && type.IsInterface == false))
            throw new System.Exception(
                "uno.Any: Constructor called with illegal arguments!");
        //If value is a polymorphic struct then type must be
        //uno.Polymorphic
        if (value != null)
        {
            TypeParametersAttribute t = (TypeParametersAttribute) Attribute.GetCustomAttribute(
                value.GetType(), typeof(TypeParametersAttribute));
            if (t != null && !(type is PolymorphicType))
                throw new System.Exception(
                    "uno.Any: The value has a polymorphic type but the type argument is not " +
                    "uno.PolymorphicType. Please use the constructor Any(Type, object) and " +
                    "supply a uno.PolymorphicType as type argument!");
        }
    }

    /** constructs an instance.

       <p>If the arguments ar invalid then an exception is thrown.</p>
       @exception System.Exception
     */
    public Any(Type type, object value)
    {
        checkArgs(type, value);
        _type = type;
        _value = value;
    }

    /** sets the type and value.
       <p>If the arguments ar invalid then an exception is thrown.</p>
       @exception System.Exception
     */
    public void setValue(Type type, object value)
    {
        checkArgs(type, value);
        _type = type;
        _value = value;
    }
    
    public Type Type
    {
        get
        {
            if (_type == null)
                _type = typeof(void);
            return _type;
        }
    }

    public Object Value
    {
        get
        {
            return _value;
        }
    }
    
    public Any(char value): this(typeof(char), value)
    {
    }

    public Any(bool value): this(typeof(bool), value)
    {
    }

    public Any(byte value): this(typeof(byte), value)
    {
    }
    
    public Any(short value): this(typeof(short), value)
    {
    }

    public Any(ushort value): this(typeof(ushort), value)
    {
    }

    public Any(int value): this(typeof(int), value)
    {
    }

    public Any(uint value): this(typeof(uint), value)
    {
    }

    public Any(long value): this(typeof(long), value)
    {
    }

    public Any(ulong value): this(typeof(ulong), value)
    {
    }

    public Any(float value): this(typeof(float), value)
    {
    }

    public Any(double value): this(typeof(double), value)
    {
    }

    public Any(Type value): this(typeof(Type), value)
    {
    }

    public Any(string value): this(typeof(string), value)
    {
    }

    public override string ToString()
    {
        StringBuilder msg = new StringBuilder("uno.Any { Type= ");
        msg.Append(Type.ToString());
        msg.Append(", Value=");
        msg.Append(Value.ToString());
        msg.Append("}");
        return msg.ToString();
    }
    
    public bool hasValue()
    {
        if (Type == null || Type == typeof(void))
            return false;
        return true;
    }

    public override bool Equals(object obj)
    {
        if (obj != null)
        {
            try
            {
                return Equals((Any) obj);
            }
            catch (InvalidCastException)
            {
            }
        }
        return false;
    }

    public bool Equals(Any obj)
    {
        return Type.Equals(obj.Type)
            && (Value == null ?
            obj.Value == null  :
            Value.Equals(obj.Value));
    }

    public override int GetHashCode()
    {
        return Type.GetHashCode() ^ (Value != null ? Value.GetHashCode() : 0);
    }   
}

}

