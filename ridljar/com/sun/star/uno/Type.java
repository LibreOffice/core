/*************************************************************************
 *
 *  $RCSfile: Type.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2001-02-19 10:07:03 $
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


import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
//import java.lang.reflect.InstantiationException;

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
 * @version     $Revision: 1.4 $ $ $Date: 2001-02-19 10:07:03 $
 * @author      Markus Meyer
 * @author      Kay Ramme
 * @since       UDK1.0
 */
public class Type {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;

    static private Method __getFromClass;
    static private Method __getFromName;
    static private Method __getFromTypeClass;

    /* these variables will be removed with UDK 3.0 */
    static private Method __isTypeClassSimple;
    static private Method __getTypeClass;
    static private Method __getTypeName;
    static private Method __getArrayTypeName;
    static private Method __getZClass;

    static {
        if(DEBUG) System.err.println("##### com.sun.star.uno.Type.<sinit>");

        Throwable throwable = null;

        try {
            Class typeDesc_Class = Class.forName("com.sun.star.lib.uno.typedesc.TypeDescription");

            __getFromName = typeDesc_Class.getMethod("getTypeDescription", new Class[]{String.class});
            __getFromClass = typeDesc_Class.getMethod("getTypeDescription", new Class[]{Class.class});
            __getFromTypeClass = typeDesc_Class.getMethod("getTypeDescription", new Class[]{TypeClass.class});

            __isTypeClassSimple = typeDesc_Class.getMethod("isTypeClassSimple", new Class[]{TypeClass.class});

            __getTypeClass = typeDesc_Class.getMethod("getTypeClass", new Class[]{});
            __getTypeName = typeDesc_Class.getMethod("getTypeName", new Class[]{});
            __getArrayTypeName = typeDesc_Class.getMethod("getArrayTypeName", new Class[]{});
            __getZClass = typeDesc_Class.getMethod("getZClass", new Class[]{});
        }
        catch(NoSuchMethodException noSuchMethodException) {
            throwable = noSuchMethodException;
        }
        catch(ClassNotFoundException classNotFoundException) {
            throwable = classNotFoundException;
        }

        if(throwable != null) {
            String message = "com.sun.star.uno.Type.<sinit> - exception:" + throwable;
            throwable.printStackTrace();
            System.err.println(message);
            throw new java.lang.RuntimeException(message);
        }
    }



    /**
     * Determines if the given <code>TypeClass</code> is simple.
     * <p>
     * @deprecated as of UDK 2.0 (moved to com.sun.star.lib.uno.typedesc.TypeDescription)
     * <p>
     * @return                <code>true</code> means simple
     * @param  typeClass      the IDL typeClass
     */
    static public boolean isTypeClassSimple(TypeClass typeClass) {
        boolean isSimple = false;

        try {
            isSimple = ((Boolean)__isTypeClassSimple.invoke(null, new Object[]{typeClass})).booleanValue();
        }
        catch(IllegalAccessException illegalAccessException) {
        }
        catch(InvocationTargetException invocationTargetException) {
        }

        return isSimple;
    }



    protected Object _typeDescription;

    protected void init(Class zClass) {
        Throwable throwable = null;

        try {
            _typeDescription = __getFromClass.invoke(null, new Object[]{zClass});
        }
        catch(InvocationTargetException invocationTargetException) {
            throwable = invocationTargetException.getTargetException();
        }
        catch(IllegalAccessException illegalAccessException) {
            throwable = illegalAccessException;
        }

        if(throwable != null)
            throw new java.lang.RuntimeException(getClass().getName() + ".<init>(Class) - fatal - couldn't create typedescription - " + throwable + " " + zClass);
    }

    protected void init(TypeClass typeClass) {
        Throwable throwable = null;

        try {
            _typeDescription = __getFromTypeClass.invoke(null, new Object[]{typeClass});
        }
        catch(InvocationTargetException invocationTargetException) {
            throwable = invocationTargetException.getTargetException();
        }
        catch(IllegalAccessException illegalAccessException) {
            throwable = illegalAccessException;
        }

        if(throwable != null)
            throw new java.lang.RuntimeException(getClass().getName() + ".<init>(TypeClass) - fatal - couldn't create typedescription - " + throwable + " " + typeClass);
    }

    protected void init(String typeName) throws ClassNotFoundException {
        Throwable throwable = null;

        try {
            _typeDescription = __getFromName.invoke(null, new Object[]{typeName});
        }
        catch(InvocationTargetException invocationTargetException) {
            throwable = invocationTargetException.getTargetException();
        }
        catch(IllegalAccessException illegalAccessException) {
            throwable = illegalAccessException;
        }

        if(throwable != null)
            throw new java.lang.ClassNotFoundException(getClass().getName() + ".<init>(String) - fatal - couldn't create typedescription - " + throwable);
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given type description.
     * <p>
     * @param typeDescription   a type description
     */
    public Type(Object typeDescription) {
        _typeDescription = typeDescription;
    }

    /**
     * Constructs a new <code>Type</code> which defaults
     * to <code>void</code>
     * <p>
     */
    public Type() {
        init(void.class);
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given <code>class</code>.
     * <p>
     * @param zClass   the class of this type
     */
    public Type(Class zClass) throws Exception { // the exception is only for compatibility reasons
        init(zClass);
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given type name.
     * <p>
     * @param typeName   the name of this type
     */
    public Type(String typeName) throws com.sun.star.uno.Exception, ClassNotFoundException { // the com.sun.star.uno.Exception is only for compatibility reasons
        init(typeName);
    }

    /**
     * Constructs a new <code>Type</code> with
     * the given <code>TypeClass</code>.
     * <p>
     * @param typeClass   the <code>TypeClass</code> of this type
     */
    public Type(TypeClass typeClass) throws com.sun.star.uno.Exception {
        init(typeClass);
    }

    /**
     * @deprecated as of UDK 2.0
     */
    public Type(TypeClass typeClass, String typeName) throws com.sun.star.uno.Exception {
        if(typeName != null) {
            try {
                init(typeName);
            }
            catch(ClassNotFoundException classNotFoundException) {
                throw new Exception(classNotFoundException.toString());
            }
        }
        else
            init(typeClass);
    }

    /**
     * @deprecated as of UDK 2.0
     */
    public Type(TypeClass typeClass, String typeName, Class zClass) throws com.sun.star.uno.Exception {
        this(typeClass, typeName);
    }


    /**
     * @deprecated as of UDK 2.0
     */
    public Type(TypeClass typeClass, String typeName, String arrayTypeName, Class zClass) throws com.sun.star.uno.Exception {
        this(typeClass, typeName);
    }


    /**
     * Gives the the type description of this type.
     * <p>
     * @return the type description
     */
    public Object getTypeDescription() {
        return _typeDescription;
    }



    /**
     * Gets the IDL <code>TypeClass</code> of the type.
     * <p>
     * @deprecated as of UDK 2.0
     * <p>
     * @return  the <code>TypeClass</code>.
     */
    public TypeClass getTypeClass() {
        TypeClass typeClass = null;

        try {
            typeClass = (TypeClass)__getTypeClass.invoke(_typeDescription, null);
        }
        catch(IllegalAccessException illegalAccessException) {
        }
        catch(InvocationTargetException invocationTargetException) {
        }

        return typeClass;
    }

    /**
     * Gets the type name.
     * <p>
     * @deprecated as of UDK 2.0
     * <p>
     * @return  the type name.
     */
    public String getTypeName() {
        String typeName = null;

        try {
            typeName = (String)__getTypeName.invoke(_typeDescription, null);
        }
        catch(IllegalAccessException illegalAccessException) {
        }
        catch(InvocationTargetException invocationTargetException) {
        }

        return typeName;
    }


    /**
     * Gets the array type name.
     * <p>
     * @deprecated as of UDK 2.0
     * <p>
     * @return  the array type name.
     */
    public String getArrayTypeName() {
        String arrayTypeName = null;

        try {
            arrayTypeName = (String)__getArrayTypeName.invoke(_typeDescription, null);
        }
        catch(IllegalAccessException illegalAccessException) {
        }
        catch(InvocationTargetException invocationTargetException) {
        }

        return arrayTypeName;
    }

    /**
     * Gets the corresponding java class for the type.
     * <p>
     * @deprecated as of UDK 2.0
     * <p>
     * @return   the corresponding java class.
     */
    public Class getDescription() throws ClassNotFoundException {
        Class zClass = null;

        try {
            zClass =  (Class)__getZClass.invoke(_typeDescription, null);
        }
        catch(IllegalAccessException illegalAccessException) {
        }
        catch(InvocationTargetException invocationTargetException) {
        }

        return zClass;
    }




    /**
     * Compares two types.
     * <p>
     * @return    true, if the given type and this type are equal
     */
    public boolean equals(Object type) {
        boolean result = false;

        if(type != null) {
            result = _typeDescription == ((Type)type)._typeDescription;

            if(!result && _typeDescription != null)
                result = _typeDescription.equals(((Type)type)._typeDescription);
        }

        return result;
    }

    /**
     * Calculates the hash code.
     * <p>
     * @return    the hash code
     */
    public int hashCode() {
        return _typeDescription.hashCode();
    }

    /**
     * Constructs a descriptive <code>String</code> for the type.
     * <p>
     * @return   a descriptive <code>String</code>
     */
    public String toString() {
        return "Type<" + _typeDescription + ">";
    }
}

