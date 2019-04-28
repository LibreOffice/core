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

import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;

/**
 * Factory that provides access to {@code Convert} objects, which are used to do
 * a conversion.
 *
 * <p>The {@code ConvertFactory} does this via the {@code ConvertInfoMgr} which
 * maintains a list of which {@code Convert} objects are available and their
 * capabilities.</p>
 *
 * @see  Convert
 * @see  org.openoffice.xmerge.util.registry.ConverterInfoMgr
 */
public class ConverterFactory {

   /**
    * Returns the {@code Convert} object that converts the specified device/office
    * mime type conversion.
    *
    * <p>If there are multiple {@code Converter} objects registered that support
    * this conversion, only the first is returned.</p>
    *
    * @param   mimeTypeIn   The mime input type.
    * @param   mimeTypeOut  The mime output type.
    *
    * @return  The first {@code Convert} object that supports the specified
    *          conversion.
    */
    public Convert getConverter(String mimeTypeIn, String mimeTypeOut) {

        ConverterInfo foundInfo;
        boolean toOffice;

        toOffice = ConverterInfo.isValidOfficeType(mimeTypeOut);

        // findConverterInfo expects the second parameter to be the
        // destination MimeType
        if (toOffice)
            foundInfo = ConverterInfoMgr.findConverterInfo(mimeTypeIn, mimeTypeOut);
        else
            foundInfo = ConverterInfoMgr.findConverterInfo(mimeTypeOut, mimeTypeIn);

        if (foundInfo != null)
            return getConverter(foundInfo, toOffice);
        else
            return null;
    }

   /**
    * Returns the {@code Convert} object that is described by the
    * {@code ConverterInfo} parameter.
    *
    * @param   ci        The {@code ConverterInfo} describing the converter.
    * @param   toOffice  {@code true} to convert to office, {@code false} to
    *                    convert to device.
    *
    * @return  The {@code Convert} object
    */
    public Convert getConverter(ConverterInfo ci, boolean toOffice) {

        Convert myConvert = new Convert(ci, toOffice);
        return myConvert;
    }
}
