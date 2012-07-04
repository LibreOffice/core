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

package org.openoffice.test.common;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * Pump data from input stream into a StringBuffer
 *
 */
public class StreamPump extends Thread {
    StringBuffer stringBuffer = null;
    InputStream inputStream = null;

    public StreamPump(StringBuffer stringBuffer, InputStream inputStream) {
        this.stringBuffer = stringBuffer;
        this.inputStream = inputStream;
    }

    public void run() {
        InputStreamReader reader = null;
        try {
            reader = new InputStreamReader(inputStream);
            char[] buf = new char[1024];
            int count = 0;
            while ((count = reader.read(buf)) != -1) {
                // If we need collect the output
                if (stringBuffer != null)
                    stringBuffer.append(buf, 0, count);
            }
        } catch (IOException e) {
            // ignore.
        } finally {
            if (reader != null)
                try {
                    reader.close();
                } catch (IOException e) {
                    // ignore
                }
        }
    }
}