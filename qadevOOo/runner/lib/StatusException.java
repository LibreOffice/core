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
 * StatusException is used to pass a Status object from a test code which is
 * terminated abnormaly. In many cases this is because of an exception thrown,
 * but that can also be any other event that hinders the test execution.
 */
public class StatusException extends RuntimeException {
    /**
     * Contains an exception if the StatusException was created with
     * StatusException(String, Throwable) constructor.
     */
    protected Throwable exceptionThrown;

    /**
     * The Status contained in the StatusException.
     */
    protected Status status;

    /**
     * Constructs a StatusException containing an exception Status.
     *
     * @param message the message of the StatusException
     * @param t the exception of the exception Status
     */
    public StatusException( String message, Throwable t ) {
        super( message );
        exceptionThrown = t;
        status = Status.exception( t );
    }

    /**
     * Creates a StatusException containing a Status.
     */
    public StatusException( Status st ) {
        super( st.getRunStateString() );
        status = st;
    }

    /**
     * @return an exception, if this represents an exception Status,
     * <tt>false</tt> otherwise.
     */
    public Throwable getThrownException() {
        return exceptionThrown;
    }

    /**
     * @return a status contained in the StatusException.
     */
    public Status getStatus() {
        return status;
    }
}