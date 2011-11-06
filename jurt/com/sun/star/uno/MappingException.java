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
 * The mapping Exception.
 * The exception is replaced by the com.sun.star.lang.DisposedException.
 * @deprecated since UDK 3.0.2
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2008-04-11 11:28:00 $
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.uno.IQueryInterface
 * @see         com.sun.star.uno.IBridge
 */
public class MappingException extends com.sun.star.uno.RuntimeException {
    /**
     * Contructs an empty <code>MappingException</code>.
     */
    public MappingException() {
        super();
    }

    /**
     * Contructs an <code>MappingException</code> with a detail message.
     * <p>
     * @param  message   the detail message.
     */
    public MappingException(String message) {
        super(message);
    }

    /**
     * Contructs an <code>MappingException</code> with a detail message
     * and a context.
     * <p>
     * @param  message   the detail message.
     * @param  context   the context.
     */
    public MappingException(String message, Object context) {
        super(message, context);
    }
}


