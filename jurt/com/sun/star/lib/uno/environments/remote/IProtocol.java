/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IProtocol.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:59:27 $
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
import java.io.DataOutput;
import java.io.IOException;
import java.io.InputStream;

/**
 * This interface provides an abstraction for protocols
 * for remote bridges.
 */
public interface IProtocol {
    /**
     * Gets the name of the protocol.
     * <p>
     * @result  the name of the protocol
     */
    String getName();

    /**
     * Reads a job from the given stream.
     * <p>
     * @return  thread read job.
     * @see     com.sun.star.lib.uno.environments.remote.Job
     */
    IMessage readMessage(InputStream inputStream) throws IOException;

    void writeRequest(
        String oid, TypeDescription zInterface, String operation,
        ThreadId threadId, Object[] params, Boolean[] synchron,
        Boolean[] mustReply);

    public void writeReply(boolean exception, ThreadId threadId, Object result);

    public void flush(DataOutput dataOutput) throws IOException;
}
