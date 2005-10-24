/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

