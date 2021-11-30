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
#ifndef INCLUDED_SW_INC_UNOMID_H
#define INCLUDED_SW_INC_UNOMID_H

#define MID_MIRROR_VERT                         0
#define MID_MIRROR_HORZ_EVEN_PAGES              1
#define MID_MIRROR_HORZ_ODD_PAGES               2

#define MID_PAGEDESC_PAGEDESCNAME               0
#define MID_PAGEDESC_PAGENUMOFFSET              1

#define MID_SURROUND_SURROUNDTYPE               0
#define MID_SURROUND_ANCHORONLY                 1
#define MID_SURROUND_CONTOUR                    2
#define MID_SURROUND_CONTOUROUTSIDE             3

#define MID_VERTORIENT_ORIENT                   0
#define MID_VERTORIENT_RELATION                 1
#define MID_VERTORIENT_POSITION                 2

#define MID_HORIORIENT_ORIENT                   0
#define MID_HORIORIENT_RELATION                 1
#define MID_HORIORIENT_POSITION                 2
#define MID_HORIORIENT_PAGETOGGLE               3

#define MID_ANCHOR_ANCHORTYPE                   0
#define MID_ANCHOR_PAGENUM                      1
#define MID_ANCHOR_ANCHORFRAME                  2

#define MID_URL_HYPERLINKNAME                   1
#define MID_URL_CLIENTMAP                       2
#define MID_URL_SERVERMAP                       3
#define MID_URL_URL                             4
#define MID_URL_TARGET                          5
#define MID_URL_VISITED_FMT                     6
#define MID_URL_UNVISITED_FMT                   7
#define MID_URL_HYPERLINKEVENTS                 8

#define MID_CHAIN_PREVNAME                      0
#define MID_CHAIN_NEXTNAME                      1
#define MID_CHAIN_NAME                          2

#define MID_LINENUMBER_COUNT                    0
#define MID_LINENUMBER_STARTVALUE               1

#define MID_DROPCAP_FORMAT                      0
#define MID_DROPCAP_WHOLE_WORD                  1
#define MID_DROPCAP_CHAR_STYLE_NAME             2
#define MID_DROPCAP_LINES                       3
#define MID_DROPCAP_COUNT                       4
#define MID_DROPCAP_DISTANCE                    5

#define MID_FRMSIZE_SIZE                        0
#define MID_FRMSIZE_REL_HEIGHT                  1
#define MID_FRMSIZE_REL_WIDTH                   2

#define MID_FRMSIZE_WIDTH                       4
#define MID_FRMSIZE_HEIGHT                      5
#define MID_FRMSIZE_SIZE_TYPE                   6
#define MID_FRMSIZE_IS_AUTO_HEIGHT              7
/* XML only */
#define MID_FRMSIZE_MIN_HEIGHT                  8
#define MID_FRMSIZE_FIX_HEIGHT                  9
#define MID_FRMSIZE_COL_WIDTH                   10
#define MID_FRMSIZE_REL_COL_WIDTH               11
/* UNO */
#define MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT     12
#define MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH     13
#define MID_FRMSIZE_WIDTH_TYPE                  14
#define MID_FRMSIZE_REL_WIDTH_RELATION          15
#define MID_FRMSIZE_REL_HEIGHT_RELATION         16

#define MID_COLUMNS                             0
#define MID_COLUMN_SEPARATOR_LINE               1

//SwFormatFootnoteAtTextEnd
#define MID_COLLECT                             0
#define MID_RESTART_NUM                         1
#define MID_NUM_START_AT                        2
#define MID_OWN_NUM                             3
#define MID_NUM_TYPE                            4
#define MID_PREFIX                              5
#define MID_SUFFIX                              6

//page style footnote
#define MID_FTN_HEIGHT                          0
#define MID_LINE_WEIGHT                         1
#define MID_LINE_COLOR                          2
#define MID_LINE_RELWIDTH                       3
#define MID_LINE_ADJUST                         4
#define MID_LINE_TEXT_DIST                      5
#define MID_LINE_FOOTNOTE_DIST                  6
#define MID_FTN_LINE_STYLE                      7

//SwFormatRuby
#define MID_RUBY_TEXT           0
#define MID_RUBY_ADJUST         1
#define MID_RUBY_CHARSTYLE      2
#define MID_RUBY_ABOVE          3
#define MID_RUBY_POSITION       4

//SwTextGridItem
#define MID_GRID_COLOR          0
#define MID_GRID_LINES          1
#define MID_GRID_BASEHEIGHT     2
#define MID_GRID_RUBYHEIGHT     3
#define MID_GRID_TYPE           4
#define MID_GRID_RUBY_BELOW     5
#define MID_GRID_PRINT          6
#define MID_GRID_DISPLAY        7
#define MID_GRID_BASEWIDTH      8
#define MID_GRID_SNAPTOCHARS    9
#define MID_GRID_STANDARD_MODE    10

//SwEnvItem
#define MID_ENV_ADDR_TEXT       1
#define MID_ENV_SEND            2
#define MID_SEND_TEXT           3
#define MID_ENV_ADDR_FROM_LEFT  4
#define MID_ENV_ADDR_FROM_TOP   5
#define MID_ENV_SEND_FROM_LEFT  6
#define MID_ENV_SEND_FROM_TOP   7
#define MID_ENV_WIDTH           8
#define MID_ENV_HEIGHT          9
#define MID_ENV_ALIGN           10
#define MID_ENV_PRINT_FROM_ABOVE 11
#define MID_ENV_SHIFT_RIGHT     12
#define MID_ENV_SHIFT_DOWN      13

// SwFormatWrapInfluenceOnObjPos
#define MID_WRAP_INFLUENCE      0
#define MID_ALLOW_OVERLAP       1

// SwFormatFollowTextFlow
#define MID_FOLLOW_TEXT_FLOW    0

#define MID_TEXT_BOX            0
#define MID_TEXT_BOX_CONTENT    1

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
