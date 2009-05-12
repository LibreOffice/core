/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyName.java,v $
 * $Revision: 1.10 $
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

package convwatch;

public interface PropertyName
{
    final public static String DOC_COMPARATOR_INPUT_PATH =                 "DOC_COMPARATOR_INPUT_PATH";
    final public static String DOC_COMPARATOR_OUTPUT_PATH =                "DOC_COMPARATOR_OUTPUT_PATH";
    final public static String DOC_COMPARATOR_DIFF_PATH =                  "DOC_COMPARATOR_DIFF_PATH";
    final public static String DOC_COMPARATOR_REFERENCE_PATH =             "DOC_COMPARATOR_REFERENCE_PATH";
    final public static String DOC_COMPARATOR_REFERENCE_INPUT_PATH =       "DOC_COMPARATOR_REFERENCE_INPUT_PATH";
    final public static String DOC_COMPARATOR_REFERENCE_TYPE =             "DOC_COMPARATOR_REFERENCE_CREATOR_TYPE";
    final public static String DOC_COMPARATOR_PRINTER_NAME =               "DOC_COMPARATOR_PRINTER_NAME";
    final public static String DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP =     "DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP";
    final public static String DOC_COMPARATOR_INCLUDE_SUBDIRS =            "DOC_COMPARATOR_INCLUDE_SUBDIRS";
    final public static String DOC_COMPARATOR_PRINT_MAX_PAGE =             "DOC_COMPARATOR_PRINT_MAX_PAGE";
    final public static String DOC_COMPARATOR_PRINT_ONLY_PAGE =            "DOC_COMPARATOR_PRINT_ONLY_PAGE";
    final public static String DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION =  "DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION";
    final public static String DOC_COMPARATOR_OVERWRITE_REFERENCE =        "DOC_COMPARATOR_OVERWRITE_REFERENCE";

    final public static String DOC_COMPARATOR_HTML_OUTPUT_PREFIX =         "DOC_COMPARATOR_HTML_OUTPUT_PREFIX";
    final public static String DOC_COMPARATOR_GFXCMP_WITH_BORDERMOVE =     "DOC_COMPARATOR_GFXCMP_WITH_BORDERMOVE";

    final public static String DOC_CONVERTER_IMPORT_FILTER_NAME =          "DOC_CONVERTER_IMPORT_FILTER_NAME";
    final public static String DOC_CONVERTER_EXPORT_FILTER_NAME =          "DOC_CONVERTER_EXPORT_FILTER_NAME";
    final public static String DOC_CONVERTER_OFFICE_PROGRAM =              "DOC_CONVERTER_OFFICE_PROGRAM";
    final public static String DOC_CONVERTER_REUSE_OFFICE =                "DOC_CONVERTER_REUSE_OFFICE";

    final public static String DOC_COMPARATOR_LEAVE_OUT_FILES =            "DOC_COMPARATOR_LEAVE_OUT_FILES";

    final public static String DOC_COMPARATOR_DB_INFO_STRING =             "DOC_COMPARATOR_DB_INFO_STRING";

    final public static String TEMPPATH =                                   "TEMPPATH";

    // set this variable to "true" or "yes" and loadComponentFromURL works with property Hidden=false
    final public static String OFFICE_VIEWABLE =                           "OFFICE_VIEWABLE";

    final public static String DB_CONNECTION_STRING =                      "DB_CONNECTION_STRING";
    final public static String CHECK_NEED_TOOLS =                          "CHECK_NEED_TOOLS";
    final public static String CREATE_DEFAULT =                            "CREATE_DEFAULT_REFERENCE";

}
