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

#ifndef _MSDFFDEF_HXX
#define _MSDFFDEF_HXX


#include <tools/solar.h>
#include <tools/stream.hxx>
#include <tools/table.hxx>
#include <tools/string.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

#define DFF_COMMON_RECORD_HEADER_SIZE           8

#define DFF_PSFLAG_CONTAINER 0x0F         // If the version field of a record
                                          //  header takes on this value, the
                                          //  record header marks the start of
                                          //  a container.

#define DFF_PST_Unknown                        0
#define DFF_PST_SubContainerCompleted          1
#define DFF_PST_IRRAtom                        2
#define DFF_PST_PSS                            3
#define DFF_PST_SubContainerException          4
#define DFF_PST_ClientSignal1                  6
#define DFF_PST_ClientSignal2                  7
#define DFF_PST_PowerPointStateInfoAtom       10
#define DFF_PST_Document                    1000
#define DFF_PST_DocumentAtom                1001
#define DFF_PST_EndDocument                 1002
#define DFF_PST_SlidePersist                1003
#define DFF_PST_SlideBase                   1004
#define DFF_PST_SlideBaseAtom               1005
#define DFF_PST_Slide                       1006
#define DFF_PST_SlideAtom                   1007
#define DFF_PST_Notes                       1008
#define DFF_PST_NotesAtom                   1009
#define DFF_PST_Environment                 1010
#define DFF_PST_SlidePersistAtom            1011
#define DFF_PST_Scheme                      1012
#define DFF_PST_SchemeAtom                  1013
#define DFF_PST_DocViewInfo                 1014
#define DFF_PST_SslideLayoutAtom            1015
#define DFF_PST_MainMaster                  1016
#define DFF_PST_SSSlideInfoAtom             1017
#define DFF_PST_SlideViewInfo               1018
#define DFF_PST_GuideAtom                   1019
#define DFF_PST_ViewInfo                    1020
#define DFF_PST_ViewInfoAtom                1021
#define DFF_PST_SlideViewInfoAtom           1022
#define DFF_PST_VBAInfo                     1023
#define DFF_PST_VBAInfoAtom                 1024
#define DFF_PST_SSDocInfoAtom               1025
#define DFF_PST_Summary                     1026
#define DFF_PST_Texture                     1027
#define DFF_PST_VBASlideInfo                1028
#define DFF_PST_VBASlideInfoAtom            1029
#define DFF_PST_DocRoutingSlip              1030
#define DFF_PST_OutlineViewInfo             1031
#define DFF_PST_SorterViewInfo              1032
#define DFF_PST_ExObjList                   1033
#define DFF_PST_ExObjListAtom               1034
#define DFF_PST_PPDrawingGroup              1035
#define DFF_PST_PPDrawing                   1036
#define DFF_PST_NamedShows                  1040
#define DFF_PST_NamedShow                   1041
#define DFF_PST_NamedShowSlides             1042
#define DFF_PST_List                        2000
#define DFF_PST_FontCollection              2005
#define DFF_PST_ListPlaceholder             2017
#define DFF_PST_BookmarkCollection          2019
#define DFF_PST_SoundCollection             2020
#define DFF_PST_SoundCollAtom               2021
#define DFF_PST_Sound                       2022
#define DFF_PST_SoundData                   2023
#define DFF_PST_BookmarkSeedAtom            2025
#define DFF_PST_GuideList                   2026
#define DFF_PST_RunArray                    2028
#define DFF_PST_RunArrayAtom                2029
#define DFF_PST_ArrayElementAtom            2030
#define DFF_PST_Int4ArrayAtom               2031
#define DFF_PST_ColorSchemeAtom             2032
#define DFF_PST_OEShape                     3008
#define DFF_PST_ExObjRefAtom                3009
#define DFF_PST_OEPlaceholderAtom           3011
#define DFF_PST_GrColor                     3020
#define DFF_PST_GrectAtom                   3025
#define DFF_PST_GratioAtom                  3031
#define DFF_PST_Gscaling                    3032
#define DFF_PST_GpointAtom                  3034
#define DFF_PST_OEShapeAtom                 3035
#define DFF_PST_OutlineTextRefAtom          3998
#define DFF_PST_TextHeaderAtom              3999
#define DFF_PST_TextCharsAtom               4000
#define DFF_PST_StyleTextPropAtom           4001
#define DFF_PST_BaseTextPropAtom            4002
#define DFF_PST_TxMasterStyleAtom           4003
#define DFF_PST_TxCFStyleAtom               4004
#define DFF_PST_TxPFStyleAtom               4005
#define DFF_PST_TextRulerAtom               4006
#define DFF_PST_TextBookmarkAtom            4007
#define DFF_PST_TextBytesAtom               4008
#define DFF_PST_TxSIStyleAtom               4009
#define DFF_PST_TextSpecInfoAtom            4010
#define DFF_PST_DefaultRulerAtom            4011
#define DFF_PST_FontEntityAtom              4023
#define DFF_PST_FontEmbedData               4024
#define DFF_PST_TypeFace                    4025
#define DFF_PST_CString                     4026
#define DFF_PST_ExternalObject              4027
#define DFF_PST_MetaFile                    4033
#define DFF_PST_ExOleObj                    4034
#define DFF_PST_ExOleObjAtom                4035
#define DFF_PST_ExPlainLinkAtom             4036
#define DFF_PST_CorePict                    4037
#define DFF_PST_CorePictAtom                4038
#define DFF_PST_ExPlainAtom                 4039
#define DFF_PST_SrKinsoku                   4040
#define DFF_PST_Handout                     4041
#define DFF_PST_ExEmbed                     4044
#define DFF_PST_ExEmbedAtom                 4045
#define DFF_PST_ExLink                      4046
#define DFF_PST_ExLinkAtom_old              4047
#define DFF_PST_BookmarkEntityAtom          4048
#define DFF_PST_ExLinkAtom                  4049
#define DFF_PST_SrKinsokuAtom               4050
#define DFF_PST_ExHyperlinkAtom             4051
#define DFF_PST_ExPlain                     4053
#define DFF_PST_ExPlainLink                 4054
#define DFF_PST_ExHyperlink                 4055
#define DFF_PST_SlideNumberMCAtom           4056
#define DFF_PST_HeadersFooters              4057
#define DFF_PST_HeadersFootersAtom          4058
#define DFF_PST_RecolorEntryAtom            4062
#define DFF_PST_TxInteractiveInfoAtom       4063
#define DFF_PST_EmFormatAtom                4065
#define DFF_PST_CharFormatAtom              4066
#define DFF_PST_ParaFormatAtom              4067
#define DFF_PST_MasterText                  4068
#define DFF_PST_RecolorInfoAtom             4071
#define DFF_PST_ExQuickTime                 4073
#define DFF_PST_ExQuickTimeMovie            4074
#define DFF_PST_ExQuickTimeMovieData        4075
#define DFF_PST_ExSubscription              4076
#define DFF_PST_ExSubscriptionSection       4077
#define DFF_PST_ExControl                   4078
#define DFF_PST_ExControlAtom               4091
#define DFF_PST_SlideListWithText           4080
#define DFF_PST_AnimationInfoAtom           4081
#define DFF_PST_InteractiveInfo             4082
#define DFF_PST_InteractiveInfoAtom         4083
#define DFF_PST_SlideList                   4084
#define DFF_PST_UserEditAtom                4085
#define DFF_PST_CurrentUserAtom             4086
#define DFF_PST_DateTimeMCAtom              4087
#define DFF_PST_GenericDateMCAtom           4088
#define DFF_PST_HeaderMCAtom                4089
#define DFF_PST_FooterMCAtom                4090
#define DFF_PST_ExMediaAtom                 4100
#define DFF_PST_ExVideo                     4101
#define DFF_PST_ExAviMovie                  4102
#define DFF_PST_ExMCIMovie                  4103
#define DFF_PST_ExMIDIAudio                 4109
#define DFF_PST_ExCDAudio                   4110
#define DFF_PST_ExWAVAudioEmbedded          4111
#define DFF_PST_ExWAVAudioLink              4112
#define DFF_PST_ExOleObjStg                 4113
#define DFF_PST_ExCDAudioAtom               4114
#define DFF_PST_ExWAVAudioEmbeddedAtom      4115
#define DFF_PST_AnimationInfo               4116
#define DFF_PST_RTFDateTimeMCAtom           4117
#define DFF_PST_ProgTags                    5000
#define DFF_PST_ProgStringTag               5001
#define DFF_PST_ProgBinaryTag               5002
#define DFF_PST_BinaryTagData               5003
#define DFF_PST_PrintOptions                6000
#define DFF_PST_PersistPtrFullBlock         6001
#define DFF_PST_PersistPtrIncrementalBlock  6002
#define DFF_PST_RulerIndentAtom            10000
#define DFF_PST_GscalingAtom               10001
#define DFF_PST_GrColorAtom                10002
#define DFF_PST_GLPointAtom                10003
#define DFF_PST_GlineAtom                  10004

//      Record Name                 FBT-Value Instance                  Contents                                                          Wrd Exl Ppt Ver

#define DFF_msofbtDggContainer      0xF000 //                           per-document data                                                  X   X   X
#define   DFF_msofbtDgg             0xF006 //                           an FDGG and several FIDCLs                                         X   X   X   0
#define   DFF_msofbtCLSID           0xF016 //                           the CLSID of the application that put the data on the clipboard    C   C   C   0
#define   DFF_msofbtOPT             0xF00B // count of properties       the document-wide default shape properties                         X   X   X   3
#define   DFF_msofbtColorMRU        0xF11A // count of colors           the colors in the MRU swatch                                       X   X   X   0
#define   DFF_msofbtSplitMenuColors 0xF11E // count of colors           the colors in the top-level split menus                            X   X   X   0
#define   DFF_msofbtBstoreContainer 0xF001 // count of BLIPs            all images in the document (JPEGs, metafiles, etc.)                X   X   X
#define     DFF_msofbtBSE           0xF007 // BLIP type                 an FBSE (one per BLIP)                                             X   X   X   2
#define     DFF_msofbtBlipFirst     0xF018 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X
#define     DFF_msofbtBlipLast      0xF117 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X

#define DFF_msofbtDgContainer       0xF002 //                           per-sheet/page/slide data                                          X   X   X
#define   DFF_msofbtDg              0xF008 // drawing ID                an FDG                                                             X   X   X   0
#define   DFF_msofbtRegroupItems    0xF118 // count of regroup entries  several FRITs                                                      X   X   X   0
#define   DFF_msofbtColorScheme     0xF120 // count of colors           the colors of the source host's color scheme                           C   C   0
#define   DFF_msofbtSpgrContainer   0xF003 //                           several SpContainers, the first of which is the group shape itself X   X   X
#define   DFF_msofbtSpContainer     0xF004 //                           a shape                                                            X   X   X
#define     DFF_msofbtSpgr          0xF009 //                           an FSPGR; only present if the shape is a group shape               X   X   X   1
#define     DFF_msofbtSp            0xF00A // shape type                an FSP                                                             X   X   X   2
#define     DFF_msofbtTextbox       0xF00C //                           RTF text                                                           C   C   C   0
#define     DFF_msofbtClientTextbox 0xF00D // host-defined              the text in the textbox, in host-defined format                    X   X   X
#define     DFF_msofbtAnchor        0xF00E //                           a RECT, in 100000ths of an inch                                    C   C   C   0
#define     DFF_msofbtChildAnchor   0xF00F //                           a RECT, in units relative to the parent group                      X   X   X   0
#define     DFF_msofbtClientAnchor  0xF010 // host-defined              the location of the shape, in a host-defined format                X   X   X
#define     DFF_msofbtClientData    0xF011 // host-defined              host-specific data                                                 X   X   X
#define     DFF_msofbtOleObject     0xF11F //                           a serialized IStorage for an OLE object                            C   C   C   0
#define     DFF_msofbtDeletedPspl   0xF11D //                           an FPSPL; only present in top-level deleted shapes                 X           0
#define     DFF_msofbtUDefProp      0xF122 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X
#define   DFF_msofbtSolverContainer 0xF005 // count of rules            the rules governing shapes                                         X   X   X
#define     DFF_msofbtConnectorRule 0xF012 //                           an FConnectorRule                                                      X   X   1
#define     DFF_msofbtAlignRule     0xF013 //                           an FAlignRule                                                      X   X   X   0
#define     DFF_msofbtArcRule       0xF014 //                           an FARCRU                                                          X   X   X   0
#define     DFF_msofbtClientRule    0xF015 // host-defined              host-defined
#define     DFF_msofbtCalloutRule   0xF017 //                           an FCORU                                                           X   X   X   0

#define DFF_msofbtSelection         0xF119 //                           an FDGSL followed by the SPIDs of the shapes in the selection              X   0

// Values for the ULONG in DFF_PST_TextHeaderAtom
enum DFF_TextHeader {
    DFFTH_TITLE,
    DFFTH_BODY,
    DFFTH_NOTES,
    DFFTH_NOTUSED,
    DFFTH_OTHER,       // Text in a Shape
    DFFTH_CENTERBODY,  // Subtitle in Title-Slide
    DFFTH_CENTERTITLE, // Title in Title-Slide
    DFFTH_HALFBODY,    // Body in two-column slide
    DFFTH_QUARTERBODY  // Body in four-body slide
};


// Shape Properties
// 1pt = 12700 EMU (English Metric Units)
// 1pt = 20 Twip = 20/1440" = 1/72"
// 1twip=635 EMU
// 1" = 12700*72 = 914400 EMU
// 1" = 25.4mm
// 1mm = 36000 EMU
// Transform
#define DFF_Prop_Rotation                      4  //  Fixed Point 16.16 degrees
// Protection
#define DFF_Prop_LockRotation                119  //  sal_Bool              No rotation
#define DFF_Prop_LockAspectRatio             120  //  sal_Bool              Don't allow changes in aspect ratio
#define DFF_Prop_LockPosition                121  //  sal_Bool              Don't allow the shape to be moved
#define DFF_Prop_LockAgainstSelect           122  //  sal_Bool              Shape may not be selected
#define DFF_Prop_LockCropping                123  //  sal_Bool              No cropping this shape
#define DFF_Prop_LockVertices                124  //  sal_Bool              Edit Points not allowed
#define DFF_Prop_LockText                    125  //  sal_Bool              Do not edit text
#define DFF_Prop_LockAdjustHandles           126  //  sal_Bool              Do not adjust
#define DFF_Prop_LockAgainstGrouping         127  //  sal_Bool              Do not group this shape
// Text
#define DFF_Prop_lTxid                       128  //  LONG              id for the text, value determined by the host
#define DFF_Prop_dxTextLeft                  129  //  LONG              margins relative to shape's inscribed text rectangle (in EMUs)
#define DFF_Prop_dyTextTop                   130  //  LONG
#define DFF_Prop_dxTextRight                 131  //  LONG
#define DFF_Prop_dyTextBottom                132  //  LONG
#define DFF_Prop_WrapText                    133  //  MSO_WRAPMODE       Wrap text at shape margins
#define DFF_Prop_scaleText                   134  //  LONG              Text zoom/scale (used if fFitTextToShape)
#define DFF_Prop_anchorText                  135  //  MSO_ANCHOR         How to anchor the text
#define DFF_Prop_txflTextFlow                136  //  MSO_TXFL           Text flow
#define DFF_Prop_cdirFont                    137  //  MSO_CDIR           Font rotation
#define DFF_Prop_hspNext                     138  //  MSO_HSP            ID of the next shape (used by Word for linked textboxes)
#define DFF_Prop_txdir                       139  //  MSO_TXDIR          Bi-Di Text direction
#define DFF_Prop_SelectText                  187  //  sal_Bool              sal_True if single click selects text, sal_False if two clicks
#define DFF_Prop_AutoTextMargin              188  //  sal_Bool              use host's margin calculations
#define DFF_Prop_RotateText                  189  //  sal_Bool              Rotate text with shape
#define DFF_Prop_FitShapeToText              190  //  sal_Bool              Size shape to fit text size
#define DFF_Prop_FitTextToShape              191  //  sal_Bool              Size text to fit shape size
// GeoText
#define DFF_Prop_gtextUNICODE                192  //  WCHAR*            UNICODE text string
#define DFF_Prop_gtextRTF                    193  //  char*             RTF text string
#define DFF_Prop_gtextAlign                  194  //  MSO_GEOTEXTALIGN   alignment on curve
#define DFF_Prop_gtextSize                   195  //  LONG              default point size
#define DFF_Prop_gtextSpacing                196  //  LONG              fixed point 16.16
#define DFF_Prop_gtextFont                   197  //  WCHAR*            font family name
#define DFF_Prop_gtextFReverseRows           240  //  sal_Bool  8000        Reverse row order
#define DFF_Prop_fGtext                      241  //  sal_Bool  4000        Has text effect
#define DFF_Prop_gtextFVertical              242  //  sal_Bool  2000        Rotate characters
#define DFF_Prop_gtextFKern                  243  //  sal_Bool  1000        Kern characters
#define DFF_Prop_gtextFTight                 244  //  sal_Bool   800        Tightening or tracking
#define DFF_Prop_gtextFStretch               245  //  sal_Bool   400        Stretch to fit shape
#define DFF_Prop_gtextFShrinkFit             246  //  sal_Bool   200        Char bounding box
#define DFF_Prop_gtextFBestFit               247  //  sal_Bool   100        Scale text-on-path
#define DFF_Prop_gtextFNormalize             248  //  sal_Bool    80        Stretch char height
#define DFF_Prop_gtextFDxMeasure             249  //  sal_Bool    40        Do not measure along path
#define DFF_Prop_gtextFBold                  250  //  sal_Bool    20        Bold font
#define DFF_Prop_gtextFItalic                251  //  sal_Bool    10        Italic font
#define DFF_Prop_gtextFUnderline             252  //  sal_Bool     8        Underline font
#define DFF_Prop_gtextFShadow                253  //  sal_Bool     4        Shadow font
#define DFF_Prop_gtextFSmallcaps             254  //  sal_Bool     2        Small caps font
#define DFF_Prop_gtextFStrikethrough         255  //  sal_Bool     1        Strike through font
// Blip
#define DFF_Prop_cropFromTop                 256  //  LONG              16.16 fraction times total image width or height, as appropriate.
#define DFF_Prop_cropFromBottom              257  //  LONG
#define DFF_Prop_cropFromLeft                258  //  LONG
#define DFF_Prop_cropFromRight               259  //  LONG
#define DFF_Prop_pib                         260  //  IMsoBlip*         Blip to display
#define DFF_Prop_pibName                     261  //  WCHAR*            Blip file name
#define DFF_Prop_pibFlags                    262  //  MSO_BLIPFLAGS      Blip flags
#define DFF_Prop_pictureTransparent          263  //  LONG              transparent color (none if ~0UL)
#define DFF_Prop_pictureContrast             264  //  LONG              contrast setting
#define DFF_Prop_pictureBrightness           265  //  LONG              brightness setting
#define DFF_Prop_pictureGamma                266  //  LONG              16.16 gamma
#define DFF_Prop_pictureId                   267  //  LONG              Host-defined ID for OLE objects (usually a pointer)
#define DFF_Prop_pictureDblCrMod             268  //  MSO_CLR            Modification used if shape has double shadow
#define DFF_Prop_pictureFillCrMod            269  //  MSO_CLR
#define DFF_Prop_pictureLineCrMod            270  //  MSO_CLR
#define DFF_Prop_pibPrint                    271  //  IMsoBlip*         Blip to display when printing
#define DFF_Prop_pibPrintName                272  //  WCHAR*            Blip file name
#define DFF_Prop_pibPrintFlags               273  //  MSO_BLIPFLAGS      Blip flags
#define DFF_Prop_fNoHitTestPicture           316  //  sal_Bool              Do not hit test the picture
#define DFF_Prop_pictureGray                 317  //  sal_Bool              grayscale display
#define DFF_Prop_pictureBiLevel              318  //  sal_Bool              bi-level display
#define DFF_Prop_pictureActive               319  //  sal_Bool              Server is active (OLE objects only)
// Geometry
#define DFF_Prop_geoLeft                     320  //  LONG              Defines the G (geometry) coordinate space.
#define DFF_Prop_geoTop                      321  //  LONG
#define DFF_Prop_geoRight                    322  //  LONG
#define DFF_Prop_geoBottom                   323  //  LONG
#define DFF_Prop_shapePath                   324  //  MSO_SHAPEPATH
#define DFF_Prop_pVertices                   325  //  IMsoArray         An array of points, in G units.
#define DFF_Prop_pSegmentInfo                326  //  IMsoArray
#define DFF_Prop_adjustValue                 327  //  LONG              Adjustment values corresponding to the positions of the
#define DFF_Prop_adjust2Value                328  //  LONG              adjust handles of the shape. The number of values used
#define DFF_Prop_adjust3Value                329  //  LONG              and their allowable ranges vary from shape type to shape type.
#define DFF_Prop_adjust4Value                330  //  LONG
#define DFF_Prop_adjust5Value                331  //  LONG
#define DFF_Prop_adjust6Value                332  //  LONG
#define DFF_Prop_adjust7Value                333  //  LONG
#define DFF_Prop_adjust8Value                334  //  LONG
#define DFF_Prop_adjust9Value                335  //  LONG
#define DFF_Prop_adjust10Value               336  //  LONG
#define DFF_Prop_connectorPoints             337  //  IMsoArray
#define DFF_Prop_stretchPointX               339  //  LONG
#define DFF_Prop_stretchPointY               340  //  LONG
#define DFF_Prop_Handles                     341  //  H*
#define DFF_Prop_pFormulas                   342  //  LONG
#define DFF_Prop_textRectangles              343  //  LONG
#define DFF_Prop_connectorType               344  //  LONG              ->0=none, 1=segments, 2=custom, 3=rect
#define DFF_Prop_fShadowOK                   378  //  sal_Bool              Shadow may be set
#define DFF_Prop_f3DOK                       379  //  sal_Bool              3D may be set
#define DFF_Prop_fLineOK                     380  //  sal_Bool              Line style may be set
#define DFF_Prop_fGtextOK                    381  //  sal_Bool              Text effect (FontWork) supported
#define DFF_Prop_fFillShadeShapeOK           382  //  BOOL
#define DFF_Prop_fFillOK                     383  //  sal_Bool              OK to fill the shape through the UI or VBA?
// FillStyle
#define DFF_Prop_fillType                    384  //  MSO_FILLTYPE       Type of fill
#define DFF_Prop_fillColor                   385  //  MSO_CLR            Foreground color
#define DFF_Prop_fillOpacity                 386  //  LONG              Fixed 16.16
#define DFF_Prop_fillBackColor               387  //  MSO_CLR            Background color
#define DFF_Prop_fillBackOpacity             388  //  LONG              Shades only
#define DFF_Prop_fillCrMod                   389  //  MSO_CLR            Modification for BW views
#define DFF_Prop_fillBlip                    390  //  IMsoBlip*         Pattern/texture
#define DFF_Prop_fillBlipName                391  //  WCHAR*            Blip file name
#define DFF_Prop_fillBlipFlags               392  //  MSO_BLIPFLAGS      Blip flags
#define DFF_Prop_fillWidth                   393  //  LONG              How big (A units) to make a metafile texture.
#define DFF_Prop_fillHeight                  394  //  LONG
#define DFF_Prop_fillAngle                   395  //  LONG              Fade angle - degrees in 16.16
#define DFF_Prop_fillFocus                   396  //  LONG              Linear shaded fill focus percent
#define DFF_Prop_fillToLeft                  397  //  LONG              Fraction 16.16
#define DFF_Prop_fillToTop                   398  //  LONG              Fraction 16.16
#define DFF_Prop_fillToRight                 399  //  LONG              Fraction 16.16
#define DFF_Prop_fillToBottom                400  //  LONG              Fraction 16.16
#define DFF_Prop_fillRectLeft                401  //  LONG              For shaded fills, use the specified rectangle instead of the shape's bounding rect to define how large the fade is going to be.
#define DFF_Prop_fillRectTop                 402  //  LONG
#define DFF_Prop_fillRectRight               403  //  LONG
#define DFF_Prop_fillRectBottom              404  //  LONG
#define DFF_Prop_fillDztype                  405  //  MSO_DZTYPE
#define DFF_Prop_fillShadePreset             406  //  LONG              Special shades
#define DFF_Prop_fillShadeColors             407  //  IMsoArray         a preset array of colors
#define DFF_Prop_fillOriginX                 408  //  LONG
#define DFF_Prop_fillOriginY                 409  //  LONG
#define DFF_Prop_fillShapeOriginX            410  //  LONG
#define DFF_Prop_fillShapeOriginY            411  //  LONG
#define DFF_Prop_fillShadeType               412  //  MSO_SHADETYPE      Type of shading, if a shaded (gradient) fill.
#define DFF_Prop_fFilled                     443  //  sal_Bool              Is shape filled?
#define DFF_Prop_fHitTestFill                444  //  sal_Bool              Should we hit test fill?
#define DFF_Prop_fillShape                   445  //  sal_Bool              Register pattern on shape
#define DFF_Prop_fillUseRect                 446  //  sal_Bool              Use the large rect?
#define DFF_Prop_fNoFillHitTest              447  //  sal_Bool              Hit test a shape as though filled
// LineStyle
#define DFF_Prop_lineColor                   448  //  MSO_CLR            Color of line
#define DFF_Prop_lineOpacity                 449  //  LONG              Not implemented
#define DFF_Prop_lineBackColor               450  //  MSO_CLR            Background color
#define DFF_Prop_lineCrMod                   451  //  MSO_CLR            Modification for BW views
#define DFF_Prop_lineType                    452  //  MSO_LINETYPE       Type of line
#define DFF_Prop_lineFillBlip                453  //  IMsoBlip*         Pattern/texture
#define DFF_Prop_lineFillBlipName            454  //  WCHAR*            Blip file name
#define DFF_Prop_lineFillBlipFlags           455  //  MSO_BLIPFLAGS      Blip flags
#define DFF_Prop_lineFillWidth               456  //  LONG              How big (A units) to make a metafile texture.
#define DFF_Prop_lineFillHeight              457  //  LONG
#define DFF_Prop_lineFillDztype              458  //  MSO_DZTYPE         How to interpret fillWidth/Height numbers.
#define DFF_Prop_lineWidth                   459  //  LONG              A units; 1pt == 12700 EMUs
#define DFF_Prop_lineMiterLimit              460  //  LONG              ratio (16.16) of width
#define DFF_Prop_lineStyle                   461  //  MSO_LINESTYLE      Draw parallel lines?
#define DFF_Prop_lineDashing                 462  //  MSO_LINEDASHING    Can be overridden by:
#define DFF_Prop_lineDashStyle               463  //  IMsoArray         As Win32 ExtCreatePen
#define DFF_Prop_lineStartArrowhead          464  //  MSO_LINEEND        Arrow at start
#define DFF_Prop_lineEndArrowhead            465  //  MSO_LINEEND        Arrow at end
#define DFF_Prop_lineStartArrowWidth         466  //  MSO_LINEENDWIDTH   Arrow at start
#define DFF_Prop_lineStartArrowLength        467  //  MSO_LINEENDLENGTH  Arrow at end
#define DFF_Prop_lineEndArrowWidth           468  //  MSO_LINEENDWIDTH   Arrow at start
#define DFF_Prop_lineEndArrowLength          469  //  MSO_LINEENDLENGTH  Arrow at end
#define DFF_Prop_lineJoinStyle               470  //  MSO_LINEJOIN       How to join lines
#define DFF_Prop_lineEndCapStyle             471  //  MSO_LINECAP        How to end lines
#define DFF_Prop_fArrowheadsOK               507  //  sal_Bool              Allow arrowheads if prop. is set
#define DFF_Prop_fLine                       508  //  sal_Bool              Any line?
#define DFF_Prop_fHitTestLine                509  //  sal_Bool              Should we hit test lines?
#define DFF_Prop_lineFillShape               510  //  sal_Bool              Register pattern on shape
#define DFF_Prop_fNoLineDrawDash             511  //  sal_Bool              Draw a dashed line if no line
// ShadowStyle
#define DFF_Prop_shadowType                  512  //  MSO_SHADOWTYPE     Type of effect
#define DFF_Prop_shadowColor                 513  //  MSO_CLR            Foreground color
#define DFF_Prop_shadowHighlight             514  //  MSO_CLR            Embossed color
#define DFF_Prop_shadowCrMod                 515  //  MSO_CLR            Modification for BW views
#define DFF_Prop_shadowOpacity               516  //  LONG              Fixed 16.16
#define DFF_Prop_shadowOffsetX               517  //  LONG              Offset shadow
#define DFF_Prop_shadowOffsetY               518  //  LONG              Offset shadow
#define DFF_Prop_shadowSecondOffsetX         519  //  LONG              Double offset shadow
#define DFF_Prop_shadowSecondOffsetY         520  //  LONG              Double offset shadow
#define DFF_Prop_shadowScaleXToX             521  //  LONG              16.16
#define DFF_Prop_shadowScaleYToX             522  //  LONG              16.16
#define DFF_Prop_shadowScaleXToY             523  //  LONG              16.16
#define DFF_Prop_shadowScaleYToY             524  //  LONG              16.16
#define DFF_Prop_shadowPerspectiveX          525  //  LONG              16.16 / weight
#define DFF_Prop_shadowPerspectiveY          526  //  LONG              16.16 / weight
#define DFF_Prop_shadowWeight                527  //  LONG              scaling factor
#define DFF_Prop_shadowOriginX               528  //  LONG
#define DFF_Prop_shadowOriginY               529  //  LONG
#define DFF_Prop_fShadow                     574  //  sal_Bool              Any shadow?
#define DFF_Prop_fshadowObscured             575  //  sal_Bool              Excel5-style shadow
// PerspectiveStyle
#define DFF_Prop_perspectiveType             576  //  MSO_XFORMTYPE      Where transform applies
#define DFF_Prop_perspectiveOffsetX          577  //  LONG              The LONG values define a transformation matrix, effectively, each value is scaled by the perspectiveWeight parameter.
#define DFF_Prop_perspectiveOffsetY          578  //  LONG
#define DFF_Prop_perspectiveScaleXToX        579  //  LONG
#define DFF_Prop_perspectiveScaleYToX        580  //  LONG
#define DFF_Prop_perspectiveScaleXToY        581  //  LONG
#define DFF_Prop_perspectiveScaleYToY        582  //  LONG
#define DFF_Prop_perspectivePerspectiveX     583  //  LONG
#define DFF_Prop_perspectivePerspectiveY     584  //  LONG
#define DFF_Prop_perspectiveWeight           585  //  LONG              Scaling factor
#define DFF_Prop_perspectiveOriginX          586  //  LONG
#define DFF_Prop_perspectiveOriginY          587  //  LONG
#define DFF_Prop_fPerspective                639  //  sal_Bool              On/off
// 3D Object
#define DFF_Prop_c3DSpecularAmt              640  //  LONG         Fixed-point 16.16
#define DFF_Prop_c3DDiffuseAmt               641  //  LONG         Fixed-point 16.16
#define DFF_Prop_c3DShininess                642  //  LONG         Default gives OK results
#define DFF_Prop_c3DEdgeThickness            643  //  LONG         Specular edge thickness
#define DFF_Prop_c3DExtrudeForward           644  //  LONG         Distance of extrusion in EMUs
#define DFF_Prop_c3DExtrudeBackward          645  //  LONG
#define DFF_Prop_c3DExtrudePlane             646  //  LONG         Extrusion direction
#define DFF_Prop_c3DExtrusionColor           647  //  MSO_CLR       Basic color of extruded part of shape; the lighting model used will determine the exact shades used when rendering.
#define DFF_Prop_c3DCrMod                    648  //  MSO_CLR       Modification for BW views
#define DFF_Prop_f3D                         700  //  sal_Bool         Does this shape have a 3D effect?
#define DFF_Prop_fc3DMetallic                701  //  sal_Bool         Use metallic specularity?
#define DFF_Prop_fc3DUseExtrusionColor       702  //  BOOL
#define DFF_Prop_fc3DLightFace               703  //  BOOL
// 3D Style
#define DFF_Prop_c3DYRotationAngle           704  //  LONG            degrees (16.16) about y axis
#define DFF_Prop_c3DXRotationAngle           705  //  LONG            degrees (16.16) about x axis
#define DFF_Prop_c3DRotationAxisX            706  //  LONG            These specify the rotation axis; only their relative magnitudes matter.
#define DFF_Prop_c3DRotationAxisY            707  //  LONG
#define DFF_Prop_c3DRotationAxisZ            708  //  LONG
#define DFF_Prop_c3DRotationAngle            709  //  LONG            degrees (16.16) about axis
#define DFF_Prop_c3DRotationCenterX          710  //  LONG            rotation center x (16.16 or g-units)
#define DFF_Prop_c3DRotationCenterY          711  //  LONG            rotation center y (16.16 or g-units)
#define DFF_Prop_c3DRotationCenterZ          712  //  LONG            rotation center z (absolute (emus))
#define DFF_Prop_c3DRenderMode               713  //  MSO_3DRENDERMODE Full,wireframe, or bcube
#define DFF_Prop_c3DTolerance                714  //  LONG            pixels (16.16)
#define DFF_Prop_c3DXViewpoint               715  //  LONG            X view point (emus)
#define DFF_Prop_c3DYViewpoint               716  //  LONG            Y view point (emus)
#define DFF_Prop_c3DZViewpoint               717  //  LONG            Z view distance (emus)
#define DFF_Prop_c3DOriginX                  718  //  LONG
#define DFF_Prop_c3DOriginY                  719  //  LONG
#define DFF_Prop_c3DSkewAngle                720  //  LONG            degree (16.16) skew angle
#define DFF_Prop_c3DSkewAmount               721  //  LONG            Percentage skew amount
#define DFF_Prop_c3DAmbientIntensity         722  //  LONG            Fixed point intensity
#define DFF_Prop_c3DKeyX                     723  //  LONG            Key light source direc-
#define DFF_Prop_c3DKeyY                     724  //  LONG            tion; only their relative
#define DFF_Prop_c3DKeyZ                     725  //  LONG            magnitudes matter
#define DFF_Prop_c3DKeyIntensity             726  //  LONG            Fixed point intensity
#define DFF_Prop_c3DFillX                    727  //  LONG            Fill light source direc-
#define DFF_Prop_c3DFillY                    728  //  LONG            tion; only their relative
#define DFF_Prop_c3DFillZ                    729  //  LONG            magnitudes matter
#define DFF_Prop_c3DFillIntensity            730  //  LONG            Fixed point intensity
#define DFF_Prop_fc3DConstrainRotation       763  //  BOOL
#define DFF_Prop_fc3DRotationCenterAuto      764  //  BOOL
#define DFF_Prop_fc3DParallel                765  //  sal_Bool            Parallel projection?
#define DFF_Prop_fc3DKeyHarsh                766  //  sal_Bool            Is key lighting harsh?
#define DFF_Prop_fc3DFillHarsh               767  //  sal_Bool            Is fill lighting harsh?
// Shape
#define DFF_Prop_hspMaster                   769  //  MSO_HSP          master shape
#define DFF_Prop_cxstyle                     771  //  MSO_CXSTYLE      Type of connector
#define DFF_Prop_bWMode                      772  //  MSO_BWMODE       Settings for modifications to be made when in different forms of black-and-white mode.
#define DFF_Prop_bWModePureBW                773  //  MSO_BWMODE
#define DFF_Prop_bWModeBW                    774  //  MSO_BWMODE
#define DFF_Prop_fOleIcon                    826  //  sal_Bool            For OLE objects, whether the object is in icon form
#define DFF_Prop_fPreferRelativeResize       827  //  sal_Bool            For UI only. Prefer relative resizing.
#define DFF_Prop_fLockShapeType              828  //  sal_Bool            Lock the shape type (don't allow Change Shape)
#define DFF_Prop_fDeleteAttachedObject       830  //  BOOL
#define DFF_Prop_fBackground                 831  //  sal_Bool            If sal_True, this is the background shape.

// Callout
#define DFF_Prop_spcot                       832  //  MSO_SPCOT        Callout type
#define DFF_Prop_dxyCalloutGap               833  //  LONG            Distance from box to first point.(EMUs)
#define DFF_Prop_spcoa                       834  //  MSO_SPCOA        Callout angle
#define DFF_Prop_spcod                       835  //  MSO_SPCOD        Callout drop type
#define DFF_Prop_dxyCalloutDropSpecified     836  //  LONG            if mso_spcodSpecified, the actual drop distance
#define DFF_Prop_dxyCalloutLengthSpecified   837  //  LONG            if fCalloutLengthSpecified, the actual distance
#define DFF_Prop_fCallout                    889  //  sal_Bool            Is the shape a callout?
#define DFF_Prop_fCalloutAccentBar           890  //  sal_Bool            does callout have accent bar
#define DFF_Prop_fCalloutTextBorder          891  //  sal_Bool            does callout have a text border
#define DFF_Prop_fCalloutMinusX              892  //  BOOL
#define DFF_Prop_fCalloutMinusY              893  //  BOOL
#define DFF_Prop_fCalloutDropAuto            894  //  sal_Bool            If true, then we occasionally invert the drop distance
#define DFF_Prop_fCalloutLengthSpecified     895  //  sal_Bool            if true, we look at dxyCalloutLengthSpecified

// GroupShape
#define DFF_Prop_wzName                      896  //  WCHAR*          Shape Name (present only if explicitly set)
#define DFF_Prop_wzDescription               897  //  WCHAR*          alternate text
#define DFF_Prop_pihlShape                   898  //  IHlink*         The hyperlink in the shape.
#define DFF_Prop_pWrapPolygonVertices        899  //  IMsoArray       The polygon that text will be wrapped around (Word)
#define DFF_Prop_dxWrapDistLeft              900  //  LONG            Left wrapping distance from text (Word)
#define DFF_Prop_dyWrapDistTop               901  //  LONG            Top wrapping distance from text (Word)
#define DFF_Prop_dxWrapDistRight             902  //  LONG            Right wrapping distance from text (Word)
#define DFF_Prop_dyWrapDistBottom            903  //  LONG            Bottom wrapping distance from text (Word)
#define DFF_Prop_lidRegroup                  904  //  LONG            Regroup ID
#define DFF_Prop_tableProperties             927  //  LONG
#define DFF_Prop_tableRowProperties          928  //  LONG*
#define DFF_Prop_fEditedWrap                 953  //  sal_Bool            Has the wrap polygon been edited?
#define DFF_Prop_fBehindDocument             954  //  sal_Bool            Word-only (shape is behind text)
#define DFF_Prop_fOnDblClickNotify           955  //  sal_Bool            Notify client on a double click
#define DFF_Prop_fIsButton                   956  //  sal_Bool            A button shape (i.e., clicking performs an action). Set for shapes with attached hyperlinks or macros.
#define DFF_Prop_fOneD                       957  //  sal_Bool            1D adjustment
#define DFF_Prop_fHidden                     958  //  sal_Bool            Do not display
#define DFF_Prop_fPrint                      959  //  sal_Bool            Print this shape
// entsprechende BitFlags
#define DFF_PBit_EditedWrap         0x00000040
#define DFF_PBit_BehindDocument     0x00000020
#define DFF_PBit_OnDblClickNotify   0x00000010
#define DFF_PBit_IsButton           0x00000008
#define DFF_PBit_OneD               0x00000004
#define DFF_PBit_Hidden             0x00000002
#define DFF_PBit_Print              0x00000001

//---------------------------------------------------------------------------
// linchpin: the shape type
//---------------------------------------------------------------------------
typedef enum
   {
   mso_sptMin = 0,
   mso_sptNotPrimitive = mso_sptMin,
   mso_sptRectangle = 1,
   mso_sptRoundRectangle = 2,
   mso_sptEllipse = 3,
   mso_sptDiamond = 4,
   mso_sptIsocelesTriangle = 5,
   mso_sptRightTriangle = 6,
   mso_sptParallelogram = 7,
   mso_sptTrapezoid = 8,
   mso_sptHexagon = 9,
   mso_sptOctagon = 10,
   mso_sptPlus = 11,
   mso_sptStar = 12,
   mso_sptArrow = 13,
   mso_sptThickArrow = 14,
   mso_sptHomePlate = 15,
   mso_sptCube = 16,
   mso_sptBalloon = 17,
   mso_sptSeal = 18,
   mso_sptArc = 19,
   mso_sptLine = 20,
   mso_sptPlaque = 21,
   mso_sptCan = 22,
   mso_sptDonut = 23,
   mso_sptTextSimple = 24,
   mso_sptTextOctagon = 25,
   mso_sptTextHexagon = 26,
   mso_sptTextCurve = 27,
   mso_sptTextWave = 28,
   mso_sptTextRing = 29,
   mso_sptTextOnCurve = 30,
   mso_sptTextOnRing = 31,
   mso_sptStraightConnector1 = 32,
   mso_sptBentConnector2 = 33,
   mso_sptBentConnector3 = 34,
   mso_sptBentConnector4 = 35,
   mso_sptBentConnector5 = 36,
   mso_sptCurvedConnector2 = 37,
   mso_sptCurvedConnector3 = 38,
   mso_sptCurvedConnector4 = 39,
   mso_sptCurvedConnector5 = 40,
   mso_sptCallout1 = 41,
   mso_sptCallout2 = 42,
   mso_sptCallout3 = 43,
   mso_sptAccentCallout1 = 44,
   mso_sptAccentCallout2 = 45,
   mso_sptAccentCallout3 = 46,
   mso_sptBorderCallout1 = 47,
   mso_sptBorderCallout2 = 48,
   mso_sptBorderCallout3 = 49,
   mso_sptAccentBorderCallout1 = 50,
   mso_sptAccentBorderCallout2 = 51,
   mso_sptAccentBorderCallout3 = 52,
   mso_sptRibbon = 53,
   mso_sptRibbon2 = 54,
   mso_sptChevron = 55,
   mso_sptPentagon = 56,
   mso_sptNoSmoking = 57,
   mso_sptSeal8 = 58,
   mso_sptSeal16 = 59,
   mso_sptSeal32 = 60,
   mso_sptWedgeRectCallout = 61,
   mso_sptWedgeRRectCallout = 62,
   mso_sptWedgeEllipseCallout = 63,
   mso_sptWave = 64,
   mso_sptFoldedCorner = 65,
   mso_sptLeftArrow = 66,
   mso_sptDownArrow = 67,
   mso_sptUpArrow = 68,
   mso_sptLeftRightArrow = 69,
   mso_sptUpDownArrow = 70,
   mso_sptIrregularSeal1 = 71,
   mso_sptIrregularSeal2 = 72,
   mso_sptLightningBolt = 73,
   mso_sptHeart = 74,
   mso_sptPictureFrame = 75,
   mso_sptQuadArrow = 76,
   mso_sptLeftArrowCallout = 77,
   mso_sptRightArrowCallout = 78,
   mso_sptUpArrowCallout = 79,
   mso_sptDownArrowCallout = 80,
   mso_sptLeftRightArrowCallout = 81,
   mso_sptUpDownArrowCallout = 82,
   mso_sptQuadArrowCallout = 83,
   mso_sptBevel = 84,
   mso_sptLeftBracket = 85,
   mso_sptRightBracket = 86,
   mso_sptLeftBrace = 87,
   mso_sptRightBrace = 88,
   mso_sptLeftUpArrow = 89,
   mso_sptBentUpArrow = 90,
   mso_sptBentArrow = 91,
   mso_sptSeal24 = 92,
   mso_sptStripedRightArrow = 93,
   mso_sptNotchedRightArrow = 94,
   mso_sptBlockArc = 95,
   mso_sptSmileyFace = 96,
   mso_sptVerticalScroll = 97,
   mso_sptHorizontalScroll = 98,
   mso_sptCircularArrow = 99,
   mso_sptNotchedCircularArrow = 100,
   mso_sptUturnArrow = 101,
   mso_sptCurvedRightArrow = 102,
   mso_sptCurvedLeftArrow = 103,
   mso_sptCurvedUpArrow = 104,
   mso_sptCurvedDownArrow = 105,
   mso_sptCloudCallout = 106,
   mso_sptEllipseRibbon = 107,
   mso_sptEllipseRibbon2 = 108,
   mso_sptFlowChartProcess = 109,
   mso_sptFlowChartDecision = 110,
   mso_sptFlowChartInputOutput = 111,
   mso_sptFlowChartPredefinedProcess = 112,
   mso_sptFlowChartInternalStorage = 113,
   mso_sptFlowChartDocument = 114,
   mso_sptFlowChartMultidocument = 115,
   mso_sptFlowChartTerminator = 116,
   mso_sptFlowChartPreparation = 117,
   mso_sptFlowChartManualInput = 118,
   mso_sptFlowChartManualOperation = 119,
   mso_sptFlowChartConnector = 120,
   mso_sptFlowChartPunchedCard = 121,
   mso_sptFlowChartPunchedTape = 122,
   mso_sptFlowChartSummingJunction = 123,
   mso_sptFlowChartOr = 124,
   mso_sptFlowChartCollate = 125,
   mso_sptFlowChartSort = 126,
   mso_sptFlowChartExtract = 127,
   mso_sptFlowChartMerge = 128,
   mso_sptFlowChartOfflineStorage = 129,
   mso_sptFlowChartOnlineStorage = 130,
   mso_sptFlowChartMagneticTape = 131,
   mso_sptFlowChartMagneticDisk = 132,
   mso_sptFlowChartMagneticDrum = 133,
   mso_sptFlowChartDisplay = 134,
   mso_sptFlowChartDelay = 135,
   mso_sptTextPlainText = 136,
   mso_sptTextStop = 137,
   mso_sptTextTriangle = 138,
   mso_sptTextTriangleInverted = 139,
   mso_sptTextChevron = 140,
   mso_sptTextChevronInverted = 141,
   mso_sptTextRingInside = 142,
   mso_sptTextRingOutside = 143,
   mso_sptTextArchUpCurve = 144,
   mso_sptTextArchDownCurve = 145,
   mso_sptTextCircleCurve = 146,
   mso_sptTextButtonCurve = 147,
   mso_sptTextArchUpPour = 148,
   mso_sptTextArchDownPour = 149,
   mso_sptTextCirclePour = 150,
   mso_sptTextButtonPour = 151,
   mso_sptTextCurveUp = 152,
   mso_sptTextCurveDown = 153,
   mso_sptTextCascadeUp = 154,
   mso_sptTextCascadeDown = 155,
   mso_sptTextWave1 = 156,
   mso_sptTextWave2 = 157,
   mso_sptTextWave3 = 158,
   mso_sptTextWave4 = 159,
   mso_sptTextInflate = 160,
   mso_sptTextDeflate = 161,
   mso_sptTextInflateBottom = 162,
   mso_sptTextDeflateBottom = 163,
   mso_sptTextInflateTop = 164,
   mso_sptTextDeflateTop = 165,
   mso_sptTextDeflateInflate = 166,
   mso_sptTextDeflateInflateDeflate = 167,
   mso_sptTextFadeRight = 168,
   mso_sptTextFadeLeft = 169,
   mso_sptTextFadeUp = 170,
   mso_sptTextFadeDown = 171,
   mso_sptTextSlantUp = 172,
   mso_sptTextSlantDown = 173,
   mso_sptTextCanUp = 174,
   mso_sptTextCanDown = 175,
   mso_sptFlowChartAlternateProcess = 176,
   mso_sptFlowChartOffpageConnector = 177,
   mso_sptCallout90 = 178,
   mso_sptAccentCallout90 = 179,
   mso_sptBorderCallout90 = 180,
   mso_sptAccentBorderCallout90 = 181,
   mso_sptLeftRightUpArrow = 182,
   mso_sptSun = 183,
   mso_sptMoon = 184,
   mso_sptBracketPair = 185,
   mso_sptBracePair = 186,
   mso_sptSeal4 = 187,
   mso_sptDoubleWave = 188,
   mso_sptActionButtonBlank = 189,
   mso_sptActionButtonHome = 190,
   mso_sptActionButtonHelp = 191,
   mso_sptActionButtonInformation = 192,
   mso_sptActionButtonForwardNext = 193,
   mso_sptActionButtonBackPrevious = 194,
   mso_sptActionButtonEnd = 195,
   mso_sptActionButtonBeginning = 196,
   mso_sptActionButtonReturn = 197,
   mso_sptActionButtonDocument = 198,
   mso_sptActionButtonSound = 199,
   mso_sptActionButtonMovie = 200,
   mso_sptHostControl = 201,
   mso_sptTextBox = 202,
   mso_sptMax = 0x0FFF,
   mso_sptNil = mso_sptMax
 } MSO_SPT;

// different enums from the OfficeDraw documentation, Appendix D - beginning
//
// note: in opposite to the MS documentation, they all
// start with "mso_" instead of "mso"
//
enum MSO_ShapePath {
   mso_shapeLines,        // A line of straight segments
   mso_shapeLinesClosed,  // A closed polygonal object
   mso_shapeCurves,       // A line of Bezier curve segments
   mso_shapeCurvesClosed, // A closed shape with curved edges
   mso_shapeComplex       // pSegmentInfo must be non-empty
};

enum MSO_WrapMode {
   mso_wrapSquare,
   mso_wrapByPoints,
   mso_wrapNone,
   mso_wrapTopBottom,
   mso_wrapThrough
};

enum MSO_BWMode {
   mso_bwColor,          // only used for predefined shades
   mso_bwAutomatic,      // depends on object type
   mso_bwGrayScale,      // shades of gray only
   mso_bwLightGrayScale, // shades of light gray only
   mso_bwInverseGray,    // dark gray mapped to light gray, etc.
   mso_bwGrayOutline,    // pure gray and white
   mso_bwBlackTextLine,  // black text and lines, all else grayscale
   mso_bwHighContrast,   // pure black and white mode (no grays)
   mso_bwBlack,          // solid black
   mso_bwWhite,          // solid white
   mso_bwDontShow,       // object not drawn
   mso_bwNumModes        // number of Black and white modes
};

enum MSO_Anchor {
   mso_anchorTop,
   mso_anchorMiddle,
   mso_anchorBottom,
   mso_anchorTopCentered,
   mso_anchorMiddleCentered,
   mso_anchorBottomCentered,
   mso_anchorTopBaseline,
   mso_anchorBottomBaseline,
   mso_anchorTopCenteredBaseline,
   mso_anchorBottomCenteredBaseline
};

enum MSO_CDir {
   mso_cdir0,       // Right
   mso_cdir90,      // Down
   mso_cdir180,     // Left
   mso_cdir270      // Up
};

// connector style
enum MSO_ConnectorStyle {
   mso_cxstyleStraight = 0,
   mso_cxstyleBent,
   mso_cxstyleCurved,
   mso_cxstyleNone
};

// MSO_TXFL -- text flow
enum MSO_TextFlow {
   mso_txflHorzN,           // Horizontal non-@
   mso_txflTtoBA,           // Top to Bottom @-font
   mso_txflBtoT,            // Bottom to Top non-@
   mso_txflTtoBN,           // Top to Bottom non-@
   mso_txflHorzA,           // Horizontal @-font
   mso_txflVertN           // Vertical, non-@
};

// MSO_TXDIR - text direction (needed for Bi-Di support)
enum MSO_TextDir {
   mso_txdirLTR,            // left-to-right text direction
   mso_txdirRTL,            // right-to-left text direction
   mso_txdirContext        // context text direction
};

// MSO_SPCOT -- Callout Type
enum MSO_SPCalloutType {
   mso_spcotRightAngle = 1,
   mso_spcotOneSegment = 2,
   mso_spcotTwoSegment = 3,
   mso_spcotThreeSegment = 4
};

// MSO_SPCOA -- Callout Angle
enum MSO_SPCalloutAngle {
   mso_spcoaAny,
   mso_spcoa30,
   mso_spcoa45,
   mso_spcoa60,
   mso_spcoa90,
   mso_spcoa0
};

// MSO_SPCOD -- Callout Drop
enum MSO_SPCalloutDrop {
   mso_spcodTop,
   mso_spcodCenter,
   mso_spcodBottom,
   mso_spcodSpecified
};

// MSO_GEOTEXTALIGN - FontWork alignment
enum MSO_GeoTextAlign {
   mso_alignTextStretch,      /* Stretch each line of text to fit width. */
   mso_alignTextCenter,       /* Center text on width. */
   mso_alignTextLeft,         /* Left justify. */
   mso_alignTextRight,        /* Right justify. */
   mso_alignTextLetterJust,   /* Spread letters out to fit width. */
   mso_alignTextWordJust,     /* Spread words out to fit width. */
   mso_alignTextInvalid       /* Invalid */
};

// MSO_BLIPFLAGS - flags for pictures
enum MSO_BlipFlags {
   mso_blipflagDefault = 0,
   mso_blipflagComment = 0,   // Blip name is a comment
   mso_blipflagFile,          // Blip name is a file name
   mso_blipflagURL,           // Blip name is a full URL
   mso_blipflagType = 3,      // Mask to extract type
   /* Or the following flags with any of the above. */
   mso_blipflagDontSave = 4,  // A "dont" is the depression in the metal
                             // body work of an automobile caused when a
                             // cyclist violently thrusts his or her nose
                             // at it, thus a DontSave is another name for
                             // a cycle lane.
   mso_blipflagDoNotSave = 4, // For those who prefer English
   mso_blipflagLinkToFile = 8
};

enum MSO_3DRenderMode {
   mso_FullRender,      // Generate a full rendering
   mso_Wireframe,       // Generate a wireframe
   mso_BoundingCube    // Generate a bounding cube
};

enum MSO_XFormType {
   mso_xformAbsolute,   // Apply transform in absolute space centered on shape
   mso_xformShape,      // Apply transform to shape geometry
   mso_xformDrawing     // Apply transform in drawing space
};

enum MSO_ShadowType {
   mso_shadowOffset,    // N pixel offset shadow
   mso_shadowDouble,    // Use second offset too
   mso_shadowRich,      // Rich perspective shadow (cast relative to shape)
   mso_shadowShape,     // Rich perspective shadow (cast in shape space)
   mso_shadowDrawing,   // Perspective shadow cast in drawing space
   mso_shadowEmbossOrEngrave
};

// MSO_DZTYPE - the type of a (length) measurement
enum MSO_DZType {
   mso_dztypeMin          = 0,
   mso_dztypeDefault      = 0,  // Default size, ignore the values
   mso_dztypeA            = 1,  // Values are in EMUs
   mso_dztypeV            = 2,  // Values are in pixels
   mso_dztypeShape        = 3,  // Values are 16.16 fractions of shape size
   mso_dztypeFixedAspect  = 4,  // Aspect ratio is fixed
   mso_dztypeAFixed       = 5,  // EMUs, fixed aspect ratio
   mso_dztypeVFixed       = 6,  // Pixels, fixed aspect ratio
   mso_dztypeShapeFixed   = 7,  // Proportion of shape, fixed aspect ratio
   mso_dztypeFixedAspectEnlarge
                         = 8,  // Aspect ratio is fixed, favor larger size
   mso_dztypeAFixedBig    = 9,  // EMUs, fixed aspect ratio
   mso_dztypeVFixedBig    = 10, // Pixels, fixed aspect ratio
   mso_dztypeShapeFixedBig= 11, // Proportion of shape, fixed aspect ratio
   mso_dztypeMax          = 11
};

enum MSO_FillType {
   mso_fillSolid,             // Fill with a solid color
   mso_fillPattern,           // Fill with a pattern (bitmap)
   mso_fillTexture,           // A texture (pattern with its own color map)
   mso_fillPicture,           // Center a picture in the shape
   mso_fillShade,             // Shade from start to end points
   mso_fillShadeCenter,       // Shade from bounding rectangle to end point
   mso_fillShadeShape,        // Shade from shape outline to end point
   mso_fillShadeScale,        // Similar to mso_fillShade, but the fillAngle
                             // is additionally scaled by the aspect ratio of
                             // the shape. If shape is square, it is the
                             // same as mso_fillShade.
   mso_fillShadeTitle,        // special type - shade to title ---  for PP
   mso_fillBackground         // Use the background fill color/pattern
};

// MSO_SHADETYPE - how to interpret the colors in a shaded fill.
enum MSO_ShadeType {
   mso_shadeNone  = 0,        // Interpolate without correction between RGBs
   mso_shadeGamma = 1,        // Apply gamma correction to colors
   mso_shadeSigma = 2,        // Apply a sigma transfer function to position
   mso_shadeBand  = 4,        // Add a flat band at the start of the shade
   mso_shadeOneColor = 8,     // This is a one color shade

   /* A parameter for the band or sigma function can be stored in the top
      16 bits of the value - this is a proportion of *each* band of the
      shade to make flat (or the approximate equal value for a sigma
      function).  NOTE: the parameter is not used for the sigma function,
      instead a built in value is used.  This value should not be changed
      from the default! */
   mso_shadeParameterShift = 16,
   mso_shadeParameterMask  = 0xffff0000,

   mso_shadeDefault = (mso_shadeGamma|mso_shadeSigma|
                     (16384<<mso_shadeParameterShift))
};

// MSOLINESTYLE - compound line style
enum MSO_LineStyle {
   mso_lineSimple,          // Single line (of width lineWidth)
   mso_lineDouble,          // Double lines of equal width
   mso_lineThickThin,       // Double lines, one thick, one thin
   mso_lineThinThick,       // Double lines, reverse order
   mso_lineTriple           // Three lines, thin, thick, thin
};

// MSO_LINETYPE - how to "fill" the line contour
enum MSO_LineType {
   mso_lineSolidType,         // Fill with a solid color
   mso_linePattern,           // Fill with a pattern (bitmap)
   mso_lineTexture,           // A texture (pattern with its own color map)
   mso_linePicture            // Center a picture in the shape
};

// MSO_LINEDASHING - dashed line style
enum MSO_LineDashing {
   mso_lineSolid,              // Solid (continuous) pen
   mso_lineDashSys,            // PS_DASH system   dash style
   mso_lineDotSys,             // PS_DOT system   dash style
   mso_lineDashDotSys,         // PS_DASHDOT system dash style
   mso_lineDashDotDotSys,      // PS_DASHDOTDOT system dash style
   mso_lineDotGEL,             // square dot style
   mso_lineDashGEL,            // dash style
   mso_lineLongDashGEL,        // long dash style
   mso_lineDashDotGEL,         // dash short dash
   mso_lineLongDashDotGEL,     // long dash short dash
   mso_lineLongDashDotDotGEL   // long dash short dash short dash
};

// MSO_LINEEND - line end effect
enum MSO_LineEnd {
   mso_lineNoEnd,
   mso_lineArrowEnd,
   mso_lineArrowStealthEnd,
   mso_lineArrowDiamondEnd,
   mso_lineArrowOvalEnd,
   mso_lineArrowOpenEnd
};

// MSO_LINEENDWIDTH - size of arrowhead
enum MSO_LineEndWidth {
   mso_lineNarrowArrow,
   mso_lineMediumWidthArrow,
   mso_lineWideArrow
};

// MSO_LINEENDLENGTH - size of arrowhead
enum MSO_LineEndLength {
   mso_lineShortArrow,
   mso_lineMediumLenArrow,
   mso_lineLongArrow
};

// MSO_LINEJOIN - line join style.
enum MSO_LineJoin {
   mso_lineJoinBevel,     // Join edges by a straight line
   mso_lineJoinMiter,     // Extend edges until they join
   mso_lineJoinRound      // Draw an arc between the two edges
};

// MSO_LINECAP - line cap style (applies to ends of dash segments too).
enum MSO_LineCap {
   mso_lineEndCapRound,   // Rounded ends - the default
   mso_lineEndCapSquare,  // Square protrudes by half line width
   mso_lineEndCapFlat     // Line ends at end point
};
// verschiedene enums aus der OfficeDraw-Doku, Appendix D - Ende

// BStore-Container
// FBSE - File Blip Store Entry
typedef struct _MSOF_BSE {
   sal_uInt8      btWin32;    // Required type on Win32
   sal_uInt8      btMacOS;    // Required type on Mac
   sal_uInt8      rgbUid[16]; // Identifier of blip
   sal_uInt16    tag;        // currently unused
   sal_uIntPtr     size;       // Blip size in stream
   sal_uIntPtr     cRef;       // Reference count on the blip
   sal_uIntPtr /*MSOFO*/ foDelay;    // File offset in the delay stream
   sal_uInt8      usage;      // How this blip is used (MSOBLIPUSAGE)
   sal_uInt8      cbName;     // length of the blip name
   sal_uInt8      unused2;    // for the future
   sal_uInt8      unused3;    // for the future
} MSO_FBSE;

typedef enum {
   mso_blipUsageDefault,  // All non-texture fill blips get this.
   mso_blipUsageTexture,
   mso_blipUsageMax = 255 // Since this is stored in a byte
} MSO_BLIPUSAGE;

typedef enum {                          // GEL provided types...
   mso_blipERROR = 0,          // An error occurred during loading
   mso_blipUNKNOWN,            // An unknown blip type
   mso_blipEMF,                // Windows Enhanced Metafile
   mso_blipWMF,                // Windows Metafile
   mso_blipPICT,               // Macintosh PICT
   mso_blipJPEG,               // JFIF
   mso_blipPNG,                // PNG
   mso_blipDIB,                // Windows DIB
   mso_blipFirstClient = 32,   // First client defined blip type
   mso_blipLastClient  = 255   // Last client defined blip type
} MSO_BLIPTYPE;

typedef enum {
   mso_biUNKNOWN = 0,
   mso_biWMF  = 0x216,      // Metafile header then compressed WMF
   mso_biEMF  = 0x3D4,      // Metafile header then compressed EMF
   mso_biPICT = 0x542,      // Metafile header then compressed PICT
   mso_biPNG  = 0x6E0,      // One byte tag then PNG data
   mso_biJFIF = 0x46A,      // One byte tag then JFIF data
   mso_biJPEG = mso_biJFIF,
   mso_biDIB  = 0x7A8,      // One byte tag then DIB data
   mso_biClient=0x800       // Clients should set this bit
} MSO_BI;                     // Blip signature as encoded in the MSO_FBH.inst

typedef enum {
   mso_compressionDeflate = 0,
   mso_compressionNone = 254,    // Used only if compression fails
   mso_compressionTest = 255     // For testing only
} MSO_BLIPCOMPRESSION;

typedef enum {
   mso_filterAdaptive = 0,       // PNG type - not used/supported for metafile
   mso_filterNone = 254,
   mso_filterTest = 255          // For testing only
} MSO_BLIPFILTER;

typedef enum {
    mso_syscolorButtonFace,             // COLOR_BTNFACE
    mso_syscolorWindowText,             // COLOR_WINDOWTEXT
    mso_syscolorMenu,                   // COLOR_MENU
    mso_syscolorHighlight,              // COLOR_HIGHLIGHT
    mso_syscolorHighlightText,          // COLOR_HIGHLIGHTTEXT
    mso_syscolorCaptionText,            // COLOR_CAPTIONTEXT
    mso_syscolorActiveCaption,          // COLOR_ACTIVECAPTION
    mso_syscolorButtonHighlight,        // COLOR_BTNHIGHLIGHT
    mso_syscolorButtonShadow,           // COLOR_BTNSHADOW
    mso_syscolorButtonText,             // COLOR_BTNTEXT
    mso_syscolorGrayText,               // COLOR_GRAYTEXT
    mso_syscolorInactiveCaption,        // COLOR_INACTIVECAPTION
    mso_syscolorInactiveCaptionText,    // COLOR_INACTIVECAPTIONTEXT
    mso_syscolorInfoBackground,         // COLOR_INFOBK
    mso_syscolorInfoText,               // COLOR_INFOTEXT
    mso_syscolorMenuText,               // COLOR_MENUTEXT
    mso_syscolorScrollbar,              // COLOR_SCROLLBAR
    mso_syscolorWindow,                 // COLOR_WINDOW
    mso_syscolorWindowFrame,            // COLOR_WINDOWFRAME
    mso_syscolor3DLight,                // COLOR_3DLIGHT
    mso_syscolorMax,                    // Count of system colors

    mso_colorFillColor = 0xF0,          // Use the fillColor property
    mso_colorLineOrFillColor,           // Use the line color only if there is a line
    mso_colorLineColor,                 // Use the lineColor property
    mso_colorShadowColor,               // Use the shadow color
    mso_colorThis,                      // Use this color (only valid as described below)
    mso_colorFillBackColor,             // Use the fillBackColor property
    mso_colorLineBackColor,             // Use the lineBackColor property
    mso_colorFillThenLine,              // Use the fillColor unless no fill and line
    mso_colorIndexMask = 0xFF,          // Extract the color index

    mso_colorProcessMask      =0xFFFF00,// All the processing bits
    mso_colorModificationMask =0x0F00,  // Just the function
    mso_colorModFlagMask      =0xF000,  // Just the additional flags
    mso_colorDarken           =0x0100,  // Darken color by parameter/255
    mso_colorLighten          =0x0200,  // Lighten color by parameter/255
    mso_colorAdd              =0x0300,  // Add grey level RGB(param,param,param)
    mso_colorSubtract         =0x0400,  // Subtract grey level RGB(p,p,p)
    mso_colorReverseSubtract  =0x0500,  // Subtract from grey level RGB(p,p,p)
    /* In the following "black" means maximum component value, white minimum.
        The operation is per component, to guarantee white combine with
    mso_colorGray */
    mso_colorBlackWhite       =0x0600,  // Black if < uParam, else white (>=)
    mso_colorInvert           =0x2000,  // Invert color (at the *end*)
    mso_colorInvert128        =0x4000,  // Invert by toggling the top bit
    mso_colorGray             =0x8000,  // Make the color gray (before the above!)
    mso_colorBParamMask       =0xFF0000,// Parameter used as above
    mso_colorBParamShift = 16           // To extract the parameter value
} MSO_SYSCOLORINDEX;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
