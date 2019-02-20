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



package com.sun.star.beans;

import java.awt.Container;

import com.sun.star.lang.XComponent;
import com.sun.star.uno.XComponentContext;

/**
 * This abstract class represents a connection to the office
 * application.
 * @deprecated
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
     * Retrieves the UNO component context.
     * Establishes a connection if necessary and initializes the
     * UNO service manager if it has not already been initialized.
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
