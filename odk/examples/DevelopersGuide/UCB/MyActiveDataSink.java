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



import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;

/**
 * XActiveDataSink interface implementation. Makes it possible to read
 * the corresponding object from an input stream.
 */
public class MyActiveDataSink implements XActiveDataSink {

    /**
     * Member properties
     */
    XInputStream m_aStream = null;

    /**
     * Constructor
     */
    public MyActiveDataSink() {
        super();
    }

    /**
     *   Plugs the input stream.
     *
     *@param  XInputStream
     */
    public void setInputStream( XInputStream aStream ) {
        m_aStream = aStream;
    }

    /**
     *  Get the plugged stream.
     *
     *@return  XInputStream  The plugged stream
     */
    public XInputStream getInputStream() {
        return m_aStream;
    }
}
