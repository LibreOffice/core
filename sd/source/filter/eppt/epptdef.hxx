/*************************************************************************
 *
 *  $RCSfile: epptdef.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:45 $
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

#ifndef _EPPT_DEF_HXX
#define _EPPT_DEF_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#define EPP_FLAG_CONTAINER 0x0F             // If the version field of a record
                                            // header takes on this value, the
                                            // record header marks the start of
                                            // a container.

#define EPP_Unknown                        0
#define EPP_SubContainerCompleted          1
#define EPP_IRRAtom                        2
#define EPP_PSS                            3
#define EPP_SubContainerException          4
#define EPP_ClientSignal1                  6
#define EPP_ClientSignal2                  7
#define EPP_PowerPointStateInfoAtom       10
#define EPP_Document                    1000
#define EPP_DocumentAtom                1001
#define EPP_EndDocument                 1002
#define EPP_SlidePersist                1003
#define EPP_SlideBase                   1004
#define EPP_SlideBaseAtom               1005
#define EPP_Slide                       1006
#define EPP_SlideAtom                   1007
#define EPP_Notes                       1008
#define EPP_NotesAtom                   1009
#define EPP_Environment                 1010
#define EPP_SlidePersistAtom            1011    //0x03F3
#define EPP_Scheme                      1012
#define EPP_SchemeAtom                  1013
#define EPP_DocViewInfo                 1014
#define EPP_SslideLayoutAtom            1015
#define EPP_MainMaster                  1016
#define EPP_SSSlideInfoAtom             1017
#define EPP_SlideViewInfo               1018
#define EPP_GuideAtom                   1019
#define EPP_ViewInfo                    1020
#define EPP_ViewInfoAtom                1021
#define EPP_SlideViewInfoAtom           1022
#define EPP_VBAInfo                     1023
#define EPP_VBAInfoAtom                 1024
#define EPP_SSDocInfoAtom               1025
#define EPP_Summary                     1026
#define EPP_Texture                     1027
#define EPP_VBASlideInfo                1028
#define EPP_VBASlideInfoAtom            1029
#define EPP_DocRoutingSlip              1030
#define EPP_OutlineViewInfo             1031
#define EPP_SorterViewInfo              1032
#define EPP_ExObjList                   1033
#define EPP_ExObjListAtom               1034
#define EPP_PPDrawingGroup              1035
#define EPP_PPDrawing                   1036
#define EPP_NamedShows                  1040
#define EPP_NamedShow                   1041
#define EPP_NamedShowSlides             1042
#define EPP_List                        2000
#define EPP_FontCollection              2005
#define EPP_ListPlaceholder             2017
#define EPP_BookmarkCollection          2019
#define EPP_SoundCollection             2020
#define EPP_SoundCollAtom               2021
#define EPP_Sound                       2022
#define EPP_SoundData                   2023
#define EPP_BookmarkSeedAtom            2025
#define EPP_GuideList                   2026
#define EPP_RunArray                    2028
#define EPP_RunArrayAtom                2029
#define EPP_ArrayElementAtom            2030
#define EPP_Int4ArrayAtom               2031
#define EPP_ColorSchemeAtom             2032

// these atoms first was seen in ppt2000 in a private Tag atom
#define EPP_PST_ExtendedBuGraContainer          2040    // consist of 4041
#define EPP_PST_ExtendedBuGraAtom               2041    // the instance of this atom indices the current graphic

#define EPP_OEShape                     3008
#define EPP_ExObjRefAtom                3009
#define EPP_OEPlaceholderAtom           3011
#define EPP_GrColor                     3020
#define EPP_GrectAtom                   3025
#define EPP_GratioAtom                  3031
#define EPP_Gscaling                    3032
#define EPP_GpointAtom                  3034
#define EPP_OEShapeAtom                 3035
#define EPP_OutlineTextRefAtom          3998
#define EPP_TextHeaderAtom              3999
#define EPP_TextCharsAtom               4000
#define EPP_StyleTextPropAtom           4001
#define EPP_BaseTextPropAtom            4002
#define EPP_TxMasterStyleAtom           4003
#define EPP_TxCFStyleAtom               4004
#define EPP_TxPFStyleAtom               4005
#define EPP_TextRulerAtom               4006
#define EPP_TextBookmarkAtom            4007
#define EPP_TextBytesAtom               4008
#define EPP_TxSIStyleAtom               4009
#define EPP_TextSpecInfoAtom            4010
#define EPP_DefaultRulerAtom            4011

// these atoms first was seen in ppt2000 in a private Tag atom
#define EPP_PST_ExtendedParagraphAtom           4012
#define EPP_PST_ExtendedParagraphMasterAtom     4013
#define EPP_PST_ExtendedPresRuleContainer       4014    // consist of 4012, 4015,
#define EPP_PST_ExtendedParagraphHeaderAtom     4015    // the instance of this atom indices the current presobj
                                                        // the first UINT32 in this atom indices the current slideId

#define EPP_FontEnityAtom               4023
#define EPP_FontEmbedData               4024
#define EPP_TypeFace                    4025
#define EPP_CString                     4026
#define EPP_ExternalObject              4027
#define EPP_MetaFile                    4033
#define EPP_ExOleObj                    4034
#define EPP_ExOleObjAtom                4035
#define EPP_ExPlainLinkAtom             4036
#define EPP_CorePict                    4037
#define EPP_CorePictAtom                4038
#define EPP_ExPlainAtom                 4039
#define EPP_SrKinsoku                   4040
#define EPP_Handout                     4041
#define EPP_ExEmbed                     4044
#define EPP_ExEmbedAtom                 4045
#define EPP_ExLink                      4046
#define EPP_ExLinkAtom_old              4047
#define EPP_BookmarkEntityAtom          4048
#define EPP_ExLinkAtom                  4049
#define EPP_SrKinsokuAtom               4050
#define EPP_ExHyperlinkAtom             4051
#define EPP_ExPlain                     4053
#define EPP_ExPlainLink                 4054
#define EPP_ExHyperlink                 4055
#define EPP_SlideNumberMCAtom           4056
#define EPP_HeadersFooters              4057
#define EPP_HeadersFootersAtom          4058
#define EPP_RecolorEntryAtom            4062
#define EPP_TxInteractiveInfoAtom       4063
#define EPP_EmFormatAtom                4065
#define EPP_CharFormatAtom              4066
#define EPP_ParaFormatAtom              4067
#define EPP_MasterText                  4068
#define EPP_RecolorInfoAtom             4071
#define EPP_ExQuickTime                 4073
#define EPP_ExQuickTimeMovie            4074
#define EPP_ExQuickTimeMovieData        4075
#define EPP_ExSubscription              4076
#define EPP_ExSubscriptionSection       4077
#define EPP_ExControl                   4078
#define EPP_ExControlAtom               4091
#define EPP_SlideListWithText           4080    // 0x0FF0
#define EPP_AnimationInfoAtom           4081
#define EPP_InteractiveInfo             4082
#define EPP_InteractiveInfoAtom         4083
#define EPP_SlideList                   4084
#define EPP_UserEditAtom                4085
#define EPP_CurrentUserAtom             4086
#define EPP_DateTimeMCAtom              4087
#define EPP_GenericDateMCAtom           4088
#define EPP_HeaderMCAtom                4089
#define EPP_FooterMCAtom                4090
#define EPP_ExMediaAtom                 4100
#define EPP_ExVideo                     4101
#define EPP_ExAviMovie                  4102
#define EPP_ExMCIMovie                  4103
#define EPP_ExMIDIAudio                 4109
#define EPP_ExCDAudio                   4110
#define EPP_ExWAVAudioEmbedded          4111
#define EPP_ExWAVAudioLink              4112
#define EPP_ExOleObjStg                 4113
#define EPP_ExCDAudioAtom               4114
#define EPP_ExWAVAudioEmbeddedAtom      4115
#define EPP_AnimationInfo               4116
#define EPP_RTFDateTimeMCAtom           4117
#define EPP_ProgTags                    5000
#define EPP_ProgStringTag               5001
#define EPP_ProgBinaryTag               5002
#define EPP_BinaryTagData               5003
#define EPP_PrintOptions                6000
#define EPP_PersistPtrFullBlock         6001
#define EPP_PersistPtrIncrementalBlock  6002
#define EPP_RulerIndentAtom            10000
#define EPP_GscalingAtom               10001
#define EPP_GrColorAtom                10002
#define EPP_GLPointAtom                10003

#define EPP_PLACEHOLDER_NONE                  0 /*  0 None                        */
#define EPP_PLACEHOLDER_MASTERTITLE           1 /*  1 Master title                */
#define EPP_PLACEHOLDER_MASTERBODY            2 /*  2 Master body                 */
#define EPP_PLACEHOLDER_MASTERCENTEREDTITLE   3 /*  3 Master centered title       */
#define EPP_PLACEHOLDER_MASTERNOTESSLIDEIMAGE 4 /*  4 Master notes slide image    */
#define EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE  5 /*  5 Master notes body image     */
#define EPP_PLACEHOLDER_MASTERDATE            6 /*  6 Master date                 */
#define EPP_PLACEHOLDER_MASTERSLIDENUMBER     7 /*  7 Master slide number         */
#define EPP_PLACEHOLDER_MASTERFOOTER          8 /*  8 Master footer               */
#define EPP_PLACEHOLDER_MASTERHEADER          9 /*  9 Master header               */
#define EPP_PLACEHOLDER_MASTERSUBTITLE       10 /* 10 Master subtitle             */
#define EPP_PLACEHOLDER_GENERICTEXTOBJECT    11 /* 11 Generic text object         */
#define EPP_PLACEHOLDER_TITLE                13 /* 12 Title                       */
#define EPP_PLACEHOLDER_BODY                 14 /* 13 Body                        */
#define EPP_PLACEHOLDER_NOTESBODY            12 /* 14 Notes body                  */
#define EPP_PLACEHOLDER_CENTEREDTITLE        15 /* 15 Centered title              */
#define EPP_PLACEHOLDER_SUBTITLE             16 /* 16 Subtitle                    */
#define EPP_PLACEHOLDER_VERTICALTEXTTITLE    17 /* 17 Vertical text title         */
#define EPP_PLACEHOLDER_VERTICALTEXTBODY     18 /* 18 Vertical text body          */
#define EPP_PLACEHOLDER_NOTESSLIDEIMAGE      19 /* 19 Notes slide image           */
#define EPP_PLACEHOLDER_OBJECT               20 /* 20 Object (no matter the size) */
#define EPP_PLACEHOLDER_GRAPH                21 /* 21 Graph                       */
#define EPP_PLACEHOLDER_TABLE                22 /* 22 Table                       */
#define EPP_PLACEHOLDER_CLIPART              23 /* 23 Clip Art                    */
#define EPP_PLACEHOLDER_ORGANISZATIONCHART   24 /* 24 Organization Chart          */
#define EPP_PLACEHOLDER_MEDIACLIP            25 /* 25 Media Clip                  */

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
#define EPP_SLIDESIZE_TYPELETTERSIZERPAPER  1
#define EPP_SLIDESIZE_TYPEA4PAPER           2
#define EPP_SLIDESIZE_TYPE35MM              3
#define EPP_SLIDESIZE_TYPEOVERHEAD          4
#define EPP_SLIDESIZE_TYPEBANNER            5
#define EPP_SLIDESIZE_TYPECUSTOM            6

#endif
