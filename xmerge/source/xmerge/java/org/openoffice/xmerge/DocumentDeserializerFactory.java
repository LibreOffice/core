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
 * A {@code DocumentDeserializer} object is used to convert from the
 * &quot;Device&quot; {@code Document} format to the &quot;Office&quot;
 * {@code Document} format.
 *
 * <p>All plug-in implementations of the {@code PluginFactory} interface that
 * also support deserialization must also implement this interface.</p>
 *
 * @see  PluginFactory
 * @see  DocumentDeserializer
 */
public interface DocumentDeserializerFactory {

    /**
     * The {@code DocumentDeserializer} is used to convert from the
     * &quot;Device&quot; {@code Document} format to the &quot;Office&quot;
     * {@code Document} format.
     *
     * <p>The {@code ConvertData} object is passed along to the created
     * {@code DocumentDeserializer} via its constructor. The {@code ConvertData}
     * is read and converted when the {@code DocumentDeserializer} object's
     * {@code deserialize} method is called.</p>
     *
     * @param  cd  {@code ConvertData} object that the created
     *             {@code DocumentDeserializer} object uses as input.
     *
     * @return  A {@code DocumentDeserializer} object.
     */

    DocumentDeserializer createDocumentDeserializer(ConvertData cd);
}