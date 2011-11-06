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



package com.sun.star.comp.beans;

import com.sun.star.connection.XConnection;

/* Connection to a locally running OOo instance.
 *
 * @deprecated.
 */
/* package */ class NativeConnection
    implements XConnection
{
    public native void connect(NativeService aNativeService)
        throws com.sun.star.io.IOException;

    public native int read(/*OUT*/byte[][] aReadBytes, /*IN*/int nBytesToRead)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void write(/*IN*/byte[] aData)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void flush()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void close()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public synchronized String getDescription()
        throws com.sun.star.uno.RuntimeException
    {
        return Description;
    }

    private long    NativeHandle;
    private String  Description;
}
