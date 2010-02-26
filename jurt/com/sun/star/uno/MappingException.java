/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.uno;


/**
 * The mapping Exception.
 * The exception is replaced by the com.sun.star.lang.DisposedException.
 * @deprecated since UDK 3.0.2
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2008-04-11 11:28:00 $
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


