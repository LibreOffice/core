/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.uno;


/**
 * The mapping Exception.
 *
 * <p>The exception is replaced by the com.sun.star.lang.DisposedException.</p>
 *
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.uno.IQueryInterface
 * @see         com.sun.star.uno.IBridge
 *
 * @deprecated since UDK 3.0.2
 */
public class MappingException extends com.sun.star.uno.RuntimeException {
    /**
     * Constructs an empty <code>MappingException</code>.
     */
    public MappingException() {
        super();
    }

    /**
     * Constructs an <code>MappingException</code> with a detail message.
     *
     * @param  message   the detail message.
     */
    public MappingException(String message) {
        super(message);
    }

    /**
     * Constructs an <code>MappingException</code> with a detail message and a
     * context.
     *
     * @param  message   the detail message.
     * @param  context   the context.
     */
    public MappingException(String message, Object context) {
        super(message, context);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
