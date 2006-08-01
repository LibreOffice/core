/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TwipsConverter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:41:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.util;


/**
 * <p>Helper class providing static methods to convert data to/from
 *    twips</p>
 *
 * @author  Martin Maher
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
