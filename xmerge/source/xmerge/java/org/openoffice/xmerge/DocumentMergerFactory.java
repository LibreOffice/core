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
 * All plug-in implementations of the {@code PluginFactory} interface that also
 * support merging must also implement this interface.
 *
 * <p>Merge is useful when an {@code OfficeDocument} is converted to a
 * &quot;Device&quot; {@code Document} format, and the &quot;Device&quot;
 * {@code Document} version is modified.</p>
 *
 * <p>Those changes can be merged back into the original {@code OfficeDocument}
 * with the merger.  The merger is capable of doing this even if the
 * &quot;Device&quot; format is lossy in comparison to the {@code OfficeDocument}
 * format.</p>
 *
 * @see  PluginFactory
 * @see  DocumentMerger
 * @see  ConverterCapabilities
 */
public interface DocumentMergerFactory {

    /**
     * Create a {@code DocumentMerger} object given a {@code Document} object.
     *
     * @param   doc  {@code Document} object that the created
     *               {@code DocumentMerger} object uses as a base {@code Document}
     *               for merging changes into.
     *
     * @return  A {@code DocumentMerger} object or {@code null} if none exists.
     */
    DocumentMerger createDocumentMerger(Document doc);
}