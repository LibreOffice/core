/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MappingException.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:06:50 $
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
 * The mapping Exception.
 * The exception is replaced by the com.sun.star.lang.DisposedException.
 * @deprecated since UDK 3.0.2
 * <p>
 * @version     $Revision: 1.5 $ $ $Date: 2005-09-07 19:06:50 $
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.uno.IQueryInterface
 * @see         com.sun.star.uno.IBridge
 */
public class MappingException extends com.sun.star.uno.RuntimeException {
    /**
     * Contructs an empty <code>MappingException</code>.
     */
    public MappingException() {
        super();
    }

    /**
     * Contructs an <code>MappingException</code> with a detail message.
     * <p>
     * @param  message   the detail message.
     */
    public MappingException(String message) {
        super(message);
    }

    /**
     * Contructs an <code>MappingException</code> with a detail message
     * and a context.
     * <p>
     * @param  message   the detail message.
     * @param  context   the context.
     */
    public MappingException(String message, Object context) {
        super(message, context);
    }
}


