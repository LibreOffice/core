/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc.minicalc;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;


/**
 *  <p>MiniCalc implementation of <code>ConverterCapabilities</code> for
 *  the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.minicalc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>Used with StarCalc SXC to/from MiniCalc conversions.  The
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

