/*************************************************************************
 *
 *  $RCSfile: Type.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2000-11-08 15:39:31 $
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


import java.util.Hashtable;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;


import com.sun.star.lib.uno.typeinfo.AttributeTypeInfo;
import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;


/**
 * The Type class represents the IDL builtin type <code>type</code>.
 * <p>
 * The IDL type is not directly mapped to <code>java.lang.Class</code>,
 * because it can be necessary to describe a type which is unknown
 * to the java runtime system, e.g. for delaying the need of a class,
 * so that it is possible to generate it one the fly.
 * <p>
 * The current implementations holds various <code>static</code> helper
 * methods, which may be changed or moved in the furture, so please
 * do not use these methods.
 * <p>
 * @version     $Revision: 1.1 $ $ $Date: 2000-11-08 15:39:31 $
 * @author      Markus Meyer
 * @author      Kay Ramme
 * @since       UDK1.0
 */
public class Type {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;

    static private final Hashtable __classToTypeClass = new Hashtable();
    static private final Hashtable __typeNameToTypeClass = new Hashtable();
    static private final Hashtable __typeClassToTypeName = new Hashtable();

    static {
        __classToTypeClass.put(Void.class,      new Object[]{TypeClass.VOID,    "void"});
        __classToTypeClass.put(void.class,      new Object[]{TypeClass.VOID,    "void"});
        __classToTypeClass.put(Character.class, new Object[]{TypeClass.CHAR,    "char"});
        __classToTypeClass.put(char.class,      new Object[]{TypeClass.CHAR,    "char"});
        __classToTypeClass.put(Boolean.class,   new Object[]{TypeClass.BOOLEAN, "boolean"});
        __classToTypeClass.put(boolean.class,   new Object[]{TypeClass.BOOLEAN, "boolean"});
        __classToTypeClass.put(Byte.class,      new Object[]{TypeClass.BYTE,    "byte"});
        __classToTypeClass.put(byte.class,      new Object[]{TypeClass.BYTE,    "byte"});
        __classToTypeClass.put(Short.class,     new Object[]{TypeClass.SHORT,   "short"});
        __classToTypeClass.put(short.class,     new Object[]{TypeClass.SHORT,   "short"});
        __classToTypeClass.put(Integer.class,   new Object[]{TypeClass.LONG,    "long"});
        __classToTypeClass.put(int.class,       new Object[]{TypeClass.LONG,    "long"});
        __classToTypeClass.put(Long.class,      new Object[]{TypeClass.HYPER,   "hyper"});
        __classToTypeClass.put(long.class,      new Object[]{TypeClass.HYPER,   "hyper"});
        __classToTypeClass.put(Float.class,     new Object[]{TypeClass.FLOAT,   "float"});
        __classToTypeClass.put(float.class,     new Object[]{TypeClass.FLOAT,   "float"});
        __classToTypeClass.put(Double.class,    new Object[]{TypeClass.DOUBLE,  "double"});
        __classToTypeClass.put(double.class,    new Object[]{TypeClass.DOUBLE,  "double"});
        __classToTypeClass.put(String.class,    new Object[]{TypeClass.STRING,  "string"});
        __classToTypeClass.put(Type.class,      new Object[]{TypeClass.TYPE,    "type"});
        __classToTypeClass.put(Any.class,       new Object[]{TypeClass.ANY,     "any"});
        __classToTypeClass.put(Object.class,    new Object[]{TypeClass.ANY,     "any"});

        __typeNameToTypeClass.put("boolean",        TypeClass.BOOLEAN);
        __typeNameToTypeClass.put("short",          TypeClass.SHORT);
        __typeNameToTypeClass.put("unsigned short", TypeClass.UNSIGNED_SHORT);
        __typeNameToTypeClass.put("long",           TypeClass.LONG);
        __typeNameToTypeClass.put("unsigned long",  TypeClass.UNSIGNED_LONG);
        __typeNameToTypeClass.put("hyper",          TypeClass.HYPER);
        __typeNameToTypeClass.put("unsigned hyper", TypeClass.UNSIGNED_HYPER);
        __typeNameToTypeClass.put("float",          TypeClass.FLOAT);
        __typeNameToTypeClass.put("double",         TypeClass.DOUBLE);
        __typeNameToTypeClass.put("char",           TypeClass.CHAR);
        __typeNameToTypeClass.put("byte",           TypeClass.BYTE);
        __typeNameToTypeClass.put("string",         TypeClass.STRING);
        __typeNameToTypeClass.put("any",            TypeClass.ANY);
        __typeNameToTypeClass.put("void",           TypeClass.VOID);

        __typeClassToTypeName.put(TypeClass.BOOLEAN,        "boolean");
        __typeClassToTypeName.put(TypeClass.SHORT,          "short");
        __typeClassToTypeName.put(TypeClass.UNSIGNED_SHORT, "unsigned short");
        __typeClassToTypeName.put(TypeClass.LONG,           "long");
        __typeClassToTypeName.put(TypeClass.UNSIGNED_LONG,  "unsigned long");
        __typeClassToTypeName.put(TypeClass.HYPER,          "hyper");
        __typeClassToTypeName.put(TypeClass.UNSIGNED_HYPER, "unsigned hyper");
        __typeClassToTypeName.put(TypeClass.FLOAT,          "float");
        __typeClassToTypeName.put(TypeClass.DOUBLE,         "double");
        __typeClassToTypeName.put(TypeClass.CHAR,           "char");
        __typeClassToTypeName.put(TypeClass.BYTE,           "byte");
        __typeClassToTypeName.put(TypeClass.STRING,         "string");
        __typeClassToTypeName.put(TypeClass.ANY,            "any");
        __typeClassToTypeName.put(TypeClass.VOID,           "void");
    }

    static private final Object[] __getTypeClassForClass(Class zClass) {
        Object data[] = (Object[])__classToTypeClass.get(zClass);

        if(data == null) { // it is a complex type
            TypeClass typeClass = null;
            String    typeName = null;

            if(Enum.class.isAssignableFrom(zClass)) {
               typeClass = TypeClass.ENUM;
               typeName  = zClass.getName();
            }
            else if(Union.class.isAssignableFrom(zClass)) {
                typeClass = TypeClass.UNION;
                typeName  = zClass.getName();
            }
            else if(XInterface.class.isAssignableFrom(zClass)) {
                typeClass = TypeClass.INTERFACE;
                typeName  = zClass.getName();
            }
            else if(zClass.isArray()) {
                typeClass = TypeClass.SEQUENCE;

                Object tmp[] = __getTypeClassForClass(zClass.getComponentType());

                typeName = "[]" + tmp[1];
            }
            else if(java.lang.Throwable.class.isAssignableFrom(zClass)) {
                typeClass = TypeClass.EXCEPTION;
                typeName  = zClass.getName();
            }
            else {
                typeClass = TypeClass.STRUCT;
                typeName  = zClass.getName();
            }

            data = new Object[]{typeClass, typeName};
        }

        return data;
    }

    static private final TypeClass __getTypeClassForTypeName(String typeName) {
        TypeClass typeClass = (TypeClass)__typeNameToTypeClass.get(typeName);

        if(typeClass == null) {
            if(typeName.startsWith("[]"))
                typeClass = TypeClass.SEQUENCE;
            else {
                try {
                    typeClass = (TypeClass)__getTypeClassForClass(Class.forName(typeName))[0];
                }
                catch(ClassNotFoundException classNotFoundException) {
                    // so we don't know the type class!
                    typeClass = TypeClass.UNKNOWN;
                }
            }
        }
        if(DEBUG) System.err.println("##### __getTypeClassForTypeName:" + typeName + " " + typeClass.getValue());

        return typeClass;
    }

    static private final Object[] __typeClassToClassNames = new Object[]{
        new String[]{"java.lang.Void",        "V"},                       // VOID
        new String[]{"java.lang.Character",   "C"},                       // CHAR
        new String[]{"java.lang.Boolean",     "Z"},                       // BOOLEAN
        new String[]{"java.lang.Byte",        "B"},                       // BYTE
        new String[]{"java.lang.Short",       "S"},                       // SHORT
        new String[]{"java.lang.Short",       "S"},                       // UNSIGNED SHORT
        new String[]{"java.lang.Integer",     "I"},                       // LONG
        new String[]{"java.lang.Integer",     "I"},                       // UNSIGNED_LONG
        new String[]{"java.lang.Long",        "L"},                       // HYPER
        new String[]{"java.lang.Long",        "L"},                       // UNSIGNED_HYPER
        new String[]{"java.lang.Float",       "F"},                       // FLOAT
        new String[]{"java.lang.Double",      "D"},                       // DOUBLE
        new String[]{"java.lang.String",      "Ljava.lang.String;"},      // STRING
        new String[]{"com.sun.star.uno.Type", "Lcom.sun.star.uno.Type;"}, // TYPE
        new String[]{"java.lang.Object",      "Ljava.lang.Object;"}       // ANY
    };

    static public boolean isTypeClassSimple(TypeClass typeClass) {
        return typeClass.getValue() < __typeClassToClassNames.length;
    }

    static private final String[] __getClassNamesForTypeClass(TypeClass typeClass, String typeName) {
        String classNames[] = null;

        if(typeClass == null)
            typeClass = __getTypeClassForTypeName(typeName);

        if(isTypeClassSimple(typeClass))
            classNames = (String[])__typeClassToClassNames[typeClass.getValue()];
        else {
            switch(typeClass.getValue()) {
            case TypeClass.ENUM_value:
            case TypeClass.TYPEDEF_value:
            case TypeClass.STRUCT_value:
            case TypeClass.UNION_value:
            case TypeClass.EXCEPTION_value:
            case TypeClass.ARRAY_value:
            case TypeClass.INTERFACE_value:
            case TypeClass.SERVICE_value:
            case TypeClass.MODULE_value:
            case TypeClass.INTERFACE_METHOD_value:
            case TypeClass.INTERFACE_ATTRIBUTE_value:
            case TypeClass.UNKNOWN_value:
                classNames = new String[]{typeName, "L" + typeName + ";"};
                break;

            case TypeClass.SEQUENCE_value:
                String tmp[] = __getClassNamesForTypeClass(null, typeName.substring(2));

                classNames = new String[]{"[" + tmp[1], "[" + tmp[1]};
                break;
            }
        }

        return classNames;
    }

    protected TypeClass _typeClass;
    protected String    _typeName;
    protected String    _arrayTypeName;
    protected Class     _class;

    public Type() {
        try {
            init(TypeClass.VOID, null, void.class);
        }
        catch(Exception exception) { // this can never happen, except something is very wrong
            System.err.println("WARNING --- " + getClass().getName() + ".<init> exception occurred - " + exception);
        }
    }

    public Type(Class zClass) throws Exception {
        init(null, null, zClass);
    }

    public Type(String typeName) throws com.sun.star.uno.Exception, ClassNotFoundException {
        init(null, typeName, null);
    }

    public Type(TypeClass typeClass) throws com.sun.star.uno.Exception {
        init(typeClass, null, null);
    }

    public Type(TypeClass typeClass, String typeName) throws com.sun.star.uno.Exception {
        init(typeClass, typeName, null);
    }

    /**
     * Constructs a new <code>Type</code>.
     * <p>
     * @param  typeClass      the IDL typeClass
     * @param  typeName       must be valid java classname, if zClass is zero
     * @param  zClass         the java class, can be omitted
     */
    public Type(TypeClass typeClass, String typeName, Class zClass) throws com.sun.star.uno.Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init>:" + typeClass + " " + typeName);

        init(typeClass, typeName, zClass);
    }

    /**
     * Constructs a new <code>Type</code>.
     * <p>
     * @param  typeClass      the IDL typeClass
     * @param  typeName       must be valid java classname, if zClass is zero
     * @param  arrayTypeName  an java array classname
     * @param  zClass         the java class, can be omitted
     * @deprecated as of UDK 1.1
     */
    public Type(TypeClass typeClass, String typeName, String arrayTypeName, Class zClass) throws com.sun.star.uno.Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init>:" + typeClass + " " + typeName);

        init(typeClass, typeName, zClass);
    }

    protected void init(TypeClass typeClass, String typeName, Class zClass) throws com.sun.star.uno.Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".init:" + typeClass + " " + typeName);

        if(typeClass == null && zClass != null) {
            Object tmp[] = __getTypeClassForClass(zClass);

            typeClass = (TypeClass)tmp[0];
            typeName  = (String)tmp[1];
        }

        if(typeClass == null && typeName != null)
            typeClass = __getTypeClassForTypeName(typeName);

        if(typeName == null && typeClass != null)
            typeName = (String)__typeClassToTypeName.get(typeClass);

        if(typeClass == null)
            throw new com.sun.star.uno.Exception("Type - TypeClass must not be null");

        if(zClass == null) {
            String tmp[] = __getClassNamesForTypeClass(typeClass, typeName);
            try {
                zClass = Class.forName(tmp[0]);
            }
            catch(ClassNotFoundException classNotFoundException) {
                throw new com.sun.star.uno.Exception("Type - ClassNotFoundException:" + tmp[0]);
            }

            _arrayTypeName = tmp[1];
        }

        if(zClass == null)
            throw new com.sun.star.uno.Exception("Type - Class must not be null");

        _typeClass     = typeClass;
        _typeName      = typeName;
        _class         = zClass;
    }

    /**
     * Gets the IDL <code>TypeClass</code> of the type.
     * <p>
     * @return  the <code>TypeClass</code>.
     */
    public TypeClass getTypeClass() {
        return _typeClass;
    }

    /**
     * Gets the type name.
     * <p>
     * @return  the type name.
     */
    public String getTypeName() {
        return _typeName;
    }

    /**
     * Gets the array type name.
     * <p>
     * @return  the array type name.
     */
    public String getArrayTypeName() {
        if(_arrayTypeName == null) {
            String tmp[] = __getClassNamesForTypeClass(_typeClass, _typeName);
            _arrayTypeName = tmp[1];
        }

        return _arrayTypeName;
    }

    /**
     * Gets the corresponding java class for the type.
     * <p>
     * @return   the corresponding java class.
     */
    public Class getDescription() throws ClassNotFoundException {
        return _class;
    }

    /**
     * Compares two types.
     * <p>
     * @return    true, if the given type and this type are equal
     */
    public boolean equals(Object type) {
        return _typeName.equals(((Type)type)._typeName);
    }


    public int hashCode() {
        return _typeName.hashCode();
    }

    /**
     * Constructs a descriptive <code>String</code> for the type.
     * <p>
     * @return   a descriptive <code>String</code>
     */
    public String toString() {
        return "Type: " + _typeClass + " " + _typeName + " " + _class;
    }
}

