/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JobDefinitionException.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:22:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
