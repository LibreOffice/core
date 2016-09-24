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


package com.sun.star.uno;


/**
 * The UNO IDL type any is mapped to java type <code>java.lang.Object</code>.
 * <p>
 * In special cases it is necessary to have an explicit any to additionally transport
 * an exact type. For instance if you want to pass an object reference via
 * an interprocess connection using an any, you should use this class to add
 * an explicit interface type, so the remote counterpart doesn't need to invoke
 * a queryInterface).
 * </p>
 */
public class Any {
    /**
     * The type of the any.
     *
     * @see #getType
     */
    protected Type  _type;

    /**
     * The data of the any.
     *
     * @see #getObject
     */
    protected Object _object;

    public static final Any VOID = new Any(new Type("void", TypeClass.VOID),
                                           null);
        // do not use Type.VOID here to avoid circular dependencies between
        // static members of Any and Type

    /**
     * Constructs a new any.
     *
     * @param   zInterface  the type of the any.
     * @param   object      the data of the any.
     * @deprecated as of UDK 2.0
     */
    public Any(Class<?> zInterface, Object object) {
        this(new Type(zInterface), object);
    }

    /**
     * Constructs a new any with a given type and value
     *
     * @param type the UNO type of the any.
     * @param object the value of the any.
     */
    public Any(Type type, Object object) {
        if (type.equals(Type.ANY)) {
            throw new IllegalArgumentException("Any cannot contain Any");
        }
        _type   = type;
        _object = object;
    }

    /**
     * Complete a UNO <code>ANY</code> (make sure it is wrapped up as an
     * <code>Any</code> instance).
     *
     * @param any a Java value representing a UNO <code>ANY</code> value.
     * @return a complete Java value (that is, an <code>Any</code> instance)
     * representing the same UNO <code>ANY</code> value as the given argument.
     * @since UDK 3.2.3
    */
    public static final Any complete(Object any) {
        return any instanceof Any
            ? (Any) any
            : new Any(
                new Type(any == null ? XInterface.class : any.getClass()), any);
    }

    /**
     * Gets the type of the value within the any.
     *
     * @return the type of the value within the any.
     */
    public Type getType() {
        return _type;
    }

    /**
     * Gets the value within the any.
     *
     * @return gets the value within the any.
     */
    public Object getObject() {
        return _object;
    }

    /**
     * Indicates whether some other object is equal to this one.
     *
     * @param obj the reference object with which to compare.
     * @return <code>true</code> if this object is the same as the obj argument;
     * <code>false</code> otherwise.
     * @see java.lang.Object#equals
     */
    @Override
    public boolean equals(Object obj) {
        return obj instanceof Any && _type.equals(((Any) obj)._type)
            && (_object == null
                ? ((Any) obj)._object == null
                : _object.equals(((Any) obj)._object));
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return a hash code value for this object.
     * @see java.lang.Object#hashCode
     */
    @Override
    public int hashCode() {
        return _type.hashCode() * 13
            + (_object == null ? 0 : _object.hashCode());
    }

    /**
     * Returns a string representation of the object.
     *
     * @return a string representation of the object.
     * @see java.lang.Object#toString
     */
    @Override
    public String toString() {
        return "Any[" + _type + ", " + _object + "]";
    }
}
