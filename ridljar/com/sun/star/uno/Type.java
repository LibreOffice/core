/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.uno;

import java.util.HashMap;

import com.sun.star.lib.uno.typedesc.TypeDescription;

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
 * description (a <code>com.sun.star.uno.typedesc.TypeDescription</code>), which can be
 * computed and set by <code>TypeDescription.getTypeDescription</code>.
 */
public class Type {
    // The following private static members and static initializer must come
    // first in the class definition, so that the class can be initialized
    // successfully:

    private static final String TYPE_NAME_VOID = "void";
    private static final String TYPE_NAME_BOOLEAN = "boolean";
    private static final String TYPE_NAME_BYTE = "byte";
    private static final String TYPE_NAME_SHORT = "short";
    private static final String TYPE_NAME_UNSIGNED_SHORT = "unsigned short";
    private static final String TYPE_NAME_LONG = "long";
    private static final String TYPE_NAME_UNSIGNED_LONG = "unsigned long";
    private static final String TYPE_NAME_HYPER = "hyper";
    private static final String TYPE_NAME_UNSIGNED_HYPER = "unsigned hyper";
    private static final String TYPE_NAME_FLOAT = "float";
    private static final String TYPE_NAME_DOUBLE = "double";
    private static final String TYPE_NAME_CHAR = "char";
    private static final String TYPE_NAME_STRING = "string";
    private static final String TYPE_NAME_TYPE = "type";
    private static final String TYPE_NAME_ANY = "any";

    // must be sorted same as TypeClass:
    private static final String[] __typeClassToTypeName = new String[] {
        TYPE_NAME_VOID,
        TYPE_NAME_CHAR,
        TYPE_NAME_BOOLEAN,
        TYPE_NAME_BYTE,
        TYPE_NAME_SHORT,
        TYPE_NAME_UNSIGNED_SHORT,
        TYPE_NAME_LONG,
        TYPE_NAME_UNSIGNED_LONG,
        TYPE_NAME_HYPER,
        TYPE_NAME_UNSIGNED_HYPER,
        TYPE_NAME_FLOAT,
        TYPE_NAME_DOUBLE,
        TYPE_NAME_STRING,
        TYPE_NAME_TYPE,
        TYPE_NAME_ANY
    };

    private static final class TypeInfo {
        TypeInfo(
            TypeClass thePrimary, TypeClass theAlternative,
            boolean theSequenceComponentType)
        {
            primary = thePrimary;
            alternative = theAlternative;
            sequenceComponentType = theSequenceComponentType;
        }

        final TypeClass primary;
        final TypeClass alternative;
        final boolean sequenceComponentType;
    }

    private static final HashMap<Class<?>, TypeInfo> __javaClassToTypeClass =
        new HashMap<Class<?>, TypeInfo>();
    static {
        __javaClassToTypeClass.put(
            void.class, new TypeInfo(TypeClass.VOID, TypeClass.VOID, false));
        __javaClassToTypeClass.put(
            Void.class, new TypeInfo(TypeClass.VOID, TypeClass.VOID, false));
        __javaClassToTypeClass.put(
            boolean.class,
            new TypeInfo(TypeClass.BOOLEAN, TypeClass.BOOLEAN, true));
        __javaClassToTypeClass.put(
            Boolean.class,
            new TypeInfo(TypeClass.BOOLEAN, TypeClass.BOOLEAN, false));
        __javaClassToTypeClass.put(
            byte.class, new TypeInfo(TypeClass.BYTE, TypeClass.BYTE, true));
        __javaClassToTypeClass.put(
            Byte.class, new TypeInfo(TypeClass.BYTE, TypeClass.BYTE, false));
        __javaClassToTypeClass.put(
            short.class,
            new TypeInfo(TypeClass.SHORT, TypeClass.UNSIGNED_SHORT, true));
        __javaClassToTypeClass.put(
            Short.class,
            new TypeInfo(TypeClass.SHORT, TypeClass.UNSIGNED_SHORT, false));
        __javaClassToTypeClass.put(
            int.class,
            new TypeInfo(TypeClass.LONG, TypeClass.UNSIGNED_LONG, true));
        __javaClassToTypeClass.put(
            Integer.class,
            new TypeInfo(TypeClass.LONG, TypeClass.UNSIGNED_LONG, false));
        __javaClassToTypeClass.put(
            long.class,
            new TypeInfo(TypeClass.HYPER, TypeClass.UNSIGNED_HYPER, true));
        __javaClassToTypeClass.put(
            Long.class,
            new TypeInfo(TypeClass.HYPER, TypeClass.UNSIGNED_HYPER, false));
        __javaClassToTypeClass.put(
            float.class, new TypeInfo(TypeClass.FLOAT, TypeClass.FLOAT, true));
        __javaClassToTypeClass.put(
            Float.class, new TypeInfo(TypeClass.FLOAT, TypeClass.FLOAT, false));
        __javaClassToTypeClass.put(
            double.class,
            new TypeInfo(TypeClass.DOUBLE, TypeClass.DOUBLE, true));
        __javaClassToTypeClass.put(
            Double.class,
            new TypeInfo(TypeClass.DOUBLE, TypeClass.DOUBLE, false));
        __javaClassToTypeClass.put(
            char.class, new TypeInfo(TypeClass.CHAR, TypeClass.CHAR, true));
        __javaClassToTypeClass.put(
            Character.class,
            new TypeInfo(TypeClass.CHAR, TypeClass.CHAR, false));
        __javaClassToTypeClass.put(
            String.class,
            new TypeInfo(TypeClass.STRING, TypeClass.STRING, true));
        __javaClassToTypeClass.put(
            Type.class, new TypeInfo(TypeClass.TYPE, TypeClass.TYPE, true));
        __javaClassToTypeClass.put(
            Any.class, new TypeInfo(TypeClass.ANY, TypeClass.ANY, true));
        __javaClassToTypeClass.put(
            Object.class,
            new TypeInfo(TypeClass.ANY, TypeClass.INTERFACE, true));
    }

    public static final Type VOID = new Type(void.class);
    public static final Type CHAR = new Type(char.class);
    public static final Type BOOLEAN = new Type(boolean.class);
    public static final Type BYTE = new Type(byte.class);
    public static final Type SHORT = new Type(short.class);
    public static final Type UNSIGNED_SHORT = new Type(
        TYPE_NAME_UNSIGNED_SHORT, TypeClass.UNSIGNED_SHORT);
    public static final Type LONG = new Type(int.class);
    public static final Type UNSIGNED_LONG = new Type(
        TYPE_NAME_UNSIGNED_LONG, TypeClass.UNSIGNED_LONG);
    public static final Type HYPER = new Type(long.class);
    public static final Type UNSIGNED_HYPER = new Type(
        TYPE_NAME_UNSIGNED_HYPER, TypeClass.UNSIGNED_HYPER);
    public static final Type FLOAT = new Type(float.class);
    public static final Type DOUBLE = new Type(double.class);
    public static final Type STRING = new Type(String.class);
    public static final Type TYPE = new Type(Type.class);
    public static final Type ANY = new Type(Any.class);

    /**
     * Constructs a new <code>Type</code> which defaults to <code>VOID</code>.
     */
    public Type() {
        init(null, void.class, false, false, false);
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
     * <p>This is equivalent to <code>Type(zClass, false)</code>.</p>
     *
     * @param zClass the Java class of this type.  Must not be
     *     <code>null</code>.
     */
    public Type(Class<?> zClass) {
        init(null, zClass, false, false, false);
    }

    /**
     * Constructs a new <code>Type</code> from the given
     * <code>java.lang.Class</code>, handling ambiguous cases.
     *
     * <p>In certain cases, one Java class corresponds to two UNO types (e.g.,
     * the Java class <code>short[].class</code> corresponds to both a sequence
     * of <code>SHORT</code> and a sequence of <code>UNSIGNED SHORT</code> in
     * UNO).  In such ambiguous cases, the parameter <code>alternative</code>
     * controls which UNO type is chosen:
     * <ul>
     *   <li>If the Java type is (an array type with element type)
     *   <code>short</code> or <code>java.lang.Short</code>:  If
     *   <code>alternative</code> is <code>false</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>SHORT</code>.  If
     *   <code>alternative</code> is <code>true</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>UNSIGNED SHORT</code>.</li>
     *
     *   <li>If the Java type is (an array type with element type)
     *   <code>int</code> or <code>java.lang.Integer</code>:  If
     *   <code>alternative</code> is <code>false</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>LONG</code>.  If
     *   <code>alternative</code> is <code>true</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>UNSIGNED LONG</code>.</li>
     *
     *   <li>If the Java type is (an array type with element type)
     *   <code>long</code> or <code>java.lang.Long</code>:  If
     *   <code>alternative</code> is <code>false</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>HYPER</code>.  If
     *   <code>alternative</code> is <code>true</code>, the chosen UNO type is
     *   (a sequence type with element type) <code>UNSIGNED HYPER</code>.</li>
     *
     *   <li>If the Java type is (an array type with element type)
     *   <code>java.lang.Object</code>:  If <code>alternative</code> is
     *   <code>false</code>, the chosen UNO type is (a sequence type with
     *   element type) <code>ANY</code>.  If <code>alternative</code> is
     *   <code>true</code>, the chosen UNO type is (a sequence type with element
     *   type) <code>com.sun.star.uno.XInterface</code>.</li>
     * </ul>
     * <p>In all other cases, the value of <code>alternative</code> is
     * ignored.</p>
     *
     * <p>This constructor cannot be used to create <code>Type</code> instances
     * that represent (sequences of) instantiated polymorphic struct types.</p>
     *
     * @param zClass the Java class of this type; must not be <code>null</code>
     * @param alternative controls which UNO type to choose in case of
     *     ambiguities
     *
     * @since UDK 3.2.0
     */
    public Type(Class<?> zClass, boolean alternative) {
        init(null, zClass, alternative, false, false);
    }

    private Type(
        Class<?> zClass, boolean alternative, boolean sequenceComponentType)
    {
        init(null, zClass, alternative, false, sequenceComponentType);
    }

    /**
     * Constructs a new <code>Type</code> from the given type description.
     *
     * <em>For internal URE use only. Not to be used by client code.</em>
     *
     * @param typeDescription a type description.  Must not be
     *     <code>null</code>.
     */
    public Type(TypeDescription typeDescription) {
        _typeName         = typeDescription.getTypeName();
        _typeClass        = typeDescription.getTypeClass();
        _iTypeDescription = typeDescription;
    }

    /**
     * Constructs a new <code>Type</code> with the given type name.
     *
     * @param typeName the name of this type; must not be <code>null</code>.
     */
    public Type(String typeName) {
        if (typeName.startsWith("[]")) {
            _typeName = typeName;
            _typeClass = TypeClass.SEQUENCE;
            return;
        }
        for (int i = 0; i < __typeClassToTypeName.length; ++i) {
            if (__typeClassToTypeName[i].equals(typeName)) {
                _typeName = typeName;
                _typeClass = TypeClass.fromInt(i);
                return;
            }
        }
        int i = typeName.indexOf('<');
        try {
            init(
                typeName,
                Class.forName(i < 0 ? typeName : typeName.substring(0, i)),
                false, i >= 0, false);
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
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
     */
    public TypeClass getTypeClass() {
        return _typeClass;
    }

    /**
     * Gets the type name.
     *
     * @return the type name; will never be <code>null</code>
     */
    public String getTypeName() {
        return _typeName;
    }

    /**
     * Gets the Java class.
     *
     * @return the type name; may be <code>null</code> in extreme situations
     *     (inconsistent <code>TypeClass</code>, error loading a class)
     */
    public Class<?> getZClass() {
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
     * <em>For internal URE use only. Not to be used by client code.</em>
     *
     * @return the type description; may be <code>null</code>
     */
    public TypeDescription getTypeDescription() {
        return _iTypeDescription;
    }

    /**
     * Sets the type description for this type.
     *
     * <em>For internal URE use only. Not to be used by client code.</em>
     *
     * @param typeDescription the type description
     */
    public void setTypeDescription(TypeDescription typeDescription) {
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
     * @since UDK 3.2.0
     */
    public boolean isSupertypeOf(Type type) {
        if (_typeClass != type._typeClass) {
            return false;
        }
        switch (_typeClass.getValue()) {
        case TypeClass.SEQUENCE_value:
        case TypeClass.ENUM_value:
            return _typeName.equals(type._typeName);

        case TypeClass.STRUCT_value:
            // This check exploits the fact that an instantiated polymorphic
            // struct type may not be the direct base of a struct type:
            if (_typeName.indexOf('<') >= 0 || type._typeName.indexOf('<') >= 0)
            {
                return _typeName.equals(type._typeName);
            }
            // fall-through
        case TypeClass.EXCEPTION_value:
        case TypeClass.INTERFACE_value:
            Class<?> c1 = getZClass();
            Class<?> c2 = type.getZClass();
            return c1 != null && c2 != null && c1.isAssignableFrom(c2);

        default:
            return true;
        }
    }

    /**
     * Indicates whether some other object is equal to this one.
     *
     * @param obj the reference object with which to compare.
     * @return <code>true</code> if this object is the same as the obj argument;
     * <code>false</code> otherwise.
     * @see java.lang.Object#equals
     */
    @Override
    public boolean equals(Object obj) {
        return obj instanceof Type
            && _typeClass == ((Type) obj)._typeClass
            && _typeName.equals(((Type) obj)._typeName);
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return a hash code value for this object.
     * @see java.lang.Object#hashCode
     */
    @Override
    public int hashCode() {
        return _typeName.hashCode();
    }

    /**
     * Returns a string representation of the object.
     *
     * @return a string representation of the object.
     * @see java.lang.Object#toString
     */
    @Override
    public String toString() {
        return "Type[" + _typeName + "]";
    }

    private void init(
        String name, Class<?> zClass, boolean alternative, boolean arguments,
        boolean sequenceComponentType)
    {
        TypeInfo info = __javaClassToTypeClass.get(zClass);
        if (info != null) {
            if (sequenceComponentType && !info.sequenceComponentType) {
                throw new IllegalArgumentException(
                    zClass + " cannot be sequence component type");
            }
            // info only contains primitive type classes, except for
            // TypeClass.INTERFACE, which stands for XInterface (the alternative
            // interpretation of java.lang.Object):
            _typeClass = alternative ? info.alternative  : info.primary;
            _typeName = _typeClass == TypeClass.INTERFACE
                ? XInterface.class.getName()
                : __typeClassToTypeName[_typeClass.getValue()];
            // do not assign _class from zClass, as _class should always be
            // normalized (e.g., boolean.class instead of
            // java.lang.Boolean.class); getZClass will later calculate the
            // correct class when needed
        } else if (zClass.isArray()) {
            Type t = new Type(zClass.getComponentType(), alternative, true);
            _typeClass = t.getTypeClass() != TypeClass.UNKNOWN
                ? TypeClass.SEQUENCE : TypeClass.UNKNOWN;
            _typeName = "[]" + t.getTypeName();
            // do not assign _class from zClass, as _class should always be
            // normalized (e.g., boolean[].class instead of
            // java.lang.Boolean[].class); getZClass will later calculate the
            // correct class when needed
        } else if (Enum.class.isAssignableFrom(zClass)) {
            _typeClass = zClass != Enum.class
                ? TypeClass.ENUM : TypeClass.UNKNOWN;
            _typeName = zClass.getName();
            _class = zClass;
        } else if (Throwable.class.isAssignableFrom(zClass)) {
            _typeClass
                = com.sun.star.uno.Exception.class.isAssignableFrom(zClass)
                || com.sun.star.uno.RuntimeException.class.isAssignableFrom(
                    zClass)
                ? TypeClass.EXCEPTION : TypeClass.UNKNOWN;
            _typeName = zClass.getName();
            _class = zClass;
        } else if (zClass.isInterface()) {
            _typeClass = XInterface.class.isAssignableFrom(zClass)
                ? TypeClass.INTERFACE : TypeClass.UNKNOWN;
            _typeName = zClass.getName();
            _class = zClass;
        } else if (XInterface.class.isAssignableFrom(zClass)) {
            // This case is needed by code that uses this constructor to
            // calculate the UNO type corresponding to a Java object:
            _typeClass = TypeClass.INTERFACE;
            _typeName = XInterface.class.getName();
            _class = XInterface.class;
        } else {
            // assert zClass != Object.class && !zClass.isPrimitive();
            _typeClass = TypeClass.STRUCT;
            _typeName = name == null ? zClass.getName() : name;
            _class = zClass;
        }
        if (arguments && _typeClass != TypeClass.STRUCT) {
            throw new IllegalArgumentException(
                zClass + " cannot have type arguments");
        }
    }

    private Class<?> determineClass() {
        switch (_typeClass.getValue()) {
        case TypeClass.VOID_value:
            return _typeName.equals(TYPE_NAME_VOID) ? void.class : null;

        case TypeClass.BOOLEAN_value:
            return _typeName.equals(TYPE_NAME_BOOLEAN) ? boolean.class : null;

        case TypeClass.BYTE_value:
            return _typeName.equals(TYPE_NAME_BYTE) ? byte.class : null;

        case TypeClass.SHORT_value:
            return _typeName.equals(TYPE_NAME_SHORT) ? short.class : null;

        case TypeClass.UNSIGNED_SHORT_value:
            return _typeName.equals(TYPE_NAME_UNSIGNED_SHORT)
                ? short.class : null;

        case TypeClass.LONG_value:
            return _typeName.equals(TYPE_NAME_LONG) ? int.class : null;

        case TypeClass.UNSIGNED_LONG_value:
            return _typeName.equals(TYPE_NAME_UNSIGNED_LONG) ? int.class : null;

        case TypeClass.HYPER_value:
            return _typeName.equals(TYPE_NAME_HYPER) ? long.class : null;

        case TypeClass.UNSIGNED_HYPER_value:
            return _typeName.equals(TYPE_NAME_UNSIGNED_HYPER)
                ? long.class : null;

        case TypeClass.FLOAT_value:
            return _typeName.equals(TYPE_NAME_FLOAT) ? float.class : null;

        case TypeClass.DOUBLE_value:
            return _typeName.equals(TYPE_NAME_DOUBLE) ? double.class : null;

        case TypeClass.CHAR_value:
            return _typeName.equals(TYPE_NAME_CHAR) ? char.class : null;

        case TypeClass.STRING_value:
            return _typeName.equals(TYPE_NAME_STRING) ? String.class : null;

        case TypeClass.TYPE_value:
            return _typeName.equals(TYPE_NAME_TYPE) ? Type.class : null;

        case TypeClass.ANY_value:
            return _typeName.equals(TYPE_NAME_ANY) ? Object.class : null;

        case TypeClass.SEQUENCE_value:
            StringBuffer buf = new StringBuffer();
            int offset = 0;
            for (; _typeName.startsWith("[]", offset); offset += "[]".length())
            {
                buf.append('[');
            }
            if (buf.length() == 0) {
                return null;
            }
            String base = _typeName.substring(offset);
            if (base.equals(TYPE_NAME_VOID)) {
                buf.append('V');
            } else if (base.equals(TYPE_NAME_BOOLEAN)) {
                buf.append('Z');
            } else if (base.equals(TYPE_NAME_BYTE)) {
                buf.append('B');
            } else if (base.equals(TYPE_NAME_SHORT)
                       || base.equals(TYPE_NAME_UNSIGNED_SHORT)) {
                buf.append('S');
            } else if (base.equals(TYPE_NAME_LONG)
                       || base.equals(TYPE_NAME_UNSIGNED_LONG)) {
                buf.append('I');
            } else if (base.equals(TYPE_NAME_HYPER)
                       || base.equals(TYPE_NAME_UNSIGNED_HYPER)) {
                buf.append('J');
            } else if (base.equals(TYPE_NAME_FLOAT)) {
                buf.append('F');
            } else if (base.equals(TYPE_NAME_DOUBLE)) {
                buf.append('D');
            } else if (base.equals(TYPE_NAME_CHAR)) {
                buf.append('C');
            } else if (base.equals(TYPE_NAME_STRING)) {
                buf.append("Ljava.lang.String;");
            } else if (base.equals(TYPE_NAME_TYPE)) {
                buf.append("Lcom.sun.star.uno.Type;");
            } else if (base.equals(TYPE_NAME_ANY)) {
                buf.append("Ljava.lang.Object;");
            } else {
                int args = base.indexOf('<');
                if (args >= 0) {
                    base = base.substring(0, args);
                }
                Class<?> c;
                try {
                    c = Class.forName(base);
                } catch (ClassNotFoundException e) {
                    return null;
                }
                if (args < 0 && new Type(c).getTypeClass() == TypeClass.UNKNOWN)
                {
                    return null;
                }
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
        case TypeClass.EXCEPTION_value:
        case TypeClass.INTERFACE_value:
            {
                Class<?> c;
                try {
                    c = Class.forName(_typeName);
                } catch (ClassNotFoundException e) {
                    return null;
                }
                return new Type(c).equals(this) ? c : null;
            }

        case TypeClass.STRUCT_value:
            {
                int args = _typeName.indexOf('<');
                Class<?> c;
                try {
                    c = Class.forName(
                        args < 0 ? _typeName : _typeName.substring(0, args));
                } catch (ClassNotFoundException e) {
                    return null;
                }
                return args >= 0 || new Type(c).equals(this) ? c : null;
            }

        default:
            return null;
        }
    }

    private static boolean __isTypeClassPrimitive(TypeClass typeClass) {
        return typeClass.getValue() < __typeClassToTypeName.length;
    }

    private TypeClass _typeClass; // TODO should be final
    private String _typeName; // TODO should be final

    private Class<?> _class;
    private TypeDescription _iTypeDescription;
}
