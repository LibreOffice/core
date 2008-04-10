/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StatusException.java,v $
 * $Revision: 1.3 $
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

package lib;

/**
 * StatusException is used to pass a Status object from a test code which is
 * terminated abnormaly. In many cases this is because of an exception thrown,
 * but that can also be any other event that hinders the test execution.
 */
public class StatusException extends RuntimeException {
    /**
     * Contains an exception if the StatusException was created with
     * StatusException(String, Throwable) constructor.
     */
    protected Throwable exceptionThrown;

    /**
     * The Status contained in the StatusException.
     */
    protected Status status;

    /**
     * Constructs a StatusException containing an exception Status.
     *
     * @param message the message of the StatusException
     * @param t the exception of the exception Status
     */
    public StatusException( String message, Throwable t ) {
        super( message );
        exceptionThrown = t;
        status = Status.exception( t );
    }

    /**
     * Creates a StatusException containing a Status.
     */
    public StatusException( Status st ) {
        super( st.getRunStateString() );
        status = st;
    }

    /**
     * @return an exception, if this represents an exception Status,
     * <tt>false</tt> otherwise.
     */
    public Throwable getThrownException() {
        return exceptionThrown;
    }

    /**
     * @return a status contained in the StatusException.
     */
    public Status getStatus() {
        return status;
    }
}