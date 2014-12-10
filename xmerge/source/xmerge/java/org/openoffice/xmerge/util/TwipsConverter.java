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

package org.openoffice.xmerge.util;

/**
 * Helper class providing static methods to convert data to/from twips.
 */
public class TwipsConverter {

    /**
     * Convert from twips to cm's.
     *
     * @param   value  The {@code short} to be converted.
     *
     * @return  {@code float} containing the converted.
     */
    public static float twips2cm(int value) {
        float inches = (float) value/1440;
        float cm = inches*(float)2.54;
        return cm;
    }

    /**
     * Convert from cm's to twips.
     *
     * @param   value  {@code float} containing the representation of the value.
     *
     * @return  {@code int} containing the converted value.
     */
    private static int cm2twips(float value) {
        int twips = (int) ((value/2.54)*1440);
        return twips;
    }

    /**
     * Convert from cm's to twips.
     *
     * @param   value  {@code float} containing the representation of the value.
     *
     * @return  {@code int} containing the converted value.
     */
    private static int inches2twips(float value) {
        return (int) (value*1440);
    }

    /**
     * Convert {@code String} to twips.
     *
     * @param  value        {@code String} in the form {@literal "1.234cm"} or
     *                      {@literal "1.234inch"}.
     * @param  defaultValue the default value.
     * @return the converted value if {@code value} is a well-formatted {@code
     *         String}, {@code defaultValue} otherwise.
     */
    public static int convert2twips(String value, int defaultValue) {
        int posi;

        if ((posi = value.indexOf("cm")) != -1) {
            float cm = Float.parseFloat(value.substring(0,posi));
            return cm2twips(cm);
        } else if ((posi = value.indexOf("inch")) != -1) {
            float inches = Float.parseFloat(value.substring(0,posi));
            return inches2twips(inches);
        }

        return defaultValue;
    }
}