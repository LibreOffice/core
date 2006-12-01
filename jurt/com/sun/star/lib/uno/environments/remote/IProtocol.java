/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IProtocol.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:50:44 $
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
