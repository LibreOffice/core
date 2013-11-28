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

package com.sun.star.lib.uno.adapter;

import java.io.IOException;
import com.sun.star.io.XOutputStream;
import java.io.OutputStream;

/** The <code>OutputStreamToXOutputStreamAdapter</code> wraps
   a a UNO <code>XOutputStream</code> into a Java <code>OutputStream</code>
   object in a Java.  This allows users to access an <code>OutputStream</code>
   as if it were an <code>XOutputStream</code>.
 */
public class OutputStreamToXOutputStreamAdapter implements XOutputStream {

    /**
     *  Internal handle to the OutputStream
     */
    OutputStream iOut;

    /**
     *  Constructor.
     *
     *  @param  out  The <code>XOutputStream</code> to be
     *          accessed as an <code>OutputStream</code>.
     */
    public OutputStreamToXOutputStreamAdapter(OutputStream out) {
        iOut = out;
    }

    public void closeOutput() throws
            com.sun.star.io.IOException
    {
        try {
            iOut.close();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }
    }

    public void flush() throws
            com.sun.star.io.IOException
    {
        try {
            iOut.flush();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }
    }

    public void writeBytes(byte[] b) throws
            com.sun.star.io.IOException
    {

        try {
                iOut.write(b);
            } catch (IOException e) {
                throw new com.sun.star.io.IOException(e.toString());
            }
    }

    public void setMediaType(String s) {}

    public String getMediaType() { return ""; }
}
