/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DynamicClassLoader.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:21:44 $
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
