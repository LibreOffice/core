/*************************************************************************
 *
 *  $RCSfile: unomap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2000-10-31 11:28:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNOMAP_HXX
#define _UNOMAP_HXX

#ifndef _SFX_ITEMPROP_HXX //autogen
#include <svtools/itemprop.hxx>
#endif

#define PROPERTY_NONE 0

#define PROPERTY_MAP_TEXT_CURSOR            0
#define PROPERTY_MAP_CHAR_STYLE             1
#define PROPERTY_MAP_PARA_STYLE             2
#define PROPERTY_MAP_FRAME_STYLE            3
#define PROPERTY_MAP_PAGE_STYLE             4
#define PROPERTY_MAP_NUM_STYLE              5
#define PROPERTY_MAP_SECTION                6
#define PROPERTY_MAP_TEXT_TABLE             7
#define PROPERTY_MAP_TABLE_CELL             8
#define PROPERTY_MAP_TABLE_RANGE            9
#define PROPERTY_MAP_TEXT_SEARCH            10
#define PROPERTY_MAP_TEXT_FRAME             11
#define PROPERTY_MAP_TEXT_GRAPHIC           12
#define PROPERTY_MAP_TEXT_SHAPE             13
#define PROPERTY_MAP_INDEX_USER             14
#define PROPERTY_MAP_INDEX_CNTNT            15
#define PROPERTY_MAP_INDEX_IDX              16
#define PROPERTY_MAP_USER_MARK              17
#define PROPERTY_MAP_CNTIDX_MARK            18
#define PROPERTY_MAP_INDEX_MARK             19
#define PROPERTY_MAP_TEXT_TABLE_ROW         20
#define PROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR  21
#define PROPERTY_MAP_TEXT_TABLE_CURSOR      22
#define PROPERTY_MAP_BOOKMARK               23
#define PROPERTY_MAP_PARAGRAPH_EXTENSIONS   24
#define PROPERTY_MAP_INDEX_ILLUSTRATIONS    25
#define PROPERTY_MAP_INDEX_OBJECTS          26
#define PROPERTY_MAP_INDEX_TABLES           27
#define PROPERTY_MAP_BIBLIOGRAPHY           28
#define PROPERTY_MAP_TEXT_DOCUMENT          29
#define PROPERTY_MAP_LINK_TARGET            30
#define PROPERTY_MAP_AUTO_TEXT_GROUP        31
#define PROPERTY_MAP_PRINT_SETTINGS         32
#define PROPERTY_MAP_VIEW_SETTINGS          33
#define PROPERTY_MAP_TEXTPORTION_EXTENSIONS 34
#define PROPERTY_MAP_FOOTNOTE               35
#define PROPERTY_MAP_TEXT_COLUMS            36
#define PROPERTY_MAP_PARAGRAPH              37
#define PROPERTY_MAP_END                    38

#define PROPERTY_SET_CHAR_STYLE             1
#define PROPERTY_SET_PARA_STYLE             2
#define PROPERTY_SET_FRAME_STYLE            3
#define PROPERTY_SET_PAGE_STYLE             4
#define PROPERTY_SET_NUM_STYLE              5

//S&E
#define WID_WORDS                0
#define WID_BACKWARDS            1
#define WID_REGULAR_EXPRESSION   2
#define WID_CASE_SENSITIVE       3
#define WID_IN_SELECTION         4
#define WID_STYLES               5
#define WID_SIMILARITY           6
#define WID_SIMILARITY_RELAX     7
#define WID_SIMILARITY_EXCHANGE  8
#define WID_SIMILARITY_ADD       9
#define WID_SIMILARITY_REMOVE    10
#define WID_SEARCH_ALL           11

//Sections
#define WID_SECT_CONDITION      0
#define WID_SECT_DDE_TYPE       1
#define WID_SECT_DDE_FILE       2
#define WID_SECT_DDE_ELEMENT    3
#define WID_SECT_LINK           4
#define WID_SECT_VISIBLE        5
#define WID_SECT_PROTECTED      6
#define WID_SECT_REGION         7

//Verzeichnisse
#define WID_PRIMARY_KEY                         1000
#define WID_SECONDARY_KEY                       1001
#define WID_ALT_TEXT                            1002
#define WID_IDX_TITLE                           1003
#define WID_LEVEL                               1004
#define WID_CREATE_FROM_MARKS                   1005
#define WID_CREATE_FROM_OUTLINE                 1006
//#define WID_PARAGRAPH_STYLE_NAMES               1007
#define WID_CREATE_FROM_CHAPTER                 1008
#define WID_CREATE_FROM_LABELS                  1009
#define WID_PROTECTED                           1000
#define WID_USE_ALPHABETICAL_SEPARATORS         1010
#define WID_USE_KEY_AS_ENTRY                    1011
#define WID_USE_COMBINED_ENTRIES                1012
#define WID_IS_CASE_SENSITIVE                   1013
#define WID_USE_P_P                             1014
#define WID_USE_DASH                            1015
#define WID_USE_UPPER_CASE                      1016
#define WID_INDEX_AUTO_MARK_FILE_U_R_L          1017
#define WID_LABEL_CATEGORY                      1018
#define WID_LABEL_DISPLAY_TYPE                  1019
#define WID_USE_LEVEL_FROM_SOURCE               1020
#define WID_LEVEL_FORMAT                        1021
#define WID_LEVEL_PARAGRAPH_STYLES              1022
#define WID_RECALC_TAB_STOPS                    1023
//#define WID_???                               1024
#define WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     1025
#define WID_CREATE_FROM_TABLES                  1026
#define WID_CREATE_FROM_TEXT_FRAMES             1027
#define WID_CREATE_FROM_GRAPHIC_OBJECTS         1028
#define WID_CREATE_FROM_EMBEDDED_OBJECTS        1029
#define WID_CREATE_FROM_STAR_MATH               1030

#define WID_CREATE_FROM_STAR_CHART              1032
#define WID_CREATE_FROM_STAR_CALC               1033
#define WID_CREATE_FROM_STAR_DRAW               1034
#define WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  1035
#define WID_USER_IDX_NAME                       1036
#define WID_PARA_HEAD                           1037
#define WID_PARA_SEP                            1038
#define WID_PARA_LEV1                           1039
#define WID_PARA_LEV2                           1040
#define WID_PARA_LEV3                           1041
#define WID_PARA_LEV4                           1042
#define WID_PARA_LEV5                           1043
#define WID_PARA_LEV6                           1044
#define WID_PARA_LEV7                           1045
#define WID_PARA_LEV8                           1046
#define WID_PARA_LEV9                           1047
#define WID_PARA_LEV10                          1048
#define WID_IS_COMMA_SEPARATED                  1049

//text document
#define WID_DOC_CHAR_COUNT                      1000
#define WID_DOC_PARA_COUNT                      1001
#define WID_DOC_WORD_COUNT                      1002
#define WID_DOC_WORD_SEPARATOR                  1003
#define WID_DOC_CHANGES_SHOW                    1004
#define WID_DOC_CHANGES_RECORD                  1005
#define WID_DOC_AUTO_MARK_URL                   1006
#define WID_DOC_HIDE_TIPS                       1007

//AutoText
#define WID_GROUP_PATH                          0
#define WID_GROUP_TITLE                         1

//ViewSettings
#define WID_VIEWSET_HRULER                  0
#define WID_VIEWSET_VRULER                  1
#define WID_VIEWSET_HSCROLL                 2
#define WID_VIEWSET_VSCROLL                 3
#define WID_VIEWSET_GRAPHICS                4
#define WID_VIEWSET_TABLES                  5
#define WID_VIEWSET_DRAWINGS                6
#define WID_VIEWSET_FIELD_COMMANDS          7
#define WID_VIEWSET_ANNOTATIONS             8
#define WID_VIEWSET_INDEX_MARK_BACKGROUND   9
#define WID_VIEWSET_FOOTNOTE_BACKGROUND     10
#define WID_VIEWSET_TEXT_FIELD_BACKGROUND   11
#define WID_VIEWSET_PARA_BREAKS             12
#define WID_VIEWSET_SOFT_HYPHENS            13
#define WID_VIEWSET_SPACES                  14
#define WID_VIEWSET_PROTECTED_SPACES        15
#define WID_VIEWSET_TABSTOPS                16
#define WID_VIEWSET_BREAKS                  17
#define WID_VIEWSET_HIDDEN_TEXT             18
#define WID_VIEWSET_HIDDEN_PARAGRAPHS       19
#define WID_VIEWSET_TABLE_BOUNDARIES        20
#define WID_VIEWSET_TEXT_BOUNDARIES         21
#define WID_VIEWSET_SMOOTH_SCROLLING        22
#define WID_VIEWSET_SOLID_MARK_HANDLES      23
#define WID_VIEWSET_ZOOM                    24
#define WID_VIEWSET_ZOOM_TYPE               25
#define WID_VIEWSET_ONLINE_LAYOUT           26

//PrintSettings
#define WID_PRTSET_LEFT_PAGES           0
#define WID_PRTSET_RIGHT_PAGES          1
#define WID_PRTSET_REVERSED             2
#define WID_PRTSET_PROSPECT             3
#define WID_PRTSET_GRAPHICS             4
#define WID_PRTSET_TABLES               5
#define WID_PRTSET_DRAWINGS             6
#define WID_PRTSET_CONTROLS             7
#define WID_PRTSET_PAGE_BACKGROUND      8
#define WID_PRTSET_BLACK_FONTS          9
#define WID_PRTSET_ANNOTATION_MODE      10

//NumberingRules
#define WID_IS_AUTOMATIC                0
#define WID_CONTINUOUS                  1
#define WID_RULE_NAME                   2
#define WID_IS_ABS_MARGINS              3

//TextColumns
#define WID_TXTCOL_LINE_WIDTH           0
#define WID_TXTCOL_LINE_COLOR           1
#define WID_TXTCOL_LINE_REL_HGT         2
#define WID_TXTCOL_LINE_ALIGN           3
#define WID_TXTCOL_LINE_IS_ON           4


/* -----------------04.07.98 11:41-------------------
 *
 * --------------------------------------------------*/
class SwItemPropertySet : public SfxItemPropertySet
{
protected:
    virtual sal_Bool            FillItem(SfxItemSet& rSet, sal_uInt16 nWhich, sal_Bool bGetProperty) const;
public:
    SwItemPropertySet( const SfxItemPropertyMap *pMap ) :
        SfxItemPropertySet( pMap ){}
};
/* -----------------04.07.98 11:41-------------------
 *
 * --------------------------------------------------*/
class SwUnoPropertyMapProvider
{
    SfxItemPropertyMap* aMapArr[PROPERTY_MAP_END];

    SfxItemPropertySet* pCharStyleMap;
    SfxItemPropertySet* pParaStyleMap;
    SfxItemPropertySet* pFrameStyleMap;
    SfxItemPropertySet* pPageStyleMap;
    SfxItemPropertySet* pNumStyleMap;

    void            Sort(sal_uInt16 nId);
public:
    SwUnoPropertyMapProvider();
    ~SwUnoPropertyMapProvider();

    const SfxItemPropertyMap*       GetPropertyMap(sal_uInt16 PropertyId);

    SfxItemPropertySet&             GetPropertySet(sal_Int8 nPropSetId);
};

extern SwUnoPropertyMapProvider aSwMapProvider;

#endif


