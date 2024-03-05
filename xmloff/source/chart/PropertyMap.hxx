/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#pragma once

#include <xmloff/xmltypes.hxx>
#include <xmloff/contextid.hxx>

// custom types
#define XML_SCH_TYPE_AXIS_ARRANGEMENT       ( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_BAR_STYLE        ( XML_SCH_TYPES_START + 1 )
// free
#define XML_SCH_TYPE_SOLID_TYPE             ( XML_SCH_TYPES_START + 3 )
#define XML_SCH_TYPE_ERROR_INDICATOR_UPPER  ( XML_SCH_TYPES_START + 4 )
#define XML_SCH_TYPE_ERROR_INDICATOR_LOWER  ( XML_SCH_TYPES_START + 5 )
#define XML_SCH_TYPE_DATAROWSOURCE          ( XML_SCH_TYPES_START + 6 )
#define XML_SCH_TYPE_TEXT_ORIENTATION       ( XML_SCH_TYPES_START + 7 )
#define XML_SCH_TYPE_INTERPOLATION          ( XML_SCH_TYPES_START + 8 )
#define XML_SCH_TYPE_SYMBOL_TYPE            ( XML_SCH_TYPES_START + 9 )
#define XML_SCH_TYPE_NAMED_SYMBOL           ( XML_SCH_TYPES_START + 10 )
#define XML_SCH_TYPE_LABEL_PLACEMENT_TYPE   ( XML_SCH_TYPES_START + 11 )
#define XML_SCH_TYPE_MISSING_VALUE_TREATMENT    ( XML_SCH_TYPES_START + 12 )
#define XML_SCH_TYPE_AXIS_POSITION          ( XML_SCH_TYPES_START + 13 )
#define XML_SCH_TYPE_AXIS_POSITION_VALUE    ( XML_SCH_TYPES_START + 14 )
#define XML_SCH_TYPE_AXIS_LABEL_POSITION    ( XML_SCH_TYPES_START + 15 )
#define XML_SCH_TYPE_TICK_MARK_POSITION     ( XML_SCH_TYPES_START + 16 )
#define XML_SCH_TYPE_LABEL_BORDER_STYLE     ( XML_SCH_TYPES_START + 17 )
#define XML_SCH_TYPE_LABEL_BORDER_OPACITY   ( XML_SCH_TYPES_START + 18 )
#define XML_SCH_TYPE_LABEL_FILL_STYLE       ( XML_SCH_TYPES_START + 19 )
#define XML_SCH_TYPE_OF_PIE_TYPE            ( XML_SCH_TYPES_START + 20 )

// context ids
#define XML_SCH_CONTEXT_USER_SYMBOL                 ( XML_SCH_CTF_START + 0 )
#define XML_SCH_CONTEXT_MIN                         ( XML_SCH_CTF_START + 1 )
#define XML_SCH_CONTEXT_MAX                         ( XML_SCH_CTF_START + 2 )
#define XML_SCH_CONTEXT_STEP_MAIN                   ( XML_SCH_CTF_START + 3 )
#define XML_SCH_CONTEXT_STEP_HELP_COUNT             ( XML_SCH_CTF_START + 4 )
#define XML_SCH_CONTEXT_ORIGIN                      ( XML_SCH_CTF_START + 5 )
#define XML_SCH_CONTEXT_LOGARITHMIC                 ( XML_SCH_CTF_START + 6 )
#define XML_SCH_CONTEXT_STOCK_WITH_VOLUME           ( XML_SCH_CTF_START + 7 )
#define XML_SCH_CONTEXT_LINES_USED                  ( XML_SCH_CTF_START + 8 )

#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER     ( XML_SCH_CTF_START + 10 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER     ( XML_SCH_CTF_START + 11 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER     ( XML_SCH_CTF_START + 12 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER     ( XML_SCH_CTF_START + 13 )
#define XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION       ( XML_SCH_CTF_START + 14 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER   ( XML_SCH_CTF_START + 15 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT     ( XML_SCH_CTF_START + 16 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL   ( XML_SCH_CTF_START + 17 )
#define XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT       ( XML_SCH_CTF_START + 18 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_ROW_SOURCE     ( XML_SCH_CTF_START + 19 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH        ( XML_SCH_CTF_START + 20 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT       ( XML_SCH_CTF_START + 21 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME   ( XML_SCH_CTF_START + 22 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE        ( XML_SCH_CTF_START + 23 )
#define XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR     ( XML_SCH_CTF_START + 24 )
#define XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE      ( XML_SCH_CTF_START + 25 )
#define XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE     ( XML_SCH_CTF_START + 26 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES   ( XML_SCH_CTF_START + 27 )
#define XML_SCH_CONTEXT_SPECIAL_MOVING_AVERAGE_TYPE ( XML_SCH_CTF_START + 28 )


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
