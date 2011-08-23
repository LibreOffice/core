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
        if (foundInfo.isValidOfficeType(mimeTypeOut))  
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
        
        toOffice = foundInfo.isValidOfficeType(mimeTypeOut);
        
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

