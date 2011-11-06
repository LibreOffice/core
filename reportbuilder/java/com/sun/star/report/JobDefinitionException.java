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


package com.sun.star.report;

/**
 * Creation-Date: 05.02.2006, 17:36:33
 *
 * @author Thomas Morgner
 */
public class JobDefinitionException extends RuntimeException
{

    /**
     * Creates a StackableRuntimeException with no message and no parent.
     */
    public JobDefinitionException()
    {
    }

    /**
     * Creates an exception.
     *
     * @param message the exception message.
     */
    public JobDefinitionException(final String message)
    {
        super(message);
    }

    /**
     * Creates an exception.
     *
     * @param message the exception message.
     * @param ex      the parent exception.
     */
    public JobDefinitionException(final String message, final Exception ex)
    {
        super(message, ex);
    }
}
