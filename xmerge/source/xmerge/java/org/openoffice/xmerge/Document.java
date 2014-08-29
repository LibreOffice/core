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

package org.openoffice.xmerge;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

/**
 * A {@code Document} represents any {@code Document} to be converted and the
 * resulting {@code Document} from any conversion.
 *
 * <p>It is created by the {@code PluginFactory} object's {@link
 * org.openoffice.xmerge.PluginFactory#createOfficeDocument createOfficeDocument}
 * method or the {@link org.openoffice.xmerge.PluginFactory#createDeviceDocument
 * createDeviceDocument} method.</p>
 *
 * @see  org.openoffice.xmerge.PluginFactory
 */
public interface Document {

    /**
     * Writes out the {@code Document} content to the specified
     * {@code OutputStream}.
     *
     * <p>This method may not be thread-safe. Implementations may or may not
     * synchronize this method.  User code (i.e. caller) must make sure that
     * calls to this method are thread-safe.</p>
     *
     * @param   os  {@code OutputStream} to write out the {@code Document}
     *              content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    void write(OutputStream os) throws IOException;

    /**
     * Reads the content from the {@code InputStream} into the {@code Document}.
     *
     * <p>This method may not be thread-safe. Implementations may or may not
     * synchronize this method.  User code (i.e. caller) must make sure that
     * calls to this method are thread-safe.</p>
     *
     * @param   is  {@code InputStream} to read in the {@code Document} content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    void read(InputStream is) throws IOException;

    /**
     * Returns  the {@code Document} name with no file extension.
     *
     * @return  The {@code Document} name with no file extension.
     */
    String getName();

    /**
     * Returns  the {@code Document} name with file extension.
     *
     * @return  The {@code Document} name with file extension.
     */
    String getFileName();
}