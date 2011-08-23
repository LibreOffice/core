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

package org.openoffice.xmerge.util;


/**
 * <p>Helper class providing static methods to convert data to/from 
 *    twips</p>
 *
 * @author	Martin Maher 
 */
public class TwipsConverter {
    
    /**
     * <p>Convert from twips to cm's</p>
     *
     * @param   value   The <code>short</code> to be converted.
     *
     * @return   float containing the converted 
     */
    public static float twips2cm(int value) {

        float inches = (float) value/1440;
        float cm = (float) inches*(float)2.54;

        return cm;
    }
    
    
    
    /**
     * <p>Convert from cm's to twips</p>
     *
     * @param   value   <code>byte</code> array containing the LE representation 
     *                  of the value.
     *
     * @return  int containing the converted value.
     */
    public static int cm2twips(float value) {
    
        int twips = (int) ((value/2.54)*1440); 
        
        return twips;
    }
    
    /**
     * <p>Convert from twips to cm's</p>
     *
     * @param   value   The <code>short</code> to be converted.
     *
     * @return   float containing the converted 
     */
    public static float twips2inches(int value) {

        return (float) value/1440;
    }
    
    
    
    /**
     * <p>Convert from cm's to twips</p>
     *
     * @param   value   <code>byte</code> array containing the LE representation 
     *                  of the value.
     *
     * @return  int containing the converted value.
     */
    public static int inches2twips(float value) {
    
        return (int) (value*1440);
    }
    
    
}
