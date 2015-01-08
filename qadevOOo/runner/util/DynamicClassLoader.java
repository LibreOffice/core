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

package util;

import java.lang.reflect.Constructor;

public class DynamicClassLoader {

    /**
    * This method returns a class created by its name
    * created by call to <code>Class.forName()</code>.<p>
    * This method must be overridden if another loading
    * policy is required for Component and Interface
    * testing classes.
    * @param className The name of the class to create.
    * @return The created class.
    */
    private static Class<?> forName(String className)
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
            Class<?> cls = DynamicClassLoader.forName(className);
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
     * given calss types is used and the instance is created using the arguments
     * for the constructor.
     * @param className The class to instantiate.
     * @param ctorClassTypes The class types matching to the constructor.
     * @param ctorArgs Arguments for the constructor.
     * @return The instance of the class.
     */
    public Object getInstance(String className, Class<?>[]ctorClassTypes, Object[] ctorArgs)
                                        throws IllegalArgumentException {
        try {
            Class<?> cls = DynamicClassLoader.forName(className);
            Constructor<?> ctor = cls.getConstructor(ctorClassTypes);
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
