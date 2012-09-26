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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import org.openoffice.xmerge.converter.palm.PdbUtil;

/**
 *  Constants used for encoding and decoding the AportisDoc format.
 *
 */
interface DocConstants {

    /** Creator id. */
    public static final int CREATOR_ID = PdbUtil.intID("REAd");

    /** Type id. */
    public static final int TYPE_ID = PdbUtil.intID("TEXt");

    /** Constant for uncompressed version. */
    public static final short UNCOMPRESSED = 1;

    /** Constant for compressed version. */
    public static final short COMPRESSED = 2;

    /** Constant used for spare fields. */
    public static final int SPARE = 0;

    /** AportisDoc record size. */
    public static final short TEXT_RECORD_SIZE = 4096;

    /** Constant for encoding scheme. */
    public static final String ENCODING = "8859_1";

    /** Constant for TAB character. */
    public final static char TAB_CHAR = '\t';

    /** Constant for EOL character. */
    public final static char EOL_CHAR = '\n';

    /** Constant for SPACE character. */
    public final static char SPACE_CHAR = ' ';
}

