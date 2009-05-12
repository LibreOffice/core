/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BootstrapException.java,v $
 * $Revision: 1.5 $
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
