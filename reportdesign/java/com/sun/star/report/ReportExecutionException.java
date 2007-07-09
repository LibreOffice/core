/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportExecutionException.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:03 $
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

public class ReportExecutionException extends Exception
{
  /**
   * Constructs a new exception with <code>null</code> as its detail message. The cause is
   * not initialized, and may subsequently be initialized by a call to {@link
   * #initCause}.
   */
  public ReportExecutionException ()
  {
  }

  /**
   * Constructs a new exception with the specified cause and a detail message of
   * <tt>(cause==null ? null : cause.toString())</tt> (which typically contains the class
   * and detail message of <tt>cause</tt>). This constructor is useful for exceptions that
   * are little more than wrappers for other throwables (for example, {@link
   * java.security.PrivilegedActionException}).
   *
   * @param cause the cause (which is saved for later retrieval by the {@link #getCause()}
   *              method).  (A <tt>null</tt> value is permitted, and indicates that the
   *              cause is nonexistent or unknown.)
   * @since 1.4
   */
  public ReportExecutionException (Throwable cause)
  {
    super(cause);
  }

  /**
   * Constructs a new exception with the specified detail message.  The cause is not
   * initialized, and may subsequently be initialized by a call to {@link #initCause}.
   *
   * @param message the detail message. The detail message is saved for later retrieval by
   *                the {@link #getMessage()} method.
   */
  public ReportExecutionException (String message)
  {
    super(message);
  }

  /**
   * Constructs a new exception with the specified detail message and cause.  <p>Note that
   * the detail message associated with <code>cause</code> is <i>not</i> automatically
   * incorporated in this exception's detail message.
   *
   * @param message the detail message (which is saved for later retrieval by the {@link
   *                #getMessage()} method).
   * @param cause   the cause (which is saved for later retrieval by the {@link
   *                #getCause()} method).  (A <tt>null</tt> value is permitted, and
   *                indicates that the cause is nonexistent or unknown.)
   * @since 1.4
   */
  public ReportExecutionException (String message, Throwable cause)
  {
    super(message, cause);
  }
}
