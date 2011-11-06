/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.lib.uno.environments.remote;

/**
 * An abstraction for giving back a reply for a request.
 *
 * @version $Revision: 1.6 $ $ $Date: 2008-04-11 11:19:43 $
 * @author Kay Ramme
 * @see com.sun.star.uno.IQueryInterface
 */
public interface IReceiver {
    /**
     * Send back a reply for a request.
     *
     * @param exception <CODE>true</CODE> if an exception (instead of a normal
     *     result) is sent back.
     * @param threadId the thread ID of the request.
     * @param result the result of executing the request, or an exception thrown
     *     while executing the request.
     */
    void sendReply(boolean exception, ThreadId threadId, Object result);
}
