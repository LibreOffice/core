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

package org.openoffice.xmerge.util;

import java.util.ResourceBundle;

/**
 * Provides a singleton resource class for converter messages.
 *
 * <p>By default, this class will search for a {@code ResourceBundle} class file
 * or properties file based on the default locale.</p>
 *
 * <p>A properties file resources.properties will be provided.</p>
 *
 * <p>Note that if the resource bundle object is not loaded, the construction of
 * the singleton object will throw a {@code MissingResourceException}, which is
 * a {@code RuntimeException}, thus I opted to not explicitly declare it.  If it
 * does throw {@code MissingResourceException}, it may be due to a packaging
 * problem.</p>
 */
public final class Resources
{
    private final ResourceBundle rb;
    private static Resources instance = null;

    /**
     * This method returns the singleton instance of this class.
     *
     * @return  The singleton {@code Resources} instance.
     */
    public synchronized static Resources getInstance() {
        if (instance == null) {
            instance = new Resources();
        }

        return instance;
    }

    /**
     * Default constructor is only accessible within this class.
     *
     * <p>Load the resource bundle that contains the resource {@code String}
     * values.</p>
     */
    private Resources() {
        rb = ResourceBundle.getBundle("org.openoffice.xmerge.util.resources");
    }

    /**
     * This method returns the corresponding {@code String} given the key.
     *
     * @param   key      Key string for getting the message {@code String}.
     * @return  Message  {@code String} corresponding to the key.
     */
    public String getString(String key) {
        return rb.getString(key);
    }
}