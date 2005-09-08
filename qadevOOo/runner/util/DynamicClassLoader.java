/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DynamicClassLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:33:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
