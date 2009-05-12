/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DynamicClassLoader.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package util;

import java.lang.reflect.Constructor;

public class DynamicClassLoader {

    /**
    * This method returns a class created by its name
    * created by call to <code>Class.forName()</code>.<p>
    * This method must be overloaded if another loading
    * policy is required for Component and Interface
    * testing classes.
    * @param className The name of the class to create.
    * @return The created class.
    */
    public static Class forName(String className)
        throws ClassNotFoundException {

        return Class.forName(className) ;
    }

    /**
     * Get an instance of a class. The empty constructor is used.
     * @param className The class to instantiate.
     * @return The instance of the class.
     */
    public Object getInstance(String className)
                                        throws IllegalArgumentException {
        try {
            Class cls = DynamicClassLoader.forName(className);
            return cls.newInstance();
        } catch ( ClassNotFoundException e ) {
            throw new IllegalArgumentException("Couldn't find " + className
                    + " " + e);
        } catch ( IllegalAccessException e ) {
            throw new IllegalArgumentException("Couldn't access " + className
                    + " " + e);
        } catch ( InstantiationException e ) {
            throw new IllegalArgumentException("Couldn't instantiate " +
                            className + " " + e);
        }
    }

    /**
     * Get an instance of a class. The constructor matching to the
     * arguments is used and the arguments are given to this constructor.
     * @param className The class to instantiate.
     * @param ctorArgs Arguments for the constructor.
     * @return The instance of the class.
     */
    public Object getInstance(String className, Object[] ctorArgs)
                                        throws IllegalArgumentException {
        Class[] ctorType = new Class[ctorArgs.length];
        for(int i=0; i<ctorType.length; i++) {
            ctorType[i] = ctorArgs[i].getClass();
        }
        return getInstance(className, ctorType, ctorArgs);

    }

    /**
     * Get an instance of a class. The constructor matching to the
     * given calss types is used and the instance is created using the arguments
     * for the constructor.
     * @param className The class to instantiate.
     * @param ctorClassTypes The class types matching to the constructor.
     * @param ctorArgs Arguments for the constructor.
     * @return The instance of the class.
     */
    public Object getInstance(String className, Class[]ctorClassTypes, Object[] ctorArgs)
                                        throws IllegalArgumentException {
        try {
            Class cls = DynamicClassLoader.forName(className);
            Constructor ctor = cls.getConstructor(ctorClassTypes);
            System.out.println("ctor: " + ctor.getName() +  "  " + ctor.getModifiers());

            return ctor.newInstance(ctorArgs);
        } catch ( ClassNotFoundException e ) {
            throw new IllegalArgumentException("Couldn't find " + className
                    + " " + e);
        } catch ( IllegalAccessException e ) {
            throw new IllegalArgumentException("Couldn't access " + className
                    + " " + e);
        } catch ( NoSuchMethodException e ) {
            throw new IllegalArgumentException("Couldn't find constructor for " + className
                    + " " + e);
        } catch ( java.lang.reflect.InvocationTargetException e ) {
            e.printStackTrace();
            throw new IllegalArgumentException("Couldn't invoke " +
                            className + " " + e);
        } catch ( InstantiationException e ) {
            throw new IllegalArgumentException("Couldn't instantiate " +
                            className + " " + e);
        }
    }
}
