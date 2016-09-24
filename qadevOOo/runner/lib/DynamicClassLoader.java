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

package lib ;

import java.lang.reflect.Constructor;

/**
 * @deprecated: moved to util package.
 */
public class DynamicClassLoader {

    /**
    * This method returns a class created by its name
    * created by call to <code>Class.forName()</code>.<p>
    * This method must be overridden if another loading
    * policy is required for Component and Interface
    * testing classes.
    */
    public static Class<?> forName(String className)
        throws ClassNotFoundException {

        return Class.forName(className) ;
    }

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

        public Object getInstance(String className, Object[] ctorArgs)
                                            throws IllegalArgumentException {
            try {
                Class<?> cls = DynamicClassLoader.forName(className);
                Class<?>[] ctorType = new Class[ctorArgs.length];
                for(int i=0; i<ctorType.length; i++) {
                    ctorType[i] = ctorArgs[i].getClass();
                }
                Constructor<?> ctor = cls.getConstructor(ctorType);
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
                throw new IllegalArgumentException("Couldn't invoke " +
                                className + " " + e);
            } catch ( InstantiationException e ) {
                throw new IllegalArgumentException("Couldn't instantiate " +
                                className + " " + e);
            }
        }
}
