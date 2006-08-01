/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PocketExcelConstants.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:58:29 $
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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;


/**
 * Interface defining constants for Pocket Excel attributes.
 *
 * @author  Martin Maher
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
