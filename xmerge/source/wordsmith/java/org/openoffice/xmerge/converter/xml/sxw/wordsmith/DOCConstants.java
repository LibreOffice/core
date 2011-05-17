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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

/**
 *  Constants used for encoding and decoding the WordSmith format.
 *
 *  @author   Herbie Ong, David Proulx
 */
interface DOCConstants {

    /** Constant for uncompressed version. */
    public static final short UNCOMPRESSED = 1;

    /** Constant for compressed version. */
    public static final short COMPRESSED = 2;

    /** Constant used for spare fields. */
    public static final int SPARE = 0;

    /** WordSmith record size. */
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

