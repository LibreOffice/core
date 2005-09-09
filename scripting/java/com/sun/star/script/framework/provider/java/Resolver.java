/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Resolver.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:03:16 $
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

package com.sun.star.script.framework.provider.java;

/**
 * The Resolver interface is an interface common to all classes which
 * want to implement an algorithm for obtaining a ScriptProxy object
 * for a particular ScriptDescriptor and Class
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 */

public interface Resolver {
    /**
     * Returns a ScriptProxy object for the given ScriptDescriptor and Class
     *
     * @param  sd  A script Descriptor
     * @param  c   A Class
     * @return     The ScriptProxy value
     */
    public ScriptProxy getProxy( ScriptDescriptor sd, Class c )
    throws NoSuchMethodException;
}

