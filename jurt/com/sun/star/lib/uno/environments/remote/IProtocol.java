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

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.lib.uno.typedesc.TypeDescription;
import java.io.IOException;

/**
 * An abstraction of remote bridge protocols.
 *
 * <p>A class implementing a given protocol <var>prot</var> must be named
 * <code>com.sun.star.lib.uno.protocols.<var>prot</var>.<var>prot</var></code>
 * and must have a public constructor that takes four arguments:  The first
 * argument of type <code>com.sun.star.uno.IBridge</code> must not be null.  The
 * second argument of type <code>String</code> represents any attributes; it may
 * be null if there are no attributes.  The third argument of type
 * <code>java.io.InputStream</code> must not be null.  The fourth argument of
 * type <code>java.io.OutputStream</code> must not be null.</p>
 */
public interface IProtocol {
    /**
     * Initializes the connection.
     *
     * <p>This method must be called exactly once, after the
     * <code>readMessage</code> loop has already been established.</p>
     */
    void init() throws IOException;

    void terminate();

    /**
     * Reads a request or reply message.
     *
     * <p>Access to this method from multiple threads must be properly
     * synchronized.</p>
     *
     * @return a non-null message; if the input stream is exhausted, a
     *     <code>java.io.IOException</code> is thrown instead
     */
    Message readMessage() throws IOException;

    /**
     * Writes a request message.
     *
     * @param oid a non-null OID
     * @param type a non-null UNO type
     * @param function a non-null function (the name of a UNO interface method
     *     or attribute compatible with the given <code>type</code>, or either
     *     <code>"queryInterface"</code> or <code>"release"</code>)
     * @param threadId a non-null TID
     * @param arguments a list of UNO arguments compatible with the given
     *     <code>type</code> and <code>function</code>; may be null to represent
     *     an empty list
     * @return <code>true</code> if the request message is sent as a synchronous
     *     request
     */
    boolean writeRequest(
        String oid, TypeDescription type, String function, ThreadId tid,
        Object[] arguments)
        throws IOException;

    /**
     * Writes a reply message.
     *
     * @param exception <code>true</code> if the reply corresponds to a raised
     *     exception
     * @param tid a non-null TID
     * @param result if <code>exception</code> is <code>true</code>, a non-null
     *     UNO exception; otherwise, a UNO return value, which may be null to
     *     represent a <code>VOID</code> return value
     */
    void writeReply(boolean exception, ThreadId tid, Object result)
        throws IOException;
}
