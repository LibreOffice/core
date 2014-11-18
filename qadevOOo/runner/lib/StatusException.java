/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package lib;

/**
 * StatusException is used to pass a Status object from a test code which is
 * terminated abnormaly. In many cases this is because of an exception thrown,
 * but that can also be any other event that hinders the test execution.
 */
public class StatusException extends RuntimeException {
    /**
     * The Status contained in the StatusException.
     */
    private final Status status;

    /**
     * Constructs a StatusException containing an exception Status.
     *
     * @param message the message of the StatusException
     * @param t the exception of the exception Status
     */
    public StatusException( String message, Throwable t ) {
        super( message, t );
        status = Status.exception( t );
    }

    /**
     * Constructs a StatusException containing an exception Status.
     */
    public StatusException( Throwable t, Status st ) {
        super( t );
        this.status = st;
    }

    /**
     * Creates a StatusException containing a Status.
     */
    public StatusException( Status st ) {
        super( st.getRunStateString() );
        status = st;
    }

    /**
     * @return a status contained in the StatusException.
     */
    public Status getStatus() {
        return status;
    }
}
