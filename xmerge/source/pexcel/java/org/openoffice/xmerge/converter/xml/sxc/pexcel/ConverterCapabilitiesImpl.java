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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;


/**
 *  <p>Pocket Excel implementation of <code>ConverterCapabilities</code> for
 *  the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.pexcel.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>Used with StarCalc SXC to/from Pocket Excel conversions.  The
 *  <code>ConverterCapibilies</code> specify which &quot;Office&quot;
 *  <code>Document</code> tags and attributes are supported on the
 *  &quot;Device&quot; <code>Document</code> format.</p>
 */
public final class ConverterCapabilitiesImpl
    implements ConverterCapabilities {

    public boolean canConvertTag(String tag) {

        if (OfficeConstants.TAG_OFFICE_BODY.equals(tag))
            return true;
        else if (OfficeConstants.TAG_PARAGRAPH.equals(tag))
            return true;
        else if (OfficeConstants.TAG_TABLE.equals(tag))
            return true;
        else if (OfficeConstants.TAG_TABLE_ROW.equals(tag))
            return true;
        else if (OfficeConstants.TAG_TABLE_COLUMN.equals(tag))
            return false;
        // TODO - we currently do not handle the table column tag
        else if (OfficeConstants.TAG_TABLE_SCENARIO.equals(tag))
            return false;
        // TODO - we currently do not handle the table scenario tag
        else if (OfficeConstants.TAG_TABLE_CELL.equals(tag))
            return true;

        return false;
    }

    public boolean canConvertAttribute(String tag,
                                       String attribute) {

        if (OfficeConstants.TAG_TABLE.equals(tag)) {

            if (OfficeConstants.ATTRIBUTE_TABLE_NAME.equals(attribute))
                return true;

        } else if (OfficeConstants.TAG_TABLE_CELL.equals(tag)) {

            if (OfficeConstants.ATTRIBUTE_TABLE_VALUE_TYPE.equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_FORMULA.
                     equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_VALUE.equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_BOOLEAN_VALUE.
                     equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_CURRENCY.
                     equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_TIME_VALUE.
                     equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_DATE_VALUE.
                     equals(attribute))
                return true;
            else if (OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED.
                     equals(attribute))
                return true;

        } else if (OfficeConstants.TAG_TABLE_ROW.equals(tag)) {

            if (OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED.
                equals(attribute))
                return true;
        }

        return false;
    }
}

