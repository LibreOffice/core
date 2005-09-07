/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BootstrapException.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:35:39 $
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

package com.sun.star.comp.helper;

/**
 * BootstrapException is a checked exception that wraps an exception
 * thrown by the original target.
 *
 * @since UDK 3.1.0
 */
public class BootstrapException extends java.lang.Exception {

    /**
     * This field holds the target exception.
     */
    private Exception m_target = null;

    /**
     * Constructs a <code>BootstrapException</code> with <code>null</code> as
     * the target exception.
     */
    public BootstrapException() {
        super();
    }

    /**
     * Constructs a <code>BootstrapException</code> with the specified
     * detail message.
     *
     * @param  message   the detail message
     */
    public BootstrapException( String message ) {
        super( message );
    }

    /**
     * Constructs a <code>BootstrapException</code> with the specified
     * detail message and a target exception.
     *
     * @param  message   the detail message
     * @param  target    the target exception
     */
    public BootstrapException( String message, Exception target ) {
        super( message );
        m_target = target;
    }

    /**
     * Constructs a <code>BootstrapException</code> with a target exception.
     *
     * @param  target    the target exception
     */
    public BootstrapException( Exception target ) {
        super();
        m_target = target;
    }

    /**
     * Get the thrown target exception.
     *
     * @return the target exception
     */
    public Exception getTargetException() {
        return m_target;
    }
}
