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



package com.sun.star.lib.uno;

/**
 * Marker interface implemented by proxies for UNO objects.
 *
 * <p>Currently, this interface is used internally by
 * <code>com.sun.star.lib.uno.environments.java.java_environment</code> to
 * distinguish between proxies and local objects.  Any proxy object that shall
 * be registered at the <code>java_environment</code> must implement this marker
 * interface.</p>
 */
public interface Proxy {
}
