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

import java.io.IOException;

/**
 *  <p>A <code>DocumentDeserializer</code> represents a converter that
 *  converts &quot;Device&quot; <code>Document</code> objects into the
 *  &quot;Office&quot; <code>Document</code> format.
 *
 *  <p>The <code>DocumentDeserializer</code> object is created by
 *  the <code>PluginFactory</code> {@link
 *  org.openoffice.xmerge.DocumentDeserializerFactory#createDocumentDeserializer
 *  createDocumentDeserializer} method.  When it is constructed, a
 *  <code>ConvertData</code> object is passed in to be used as input.
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentDeserializerFactory
 */
public interface DocumentDeserializer {

    /**
     *  <p>Convert the data passed into the <code>DocumentDeserializer</code>
     *  constructor into the &quot;Office&quot; <code>Document</code>
     *  format.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  The resulting <code>Document</code> object from conversion.
     *
     *  @throws  ConvertException  If any Convert error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public Document deserialize() throws ConvertException, IOException;
}

