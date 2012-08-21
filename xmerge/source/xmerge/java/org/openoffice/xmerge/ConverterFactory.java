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
 *  Factory that provides access to <code>Convert</code> objects, which
 *  are used to do a conversion.  The <code>ConvertFactory</code> does
 *  this via the <code>ConvertInfoMgr</code> which maintains a list of
 *  which <code>Convert</code> objects are available and their
 *  capabilities.
 *
 *  @see  Convert
 *  @see  org.openoffice.xmerge.util.registry.ConverterInfoMgr
 *
 *  @author  Martin Maher
 */
public class ConverterFactory {

   /**
    *  Confirms whether or not a particular conversion can be done
    *  based on the Mime types of the files to be converted to and
    *  from.
    *
    *  @param  mimeTypeIn   The mime input type.
    *  @param  mimeTypeOut  The mime output type.
    *
    *  @return  true if the conversion is possible, false otherwise.
    */
    public boolean canConvert(String mimeTypeIn, String mimeTypeOut) {

        ConverterInfo foundInfo = null;

        // findConverterInfo expects the second paramenter to be the
        // destination MimeType
        if (ConverterInfo.isValidOfficeType(mimeTypeOut))
            foundInfo = ConverterInfoMgr.findConverterInfo(mimeTypeIn, mimeTypeOut);
        else
            foundInfo = ConverterInfoMgr.findConverterInfo(mimeTypeOut, mimeTypeIn);

        if (foundInfo != null)
            return true;
        else
            return false;
    }


   /**
    *  Returns the <code>Convert</code> object that converts
    *  the specified device/office mime type conversion.  If there
    *  are multiple <code>Converter</code> objects registered
    *  that support this conversion, only the first is returned.
    *
    *  @param  mimeTypeIn   The mime input type.
    *  @param  mimeTypeOut  The mime output type.
    *
    *  @return  The first <code>Convert</code> object that supports
    *           the specified conversion.
    */
    public Convert getConverter(String mimeTypeIn, String mimeTypeOut) {

        ConverterInfo foundInfo = null;
        boolean toOffice;

        toOffice = ConverterInfo.isValidOfficeType(mimeTypeOut);

        // findConverterInfo expects the second paramenter to be the
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
    *  Returns the <code>Convert</code> object that is described
    *  by the <code>ConverterInfo</code> parameter.
    *
    *  @param  ci  The <code>ConverterInfo</code> describing the converter.
    *
    *  @param  toOffice  true to convert to office, false to convert to device.
    *
    *  @return  The <code>Convert</code> object
    */
    public Convert getConverter(ConverterInfo ci, boolean toOffice) {

        Convert myConvert = new Convert(ci, toOffice);
        return myConvert;
    }
}

