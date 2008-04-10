/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DynamicClassLoader.java,v $
 * $Revision: 1.5 $
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

package lib ;

import java.lang.reflect.Constructor;

/**
 * @deprecated: moved to util package.
 */
public class DynamicClassLoader {

    /**
    * This method returns a class created by it's name
    * created by call to <code>Class.forName()</code>.<p>
    * This method must be overloaded if another loading
    * policy is required for Component and Interface
    * testing classes.
    */
    public static Class forName(String className)
        throws ClassNotFoundException {

        return Class.forName(className) ;
    }

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

        public Object getInstance(String className, Object[] ctorArgs)
                                            throws IllegalArgumentException {
            try {
                Class cls = DynamicClassLoader.forName(className);
                Class[] ctorType = new Class[ctorArgs.length];
                for(int i=0; i<ctorType.length; i++) {
                    ctorType[i] = ctorArgs[i].getClass();
                }
                Constructor ctor = cls.getConstructor(ctorType);
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
