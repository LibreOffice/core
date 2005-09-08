/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IMemberDescription.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:23:10 $
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
 * The <code>IMemberDescription</code> is the base interface
 * for for the special subset of typedescriptions, which describe
 * members of IDL structs or interfeces.
 *
 * @deprecated This interface does not cover all the features supported by the
 * corresponding (unpublished) implementation.  But no client code should need
 * to access this functionality, anyway.
 */
public interface IMemberDescription {
    /**
     * Gives the name of this member.
     * <p>
     * @return  the name
     */
    String getName();

    /**
     * Indicates if this member is unsigned. (Not useful for IMethodDescription).
     * <p>
     * @return  the unsigned state
     */
    boolean isUnsigned();

    /**
     * Indicates if this member is an any.
     * <p>
     * @return  the any state
     */
    boolean isAny();

    /**
     * Indicates if this member is an interface.
     * <p>
     * @return  the interface state
     */
    boolean isInterface();

    /**
     * Gives the relative index of this member in the declaring
     * interface or struct (including superclasses).
     * <p>
     * @return  the relative index of this member
     */
    int getIndex();
}
