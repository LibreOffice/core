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
package com.sun.star.comp.sdbc;

import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;

public class ReaderInputStream extends InputStream {
    private final Reader reader;
    private int nextByte = -1;

    public ReaderInputStream(Reader reader) {
        this.reader = reader;
    }

    @Override
    public void close() throws IOException {
        reader.close();
    }

    @Override
    public int read() throws IOException {
        if (nextByte >= 0) {
            int currentByte = nextByte;
            nextByte = -1;
            return currentByte;
        } else {
            int c = reader.read();
            if (c < 0) {
                return c;
            }
            nextByte = (byte) ((c >>> 8) & 0xff);
            return c & 0xff;
        }
    }


    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        if ((off < 0) || (len < 0) || (off + len > b.length)) {
            throw new IndexOutOfBoundsException();
        } else if (len == 0) {
            return 0;
        } else if (len == 1) {
            int next = read();
            if (next < 0) {
                return next;
            }
            b[off] = (byte) next;
            return 1;
        } else {
            int charCount = len / 2;
            char[] chars = new char[charCount];
            int charsRead = reader.read(chars);
            if (charsRead < 0) {
                return charsRead;
            }
            int byteLength = len & ~1;
            for (int i = 0; i < byteLength; i++) {
                b[off + i] = (byte)(chars[i/2] >>> (8*(i&1)));
            }
            return byteLength;
        }
    }
}
