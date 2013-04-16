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

package com.sun.star.comp.beans;

import java.awt.Container;

import com.sun.star.lang.XComponent;
import com.sun.star.uno.XComponentContext;

/**
 * This abstract class represents a connection to the office
 * application.

    @deprecated
 */
public interface OfficeConnection
    extends XComponent
{
    /**
     * Sets a connection URL.
     *
     * @param url This is UNO URL which describes the type of a connection.
     */
    void setUnoUrl(String url)
        throws java.net.MalformedURLException;

    /**
     * Sets an AWT container catory.
     *
     * @param containerFactory This is a application provided AWT container
     *  factory.
     */
    void setContainerFactory(ContainerFactory containerFactory);

    /**
     * Retrives the UNO component context.
     * Establishes a connection if necessary and initialises the
     * UNO service manager if it has not already been initialised.
     *
     * @return The office UNO component context.
     */
    XComponentContext getComponentContext();

    /**
     * Creates an office window.
     * The window is either a sub-class of java.awt.Canvas (local) or
     * java.awt.Container (RVP).
     *
     * This method does not add the office window to its container.
     *
     * @param container This is an AWT container.
     * @return The office window instance.
     */
    OfficeWindow createOfficeWindow(Container container);
}
