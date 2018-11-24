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

/**
 *  <p>A <code>DocumentSerializer</code> object is used to convert
 *  from the &quot;Office&quot; <code>Document</code> format to the
 *  &quot;Device&quot; <code>Document</code> format.</p>
 *
 *  <p>All plug-in implementations of the <code>PluginFactory</code>
 *  interface that also support serialization must also
 *  implement this interface.</p>
 *
 *  @see  PluginFactory
 *  @see  DocumentSerializer
 */
public interface DocumentSerializerFactory {

    /**
     *  <p>The <code>DocumentSerializer</code> is used to convert
     *  from the &quot;Office&quot; <code>Document</code> format
     *  to the &quot;Device&quot; <code>Document</code> format.</p>
     *
     *  The <code>ConvertData</code> object is passed along to the
     *  created <code>DocumentSerializer</code> via its constructor.
     *  The <code>ConvertData</code> is read and converted when the
     *  the <code>DocumentSerializer</code> object's
     *  <code>serialize</code> method is called.
     *
     *  @param  doc  <code>Document</code> object that the created
     *               <code>DocumentSerializer</code> object uses
     *               as input.
     *
     *  @return  A <code>DocumentSerializer</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc);
}

