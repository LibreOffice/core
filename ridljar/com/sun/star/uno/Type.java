/*************************************************************************
 *
 *  $RCSfile: Type.java,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-13 17:23:42 $
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

import java.util.HashMap;

/**
 * Represents the UNO built-in type <code>TYPE</code>.
 *
 * <p>The UNO type is not directly mapped to <code>java.lang.Class</code> for at
 * least two reasons.  For one, some UNO types (like <code>UNSIGNED
 * SHORT</code>) do not have a matching Java class.  For another, it can be
 * necessary to describe a type which is unknown to the Java runtime system
 * (for example, for delaying the need of a class, so that it is possible to
 * generate it on the fly.)</p>
 *
 * <p>A <code>Type</code> is uniquely determined by its type class (a
 * <code>TypeClass</code>) and its type name (a <code>String</code>); these two
 * will never be <code>null</code>.  A <code>Type</code> may have an additional
 * "z class" (a <code>java.lang.Class</code>), giving a Java class type that
 * corresponds to the UNO type.  Also, a <code>Type</code> can cache a type
 * description (a <code>com.sun.star.uno.ITypeDescription</code>), which can be
 * computed and set by <code>TypeDescription.getTypeDescription</code>.
 *
 * @since UDK1.0
 */
public class Type {
    // The following private static members and static initializer must come
    // first in the class definition, so that the class can be initialized
    // sucessfully:

    // must be sorted same as TypeClass:
    private static final String[] __typeClassToTypeName = new String[] {
        "void",
        "char",
        "boolean",
        "byte",
        "short",
        "unsigned short",
        "long",
        "unsigned long",
        "hyper",
        "unsigned hyper",
        "float",
        "double",
        "string",
        "type",
        "any"
    };

    private static final HashMap __javaClassToTypeName = new HashMap();

    private static final HashMap __typeNameToTypeClass = new HashMap();

    static {
        for (int i = 0; i < __typeClassToTypeName.length; ++i) {
            __typeNameToTypeClass.put(__typeClassToTypeName[i],
                                      TypeClass.fromInt(i));
        }
        __javaClassToTypeName.put(Void.class, "void");
        __javaClassToTypeName.put(void.class, "void");
        __javaClassToTypeName.put(Character.class, "char");
        __javaClassToTypeName.put(char.class, "char");
        __javaClassToTypeName.put(Boolean.class, "boolean");
        __javaClassToTypeName.put(boolean.class, "boolean");
        __javaClassToTypeName.put(Byte.class, "byte");
        __javaClassToTypeName.put(byte.class, "byte");
        __javaClassToTypeName.put(Short.class, "short");
        __javaClassToTypeName.put(short.class, "short");
        __javaClassToTypeName.put(Integer.class, "long");
        __javaClassToTypeName.put(int.class, "long");
        __javaClassToTypeName.put(Long.class, "hyper");
        __javaClassToTypeName.put(long.class, "hyper");
        __javaClassToTypeName.put(Float.class, "float");
        __javaClassToTypeName.put(float.class, "float");
        __javaClassToTypeName.put(Double.class, "double");
        __javaClassToTypeName.put(double.class, "double");
        __javaClassToTypeName.put(String.class, "string");
        __javaClassToTypeName.put(Type.class, "type");
        __javaClassToTypeName.put(Any.class, "any");
        __javaClassToTypeName.put(Object.class, "any");
    }

    public static final Type VOID = new Type(void.class);
    public static final Type CHAR = new Type(char.class);
    public static final Type BOOLEAN = new Type(boolean.class);
    public static final Type BYTE = new Type(byte.class);
    public static final Type SHORT = new Type(short.class);
    public static final Type UNSIGNED_SHORT = new Type(
        "unsigned short", TypeClass.UNSIGNED_SHORT);
    public static final Type LONG = new Type(int.class);
    public static final Type UNSIGNED_LONG = new Type(
        "unsigned long", TypeClass.UNSIGNED_LONG);
    public static final Type HYPER = new Type(long.class);
    public static final Type UNSIGNED_HYPER = new Type(
        "unsigned hyper", TypeClass.UNSIGNED_HYPER);
    public static final Type FLOAT = new Type(float.class);
    public static final Type DOUBLE = new Type(double.class);
    public static final Type STRING = new Type(String.class);
    public static final Type TYPE = new Type(Type.class);
    public static final Type ANY = new Type(Any.class);

    /**
     * Constructs a new <code>Type</code> which defaults to <code>VOID</code>.
     *
     * @since UDK3.0
     */
    public Type() {
        this(void.class);
    }

    /**
     * Constructs a new <code>Type</code> with the given type class and type
     * name.
     *
     * @param typeName the type name.  Must not be <code>null</code>.
     * @param typeClass the type class.  Must not be <code>null</code>, and must
     *     match the <code>typeName</code> (for example, it is illegal to
     *     combine a <code>typeName</code> of <code>"void"</code> with a
     *     <code>typeClass</code> of <code>BOOLEAN</code>).
     */
    public Type(String typeName, TypeClass typeClass) {
        _typeClass = typeClass;
        _typeName = typeName;
    }

    /**
     * Constructs a new <code>Type</code> from the given
     * <code>java.lang.Class</code>.
     *
     * @param zClass the Java class of this type.  Must not be
     *     <code>null</code>.
     *
     * @since UDK3.0
     */
    public Type(Class zClass) {
        _class = zClass;

        _typeName = (String)__javaClassToTypeName.get(zClass);
        if(_typeName != null)
            _typeClass = (TypeClass)__typeNameToTypeClass.get(_typeName);

        else {
            if (XInterface.class.isAssignableFrom(zClass))
            {
                _typeClass = TypeClass.INTERFACE;
                if (zClass.isInterface())
                    _typeName = zClass.getName();
                else
                    _typeName = XInterface.class.getName();
            }
            else if(zClass.isArray()) {
                _typeClass = TypeClass.SEQUENCE;
                _typeName = "[]" + (new Type(_class.getComponentType()).getTypeName());
            }
            else if(Enum.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.ENUM;
                _typeName  = zClass.getName();
            }
            else if(Throwable.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.EXCEPTION;
                _typeName  = zClass.getName();
            }
            else if(Union.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.UNION;
                _typeName  = zClass.getName();
            }
            else {
                _typeClass = TypeClass.STRUCT;
                _typeName  = zClass.getName();
            }
        }
    }

    /**
     * Constructs a new <code>Type</code> from the given type description.
     *
     * @param typeDescription a type description.  Must not be
     *     <code>null</code>.
     *
     * @since UDK3.0
     */
    public Type(ITypeDescription typeDescription) {
        _typeName         = typeDescription.getTypeName();
        _typeClass        = typeDescription.getTypeClass();
        _iTypeDescription = typeDescription;
    }

    /**
     * Constructs a new <code>Type</code> with the given type name.
     *
     * <p>TODO:  This constructor is dangerous, as it can create a
     * <code>Type</code> with an <code>UNKNOWN</code> type class.  It would be
     * better if this constructor threw a <code>IllegalArgumentException</code>
     * instead.</p>
     *
     * @param typeName the name of this type; must not be <code>null</code>.
     *     For simple types (<code>VOID</code>, <code>BOOLEAN</code>,
     *     <code>CHAR</code>, <code>BYTE</code>, <code>SHORT</code>,
     *     <code>UNSIGNED SHORT</code>, <code>LONG</code>, <code>UNSIGNED
     *     LONG</code>, <code>HYPER</code>, <code>UNSIGNED HYPER</code>,
     *     <code>FLOAT</code>, <code>DOUBLE</code>, <code>STRING</code>,
     *     <code>TYPE</code>, <code>ANY</code>), the type class is calculated;
     *     for other types, the type class is set to <code>UNKNOWN</code>.
     *
     * @since UDK3.0
     */
    public Type(String typeName) {
        _typeClass = (TypeClass)__typeNameToTypeClass.get(typeName);

        if(_typeClass == null)
            _typeClass = TypeClass.UNKNOWN;

        _typeName = typeName;
    }

    /**
     * Constructs a new <code>Type</code> with the given type class.
     *
     * @param typeClass the type class of this type; must not be
     *     <code>null</code>.  Only type classes for simple types are allowed
     *     here.
     *
     * @throws IllegalArgumentException if the given <code>typeClass</code> is
     *     not simple (for example, a struct or an interface type).  This
     *     constructor could not find out the type name in such a case.
     *
     * @since UDK3.0
     */
    public Type(TypeClass typeClass) {
        if(__isTypeClassPrimitive(typeClass)) {
            _typeClass = typeClass;
            _typeName = __typeClassToTypeName[typeClass.getValue()];
        }
        else
            throw new IllegalArgumentException(typeClass + " is not primitive");
    }

    /**
     * Gets the type class.
     *
     * @return the type class.  Will never be <code>null</code>, but might be
     *     <code>UNKNOWN</code>.
     *
     * @since UDK1.0
     */
    public TypeClass getTypeClass() {
        return _typeClass;
    }

    /**
     * Gets the type name.
     *
     * @return the type name; will never be <code>null</code>
     *
     * @since UDK1.0
     */
    public String getTypeName() {
        return _typeName;
    }

    /**
     * Gets the Java class.
     *
     * <p>The implementation of
     * <code>com.sun.star.lib.uno.typedesc.TypeDescription</code> (for example,
     * <code>getTypeDescription(Type)</code>) seems to require that the mapping
     * from UNO types to Java classes is an injection.  Therefore, for example,
     * the UNO type <code>SHORT</code> maps to the Java class
     * <code>short.class</code>, but the UNO type <code>UNSIGNED SHORT</code>
     * maps to <code>null</code>.
     *
     * @return the type name; may be <code>null</code> in extreme situations
     *     (inconsistent <code>TypeClass</code>, error loading a class), or when
     *     there is no distinct Java class to represent a UNO type (so that the
     *     mapping from UNO types to Java classes is an injection)
     *
     * @since UDK1.0
     */
    public Class getZClass() {
        synchronized (this) {
            if (_class == null) {
                _class = determineClass();
            }
        }
        return _class;
    }

    /**
     * Gives the type description of this type.
     *
     * @return the type description; may be <code>null</code>
     *
     * @since UDK3.0
     */
    public ITypeDescription getTypeDescription() {
        return _iTypeDescription;
    }

    /**
     * Sets the type description for this type.
     *
     * @param typeDescription the type description
     *
     * @since UDK3.0
     */
    public void setTypeDescription(ITypeDescription typeDescription) {
        _iTypeDescription = typeDescription;
    }

    /**
     * Determines whether this UNO type is a supertype of another UNO type.
     *
     * UNO only defines the following supertype relations:
     * (1)  A struct type t1 is a supertype of a struct type t2, if either t1
     * and t2 are the same, or t1 is a direct or indirect parent of t2.
     * (2)  An exception type t1 is a supertype of an exception type t2, if
     * either t1 and t2 are the same, or t1 is a direct or indirect parent
     * of t2.
     * (3)  An interface type t1 is a supertype of an interface type t2, if
     * either t1 and t2 are the same, or t1 is a direct or indirect parent
     * of t2.
     *
     * Following the conventions of the Java UNO language binding,
     * com.sun.star.uno.Exception is not considered a supertype of
     * com.sun.star.uno.RuntimeException or any exception type derived from
     * com.sun.star.uno.RuntimeException.
     *
     * @param type some Type
     * @return true if this type is a supertype of the given type
     *
     * @since OOo 2.0
     */
    public boolean isSupertypeOf(Type type) {
        switch (_typeClass.getValue()) {
        case TypeClass.STRUCT_value:
        case TypeClass.EXCEPTION_value:
        case TypeClass.INTERFACE_value:
            if (type._typeClass.equals(_typeClass)) {
                Class c1 = getZClass();
                Class c2 = type.getZClass();
                return c1 != null && c2 != null && c1.isAssignableFrom(c2);
            }
            break;
        }
        return false;
    }

    // @see java.lang.Object#equals
    public boolean equals(Object obj) {
        return this == obj
            || (obj instanceof Type
                && _typeName.equals(((Type) obj)._typeName));
    }

    // @see java.lang.Object#hashCode
    public int hashCode() {
        return _typeName.hashCode();
    }

    // @see java.lang.Object#toString
    public String toString() {
        return "Type[" + _typeName + "]";
    }

    private Class determineClass() {
        switch (_typeClass.getValue()) {
        case TypeClass.VOID_value:
            return void.class;
        case TypeClass.BOOLEAN_value:
            return boolean.class;
        case TypeClass.BYTE_value:
            return byte.class;
        case TypeClass.SHORT_value:
            return short.class;
        case TypeClass.LONG_value:
            return int.class;
        case TypeClass.HYPER_value:
            return long.class;
        case TypeClass.FLOAT_value:
            return float.class;
        case TypeClass.DOUBLE_value:
            return double.class;
        case TypeClass.CHAR_value:
            return char.class;
        case TypeClass.STRING_value:
            return String.class;
        case TypeClass.TYPE_value:
            return Type.class;
        case TypeClass.ANY_value:
            return Object.class;
        case TypeClass.SEQUENCE_value:
            StringBuffer buf = new StringBuffer();
            int offset = 0;
            for (; _typeName.startsWith("[]", offset); offset += "[]".length())
            {
                buf.append('[');
            }
            String base = _typeName.substring(offset);
            if (base.equals("void")) {
                buf.append('V');
            } else if (base.equals("boolean")) {
                buf.append('Z');
            } else if (base.equals("byte")) {
                buf.append('B');
            } else if (base.equals("short")) {
                buf.append('S');
            } else if (base.equals("long")) {
                buf.append('I');
            } else if (base.equals("hyper")) {
                buf.append('J');
            } else if (base.equals("float")) {
                buf.append('F');
            } else if (base.equals("double")) {
                buf.append('D');
            } else if (base.equals("char")) {
                buf.append('C');
            } else if (base.equals("string")) {
                buf.append("Ljava.lang.String;");
            } else if (base.equals("type")) {
                buf.append("Lcom.sun.star.uno.Type;");
            } else if (base.equals("any")) {
                buf.append("Ljava.lang.Object;");
            } else if (base.equals("unsigned short")
                       || base.equals("unsigned long")
                       || base.equals("unsigned hyper")) {
                return null;
            } else {
                buf.append('L');
                buf.append(base);
                buf.append(';');
            }
            try {
                return Class.forName(buf.toString());
            } catch (ClassNotFoundException e) {
                return null;
            }
        case TypeClass.ENUM_value:
        case TypeClass.STRUCT_value:
        case TypeClass.EXCEPTION_value:
        case TypeClass.INTERFACE_value:
            try {
                return Class.forName(_typeName);
            } catch (ClassNotFoundException e) {
                return null;
            }
        default:
            return null;
        }
    }

    private static boolean __isTypeClassPrimitive(TypeClass typeClass) {
        return typeClass.getValue() < __typeClassToTypeName.length;
    }

    protected TypeClass _typeClass; // TODO should be final
    protected String _typeName; // TODO should be final
    protected Class _class; // TODO should be final

    protected ITypeDescription _iTypeDescription;
}
