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

package com.sun.star.lib.uno.environments.remote;

/**
 * An abstraction for giving back a reply for a request.
 *
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
