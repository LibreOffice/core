/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SW_UNOMID_HXX
#define SW_UNOMID_HXX


#define MID_MIRROR_VERT							0
#define MID_MIRROR_HORZ_EVEN_PAGES				1
#define MID_MIRROR_HORZ_ODD_PAGES				2

#define MID_PAGEDESC_PAGEDESCNAME				0
#define MID_PAGEDESC_PAGENUMOFFSET				1

#define MID_SURROUND_SURROUNDTYPE				0
#define MID_SURROUND_ANCHORONLY					1
#define MID_SURROUND_CONTOUR					2
#define MID_SURROUND_CONTOUROUTSIDE				3

#define MID_VERTORIENT_ORIENT					0
#define MID_VERTORIENT_RELATION					1
#define MID_VERTORIENT_POSITION					2

#define MID_HORIORIENT_ORIENT					0
#define MID_HORIORIENT_RELATION					1
#define MID_HORIORIENT_POSITION					2
#define MID_HORIORIENT_PAGETOGGLE				3

#define MID_ANCHOR_ANCHORTYPE					0
#define MID_ANCHOR_PAGENUM						1
#define MID_ANCHOR_ANCHORFRAME                  2

#define MID_URL_HYPERLINKFORMAT					0
#define MID_URL_HYPERLINKNAME					1
#define MID_URL_CLIENTMAP						2
#define MID_URL_SERVERMAP						3
#define MID_URL_URL								4
#define MID_URL_TARGET							5
#define MID_URL_VISITED_FMT						6
#define MID_URL_UNVISITED_FMT					7
#define MID_URL_HYPERLINKEVENTS					8

#define MID_CHAIN_PREVNAME						0
#define MID_CHAIN_NEXTNAME						1

#define MID_LINENUMBER_COUNT					0
#define MID_LINENUMBER_STARTVALUE				1

#define MID_DROPCAP_FORMAT                      0
#define MID_DROPCAP_WHOLE_WORD                  1
#define MID_DROPCAP_CHAR_STYLE_NAME				2
#define MID_DROPCAP_LINES                       3
#define MID_DROPCAP_COUNT                       4
#define MID_DROPCAP_DISTANCE                    5

#define MID_FRMSIZE_SIZE						0
#define MID_FRMSIZE_REL_HEIGHT                  1
#define MID_FRMSIZE_REL_WIDTH                   2

#define MID_FRMSIZE_WIDTH                       4
#define MID_FRMSIZE_HEIGHT                      5
#define MID_FRMSIZE_SIZE_TYPE                   6
#define MID_FRMSIZE_IS_AUTO_HEIGHT              7
/* XML only */
#define MID_FRMSIZE_MIN_HEIGHT                  8
#define MID_FRMSIZE_FIX_HEIGHT                  9
#define MID_FRMSIZE_COL_WIDTH					10
#define MID_FRMSIZE_REL_COL_WIDTH				11
/* UNO */
#define MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT     12
#define MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH     13

#define MID_COLUMNS								0
#define MID_COLUMN_SEPARATOR_LINE				1

//SwFmtFtnAtTxtEnd
#define MID_COLLECT					   			0
#define MID_RESTART_NUM                         1
#define MID_NUM_START_AT                        2
#define MID_OWN_NUM                             3
#define MID_NUM_TYPE                            4
#define MID_PREFIX                              5
#define MID_SUFFIX                              6

//page style footnote
#define MID_FTN_HEIGHT		  					0
#define MID_LINE_WEIGHT                         1
#define MID_LINE_COLOR                          2
#define MID_LINE_RELWIDTH                       3
#define MID_LINE_ADJUST                         4
#define MID_LINE_TEXT_DIST                      5
#define MID_LINE_FOOTNOTE_DIST                  6

//SwFmtRuby
#define MID_RUBY_TEXT			0
#define MID_RUBY_ADJUST         1
#define MID_RUBY_CHARSTYLE      2
#define MID_RUBY_ABOVE          3

//SwTextGridItem
#define MID_GRID_COLOR          0
#define MID_GRID_LINES          1
#define MID_GRID_BASEHEIGHT     2
#define MID_GRID_RUBYHEIGHT     3
#define MID_GRID_TYPE           4
#define MID_GRID_RUBY_BELOW     5
#define MID_GRID_PRINT          6
#define MID_GRID_DISPLAY        7

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
