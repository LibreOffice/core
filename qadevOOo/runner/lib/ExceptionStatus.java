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



package lib;

/**
 * The class implements Status behaviour for exception runstate Status objects.
 */
class ExceptionStatus extends Status {

    /**
     * Creates an instance of Status object with EXCEPTION runstate.
     *
     * @param t the exception an activity terminated with.
     */
    ExceptionStatus( Throwable t ) {
        super(EXCEPTION, FAILED);
        String message = t.getMessage();
        if (message != null)
            runStateString = message;
        else
            runStateString = t.toString();
    }
}