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

package convwatch;

public interface PropertyName
{
    String DOC_COMPARATOR_INPUT_PATH =                 "DOC_COMPARATOR_INPUT_PATH";
    String DOC_COMPARATOR_OUTPUT_PATH =                "DOC_COMPARATOR_OUTPUT_PATH";
    String DOC_COMPARATOR_DIFF_PATH =                  "DOC_COMPARATOR_DIFF_PATH";
    String DOC_COMPARATOR_REFERENCE_PATH =             "DOC_COMPARATOR_REFERENCE_PATH";
    String DOC_COMPARATOR_REFERENCE_INPUT_PATH =       "DOC_COMPARATOR_REFERENCE_INPUT_PATH";
    String DOC_COMPARATOR_REFERENCE_TYPE =             "DOC_COMPARATOR_REFERENCE_CREATOR_TYPE";
    String DOC_COMPARATOR_PRINTER_NAME =               "DOC_COMPARATOR_PRINTER_NAME";
    String DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP =     "DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP";
    String DOC_COMPARATOR_INCLUDE_SUBDIRS =            "DOC_COMPARATOR_INCLUDE_SUBDIRS";
    String DOC_COMPARATOR_PRINT_MAX_PAGE =             "DOC_COMPARATOR_PRINT_MAX_PAGE";
    String DOC_COMPARATOR_PRINT_ONLY_PAGE =            "DOC_COMPARATOR_PRINT_ONLY_PAGE";
    String DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION =  "DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION";
    String DOC_COMPARATOR_OVERWRITE_REFERENCE =        "DOC_COMPARATOR_OVERWRITE_REFERENCE";

    String DOC_COMPARATOR_HTML_OUTPUT_PREFIX =         "DOC_COMPARATOR_HTML_OUTPUT_PREFIX";
    String DOC_COMPARATOR_GFXCMP_WITH_BORDERMOVE =     "DOC_COMPARATOR_GFXCMP_WITH_BORDERMOVE";

    String DOC_CONVERTER_IMPORT_FILTER_NAME =          "DOC_CONVERTER_IMPORT_FILTER_NAME";
    String DOC_CONVERTER_EXPORT_FILTER_NAME =          "DOC_CONVERTER_EXPORT_FILTER_NAME";
    String DOC_CONVERTER_OFFICE_PROGRAM =              "DOC_CONVERTER_OFFICE_PROGRAM";
    String DOC_CONVERTER_REUSE_OFFICE =                "DOC_CONVERTER_REUSE_OFFICE";

    String DOC_COMPARATOR_LEAVE_OUT_FILES =            "DOC_COMPARATOR_LEAVE_OUT_FILES";

    String DOC_COMPARATOR_DB_INFO_STRING =             "DOC_COMPARATOR_DB_INFO_STRING";

    String TEMPPATH =                                   "TEMPPATH";

    // set this variable to "true" or "yes" and loadComponentFromURL works with property Hidden=false
    String OFFICE_VIEWABLE =                           "OFFICE_VIEWABLE";

    String DB_CONNECTION_STRING =                      "DB_CONNECTION_STRING";
    String CHECK_NEED_TOOLS =                          "CHECK_NEED_TOOLS";
    String CREATE_DEFAULT =                            "CREATE_DEFAULT_REFERENCE";

}
