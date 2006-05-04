/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Any.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-04 08:05:08 $
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
 * @version     $Revision: 1.10 $ $ $Date: 2006-05-04 08:05:08 $
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
