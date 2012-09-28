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
 * <p>Helper class providing static methods to convert data to/from
 *    twips</p>
 *
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
        float cm = inches*(float)2.54;

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
