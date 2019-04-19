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

#define EPP_Document                    1000
#define EPP_DocumentAtom                1001
#define EPP_EndDocument                 1002
#define EPP_Slide                       1006
#define EPP_SlideAtom                   1007
#define EPP_Notes                       1008
#define EPP_NotesAtom                   1009
#define EPP_Environment                 1010
#define EPP_SlidePersistAtom            1011    //0x03F3
#define EPP_MainMaster                  1016
#define EPP_SSSlideInfoAtom             1017
#define EPP_SlideViewInfo               1018
#define EPP_GuideAtom                   1019
#define EPP_ViewInfoAtom                1021
#define EPP_SlideViewInfoAtom           1022
#define EPP_VBAInfo                     1023
#define EPP_VBAInfoAtom                 1024
#define EPP_SSDocInfoAtom               1025
#define EPP_OutlineViewInfo             1031
#define EPP_ExObjList                   1033
#define EPP_ExObjListAtom               1034
#define EPP_PPDrawingGroup              1035
#define EPP_PPDrawing                   1036
#define EPP_NamedShows                  1040
#define EPP_NamedShow                   1041
#define EPP_NamedShowSlides             1042
#define EPP_List                        2000
#define EPP_FontCollection              2005
#define EPP_SoundCollection             2020
#define EPP_SoundCollAtom               2021
#define EPP_Sound                       2022
#define EPP_SoundData                   2023
#define EPP_ColorSchemeAtom             2032

// these atoms first was seen in ppt2000 in a private Tag atom
#define EPP_PST_ExtendedBuGraContainer          2040    // consist of 4041

#define EPP_ExObjRefAtom                3009
#define EPP_OEPlaceholderAtom           3011
#define EPP_TextHeaderAtom              3999
#define EPP_TextCharsAtom               4000
#define EPP_StyleTextPropAtom           4001
#define EPP_BaseTextPropAtom            4002
#define EPP_TxMasterStyleAtom           4003
#define EPP_TxCFStyleAtom               4004
#define EPP_TextRulerAtom               4006
#define EPP_TxSIStyleAtom               4009
#define EPP_TextSpecInfoAtom            4010

// these atoms first was seen in ppt2000 in a private Tag atom
#define EPP_PST_ExtendedParagraphAtom           4012
#define EPP_PST_ExtendedParagraphMasterAtom     4013
#define EPP_PST_ExtendedPresRuleContainer       4014    // consist of 4012, 4015,
#define EPP_PST_ExtendedParagraphHeaderAtom     4015    // the instance of this atom indices the current presobj
                                                        // the first sal_uInt32 in this atom indices the current slideId

#define EPP_FontEnityAtom               4023
#define EPP_CString                     4026
#define EPP_ExOleObjAtom                4035
#define EPP_SrKinsoku                   4040
#define EPP_ExEmbed                     4044
#define EPP_ExEmbedAtom                 4045
#define EPP_SrKinsokuAtom               4050
#define EPP_ExHyperlinkAtom             4051
#define EPP_ExHyperlink                 4055
#define EPP_SlideNumberMCAtom           4056
#define EPP_HeadersFooters              4057
#define EPP_HeadersFootersAtom          4058
#define EPP_TxInteractiveInfoAtom       4063
#define EPP_ExControl                   4078
#define EPP_ExControlAtom               4091
#define EPP_SlideListWithText           4080    // 0x0FF0
#define EPP_AnimationInfoAtom           4081
#define EPP_InteractiveInfo             4082
#define EPP_InteractiveInfoAtom         4083
#define EPP_UserEditAtom                4085
#define EPP_CurrentUserAtom             4086
#define EPP_DateTimeMCAtom              4087
#define EPP_GenericDateMCAtom           4088
#define EPP_HeaderMCAtom                4089
#define EPP_FooterMCAtom                4090
#define EPP_ExMediaAtom                 4100
#define EPP_ExVideo                     4101
#define EPP_ExMCIMovie                  4103
#define EPP_ExOleObjStg                 4113
#define EPP_AnimationInfo               4116
#define EPP_ProgTags                    5000
#define EPP_ProgBinaryTag               5002
#define EPP_BinaryTagData               5003
#define EPP_PersistPtrIncrementalBlock  6002
#define EPP_Comment10                   12000
#define EPP_CommentAtom10               12001

#define EPP_PLACEHOLDER_NONE                    0   //  0 None
#define EPP_PLACEHOLDER_MASTERTITLE             1   //  1 Master title
#define EPP_PLACEHOLDER_MASTERBODY              2   //  2 Master body
#define EPP_PLACEHOLDER_MASTERSUBTITLE          4   // 10 Master subtitle
#define EPP_PLACEHOLDER_MASTERNOTESSLIDEIMAGE   5   //  4 Master notes slide image
#define EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE    6   //  5 Master notes body image
#define EPP_PLACEHOLDER_MASTERDATE              7   //  6 Master date
#define EPP_PLACEHOLDER_MASTERSLIDENUMBER       8   //  7 Master slide number
#define EPP_PLACEHOLDER_MASTERFOOTER            9   //  8 Master footer
#define EPP_PLACEHOLDER_MASTERHEADER            10  //  9 Master header
#define EPP_PLACEHOLDER_GENERICTEXTOBJECT           // 11 Generic text object
#define EPP_PLACEHOLDER_NOTESBODY               12  // 14 Notes body
#define EPP_PLACEHOLDER_NOTESSLIDEIMAGE         11  // 19 Notes slide image

#define EPP_TEXTTYPE_Title              0
#define EPP_TEXTTYPE_Body               1
#define EPP_TEXTTYPE_Notes              2
#define EPP_TEXTTYPE_notUsed            3
#define EPP_TEXTTYPE_Other              4   // ( Text in a shape )
#define EPP_TEXTTYPE_CenterBody         5   // ( subtitle in title slide )
#define EPP_TEXTTYPE_CenterTitle        6   // ( title in title slide )
#define EPP_TEXTTYPE_HalfBody           7   // ( body in two-column slide )
#define EPP_TEXTTYPE_QuarterBody        8   // ( body in four-body slide )

#define EPP_SLIDESIZE_TYPEONSCREEN          0
#define EPP_SLIDESIZE_TYPEA4PAPER           2
#define EPP_SLIDESIZE_TYPE35MM              3
#define EPP_SLIDESIZE_TYPEBANNER            5
#define EPP_SLIDESIZE_TYPECUSTOM            6

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
