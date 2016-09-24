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
 * A {@code DocumentSerializer2} represents a converter that converts a
 * &quot;Office&quot; {@code Document} to a &quot;Device&quot; {@code Document}
 * format.
 *
 * <p>The {@code PluginFactory} {@link
 * org.openoffice.xmerge.DocumentSerializerFactory#createDocumentSerializer
 * createDocumentSerializer} method creates a {@code DocumentSerializer}, which
 * may or may not implement {@code DocumentSerializer2}. When it is constructed,
 * a &quot;Office&quot; {@code Document} object is passed in to be used as
 * input.</p>
 *
 * @see  org.openoffice.xmerge.PluginFactory
 * @see  org.openoffice.xmerge.DocumentSerializerFactory
 */
public interface DocumentSerializer2 extends DocumentSerializer {

    /**
     * Convert the data passed into the {@code DocumentSerializer2} constructor
     * into the &quot;Device&quot; {@code Document} format.
     *
     * <p>The URL's passed may be used to resolve links and to name the output
     * device document(s).</p>
     *
     * <p>This method may or may not be thread-safe.  It is expected that the
     * user code does not call this method in more than one thread.  And for
     * most cases, this method is only done once.</p>
     *
     * @return  {@code ConvertData} object to pass back the converted data.
     *
     * @param   officeURL         URL of the office document (may be null if
     *                            unknown)
     * @param   deviceURL         URL of the device document (may be null if
     *                            unknown)
     *
     * @throws  ConvertException  If any conversion error occurs.
     * @throws  IOException       If any I/O error occurs.
     */
    ConvertData serialize(String officeURL, String deviceURL) throws
        ConvertException, IOException;
}