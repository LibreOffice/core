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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

/**
 *  WordSmith utility class.
 *
 */
class util {

    /**
     *  Convert 2 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom2bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 8)
                | (data[index+1] & 0xFF));

    }


    /**
     *  Convert 4 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom4bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 24)
          | ((data[index + 1] & 0xFF) << 16)
          | ((data[index + 2] & 0xFF) << 8)
                | (data[index+3] & 0xFF));

    }
}

