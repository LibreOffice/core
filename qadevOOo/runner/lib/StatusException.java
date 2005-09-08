/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatusException.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:23:27 $
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