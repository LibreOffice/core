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



package org.openoffice.xmerge;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

/**
 *  <p>A <code>Document</code> represents any <code>Document</code>
 *  to be converted and the resulting <code>Document</code> from any
 *  conversion.</p>
 *
 *  <p>It is created by the <code>PluginFactory</code> object's {@link
 *  org.openoffice.xmerge.PluginFactory#createOfficeDocument
 *  createOfficeDocument} method or the {@link
 *  org.openoffice.xmerge.PluginFactory#createDeviceDocument
 *  createDeviceDocument} method.</p>
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 */
public interface Document {

    /**
     *  <p>Writes out the <code>Document</code> content to the specified
     *  <code>OutputStream</code>.</p>
     *
     *  <p>This method may not be thread-safe.
     *  Implementations may or may not synchronize this
     *  method.  User code (i.e. caller) must make sure that
     *  calls to this method are thread-safe.</p>
     *
     *  @param  os  <code>OutputStream</code> to write out the
     *              <code>Document</code> content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException;


    /**
     *  <p>Reads the content from the <code>InputStream</code> into
     *  the <code>Document</code>.</p>
     *
     *  <p>This method may not be thread-safe.
     *  Implementations may or may not synchronize this
     *  method.  User code (i.e. caller) must make sure that
     *  calls to this method are thread-safe.</p>
     *
     *  @param  is  <code>InputStream</code> to read in the
     *              <code>Document</code> content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
     public void read(InputStream is) throws IOException;


    /**
     *  Returns the <code>Document</code> name with no file extension.
     *
     *  @return  The <code>Document</code> name with no file extension.
     */
    public String getName();


    /**
     *  Returns the <code>Document</code> name with file extension.
     *
     *  @return  The <code>Document</code> name with file extension.
     */
    public String getFileName();
}

