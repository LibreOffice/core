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



package com.sun.star.uno;

/**
 * With a mapping objets can be mapped from one environment to another.
 *
 * <p>This interface exists for compatibility with the binary UNO API.</p>
 *
 * @see com.sun.star.uno.IBridge
 *
 * @deprecated As of UDK 3.2, this interface is deprecated, without offering a
 * replacement.
 */
public interface IMapping {
    /**
     * Maps an interface from one environment to another.
     *
     * @param object source object that is to be mapped
     * @param type description of the interface that is to be mapped
     * @return the object mapped to the destination environment
     */
    Object mapInterface(Object object, Type type);
}
