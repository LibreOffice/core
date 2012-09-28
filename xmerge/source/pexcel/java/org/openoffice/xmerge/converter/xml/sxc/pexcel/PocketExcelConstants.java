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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;


/**
 * Interface defining constants for Pocket Excel attributes.
 *
 */
public interface PocketExcelConstants {
    /** File extension for Pocket Word files. */
    public static final String FILE_EXTENSION = ".pxl";

    /** Constants for pexcel BIFF records */
    public static final int BLANK_CELL         = 0x01;
    public static final int NUMBER_CELL        = 0x03;
    public static final int LABEL_CELL         = 0x04;
    public static final int BOOLERR_CELL       = 0x05;
    public static final int FORMULA_CELL       = 0x06;
    public static final int FORMULA_STRING     = 0x07;
    public static final int ROW_DESCRIPTION    = 0x08;
    public static final int BOF_RECORD         = 0x09;
    public static final int EOF_MARKER         = 0x0A;
    public static final int DEFINED_NAME       = 0x18;
    public static final int CURRENT_SELECTION  = 0x1D;
    public static final int NUMBER_FORMAT      = 0x1E;
    public static final int DEFAULT_ROW_HEIGHT = 0x25;
    public static final int FONT_DESCRIPTION   = 0x31;
    public static final int WINDOW_INFO        = 0x3D;
    public static final int SHEET_WINDOW_INFO  = 0x3E;
    public static final int PANE_INFO          = 0x41;
    public static final int CODEPAGE           = 0x42;
    public static final int DEF_COL_WIDTH      = 0x55;
    public static final int COLINFO            = 0x7D;
    public static final int BOUND_SHEET        = 0x85;
    public static final int EXTENDED_FORMAT    = 0xE0;

    /** Colour lookup table for mapping pexcel color values
        (See util/ColourConverter.java */
    public short cLookup[] = { 0, 14, 15, 1, 2, 3, 4, 7, 6, 5, 8, 9, 10, 13, 12, 11 };
}
