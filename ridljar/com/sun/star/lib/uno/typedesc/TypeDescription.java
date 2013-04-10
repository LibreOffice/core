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

package com.sun.star.lib.uno.typedesc;

import com.sun.star.lib.uno.typeinfo.AttributeTypeInfo;
import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.SoftReference;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Supplies information about UNO types.
 *
 * @since UDK2.0
 */
public final class TypeDescription implements ITypeDescription {
    public static TypeDescription getTypeDescription(String typeName)
        throws ClassNotFoundException
    {
        Type t = new Type(typeName);
        if (t.getTypeClass() == TypeClass.UNKNOWN) {
            if (typeName.startsWith("[]")) {
                t = new Type(typeName, TypeClass.SEQUENCE);
            } else {
                t = new Type(Class.forName(typeName));
            }
        }
        return get(t);
    }

    public static TypeDescription getTypeDescription(Class<?> zClass) {
        return getDefinitely(new Type(zClass));
    }

    public static TypeDescription getTypeDescription(Type type)
        throws ClassNotFoundException
    {
        //TODO: synchronize on type?
        TypeDescription desc = (TypeDescription) type.getTypeDescription();
        if (desc == null) {
            desc = getTypeDescription(type.getTypeName());
            type.setTypeDescription(desc);
        }
        return desc;
    }

    public static TypeDescription getTypeDescription(TypeClass typeClass) {
        switch (typeClass.getValue()) {
        case TypeClass.VOID_value:
            return getDefinitely(Type.VOID);

        case TypeClass.BOOLEAN_value:
            return getDefinitely(Type.BOOLEAN);

        case TypeClass.BYTE_value:
            return getDefinitely(Type.BYTE);

        case TypeClass.SHORT_value:
            return getDefinitely(Type.SHORT);

        case TypeClass.UNSIGNED_SHORT_value:
            return getDefinitely(Type.UNSIGNED_SHORT);

        case TypeClass.LONG_value:
            return getDefinitely(Type.LONG);

        case TypeClass.UNSIGNED_LONG_value:
            return getDefinitely(Type.UNSIGNED_LONG);

        case TypeClass.HYPER_value:
            return getDefinitely(Type.HYPER);

        case TypeClass.UNSIGNED_HYPER_value:
            return getDefinitely(Type.UNSIGNED_HYPER);

        case TypeClass.FLOAT_value:
            return getDefinitely(Type.FLOAT);

        case TypeClass.DOUBLE_value:
            return getDefinitely(Type.DOUBLE);

        case TypeClass.CHAR_value:
            return getDefinitely(Type.CHAR);

        case TypeClass.STRING_value:
            return getDefinitely(Type.STRING);

        case TypeClass.TYPE_value:
            return getDefinitely(Type.TYPE);

        case TypeClass.ANY_value:
            return getDefinitely(Type.ANY);

        default:
            return null;
        }
    }

    public static boolean isTypeClassSimple(TypeClass typeClass) {
        return getTypeDescription(typeClass) != null;
    }

    // @see ITypeDescription#getSuperType
    public ITypeDescription getSuperType() {
        // Arbitrarily take the first super type:
        return superTypes == null || superTypes.length == 0
            ? null : superTypes[0];
    }

    // @see ITypeDescription#getMethodDescriptions
    public IMethodDescription[] getMethodDescriptions() {
        initMethodDescriptions();
        return methodDescriptions; //TODO: clone?
    }

    // @see ITypeDescription#getMethodDescription(int)
    public IMethodDescription getMethodDescription(int methodId) {
        initMethodDescriptions();
        return methodId < 0
            ? null
            : methodId < superMethodDescriptions.length
            ? superMethodDescriptions[methodId]
            : (methodId - superMethodDescriptions.length
               < methodDescriptions.length)
            ? methodDescriptions[methodId - superMethodDescriptions.length]
            : null;
    }

    // @see ITypeDescription#getMethodDescription(String)
    public IMethodDescription getMethodDescription(String name) {
        initMethodDescriptions();
        for (int i = 0; i < superMethodDescriptions.length; ++i) {
            if (superMethodDescriptions[i].getName().equals(name)) {
                return superMethodDescriptions[i];
            }
        }
        for (int i = 0; i < methodDescriptions.length; ++i) {
            if (methodDescriptions[i].getName().equals(name)) {
                return methodDescriptions[i];
            }
        }
        return null;
    }

    // @see ITypeDescription#getFieldDescriptions
    public IFieldDescription[] getFieldDescriptions() {
        return fieldDescriptions; //TODO: clone?
    }

    // @see ITypeDescription#getFieldDescription
    public IFieldDescription getFieldDescription(String name) {
        for (int i = 0; i < fieldDescriptions.length; ++i) {
            if (fieldDescriptions[i].getName().equals(name)) {
                return fieldDescriptions[i];
            }
        }
        return superTypes != null && superTypes.length == 1
            ? superTypes[0].getFieldDescription(name) : null;
    }

    // @see ITypeDescription#getTypeClass
    public TypeClass getTypeClass() {
        return typeClass;
    }

    // @see ITypeDescription#getComponentType
    public ITypeDescription getComponentType() {
        return componentType;
    }

    // @see ITypeDescription#getTypeName
    public String getTypeName() {
        return typeName;
    }

    // @see ITypeDescription#getArrayTypeName
    public String getArrayTypeName() {
        return arrayTypeName;
    }

    // @see ITypeDescription#getZClass
    public Class<?> getZClass() {
        return zClass;
    }

    public boolean hasTypeArguments() {
        return hasTypeArguments;
    }

    // @see Object#toString
    public String toString() {
        return "[" + getClass().getName() + ": " + getTypeClass() + ", "
            + getTypeName() + "]";
    }

    private static TypeDescription getDefinitely(Type type) {
        try {
            return get(type);
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException("this cannot happen: " + e);
        }
    }

    private static TypeDescription get(Type type) throws ClassNotFoundException
    {
        String typeName = type.getTypeName();
        TypeDescription desc = cache.get(typeName);
        if (desc == null) {
            desc = create(type);
            cache.put(desc);
        }
        return desc;
    }

    private static TypeDescription create(Type type)
        throws ClassNotFoundException
    {
        TypeClass typeClass = type.getTypeClass();
        String typeName = type.getTypeName();
        Class<?> zClass = type.getZClass();
        if (zClass == null) {
            throw new ClassNotFoundException("UNO type " + type);
        }
        switch (typeClass.getValue()) {
        case TypeClass.VOID_value:
            return new TypeDescription(
                typeClass, typeName, "[Ljava.lang.Void;", zClass, null, null);

        case TypeClass.BOOLEAN_value:
            return new TypeDescription(
                typeClass, typeName, "[Z", zClass, null, null);

        case TypeClass.BYTE_value:
            return new TypeDescription(
                typeClass, typeName, "[B", zClass, null, null);

        case TypeClass.SHORT_value:
        case TypeClass.UNSIGNED_SHORT_value:
            return new TypeDescription(
                typeClass, typeName, "[S", zClass, null, null);

        case TypeClass.LONG_value:
        case TypeClass.UNSIGNED_LONG_value:
            return new TypeDescription(
                typeClass, typeName, "[I", zClass, null, null);

        case TypeClass.HYPER_value:
        case TypeClass.UNSIGNED_HYPER_value:
            return new TypeDescription(
                typeClass, typeName, "[J", zClass, null, null);

        case TypeClass.FLOAT_value:
            return new TypeDescription(
                typeClass, typeName, "[F", zClass, null, null);

        case TypeClass.DOUBLE_value:
            return new TypeDescription(
                typeClass, typeName, "[D", zClass, null, null);

        case TypeClass.CHAR_value:
            return new TypeDescription(
                typeClass, typeName, "[C", zClass, null, null);

        case TypeClass.STRING_value:
            return new TypeDescription(
                typeClass, typeName, "[Ljava.lang.String;", zClass, null, null);

        case TypeClass.TYPE_value:
            return new TypeDescription(
                typeClass, typeName, "[Lcom.sun.star.uno.Type;", zClass, null,
                null);

        case TypeClass.ANY_value:
            return new TypeDescription(
                typeClass, typeName, "[Ljava.lang.Object;", zClass, null, null);

        case TypeClass.SEQUENCE_value:
            {
                // assert typeName.startsWith("[]");
                ITypeDescription componentType = getTypeDescription(
                    typeName.substring("[]".length()));
                // assert zClass.getName().startsWith("[");
                return new TypeDescription(
                    typeClass, typeName, "[" + zClass.getName(), zClass, null,
                    componentType);
            }

        case TypeClass.ENUM_value:
            // assert !zClass.getName().startsWith("[");
            return new TypeDescription(
                typeClass, typeName, "[L" + zClass.getName() + ";", zClass,
                null, null);

        case TypeClass.STRUCT_value:
            {
                // This code exploits the fact that an instantiated polymorphic
                // struct type may not be the direct base of a struct type:
                Class<?> superClass = zClass.getSuperclass();
                TypeDescription[] superTypes = superClass != Object.class
                    ? new TypeDescription[] { get(new Type(superClass)) }
                    : null;
                // assert !zClass.getName().startsWith("[");
                return new TypeDescription(
                    typeClass, typeName, "[L" + zClass.getName() + ";", zClass,
                    superTypes, null);
            }

        case TypeClass.EXCEPTION_value:
            {
                TypeDescription[] superTypes
                    = typeName.equals("com.sun.star.uno.Exception")
                    || typeName.equals("com.sun.star.uno.RuntimeException")
                    ? null
                    : new TypeDescription[] {
                            get(new Type(zClass.getSuperclass())) };
                // assert !zClass.getName().startsWith("[");
                return new TypeDescription(
                    typeClass, typeName, "[L" + zClass.getName() + ";", zClass,
                    superTypes, null);
            }

        case TypeClass.INTERFACE_value:
            {
                List superTypes = new List();
                Class<?>[] interfaces = zClass.getInterfaces();
                for (int i = 0; i < interfaces.length; ++i) {
                    Type t = new Type(interfaces[i]);
                    if (t.getTypeClass() == TypeClass.INTERFACE) {
                        TypeDescription desc = getDefinitely(t);
                        TypeDescription[] descs = desc.superTypes;
                        for (int j = 0; j < descs.length; ++j) {
                            superTypes.add(descs[j]);
                        }
                        superTypes.add(desc);
                    }
                }
                // assert !zClass.getName().startsWith("[");
                return new TypeDescription(
                    typeClass, typeName, "[L" + zClass.getName() + ";", zClass,
                    superTypes.toArray(), null);
            }

        default:
            throw new IllegalArgumentException("given type has bad type class");
        }
    }

    private TypeDescription(
        TypeClass typeClass, String typeName, String arrayTypeName,
        Class<?> zClass, TypeDescription[] superTypes,
        ITypeDescription componentType)
    {
        this.typeClass = typeClass;
        this.typeName = typeName;
        this.arrayTypeName = arrayTypeName;
        this.zClass = zClass;
        this.superTypes = superTypes;
        this.componentType = componentType;
        TypeDescription[] args = calculateTypeArguments();
        this.hasTypeArguments = args != null;
        this.fieldDescriptions = calculateFieldDescriptions(args);
        // methodDescriptions must be initialized lazily, to avoid problems with
        // circular dependencies (a super-interface that has a sub-interface as
        // method parameter type; an interface that has a struct as method
        // parameter type, and the struct has the interface as member type)
    }

    private synchronized void initMethodDescriptions() {
        if (methodDescriptions != null || typeClass != TypeClass.INTERFACE) {
            return;
        }
        if (superTypes.length == 0) { // com.sun.star.uno.XInterface
            superMethodDescriptions = new IMethodDescription[0];
            methodDescriptions = new IMethodDescription[] {
                new MethodDescription(
                    "queryInterface", MethodDescription.ID_QUERY_INTERFACE,
                    false, new ITypeDescription[] { getDefinitely(Type.TYPE) },
                    new ITypeDescription[] { null }, getDefinitely(Type.ANY),
                    null),
                new MethodDescription(
                    "acquire", MethodDescription.ID_ACQUIRE, true,
                    new ITypeDescription[0], new ITypeDescription[0],
                    getDefinitely(Type.VOID), null),
                new MethodDescription(
                    "release", MethodDescription.ID_RELEASE, true,
                    new ITypeDescription[0], new ITypeDescription[0],
                    getDefinitely(Type.VOID), null) };
        } else {
            int methodOffset = 0;
            ArrayList<MethodDescription> superList = new ArrayList<MethodDescription>();
            for (int i = 0; i < superTypes.length; ++i) {
                IMethodDescription[] ds = superTypes[i].getMethodDescriptions();
                for (int j = 0; j < ds.length; ++j) {
                    superList.add(new MethodDescription(ds[j], methodOffset++));
                }
            }
            superMethodDescriptions = superList.toArray(
                new IMethodDescription[superList.size()]);
            ArrayList<MethodDescription> directList = new ArrayList<MethodDescription>();
            TypeInfo[] infos = getTypeInfo();
            int infoCount = infos == null ? 0 : infos.length;
            int index = 0;
            Method[] methods = zClass.getDeclaredMethods();
            for (int i = 0; i < infoCount;) {
                if (infos[i] instanceof AttributeTypeInfo) {
                    AttributeTypeInfo info = (AttributeTypeInfo) infos[i++];
                    if (info.getIndex() != index) {
                        throw new IllegalArgumentException(
                            "Bad UNOTYPEINFO for " + zClass
                            + ": entries not ordererd");
                    }
                    String getterName = "get" + info.getName();
                    Method getter = findMethod(methods, getterName);
                    Type t = info.getUnoType();
                    ITypeDescription type = t == null
                        ? getTypeDescription(getter.getReturnType(), info)
                        : getDefinitely(t);
                    directList.add(
                        new MethodDescription(
                            getterName, index++ + methodOffset, false,
                            new ITypeDescription[0], new ITypeDescription[0],
                            type, getter));
                    if (!info.isReadOnly()) {
                        String setterName = "set" + info.getName();
                        Method setter = findMethod(methods, setterName);
                        directList.add(
                            new MethodDescription(
                                setterName, index++ + methodOffset, false,
                                new ITypeDescription[] { type },
                                new ITypeDescription[] { null },
                                getDefinitely(Type.VOID), setter));
                    }
                } else {
                    MethodTypeInfo info = (MethodTypeInfo) infos[i++];
                    if (info.getIndex() != index) {
                        throw new IllegalArgumentException(
                            "Bad UNOTYPEINFO for " + zClass
                            + ": entries not ordererd");
                    }
                    Method method = findMethod(methods, info.getName());
                    Class<?>[] params = method.getParameterTypes();
                    ITypeDescription[] in = new ITypeDescription[params.length];
                    ITypeDescription[] out
                        = new ITypeDescription[params.length];
                    for (int j = 0; j < params.length; ++j) {
                        ParameterTypeInfo p = null;
                        if (i < infoCount
                            && infos[i] instanceof ParameterTypeInfo
                            && ((ParameterTypeInfo) infos[i]).getIndex() == j)
                        {
                            p = (ParameterTypeInfo) infos[i++];
                        }
                        Type pt = p == null ? null : p.getUnoType();
                        ITypeDescription d = pt == null
                            ? getTypeDescription(params[j], p)
                            : getDefinitely(pt);
                        if (p == null || p.isIN()) {
                            in[j] = d;
                        }
                        if (p != null && p.isOUT()) {
                            out[j] = d;
                        }
                    }
                    Type t = info.getUnoType();
                    directList.add(
                        new MethodDescription(
                            info.getName(), index++ + methodOffset,
                            info.isOneway(), in, out,
                            (t == null
                             ? getTypeDescription(method.getReturnType(), info)
                             : getDefinitely(t)),
                            method));
                }
            }
            methodDescriptions = directList.toArray(
                new IMethodDescription[directList.size()]);
        }
    }

    private TypeDescription[] calculateTypeArguments() {
        if (typeClass != TypeClass.STRUCT) {
            return null;
        }
        int i = typeName.indexOf('<');
        if (i < 0) {
            return null;
        }
        java.util.List<TypeDescription> args = new java.util.ArrayList<TypeDescription>();
        do {
            ++i; // skip '<' or ','
            int j = i;
        loop:
            for (int level = 0; j != typeName.length(); ++j) {
                switch (typeName.charAt(j)) {
                case ',':
                    if (level == 0) {
                        break loop;
                    }
                    break;

                case '<':
                    ++level;
                    break;

                case '>':
                    if (level == 0) {
                        break loop;
                    }
                    --level;
                    break;
                }
            }
            if (j != typeName.length()) {
                Type t = new Type(typeName.substring(i, j));
                if (t.getZClass() == null) {
                    throw new IllegalArgumentException(
                        "UNO type name \"" + typeName
                        + "\" contains bad type argument \""
                        + typeName.substring(i, j) + "\"");
                }
                args.add(getDefinitely(t));
            }
            i = j;
        } while (i != typeName.length() && typeName.charAt(i) != '>');
        if (i != typeName.length() - 1 || typeName.charAt(i) != '>'
            || args.isEmpty())
        {
            throw new IllegalArgumentException(
                "UNO type name \"" + typeName + "\" is syntactically invalid");
        }
        return args.toArray(
                new TypeDescription[args.size()]);
    }

    private IFieldDescription[] calculateFieldDescriptions(
        TypeDescription[] typeArguments)
    {
        if (typeClass != TypeClass.STRUCT && typeClass != TypeClass.EXCEPTION) {
            return null;
        }
        TypeInfo[] infos = getTypeInfo();
        int infoCount = infos == null ? 0 : infos.length;
        ITypeDescription superType = getSuperType();
        IFieldDescription[] superDescs = superType == null
            ? null : superType.getFieldDescriptions();
        int superCount = superDescs == null ? 0 : superDescs.length;
        IFieldDescription[] descs = new IFieldDescription[
            superCount + infoCount];
        if (superCount != 0) {
            System.arraycopy(superDescs, 0, descs, 0, superCount);
        }
        for (int i = 0; i < infoCount; ++i) {
            MemberTypeInfo info = (MemberTypeInfo) infos[i];
            if (info.getIndex() != i) {
                throw new IllegalArgumentException(
                    "Bad UNOTYPEINFO for " + zClass + ": entries not ordererd");
            }
            Field field;
            try {
                field = zClass.getDeclaredField(info.getName());
            } catch (NoSuchFieldException e) {
                throw new IllegalArgumentException(
                    "Bad UNOTYPEINFO for " + zClass + ": " + e);
            }
            Type t = info.getUnoType();
            int index = info.getTypeParameterIndex();
            descs[i + superCount] = new FieldDescription(
                info.getName(), i + superCount,
                (index >= 0
                 ? typeArguments[index]
                 : t == null
                 ? getTypeDescription(field.getType(), info)
                 : getDefinitely(t)),
                field);
        }
        return descs;
    }

    private TypeInfo[] getTypeInfo() {
        try {
            return (TypeInfo[])
                zClass.getDeclaredField("UNOTYPEINFO").get(null);
        } catch (NoSuchFieldException e) {
            return null;
        } catch (IllegalAccessException e) {
            throw new IllegalArgumentException(
                "Bad UNOTYPEINFO for " + zClass + ": " + e);
        }
    }

    private Method findMethod(Method[] methods, String name) {
        for (int i = 0; i < methods.length; ++i) {
            if (methods[i].getName().equals(name)) {
                return methods[i];
            }
        }
        throw new IllegalArgumentException(
            "Bad UNOTYPEINFO for " + zClass + ": no method " + name);
    }

    private static ITypeDescription getTypeDescription(
        Class<?> zClass, TypeInfo typeInfo)
    {
        return getDefinitely(
            new Type(
                zClass,
                typeInfo != null
                && (typeInfo.isUnsigned() || typeInfo.isInterface())));
    }

    private static final class List {
        public List() {}

        public void add(TypeDescription desc) {
            if (!list.contains(desc)) {
                list.add(desc);
            }
        }

        public boolean isEmpty() {
            return list.isEmpty();
        }

        public TypeDescription[] toArray() {
            return list.toArray(
                new TypeDescription[list.size()]);
        }

        private final ArrayList<TypeDescription> list = new ArrayList<TypeDescription>();
    }

    private static final class Cache {
        public Cache() {}

        public TypeDescription get(String typeName) {
            synchronized (map) {
                cleanUp();
                Entry e = map.get(typeName);
                return e == null ? null : (TypeDescription) e.get();
            }
        }

        public void put(TypeDescription desc) {
            synchronized (map) {
                cleanUp();
                map.put(desc.getTypeName(), new Entry(desc, queue));
            }
        }

        private void cleanUp() {
            for (;;) {
                Object tmp = queue.poll();
                Entry e = (Entry)tmp;
                if (e == null) {
                    break;
                }
                map.remove(e.typeName);
            }
        }

        private static final class Entry extends SoftReference<TypeDescription> {
            public Entry(TypeDescription desc, ReferenceQueue<TypeDescription> queue) {
                super(desc, queue);
                typeName = desc.getTypeName();
            }

            public final String typeName;
        }

        private final HashMap<String, Entry> map = new HashMap<String, Entry>();
        private final ReferenceQueue<TypeDescription> queue = new ReferenceQueue<TypeDescription>();
    }

    private static final Cache cache = new Cache();

    private final TypeClass typeClass;
    private final String typeName;
    private final String arrayTypeName;
    private final Class<?> zClass;
    private final TypeDescription[] superTypes;
    private final ITypeDescription componentType;
    private final boolean hasTypeArguments;
    private final IFieldDescription[] fieldDescriptions;
    private IMethodDescription[] methodDescriptions = null;
    private IMethodDescription[] superMethodDescriptions;
}
