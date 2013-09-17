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
#ifndef _UNOMAP_HXX
#define _UNOMAP_HXX

#include <svl/itemprop.hxx>

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

#define PROPERTY_MAP_FLDTYP_DATETIME                    41
#define PROPERTY_MAP_FLDTYP_USER                        42
#define PROPERTY_MAP_FLDTYP_SET_EXP                     43
#define PROPERTY_MAP_FLDTYP_GET_EXP                     44
#define PROPERTY_MAP_FLDTYP_FILE_NAME                   45
#define PROPERTY_MAP_FLDTYP_PAGE_NUM                    46
#define PROPERTY_MAP_FLDTYP_AUTHOR                      47
#define PROPERTY_MAP_FLDTYP_CHAPTER                     48
#define PROPERTY_MAP_FLDTYP_GET_REFERENCE               49
#define PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT            50
#define PROPERTY_MAP_FLDTYP_HIDDEN_TEXT                 51
#define PROPERTY_MAP_FLDTYP_ANNOTATION                  52
#define PROPERTY_MAP_FLDTYP_INPUT                       53
#define PROPERTY_MAP_FLDTYP_MACRO                       54
#define PROPERTY_MAP_FLDTYP_DDE                         55
#define PROPERTY_MAP_FLDTYP_HIDDEN_PARA                 56
#define PROPERTY_MAP_FLDTYP_DOC_INFO                    57
#define PROPERTY_MAP_FLDTYP_TEMPLATE_NAME               58
#define PROPERTY_MAP_FLDTYP_USER_EXT                    59
#define PROPERTY_MAP_FLDTYP_REF_PAGE_SET                60
#define PROPERTY_MAP_FLDTYP_REF_PAGE_GET                61
#define PROPERTY_MAP_FLDTYP_JUMP_EDIT                   62
#define PROPERTY_MAP_FLDTYP_SCRIPT                      63
#define PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET           64
#define PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET            65
#define PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM            66
#define PROPERTY_MAP_FLDTYP_DATABASE                    67
#define PROPERTY_MAP_FLDTYP_DATABASE_NAME               68
#define PROPERTY_MAP_FLDTYP_DOCSTAT                     69
#define PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR              70
#define PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME           71
#define PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME    72
#define PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME    73
#define PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME           74
#define PROPERTY_MAP_FLDTYP_DOCINFO_MISC                75
#define PROPERTY_MAP_FLDTYP_DOCINFO_REVISION            76
#define PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS         77
#define PROPERTY_MAP_FLDTYP_DUMMY_0                     78
#define PROPERTY_MAP_FLDTYP_TABLE_FORMULA               79
#define PROPERTY_MAP_FLDMSTR_USER                       80
#define PROPERTY_MAP_FLDMSTR_DDE                        81
#define PROPERTY_MAP_FLDMSTR_SET_EXP                    82
#define PROPERTY_MAP_FLDMSTR_DATABASE                   83
#define PROPERTY_MAP_FLDMSTR_DUMMY0                     84
#define PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY                85
#define PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY               86
#define PROPERTY_MAP_TEXT                               87
#define PROPERTY_MAP_REDLINE_PORTION                    88
#define PROPERTY_MAP_MAILMERGE                          89
#define PROPERTY_MAP_FLDTYP_DROPDOWN                    90
#define PROPERTY_MAP_CHART2_DATA_SEQUENCE               91
#define PROPERTY_MAP_TEXT_VIEW                          92
#define PROPERTY_MAP_CONDITIONAL_PARA_STYLE             93
#define PROPERTY_MAP_CHAR_AUTO_STYLE                    94
#define PROPERTY_MAP_RUBY_AUTO_STYLE                    95
#define PROPERTY_MAP_PARA_AUTO_STYLE                    96
#define PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM              97
#define PROPERTY_MAP_METAFIELD                          98
#define PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE       99
#define PROPERTY_MAP_END                                100

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
#define WID_SECT_CURRENTLY_VISIBLE 8012
#define WID_SECT_EDIT_IN_READONLY 8013


// Indices/Tables of content
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
#define WID_USE_ALPHABETICAL_SEPARATORS         1010
#define WID_USE_KEY_AS_ENTRY                    1011
#define WID_USE_COMBINED_ENTRIES                1012
#define WID_IS_CASE_SENSITIVE                   1013
#define WID_USE_P_P                             1014
#define WID_USE_DASH                            1015
#define WID_USE_UPPER_CASE                      1016
#define WID_LABEL_CATEGORY                      1018
#define WID_LABEL_DISPLAY_TYPE                  1019
#define WID_USE_LEVEL_FROM_SOURCE               1020
#define WID_LEVEL_FORMAT                        1021
#define WID_LEVEL_PARAGRAPH_STYLES              1022
#define WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     1025
#define WID_CREATE_FROM_TABLES                  1026
#define WID_CREATE_FROM_TEXT_FRAMES             1027
#define WID_CREATE_FROM_GRAPHIC_OBJECTS         1028
#define WID_CREATE_FROM_EMBEDDED_OBJECTS        1029
#define WID_CREATE_FROM_STAR_MATH               1030
#define WID_PROTECTED                           1031

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

#define WID_TEXT_READING                        1058
#define WID_PRIMARY_KEY_READING                 1059
#define WID_SECONDARY_KEY_READING               1060

// Text document
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
#define WID_DOC_BASIC_LIBRARIES                 1014
#define WID_DOC_RUNTIME_UID                     1015
#define WID_DOC_LOCK_UPDATES                    1016
#define WID_DOC_HAS_VALID_SIGNATURES            1017
#define WID_DOC_INTEROP_GRAB_BAG                1018
#define WID_DOC_BUILDID                         1024
#define WID_DOC_ISTEMPLATEID                    1025
#define WID_DOC_DEFAULT_PAGE_MODE               1069

// MailMerge
#define WID_SELECTION                           1071
#define WID_RESULT_SET                          1072
#define WID_CONNECTION                          1073
#define WID_MODEL                               1074
#define WID_DATA_SOURCE_NAME                    1075
#define WID_DATA_COMMAND                        1076
#define WID_FILTER                              1077
#define WID_DOCUMENT_URL                        1078
#define WID_OUTPUT_URL                          1079
#define WID_DATA_COMMAND_TYPE                   1080
#define WID_OUTPUT_TYPE                         1081
#define WID_ESCAPE_PROCESSING                   1082
#define WID_SINGLE_PRINT_JOBS                   1083
#define WID_FILE_NAME_FROM_COLUMN               1084
#define WID_FILE_NAME_PREFIX                    1085
#define WID_MAIL_SUBJECT                        1086
#define WID_ADDRESS_FROM_COLUMN                 1087
#define WID_SEND_AS_HTML                        1088
#define WID_SEND_AS_ATTACHMENT                  1089
#define WID_MAIL_BODY                           1090
#define WID_ATTACHMENT_NAME                     1091
#define WID_ATTACHMENT_FILTER                   1092
#define WID_PRINT_OPTIONS                       1093
#define WID_SAVE_AS_SINGLE_FILE                 1094
#define WID_SAVE_FILTER                         1095
#define WID_COPIES_TO                           1096
#define WID_BLIND_COPIES_TO                     1097
#define WID_IN_SERVER_PASSWORD                  1098
#define WID_OUT_SERVER_PASSWORD                 1099
#define WID_SAVE_FILTER_OPTIONS                 1100
#define WID_SAVE_FILTER_DATA                    1101

#define WID_PAGE_COUNT                          1100
#define WID_LINE_COUNT                          1101
#define WID_IS_CONSTANT_SPELLCHECK              1102
#define WID_IS_HIDE_SPELL_MARKS                 1103    /* deprecated #i91949 */

#define WID_LAYOUT_SIZE                         1104
#define WID_DOC_DIALOG_LIBRARIES                1105
#define WID_DOC_VBA_DOCOBJ                      1106


// AutoText
#define WID_GROUP_PATH                          0
#define WID_GROUP_TITLE                         1

// NumberingRules
#define WID_IS_AUTOMATIC                0
#define WID_CONTINUOUS                  1
#define WID_RULE_NAME                   2
#define WID_IS_ABS_MARGINS              3
#define WID_IS_OUTLINE                  4
#define WID_DEFAULT_LIST_ID             5

// TextColumns
#define WID_TXTCOL_LINE_WIDTH           0
#define WID_TXTCOL_LINE_COLOR           1
#define WID_TXTCOL_LINE_REL_HGT         2
#define WID_TXTCOL_LINE_ALIGN           3
#define WID_TXTCOL_LINE_IS_ON           4
#define WID_TXTCOL_IS_AUTOMATIC         5
#define WID_TXTCOL_AUTO_DISTANCE        6
#define WID_TXTCOL_LINE_STYLE           7

class SwItemPropertySet : public SfxItemPropertySet
{
protected:
    virtual sal_Bool            FillItem(SfxItemSet& rSet, sal_uInt16 nWhich, sal_Bool bGetProperty) const;
public:
    SwItemPropertySet( const SfxItemPropertyMapEntry *pMap ) :
        SfxItemPropertySet( pMap ){}
};
class SwUnoPropertyMapProvider
{
    SfxItemPropertyMapEntry*        aMapEntriesArr[PROPERTY_MAP_END];
    SfxItemPropertySet*             aPropertySetArr[PROPERTY_MAP_END];

public:
    SwUnoPropertyMapProvider();
    ~SwUnoPropertyMapProvider();

    const SfxItemPropertyMapEntry*  GetPropertyMapEntries(sal_uInt16 PropertyId);
    const SfxItemPropertySet*       GetPropertySet( sal_uInt16 PropertyId );

};

extern SwUnoPropertyMapProvider aSwMapProvider;
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
