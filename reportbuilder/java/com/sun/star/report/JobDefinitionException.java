/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JobDefinitionException.java,v $
 * $Revision: 1.4 $
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
package com.sun.star.report;

/**
 * Creation-Date: 05.02.2006, 17:36:33
 *
 * @author Thomas Morgner
 */
public class JobDefinitionException extends RuntimeException
{

    /**
     * Creates a StackableRuntimeException with no message and no parent.
     */
    public JobDefinitionException()
    {
    }

    /**
     * Creates an exception.
     *
     * @param message the exception message.
     */
    public JobDefinitionException(final String message)
    {
        super(message);
    }

    /**
     * Creates an exception.
     *
     * @param message the exception message.
     * @param ex      the parent exception.
     */
    public JobDefinitionException(final String message, final Exception ex)
    {
        super(message, ex);
    }
}
