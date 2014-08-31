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

import java.io.IOException;

/**
 * A {@code DocumentDeserializer} represents a converter that converts
 * &quot;Device&quot; {@code Document} objects into the &quot;Office&quot;
 * {@code Document} format.
 *
 * <p>The {@code PluginFactory} {@link
 * org.openoffice.xmerge.DocumentDeserializerFactory#createDocumentDeserializer
 * createDocumentDeserializer} method creates a {@code DocumentDeserializer},
 * which may or may not implement {@code DocumentDeserializer2}.  When it is
 * constructed, a {@code ConvertData} object is passed in to be used as input.
 * </p>
 *
 * @see  org.openoffice.xmerge.PluginFactory
 * @see  org.openoffice.xmerge.DocumentDeserializerFactory
 */
public interface DocumentDeserializer2 extends DocumentSerializer {

    /**
     * Convert the data passed into the {@code DocumentDeserializer2}
     * constructor into the &quot;Office&quot; {@code Document} format.
     *
     * <p>The URL's passed may be used to resolve links and to choose the name
     * of the output office document.</p>
     *
     * <p>This method may or may not be thread-safe.  It is expected that the
     * user code does not call this method in more than one thread.  And for
     * most cases, this method is only done once.</p>
     *
     * @return  The resulting {@code Document} object from conversion.
     *
     * @param   deviceURL         URL of the device document (may be null if unknown)
     * @param   officeURL         URL of the office document (may be null if unknown)
     *
     * @throws  ConvertException  If any Convert error occurs.
     * @throws  IOException       If any I/O error occurs.
     */
    Document deserialize(String deviceURL, String officeURL) throws
        ConvertException, IOException;
}