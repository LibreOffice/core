/*************************************************************************
 *
 *  $RCSfile: unomap.hxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: dvo $ $Date: 2001-08-23 09:38:30 $
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
#define PROPERTY_MAP_TEXTPORTION_EXTENSIONS 34
#define PROPERTY_MAP_FOOTNOTE               35
#define PROPERTY_MAP_TEXT_COLUMS            36
#define PROPERTY_MAP_PARAGRAPH              37
#define PROPERTY_MAP_EMBEDDED_OBJECT        38
#define PROPERTY_MAP_REDLINE                39
#define PROPERTY_MAP_TEXT_DEFAULT           40
#define PROPERTY_MAP_END                    41

#define PROPERTY_MAP_FLDTYP_DATETIME                    42
#define PROPERTY_MAP_FLDTYP_USER                        43
#define PROPERTY_MAP_FLDTYP_SET_EXP                     44
#define PROPERTY_MAP_FLDTYP_GET_EXP                     45
#define PROPERTY_MAP_FLDTYP_FILE_NAME                   46
#define PROPERTY_MAP_FLDTYP_PAGE_NUM                    47
#define PROPERTY_MAP_FLDTYP_AUTHOR                      48
#define PROPERTY_MAP_FLDTYP_CHAPTER                     49
#define PROPERTY_MAP_FLDTYP_GET_REFERENCE               50
#define PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT            51
#define PROPERTY_MAP_FLDTYP_HIDDEN_TEXT                 52
#define PROPERTY_MAP_FLDTYP_ANNOTATION                  53
#define PROPERTY_MAP_FLDTYP_INPUT_USER                  54
#define PROPERTY_MAP_FLDTYP_INPUT                       55
#define PROPERTY_MAP_FLDTYP_MACRO                       56
#define PROPERTY_MAP_FLDTYP_DDE                         57
#define PROPERTY_MAP_FLDTYP_HIDDEN_PARA                 58
#define PROPERTY_MAP_FLDTYP_DOC_INFO                    59
#define PROPERTY_MAP_FLDTYP_TEMPLATE_NAME               60
#define PROPERTY_MAP_FLDTYP_USER_EXT                    61
#define PROPERTY_MAP_FLDTYP_REF_PAGE_SET                62
#define PROPERTY_MAP_FLDTYP_REF_PAGE_GET                63
#define PROPERTY_MAP_FLDTYP_JUMP_EDIT                   64
#define PROPERTY_MAP_FLDTYP_SCRIPT                      65
#define PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET           66
#define PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET            67
#define PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM            68
#define PROPERTY_MAP_FLDTYP_DATABASE                    69
#define PROPERTY_MAP_FLDTYP_DATABASE_NAME               70
#define PROPERTY_MAP_FLDTYP_PAGE_COUNT                  71
#define PROPERTY_MAP_FLDTYP_PARAGRAPH_COUNT             72
#define PROPERTY_MAP_FLDTYP_WORD_COUNT                  73
#define PROPERTY_MAP_FLDTYP_CHARACTER_COUNT             74
#define PROPERTY_MAP_FLDTYP_TABLE_COUNT                 75
#define PROPERTY_MAP_FLDTYP_GRAPHIC_OBJECT_COUNT        76
#define PROPERTY_MAP_FLDTYP_EMBEDDED_OBJECT_COUNT       77
#define PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_AUTHOR       78
#define PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_AUTHOR       79
#define PROPERTY_MAP_FLDTYP_DOCINFO_PRINT_AUTHOR        80
#define PROPERTY_MAP_FLDTYP_DOCINFO_PRINT_DATE_TIME     81
#define PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME    82
#define PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME    83
#define PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME           84
#define PROPERTY_MAP_FLDTYP_DOCINFO_DESCRIPTION         85
#define PROPERTY_MAP_FLDTYP_DOCINFO_INFO_0              86
#define PROPERTY_MAP_FLDTYP_DOCINFO_INFO_1              87
#define PROPERTY_MAP_FLDTYP_DOCINFO_INFO_2              88
#define PROPERTY_MAP_FLDTYP_DOCINFO_INFO_3              89
#define PROPERTY_MAP_FLDTYP_DOCINFO_KEY_WORDS           90
#define PROPERTY_MAP_FLDTYP_DOCINFO_SUBJECT             91
#define PROPERTY_MAP_FLDTYP_DOCINFO_TITLE               92
#define PROPERTY_MAP_FLDTYP_DOCINFO_REVISION            93
#define PROPERTY_MAP_FLDTYP_DUMMY_0                     94
#define PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS         95
#define PROPERTY_MAP_FLDTYP_DUMMY_3                     96
#define PROPERTY_MAP_FLDTYP_DUMMY_4                     97
#define PROPERTY_MAP_FLDTYP_DUMMY_5                     98
#define PROPERTY_MAP_FLDTYP_DUMMY_6                     99
#define PROPERTY_MAP_FLDTYP_DUMMY_7                    100
#define PROPERTY_MAP_FLDTYP_DUMMY_8                    101
#define PROPERTY_MAP_FLDTYP_TABLEFIELD                 102
#define PROPERTY_MAP_FLDMSTR_USER                      103
#define PROPERTY_MAP_FLDMSTR_DDE                       104
#define PROPERTY_MAP_FLDMSTR_SET_EXP                   105
#define PROPERTY_MAP_FLDMSTR_DATABASE                  106
#define PROPERTY_MAP_FLDMSTR_DUMMY2                    107
#define PROPERTY_MAP_FLDMSTR_DUMMY3                    108
#define PROPERTY_MAP_FLDMSTR_DUMMY4                    109
#define PROPERTY_MAP_FLDMSTR_DUMMY5                    110
#define PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY               111
#define PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY              112


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
#define WID_SECT_CONDITION   8000
#define WID_SECT_DDE_TYPE    8001
#define WID_SECT_DDE_FILE    8002
#define WID_SECT_DDE_ELEMENT 8003
#define WID_SECT_LINK        8004
#define WID_SECT_VISIBLE     8005
#define WID_SECT_PROTECTED   8006
#define WID_SECT_REGION      8007
#define WID_SECT_DDE_AUTOUPDATE 8008
#define WID_SECT_DOCUMENT_INDEX 8009
#define WID_SECT_IS_GLOBAL_DOC_SECTION 8010
#define WID_SECT_PASSWORD    8011

//Verzeichnisse
#define WID_PRIMARY_KEY                         1000
#define WID_SECONDARY_KEY                       1001
#define WID_ALT_TEXT                            1002
#define WID_IDX_TITLE                           1003
#define WID_LEVEL                               1004
#define WID_CREATE_FROM_MARKS                   1005
#define WID_CREATE_FROM_OUTLINE                 1006
#define WID_MAIN_ENTRY                          1007
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
#define WID_INDEX_MARKS                         1050
#define WID_IS_RELATIVE_TABSTOPS                1051
#define WID_CREATE_FROM_PARAGRAPH_STYLES        1052
#define WID_IDX_CONTENT_SECTION                 1053
#define WID_IDX_HEADER_SECTION                  1054
#define WID_IDX_LOCALE                          1055
#define WID_IDX_SORT_ALGORITHM                  1056
#define WID_IDX_NAME                            1057

//text document
#define WID_DOC_CHAR_COUNT                      1000
#define WID_DOC_PARA_COUNT                      1001
#define WID_DOC_WORD_COUNT                      1002
#define WID_DOC_WORD_SEPARATOR                  1003
#define WID_DOC_CHANGES_SHOW                    1004
#define WID_DOC_CHANGES_RECORD                  1005
#define WID_DOC_AUTO_MARK_URL                   1006
#define WID_DOC_HIDE_TIPS                       1007
#define WID_DOC_REDLINE_DISPLAY                 1008
#define WID_DOC_FORBIDDEN_CHARS                 1009
#define WID_DOC_CHANGES_PASSWORD                1010
#define WID_DOC_TWO_DIGIT_YEAR                  1011
#define WID_DOC_AUTOMATIC_CONTROL_FOCUS         1012
#define WID_DOC_APPLY_FORM_DESIGN_MODE          1013


//AutoText
#define WID_GROUP_PATH                          0
#define WID_GROUP_TITLE                         1

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
#define WID_TXTCOL_IS_AUTOMATIC         5
#define WID_TXTCOL_AUTO_DISTANCE        6

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
