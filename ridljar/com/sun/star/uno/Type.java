/*************************************************************************
 *
 *  $RCSfile: Type.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jbu $ $Date: 2002-01-18 14:04:55 $
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

/**
 * The Type class represents the IDL builtin type <code>type</code>.
 * <p>
 * The IDL type is not directly mapped to <code>java.lang.Class</code>,
 * because it can be necessary to describe a type which is unknown
 * to the java runtime system, e.g. for delaying the need of a class,
 * so that it is possible to generate it on the fly.
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2002-01-18 14:04:55 $
 * @since       UDK1.0
 */
public class Type {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static private final String[] __typeClassToTypeName = new String[]{
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

    static private final Hashtable __javaClassToTypeName = new Hashtable();

    static private final Hashtable __typeNameToTypeClass = new Hashtable();

    static {
        for(int i = 0; i < __typeClassToTypeName.length; ++ i)
            __typeNameToTypeClass.put(__typeClassToTypeName[i], TypeClass.fromInt(i));

        __javaClassToTypeName.put(Void.class,      "void");
        __javaClassToTypeName.put(void.class,      "void");
        __javaClassToTypeName.put(Character.class, "char");
        __javaClassToTypeName.put(char.class, "char");
        __javaClassToTypeName.put(Boolean.class,   "boolean");
        __javaClassToTypeName.put(boolean.class,   "boolean");
        __javaClassToTypeName.put(Byte.class,      "byte");
        __javaClassToTypeName.put(byte.class,      "byte");
        __javaClassToTypeName.put(Short.class,     "short");
        __javaClassToTypeName.put(short.class,     "short");
        __javaClassToTypeName.put(Integer.class,   "long");
        __javaClassToTypeName.put(int.class,       "long");
        __javaClassToTypeName.put(Long.class,      "hyper");
        __javaClassToTypeName.put(long.class,      "hyper");
        __javaClassToTypeName.put(Float.class,     "float");
        __javaClassToTypeName.put(float.class,     "float");
        __javaClassToTypeName.put(Double.class,    "double");
        __javaClassToTypeName.put(double.class,    "double");
        __javaClassToTypeName.put(String.class,    "string");
        __javaClassToTypeName.put(Type.class,      "type");
        __javaClassToTypeName.put(Any.class,       "any");
        __javaClassToTypeName.put(Object.class,       "any");
    }

    static private boolean __isTypeClassPrimitive(TypeClass typeClass) {
        return typeClass.getValue() < 15;
    }


    protected String           _typeName;
    protected Class            _class;
    protected TypeClass        _typeClass;

    protected ITypeDescription _iTypeDescription;


    /**
     * Constructs a new <code>Type</code> which defaults
     * to <code>void</code>
     * <p>
     * @since       UDK3.0
     */
    public Type() {
        this(void.class);
    }


    /**
     * Constructs a new <code>Type</code> with
     * the given <code>class</code>.
     * <p>
     * @since       UDK3.0
     * @param zClass   the java class of this type
     */
    public Type(Class zClass) {
        _class = zClass;

        _typeName = (String)__javaClassToTypeName.get(zClass);
        if(_typeName != null)
            _typeClass = (TypeClass)__typeNameToTypeClass.get(_typeName);

        else {
            if(Enum.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.ENUM;
                _typeName  = zClass.getName();
            }
            else if(Union.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.UNION;
                _typeName  = zClass.getName();
            }
            else if(zClass.isInterface()) {
                _typeClass = TypeClass.INTERFACE;
                _typeName  = zClass.getName();
            }
            else if(zClass.isArray()) {
                _typeClass = TypeClass.SEQUENCE;
                _typeName = "[]" + (new Type(_class.getComponentType()).getTypeName());
            }
            else if(Throwable.class.isAssignableFrom(zClass)) {
                _typeClass = TypeClass.EXCEPTION;
                _typeName  = zClass.getName();
            }
            else {
                _typeClass = TypeClass.UNKNOWN;
                _typeName  = zClass.getName();
            }
        }
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given type description.
     * <p>
     * @since       UDK3.0
     * @param typeDescription   a type description
     */
    public Type(ITypeDescription iTypeDescription) {
        _typeName         = iTypeDescription.getTypeName();
        _typeClass        = iTypeDescription.getTypeClass();
        _iTypeDescription = iTypeDescription;
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given type name.
     * <p>
     * @since       UDK3.0
     * @param typeName   the name of this type. For simple types
     *                   (numbers,string,type,any), the typeclass is calculated,
     *                   for complex types (structs,interfaces), the
     *                   typeclass of this object is set to UNKNOWN
     */
    public Type(String typeName) {
        _typeClass = (TypeClass)__typeNameToTypeClass.get(typeName);

        if(_typeClass == null)
            _typeClass = TypeClass.UNKNOWN;

        _typeName = typeName;
    }


    /**
     * Constructs a new <code>Type</code> with
     * the given <code>TypeClass</code>.
     * <p>
     * @since       UDK3.0
     * @param typeClass   the <code>TypeClass</code> of this type. Only typeclass for
     *                    simple types is allowed.
     *
     * @throws IllegalArgumentException when the typeClass is not simple (e.g.
               a struct or an interface. The Constructor cannot find out the
               name of the type in this case.
     */
    public Type(TypeClass typeClass) throws IllegalArgumentException {
        if(__isTypeClassPrimitive(typeClass)) {
            _typeClass = typeClass;
            _typeName = __typeClassToTypeName[typeClass.getValue()];
        }
        else
            throw new IllegalArgumentException(typeClass + " is not primitive");
    }

    /**
     * Gives the type description of this type.
     * <p>
     * @since       UDK3.0
     * @return the type description
     */
    public ITypeDescription getTypeDescription() {
        return _iTypeDescription;
    }

    /**
     * Sets the type description for this type.
     * <p>
     * @since       UDK3.0
     * @return the type description
     */
    public void setTypeDescription(ITypeDescription iTypeDescription) {
        _iTypeDescription = iTypeDescription;
    }

    /**
     * Gets the type name.
     * Returns <code>null</code> if this
     * type has not been constructed by name.
     * <p>
     * @since       UDK1.0
     * @return  the type name.
     */
    public String getTypeName() {
        return _typeName;
    }

    /**
     * Gets the java class.
     * Returns <code>null</code> if this
     * type has not been constructed by <code>Class</code>.
     * <p>
     * @since       UDK1.0
     * @return  the type name. Maybe null.
     */
    public Class getZClass() {
        return _class;
    }

    /**
     * Gets the typeClass.
     * Returns <code>null</code> if this
     * type has not been constructed by <code>TypeClass</code>.
     * <p>
     * @since       UDK1.0
     * @return  the type class. May be TypeClass.UNKNOWN.
     */
    public TypeClass getTypeClass() {
        return _typeClass;
    }

    /**
     * Compares two types.
     * <p>
     * @return    true, if the given type and this type are equal
     */
    public boolean equals(Object object) {
        boolean result = false;

        Type type = (Type)object;

        if(type != null)
            result = _typeName.equals(type._typeName);

        return result;
    }

    /**
     * Calculates the hash code.
     * <p>
     * @return    the hash code
     */
    public int hashCode() {
        return _typeName.hashCode();
    }

    /**
     * Constructs a descriptive <code>String</code> for the type.
     * <p>
     * @return   a descriptive <code>String</code>
     */
    public String toString() {
        return "Type<" + _typeName + ">";
    }
}

