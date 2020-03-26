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

package com.sun.star.comp.helper;

/**
 * BootstrapException is a checked exception that wraps an exception
 * thrown by the original target.
 *
 * @since UDK 3.1.0
 */
public class BootstrapException extends java.lang.Exception {

    /**
     * This field holds the target exception.
     */
    private Exception m_target = null;

    /**
     * Constructs a <code>BootstrapException</code> with <code>null</code> as
     * the target exception.
     */
    public BootstrapException() {
        super();
    }

    /**
     * Constructs a <code>BootstrapException</code> with the specified
     * detail message.
     *
     * @param  message   the detail message
     */
    public BootstrapException( String message ) {
        super( message );
    }

    /**
     * Constructs a <code>BootstrapException</code> with the specified
     * detail message and a target exception.
     *
     * @param  message   the detail message
     * @param  target    the target exception
     */
    public BootstrapException( String message, Exception target ) {
        super( message );
        m_target = target;
    }

    /**
     * Constructs a <code>BootstrapException</code> with a target exception.
     *
     * @param  target    the target exception
     */
    public BootstrapException( Exception target ) {
        super();
        m_target = target;
    }

    /**
     * Get the thrown target exception.
     *
     * @return the target exception
     */
    public Exception getTargetException() {
        return m_target;
    }
}
