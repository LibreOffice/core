/*************************************************************************
 *
 *  $RCSfile: ITypeDescription.java,v $
 *
 *  $Revision: 1.2 $
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
 * The <code>ITypeDescription</code> allows to examine a type
 * in detail (e.g. it is used for marshaling/unmarshaling).
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2003-03-26 15:44:54 $
 * @author      Kay Ramme
 * @since       UDK3.0
 */
public interface ITypeDescription {
    /**
     * Gets the <code>ITypeDescription</code> of the
     * super, if it exists.
     * <p>
     * @return  the <code>ITypeDescription</code>.
     */
    ITypeDescription getSuperType();

    /**
     * Gets the <code>IMethodDescription</code> for every
     * method, if this type is an interface. Otherwise
     * returns <code>null</code>.
     * <p>
     * @return  the <code>IMethodDescription[]</code>.
     */
    IMethodDescription []getMethodDescriptions();

    /**
     * Gets the <code>IMethodDescription</code> for the
     * method with index methodId, if it exists, otherwise
     * returns <code>null</code>.
     * <p>
     * @return  the <code>IMethodDescription</code>.
     */
    IMethodDescription getMethodDescription(int methodId);

    /**
     * Gets the <code>IMethodDescription</code> for the
     * method with the name <code>name</code>, if it exists,
     * otherwise returns <code>null</code>.
     * <p>
     * @return  the <code>IMethodDescription</code>.
     */
    IMethodDescription getMethodDescription(String name);

    /**
     * Gets the <code>IFieldDescription</code> for every
     * field, if this type is an interface. Otherwise
     * returns <code>null</code>.
     * <p>
     * @return  the <code>IFieldDescription[]</code>.
     */
      IFieldDescription []getFieldDescriptions();

    /**
     * Gets the <code>IFieldDescription</code> for the
     * field with the name <code>name</code>, if it exists,
     * otherwise returns <code>null</code>.
     * <p>
     * @return  the <code>IFieldDescription</code>.
     */
      IFieldDescription getFieldDescription(String name);

    /**
     * Gets the IDL <code>TypeClass</code> of the type.
     * <p>
     * @return  the <code>TypeClass</code>.
     */
    TypeClass getTypeClass();

    /**
     * Gets the component <code>ITypeDescription</code> if
     * this is an array type, otherwise returns <code>null</code>.
     * <p>
     * @return the <code>ITypeDescription</code>
     */
    ITypeDescription getComponentType();

    /**
     * Gets the (UNO) type name.
     *
     * <p>The following table lists how UNO types map to type names:</p>
     * <table>
     *   <thead>
     *     <tr><th>UNO type</th><th>type name</th></tr>
     *   </thead>
     *   <tbody>
     *     <tr><td>VOID</td><td><code>"void"</code></td></tr>
     *     <tr><td>BOOLEAN</td><td><code>"boolean"</code></td></tr>
     *     <tr><td>CHAR</td><td><code>"char"</code></td></tr>
     *     <tr><td>BYTE</td><td><code>"byte"</code></td></tr>
     *     <tr><td>SHORT</td><td><code>"short"</code></td></tr>
     *     <tr>
     *       <td>UNSIGNED SHORT</td><td><code>"unsigned short"</code></td>
     *     </tr>
     *     <tr><td>LONG</td><td><code>"long"</code></td></tr>
     *     <tr><td>UNSIGNED LONG</td><td><code>"unsigned long"</code></td></tr>
     *     <tr><td>HYPER</td><td></td><code>"hyper"</code></tr>
     *     <tr>
     *       <td>UNSIGNED HYPER</td><td></td><code>"unsigned hyper"</code>
     *     </tr>
     *     <tr><td>FLOAT</td><td></td><code>"float"</code></tr>
     *     <tr><td>DOUBLE</td><td></td><code>"double"</code></tr>
     *     <tr><td>STRING</td><td></td><code>"string"</code></tr>
     *     <tr><td>TYPE</td><td></td><code>"type"</code></tr>
     *     <tr><td>ANY</td><td></td><code>"any"</code></tr>
     *     <tr>
     *       <td>sequence type of base type <var>T</var></td>
     *       <td><code>"[]"</code> followed by type name for <var>T</var></td>
     *     </tr>
     *     <tr>
     *       <td>enum type named <var>N</var></td>
     *       <td><var>N</var> (see below)</td>
     *     </tr>
     *     <tr>
     *       <td>struct type named <var>N</var></td>
     *       <td><var>N</var> (see below)</td>
     *     </tr>
     *     <tr>
     *       <td>exception type named <var>N</var>
     *       </td><td><var>N</var> (see below)</td>
     *     </tr>
     *     <tr>
     *       <td>interface type named <var>N</var></td>
     *       <td><var>N</var> (see below)</td>
     *     </tr>
     *   <tbody>
     * </table>
     * <p>For a UNO type named <var>N</var>, consisting of a sequence of module
     * names <var>M<sub>1</sub></var>, ..., <var>M<sub>n</sub></var> followed by
     * a simple name <var>S</var>, the corresponding type name consists of the
     * same sequence of module names and simple name, with <code>"."</code>
     * seperating the individual elements.</p>
     *
     * @return the type name.
     */
    String getTypeName();

    /**
     * Gets the (Java) array type name.
     *
     * <p>The array type name is defined to be the Java class name (as returned
     * by <code>Class.forName</code>) of the Java array class that corresponds
     * to the UNO sequence type with this type (the UNO type represented by this
     * <code>ITypeDescription</code> instance) as base type.  For an
     * <code>ITypeDescription</code> instance representing the UNO type VOID,
     * the array type name is defined to be
     * <code>"[Ljava.lang.Void;"</code>.</p>
     *
     * @return the array type name.
     */
    String getArrayTypeName();

    /**
     * Gets the corresponding java class for the type.
     * <p>
     * @return   the corresponding java class.
     */
    Class getZClass();
}
