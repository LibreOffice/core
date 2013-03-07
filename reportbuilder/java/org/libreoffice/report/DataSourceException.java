/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report;

/**
 * A general exception to indicate that there was an error while accessing the
 * datasource.
 *
 */
public class DataSourceException extends Exception
{

    /**
     * Constructs a new exception with <code>null</code> as its detail message.
     * The cause is not initialized, and may subsequently be initialized by a call
     * to {@link #initCause}.
     */
    public DataSourceException()
    {
        super();
    }

    /**
     * Constructs a new exception with the specified detail message.  The cause is
     * not initialized, and may subsequently be initialized by a call to {@link
     * #initCause}.
     *
     * @param message the detail message. The detail message is saved for later
     *                retrieval by the {@link #getMessage()} method.
     */
    public DataSourceException(String message)
    {
        super(message);
    }

    /**
     * Constructs a new exception with the specified detail message and cause.
     * <p>Note that the detail message associated with <code>cause</code> is
     * <i>not</i> automatically incorporated in this exception's detail message.
     *
     * @param message the detail message (which is saved for later retrieval by
     *                the {@link #getMessage()} method).
     * @param cause   the cause (which is saved for later retrieval by the {@link
     *                #getCause()} method).  (A <tt>null</tt> value is permitted,
     *                and indicates that the cause is nonexistent or unknown.)
     * @since 1.4
     */
    public DataSourceException(String message, Throwable cause)
    {
        super(message, cause);
    }

    /**
     * Constructs a new exception with the specified cause and a detail message of
     * <tt>(cause==null ? null : cause.toString())</tt> (which typically contains
     * the class and detail message of <tt>cause</tt>). This constructor is useful
     * for exceptions that are little more than wrappers for other throwables (for
     * example, {@link PrivilegedActionException}).
     *
     * @param cause the cause (which is saved for later retrieval by the {@link
     *              #getCause()} method).  (A <tt>null</tt> value is permitted,
     *              and indicates that the cause is nonexistent or unknown.)
     * @since 1.4
     */
    public DataSourceException(Throwable cause)
    {
        super(cause);
    }
}
