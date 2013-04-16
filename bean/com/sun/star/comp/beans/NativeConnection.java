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

package com.sun.star.comp.beans;

import com.sun.star.connection.XConnection;

/* Connection to a locally running OOo instance.
 *
 * @deprecated.
 */
class NativeConnection
    implements XConnection
{
    public native void connect(NativeService aNativeService)
        throws com.sun.star.io.IOException;

    public native int read(/*OUT*/ byte[][] aReadBytes, /*IN*/ int nBytesToRead)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void write(/*IN*/ byte[] aData)
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
