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

/**
 * A {@code DocumentSerializer} object is used to convert from the
 * &quot;Office&quot; {@code Document} format to the &quot;Device&quot;
 * {@code Document} format.
 *
 * <p>All plug-in implementations of the {@code PluginFactory} interface that
 * also support serialization must also implement this interface.</p>
 *
 * @see  PluginFactory
 * @see  DocumentSerializer
 */
public interface DocumentSerializerFactory {

    /**
     * The {@code DocumentSerializer} is used to convert from the
     * &quot;Office&quot; {@code Document} format to the &quot;Device&quot;
     * {@code Document} format.
     *
     * <p>The {@code ConvertData} object is passed along to the created
     * {@code DocumentSerializer} via its constructor.  The {@code ConvertData}
     * is read and converted when the {@code DocumentSerializer} object's
     * {@code serialize} method is called.</p>
     *
     * @param   doc  {@code Document} object that the created
     *               {@code DocumentSerializer} object uses as input.
     *
     * @return  A <code>DocumentSerializer</code> object.
     */
    DocumentSerializer createDocumentSerializer(Document doc);
}
