/*************************************************************************
 *
 *  $RCSfile: Any.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 15:44:54 $
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


/**
 * The UNO IDL type any is mapped to java type <code>java.lang.Object</code>.
 * <p>
 * In special cases it is necessary to have an explicit any to additionally transport
 * an exact type. For instance if you want to pass an object reference via
 * an interprocess connection using an any, you should use this class to add
 * an explicit interface type, so the remote counterpart doesn't need to invoke
 * a queryInterface).
 * <p>
 * @version     $Revision: 1.5 $ $ $Date: 2003-03-26 15:44:54 $
 * @since       UDK1.0
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
        _type   = new Type(zInterface);
        _object = object;
    }

    /** Constructs a new any with a given type and value
        @param type the UNO type of the any.
        @param object the value of the any.
     */
    public Any(Type type, Object object) {
        _type   = type;
        _object = object;
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

    // @see java.lang.Object#toString
    public String toString() {
        return "Any[" + _type + ", " + _object + "]";
    }
}
