/*************************************************************************
 *
 *  $RCSfile: ITypeDescription.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kr $ $Date: 2001-05-08 09:34:18 $
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
 * @version     $Revision: 1.1 $ $ $Date: 2001-05-08 09:34:18 $
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
     * Gets the type name.
     * <p>
     * @return  the type name.
     */
    String getTypeName();

    /**
     * Gets the array type name.
     * <p>
     * @return  the array type name.
     */
    String getArrayTypeName();

    /**
     * Gets the corresponding java class for the type.
     * <p>
     * @return   the corresponding java class.
     */
    Class getZClass();
}
