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
 *  <p>A <code>ConverterCapabilities</code> object is used by
 *  <code>DocumentMerger</code> implementations.  The
 *  <code>ConverterCapabilities</code> indicates which
 *  &quot;Office&quot; XML tags are supported by the
 *  &quot;Device&quot; format.</p>
 *
 *  @see  org.openoffice.xmerge.PluginFactory
 *  @see  org.openoffice.xmerge.DocumentMerger
 */
public interface ConverterCapabilities {


    /**
     *  Test to see if the device document format supports the
     *  tag in question.
     *
     *  @param  tag  The tag to check.
     *
     *  @return  true if the device format supports the
     *           tag, false otherwise.
     */
    boolean canConvertTag(String tag);


    /**
     *  Test to see if the device document format supports the
     *  tag attribute in question.
     *
     *  @param  tag        The tag to check.
     *  @param  attribute  The tag attribute to check.
     *
     *  @return  true if the device format supports the
     *          attribute, false otherwise.
     */
    boolean canConvertAttribute(String tag, String attribute);
}

