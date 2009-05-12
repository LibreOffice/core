/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Any.java,v $
 * $Revision: 1.11 $
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


package com.sun.star.uno;


/**
 * The UNO IDL type any is mapped to java type <code>java.lang.Object</code>.
 * <p>
 * In special cases it is necessary to have an explicit any to additionally transport
 * an exact type. For instance if you want to pass an object reference via
 * an interprocess connection using an any, you should use this class to add
 * an explicit interface type, so the remote counterpart doesn't need to invoke
 * a queryInterface).
 * <p>
 * @version     $Revision: 1.11 $ $ $Date: 2008-04-11 11:11:43 $
 */
public class Any {
    /**
     * The type of the any.
     * <p>
     * @see #getType
     */
    protected Type  _type;

    /**
     * The data of the any.
     * <p>
     * @see #getObject
     */
    protected Object _object;

    public static final Any VOID = new Any(new Type("void", TypeClass.VOID),
                                           null);
        // do not use Type.VOID here to avoid circular dependencies between
        // static members of Any and Type

    /**
     * Constructs a new any.
     * <p>
     * @param   zInterface  the type of the any.
     * @param   object      the data of the any.
     * @deprecated as of UDK 2.0
     */
    public Any(Class zInterface, Object object) {
        this(new Type(zInterface), object);
    }

    /** Constructs a new any with a given type and value
        @param type the UNO type of the any.
        @param object the value of the any.
     */
    public Any(Type type, Object object) {
        if (type.equals(Type.ANY)) {
            throw new IllegalArgumentException("Any cannot contain Any");
        }
        _type   = type;
        _object = object;
    }

    /**
       Complete a UNO <code>ANY</code> (make sure it is wrapped up as an
       <code>Any</code> instance).

       @param any a Java value representing a UNO <code>ANY</code> value.

       @return a complete Java value (that is, an <code>Any</code> instance)
       representing the same UNO <code>ANY</code> value as the given argument.

       @since UDK 3.2.3
    */
    public static final Any complete(Object any) {
        return any instanceof Any
            ? (Any) any
            : new Any(
                new Type(any == null ? XInterface.class : any.getClass()), any);
    }

    /**
     * Gets the type of the value within the any.
     * <p>
     * @return   the type of the value within the any.
     */
    public Type getType() {
        return _type;
    }

    /**
     * Gets the value within the any.
     * <p>
     * @return   gets the value within the any.
     */
    public Object getObject() {
        return _object;
    }

    // @see java.lang.Object#equals
    public boolean equals(Object obj) {
        return obj instanceof Any && _type.equals(((Any) obj)._type)
            && (_object == null
                ? ((Any) obj)._object == null
                : _object.equals(((Any) obj)._object));
    }

    // @see java.lang.Object#hashCode
    public int hashCode() {
        return _type.hashCode() * 13
            + (_object == null ? 0 : _object.hashCode());
    }

    // @see java.lang.Object#toString
    public String toString() {
        return "Any[" + _type + ", " + _object + "]";
    }
}
