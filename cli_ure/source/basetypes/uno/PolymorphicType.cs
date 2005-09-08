/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolymorphicType.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:59:54 $
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
using System.Collections;
using System.Reflection;
using System.Globalization;

namespace uno {



/** represents a polymorphic type.

    <p> In .NET 1.1 it is not possible to have templated types. The
    polymorphic struct types from uno would be easiest mapped to templates. Since
    this is not possible, one needs to conserve the information (e.g. the type list)
    so that a lossless mapping from UNO to CLI and back is possible. To do that, this
    class is used. It keeps the actual type object and additionally its polymorphic
    name, for example: unoidl.com.sun.star.beans.Defaulted<System.Char>. This name
    is an invalid Name in .NET 1.1. But it is needed by the cli-uno bridge to perform
    a proper conversion</p>

    <p>The full polymorphic name contains a list of type names. Only type names common
    to all CLI languages can be used. That is, instead of using names, such as
    <code>char, int, float</code>, the names <code>System.Char, System.Int32 and
    System.Single</code> are
    to be used.</p>

    <p>Once the  CLI supports templates we will adapt the cli-uno bridge accordingly.
    Then this class will become  obsolete. </p>
    
    <p>This class derives from System::Type, so it can be used in API
    calls whereever a  Type is required. Of the inherited methods
    it only supplies implementations for the abstract types. These functions
    simply delegate to the orignal Type object. </p>
 */
public class PolymorphicType: Type
{
    private Type m_base;
    private string m_type_name;
    
    private static Hashtable m_ht_types = Hashtable.Synchronized(new Hashtable(256));

    /** provides a unique instance of this class.
      
       This function returns null if the specified type is no polymorphic struct.
       
       @param type
       the type of the polymorphic struct. For example, created by
       <code>typeof(unoidl.com.sun.star.beans.Defaulted)</code>
       @param name
       the full name of the struct (including the type list), for example:
       unoidl.com.sun.star.beans.Defaulted<System.Int32>
       The type names must not be programming specific but rather those commen to all
       languages, that is the names of the respective structures from System namespace.
       They must be fully qualified.
       @return
       null - the argument type is no valid polymorphic struct or <br>
       an instance of this class.
       @exception System.ArgumentNullException
       The argument was null.
     */
    public static PolymorphicType GetType(Type type, string name)
    {
        if (name == null || type == null)
            throw new ArgumentNullException(
                "cli-uno: uno.PolymorphicType.GetType was called with a null argument");
        if (Attribute.GetCustomAttribute(type, typeof(uno.TypeParametersAttribute))
            == null)
            return null;
        lock (m_ht_types.SyncRoot)
        {
            PolymorphicType t = (PolymorphicType) m_ht_types[name];
            if (t == null)
            {
                t = new PolymorphicType(type, name);
                m_ht_types.Add(name, t);
            }
            return t;
        }
    }

    private PolymorphicType(Type type, string name)
    {
        m_type_name = name;
        m_base = type;
    }

    public string PolymorphicName
    {
        get
        {
            return m_type_name;
        }
    }

    public Type OriginalType
    {
        get
        {
            return m_base;
        }
    }
            
    
    //implementations of abstract methods and properties from base class
    public override string Name
    {
        get
        {
            return m_base.Name;
        }
    }
    
    public override Assembly Assembly
    {
        get
        {
            return m_base.Assembly;
        }
    }

    public override string AssemblyQualifiedName
    {
        get
        {
            return m_base.AssemblyQualifiedName;
        }
    }

    public override Type BaseType
    {
        get
        {
            return m_base.BaseType;
        }
    }
    
    public override string FullName
    {
        get
        {
            return m_base.FullName;
        }
    }

    public override Guid GUID
    {
        get
        {
            return m_base.GUID;
        }
    }

    public override Module Module
    {
        get
        {
            return m_base.Module;
        }
    }

    public override string Namespace
    {
        get
        {
            return m_base.Namespace;
        }
    }

    public override RuntimeTypeHandle TypeHandle
    {
        get
        {
            return m_base.TypeHandle;
        }
    }

    public override Type UnderlyingSystemType
    {
        get
        {
            return m_base.UnderlyingSystemType;
        }
    }

    public override  object[] GetCustomAttributes(
        bool inherit)
    {
        //todo
        return null;
    }

    public override object[] GetCustomAttributes(
        Type attributeType,
        bool inherit)
    {
        //todo
        return null;        
    }

    public override bool IsDefined(
        Type attributeType,
        bool inherit)
    {
        //todo
        return false;
    }

    protected override TypeAttributes GetAttributeFlagsImpl()
    {
        return m_base.Attributes;
    }

    protected override ConstructorInfo GetConstructorImpl(
        BindingFlags bindingAttr,
        Binder binder,
        CallingConventions callConvention,
        Type[] types,
        ParameterModifier[] modifiers)
    {
        return m_base.GetConstructor(
            bindingAttr, binder, callConvention, types, modifiers);
    }

    public override ConstructorInfo[] GetConstructors(
        BindingFlags bindingAttr)
    {
        return m_base.GetConstructors(bindingAttr);
    }

    public override Type GetElementType()
    {
        return m_base.GetElementType();
    }

    public override EventInfo GetEvent(
        string name,
        BindingFlags bindingAttr)
    {
        return m_base.GetEvent(name, bindingAttr);
    }

    public override EventInfo[] GetEvents(
        BindingFlags bindingAttr)
    {
        return m_base.GetEvents(bindingAttr);
    }

    public override FieldInfo GetField(
        string name,
        BindingFlags bindingAttr)
    {
        return m_base.GetField(name, bindingAttr);
    }

    public override FieldInfo[] GetFields(
        BindingFlags bindingAttr)
    {
        return m_base.GetFields(bindingAttr);
    }

    public override Type GetInterface(
        string name, bool ignoreCase)
    {
        return m_base.GetInterface(name, ignoreCase);
    }

    public override Type[] GetInterfaces()
    {
        return m_base.GetInterfaces();
    }

    public override MemberInfo[] GetMembers(
        BindingFlags bindingAttr)
    {
        return m_base.GetMembers(bindingAttr);
    }

    protected override MethodInfo GetMethodImpl(
        string name,
        BindingFlags bindingAttr,
        Binder binder,
        CallingConventions callConvention,
        Type[] types,
        ParameterModifier[] modifiers)
    {
        return m_base.GetMethod(
            name, bindingAttr, binder, callConvention, types, modifiers);
    }

    public override MethodInfo[] GetMethods(
        BindingFlags bindingAttr)
    {
        return m_base.GetMethods(bindingAttr);
    }

    public override Type GetNestedType(
        string name, BindingFlags bindingAttr)
    {
        return m_base.GetNestedType(name, bindingAttr);
    }

    public override Type[] GetNestedTypes(
        BindingFlags bindingAttr)
    {
        return m_base.GetNestedTypes(bindingAttr);
    }

    public override PropertyInfo[] GetProperties(
        BindingFlags bindingAttr)
    {
        return m_base.GetProperties(bindingAttr);
    }

    protected override PropertyInfo GetPropertyImpl(
        string name,
        BindingFlags bindingAttr,
        Binder binder,
        Type returnType,
        Type[] types,
        ParameterModifier[] modifiers)
    {
        return m_base.GetProperty(
            name, bindingAttr, binder, returnType, types, modifiers);
    }

    protected override  bool HasElementTypeImpl()
    {
        return m_base.HasElementType;
    }

    public override object InvokeMember(
        string name,
        BindingFlags invokeAttr,
        Binder binder,
        object target,
        object[] args,
        ParameterModifier[] modifiers,
        CultureInfo culture,
        string[] namedParameters)
    {
        return m_base.InvokeMember(
            name, invokeAttr, binder, target, args, modifiers, culture, namedParameters);
    }

    protected override bool IsArrayImpl()
    {
        return m_base.IsArray;
    }

    protected override bool IsByRefImpl()
    {
        return m_base.IsByRef;
    }

    protected override bool IsCOMObjectImpl()
    {
        return m_base.IsCOMObject;
    }

    protected override bool IsPointerImpl()
    {
        return m_base.IsPointer;
    }

    protected override bool IsPrimitiveImpl()
    {
        return m_base.IsPrimitive;
    }
}
}
