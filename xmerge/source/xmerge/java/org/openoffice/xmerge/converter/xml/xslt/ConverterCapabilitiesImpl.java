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

package org.openoffice.xmerge.converter.xml.xslt;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 * Xslt implementation of {@code ConverterCapabilities} for the {@link
 * org.openoffice.xmerge.converter.xml.xslt.PluginFactoryImpl PluginFactoryImpl}.
 *
 * <p>Used with StarWriter XML to/from XSLT supported formats conversions.  The
 * {@code ConverterCapibilies} specify which &quot;Office&quot; {@code Document}
 * tags and attributes are supported on the &quot;Device&quot; {@code Document}
 * format.</p>
 */
public final class ConverterCapabilitiesImpl
    implements ConverterCapabilities {

    public boolean canConvertTag(String tag) {

        if (OfficeConstants.TAG_OFFICE_DOCUMENT.equals(tag))
            return true;
        else if (OfficeConstants.TAG_OFFICE_DOCUMENT_CONTENT.equals(tag))
            return true;
        else if (OfficeConstants.TAG_OFFICE_BODY.equals(tag))
            return true;
        else if (OfficeConstants.TAG_PARAGRAPH.equals(tag))
            return true;
        else if (OfficeConstants.TAG_HEADING.equals(tag))
            return true;
        else if (OfficeConstants.TAG_ORDERED_LIST.equals(tag))
            return true;
        else if (OfficeConstants.TAG_UNORDERED_LIST.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LIST_ITEM.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LIST_HEADER.equals(tag))
            return true;
        else if (OfficeConstants.TAG_SPAN.equals(tag))
            return true;
        else if (OfficeConstants.TAG_HYPERLINK.equals(tag))
            return true;
        else if (OfficeConstants.TAG_LINE_BREAK.equals(tag))
            return true;
        else if (OfficeConstants.TAG_SPACE.equals(tag))
            return true;
        else if (OfficeConstants.TAG_TAB_STOP.equals(tag))
            return true;

        return false;
    }

    public boolean canConvertAttribute(String tag, String attribute) {
        return OfficeConstants.TAG_SPACE.equals(tag)
                && OfficeConstants.ATTRIBUTE_SPACE_COUNT.equals(attribute);
    }
}