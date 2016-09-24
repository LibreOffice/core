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
 * A {@code ConverterCapabilities} object is used by {@code DocumentMerger}
 * implementations.
 *
 * <p>The {@code ConverterCapabilities} indicates which &quot;Office&quot; XML
 * tags are supported by the &quot;Device&quot; format.</p>
 *
 * @see  org.openoffice.xmerge.PluginFactory
 * @see  org.openoffice.xmerge.DocumentMerger
 */
public interface ConverterCapabilities {

    /**
     * Test to see if the device document format supports the tag in question.
     *
     * @param   tag  The tag to check.
     *
     * @return  {@code true} if the device format supports the tag,
     *          {@code false} otherwise.
     */
    boolean canConvertTag(String tag);

    /**
     * Test to see if the device document format supports the tag attribute in
     * question.
     *
     * @param   tag        The tag to check.
     * @param   attribute  The tag attribute to check.
     *
     * @return  {@code true} if the device format supports the attribute,
     *          {@code false} otherwise.
     */
    boolean canConvertAttribute(String tag, String attribute);
}