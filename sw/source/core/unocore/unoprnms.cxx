/*************************************************************************
 *
 *  $RCSfile: unoprnms.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-06 12:40:46 $
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
#pragma hdrstop

#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

//#define MAP_CHAR_LEN(cchar) cchar, sizeof(cchar) - 1

//struct SwPropNameLen
//{
//  const char*                         pName;
//  USHORT                              nNameLen;
//};

//extern const SwPropNameLen                UNO_NAME_FOLLOW_STYLE;
const SwPropNameLen __FAR_DATA  UNO_NAME_FOLLOW_STYLE(MAP_CHAR_LEN("FollowStyle"));


const SwPropNameLen __FAR_DATA  UNO_NAME_IS_PHYSICAL                            (MAP_CHAR_LEN("IsPhysical"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_AUTO_UPDATE                         (MAP_CHAR_LEN("IsAutoUpdate"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DISPLAY_NAME                           (MAP_CHAR_LEN("DisplayName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_GRAPHIC_URL                       (MAP_CHAR_LEN("ParaBackGraphicURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_GRAPHIC_FILTER                    (MAP_CHAR_LEN("ParaBackGraphicFilter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_GRAPHIC_URL                     (MAP_CHAR_LEN("HeaderBackGraphicURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_GRAPHIC_FILTER                  (MAP_CHAR_LEN("HeaderBackGraphicFilter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_GRAPHIC_URL                     (MAP_CHAR_LEN("FooterBackGraphicURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_GRAPHIC_FILTER                  (MAP_CHAR_LEN("FooterBackGraphicFilter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_GRAPHIC_URL                       (MAP_CHAR_LEN("BackGraphicURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_GRAPHIC_FILTER                    (MAP_CHAR_LEN("BackGraphicFilter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_GRAPHIC_LOCATION                  (MAP_CHAR_LEN("BackGraphicLocation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_GRAPHIC_BITMAP                    (MAP_CHAR_LEN("BackGraphicBitmap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_URL                            (MAP_CHAR_LEN("GraphicURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_FILTER                         (MAP_CHAR_LEN("GraphicFilter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_LOCATION                       (MAP_CHAR_LEN("GraphicLocation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_SIZE                           (MAP_CHAR_LEN("GraphicSize"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_BITMAP                         (MAP_CHAR_LEN("GraphicBitmap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BULLET_FONT                            (MAP_CHAR_LEN("BulletFont"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_GRAPHIC_LOCATION                  (MAP_CHAR_LEN("ParaBackGraphicLocation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_GRAPHIC_LOCATION                (MAP_CHAR_LEN("HeaderBackGraphicLocation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_GRAPHIC_LOCATION                (MAP_CHAR_LEN("FooterBackGraphicLocation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LEFT_PARA_MARGIN                  (MAP_CHAR_LEN("ParaLeftParaMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_RIGHT_PARA_MARGIN                 (MAP_CHAR_LEN("ParaRightParaMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LEFT_MARGIN                       (MAP_CHAR_LEN("ParaLeftMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_RIGHT_MARGIN                      (MAP_CHAR_LEN("ParaRightMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LEFT_MARGIN_RELATIVE              (MAP_CHAR_LEN("ParaLeftMarginRelative"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE             (MAP_CHAR_LEN("ParaRightMarginRelative"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_FIRST_LINE_INDENT                 (MAP_CHAR_LEN("ParaFirstLineIndent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE        (MAP_CHAR_LEN("ParaFirstLineIndentRelative"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_IS_HYPHENATION                    (MAP_CHAR_LEN("ParaIsHyphenation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS     (MAP_CHAR_LEN("HyphenationMaxLeadingChars"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS    (MAP_CHAR_LEN("HyphenationMaxTrailingChars"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS           (MAP_CHAR_LEN("HyphenationMaxHyphens"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEFT_MARGIN                            (MAP_CHAR_LEN("LeftMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RIGHT_MARGIN                           (MAP_CHAR_LEN("RightMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_LEFT_MARGIN                     (MAP_CHAR_LEN("HeaderLeftMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_RIGHT_MARGIN                    (MAP_CHAR_LEN("HeaderRightMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_LEFT_MARGIN                     (MAP_CHAR_LEN("FooterLeftMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_RIGHT_MARGIN                    (MAP_CHAR_LEN("FooterRightMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_RANGE                             (MAP_CHAR_LEN("TextRange"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NAME                                   (MAP_CHAR_LEN("Name"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_ALIGNMENT                    (MAP_CHAR_LEN("NumberingAlignment"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BULLET_FONT_NAME                       (MAP_CHAR_LEN("BulletFontName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BULLET_ID                              (MAP_CHAR_LEN("BulletId"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_STYLE_NAME                        (MAP_CHAR_LEN("CharStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ANCHOR_CHAR_STYLE_NAME                 (MAP_CHAR_LEN("AnchorCharStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SUFFIX                                 (MAP_CHAR_LEN("Suffix"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PREFIX                                 (MAP_CHAR_LEN("Prefix"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARENT_NUMBERING                       (MAP_CHAR_LEN("ParentNumbering"));
const SwPropNameLen __FAR_DATA  UNO_NAME_START_WITH                             (MAP_CHAR_LEN("StartWith"));

const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_HEIGHT                            (MAP_CHAR_LEN("CharHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_NAME                         (MAP_CHAR_LEN("CharFontName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_STYLE_NAME                   (MAP_CHAR_LEN("CharFontStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_FAMILY                       (MAP_CHAR_LEN("CharFontFamily"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_CHAR_SET                     (MAP_CHAR_LEN("CharFontCharSet"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_PITCH                        (MAP_CHAR_LEN("CharFontPitch"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_POSTURE                           (MAP_CHAR_LEN("CharPosture"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_WEIGHT                            (MAP_CHAR_LEN("CharWeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_LOCALE                            (MAP_CHAR_LEN("CharLocale"));

const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_HEIGHT_ASIAN                      (MAP_CHAR_LEN("CharHeightAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_NAME_ASIAN                   (MAP_CHAR_LEN("CharFontNameAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN             (MAP_CHAR_LEN("CharFontStyleNameAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_FAMILY_ASIAN                 (MAP_CHAR_LEN("CharFontFamilyAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN               (MAP_CHAR_LEN("CharFontCharSetAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_PITCH_ASIAN                  (MAP_CHAR_LEN("CharFontPitchAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_POSTURE_ASIAN                     (MAP_CHAR_LEN("CharPostureAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_WEIGHT_ASIAN                      (MAP_CHAR_LEN("CharWeightAsian"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_LOCALE_ASIAN                      (MAP_CHAR_LEN("CharLocaleAsian"));

const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_HEIGHT_COMPLEX                    (MAP_CHAR_LEN("CharHeightComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_NAME_COMPLEX                 (MAP_CHAR_LEN("CharFontNameComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX           (MAP_CHAR_LEN("CharFontStyleNameComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_FAMILY_COMPLEX               (MAP_CHAR_LEN("CharFontFamilyComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX             (MAP_CHAR_LEN("CharFontCharSetComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FONT_PITCH_COMPLEX                (MAP_CHAR_LEN("CharFontPitchComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_POSTURE_COMPLEX                   (MAP_CHAR_LEN("CharPostureComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_WEIGHT_COMPLEX                    (MAP_CHAR_LEN("CharWeightComplex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_LOCALE_COMPLEX                    (MAP_CHAR_LEN("CharLocaleComplex"));

const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_AUTO_KERNING                      (MAP_CHAR_LEN("CharAutoKerning"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_UNDERLINE                         (MAP_CHAR_LEN("CharUnderline"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_UNDERLINE_COLOR                           (MAP_CHAR_LEN("CharUnderlineColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_UNDERLINE_HAS_COLOR                           (MAP_CHAR_LEN("CharUnderlineHasColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_ESCAPEMENT                        (MAP_CHAR_LEN("CharEscapement"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_CASE_MAP                          (MAP_CHAR_LEN("CharCaseMap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_SHADOWED                          (MAP_CHAR_LEN("CharShadowed"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_STRIKEOUT                         (MAP_CHAR_LEN("CharStrikeout"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_CROSSED_OUT                       (MAP_CHAR_LEN("CharCrossedOut"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_NO_HYPHENATION                    (MAP_CHAR_LEN("CharNoHyphenation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_AUTO_ESCAPEMENT                   (MAP_CHAR_LEN("CharAutoEscapement"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_PROP_HEIGHT                       (MAP_CHAR_LEN("CharPropHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_DIFF_HEIGHT                       (MAP_CHAR_LEN("CharDiffHeight"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_PROP_HEIGHT_ASIAN                  (MAP_CHAR_LEN("CharPropHeightAsian"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN                  (MAP_CHAR_LEN("CharDiffHeightAsian"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX                (MAP_CHAR_LEN("CharPropHeightComplex"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX                (MAP_CHAR_LEN("CharDiffHeightComplex"));




const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_ESCAPEMENT_HEIGHT                 (MAP_CHAR_LEN("CharEscapementHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_COLOR                             (MAP_CHAR_LEN("CharColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_FLASH                             (MAP_CHAR_LEN("CharFlash"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_KERNING                           (MAP_CHAR_LEN("CharKerning"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_BACK_COLOR                        (MAP_CHAR_LEN("CharBackColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_BACK_TRANSPARENT          (MAP_CHAR_LEN("CharBackTransparent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_COMBINE_IS_ON         (MAP_CHAR_LEN("CharCombineIsOn"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_COMBINE_PREFIX            (MAP_CHAR_LEN("CharCombinePrefix"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_COMBINE_SUFFIX            (MAP_CHAR_LEN("CharCombineSuffix"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_EMPHASIS                  (MAP_CHAR_LEN("CharEmphasis"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LINE_SPACING                      (MAP_CHAR_LEN("ParaLineSpacing"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_TOP_MARGIN                        (MAP_CHAR_LEN("ParaTopMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_BOTTOM_MARGIN                     (MAP_CHAR_LEN("ParaBottomMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_TOP_MARGIN_RELATIVE               (MAP_CHAR_LEN("ParaTopMarginRelative"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE            (MAP_CHAR_LEN("ParaBottomMarginRelative"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_EXPAND_SINGLE_WORD                (MAP_CHAR_LEN("ParaExpandSingleWord"));
const SwPropNameLen __FAR_DATA  UNO_NAME_END_NOTICE                             (MAP_CHAR_LEN("EndNotice"));
const SwPropNameLen __FAR_DATA  UNO_NAME_EMBEDDED_OBJECTS                       (MAP_CHAR_LEN("EmbeddedObjects"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ALPHABETICAL_SEPARATORS                (MAP_CHAR_LEN("AlphabeticalSeparators"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACKGROUND_COLOR                       (MAP_CHAR_LEN("BackgroundColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BEGIN_NOTICE                           (MAP_CHAR_LEN("BeginNotice"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CASE_SENSITIVE                         (MAP_CHAR_LEN("CaseSensitive"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FRAME_STYLE_NAME                       (MAP_CHAR_LEN("FrameStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_STYLE_NAME                   (MAP_CHAR_LEN("NumberingStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_LEVEL                        (MAP_CHAR_LEN("NumberingLevel"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_START_VALUE                  (MAP_CHAR_LEN("NumberingStartValue"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_IS_NUMBER                    (MAP_CHAR_LEN("NumberingIsNumber"));
const SwPropNameLen __FAR_DATA  UNO_NAME_COMBINE_ENTRIES                        (MAP_CHAR_LEN("CombineEntries"));
const SwPropNameLen __FAR_DATA  UNO_NAME_COUNT_LINES_IN_FRAMES                  (MAP_CHAR_LEN("CountLinesInFrames"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DDE_COMMAND_TYPE                       (MAP_CHAR_LEN("DDECommandType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DDE_COMMAND_FILE                       (MAP_CHAR_LEN("DDECommandFile"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DDE_COMMAND_ELEMENT                    (MAP_CHAR_LEN("DDECommandElement"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_AUTOMATIC_UPDATE                    (MAP_CHAR_LEN("IsAutomaticUpdate"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DEFAULT_TABSTOP_DISTANCE               (MAP_CHAR_LEN("DefaultTabstopDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DISTANCE                               (MAP_CHAR_LEN("Distance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DROP_CAP_FORMAT                        (MAP_CHAR_LEN("DropCapFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DROP_CAP_WHOLE_WORD                    (MAP_CHAR_LEN("DropCapWholeWord"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DROP_CAP_CHAR_STYLE_NAME               (MAP_CHAR_LEN("DropCapCharStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILE_LINK                              (MAP_CHAR_LEN("FileLink"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC                                (MAP_CHAR_LEN("Graphic"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHICS                               (MAP_CHAR_LEN("Graphics"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_PROTECTED                           (MAP_CHAR_LEN("IsProtected"));
const SwPropNameLen __FAR_DATA  UNO_NAME_KEY_AS_ENTRY                           (MAP_CHAR_LEN("KeyAsEntry"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_KEEP_TOGETHER                     (MAP_CHAR_LEN("ParaKeepTogether"));
const SwPropNameLen __FAR_DATA  UNO_NAME_KEEP_TOGETHER                          (MAP_CHAR_LEN("KeepTogether"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_LANDSCAPE                           (MAP_CHAR_LEN("IsLandscape"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEPARATOR_TEXT                         (MAP_CHAR_LEN("SeparatorText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_MARKS                                  (MAP_CHAR_LEN("Marks"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBER_POSITION                        (MAP_CHAR_LEN("NumberPosition"));
const SwPropNameLen __FAR_DATA  UNO_NAME_OUTLINES                               (MAP_CHAR_LEN("Outlines"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGE_STYLE_NAME                        (MAP_CHAR_LEN("PageStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGE_STYLE_LAYOUT                      (MAP_CHAR_LEN("PageStyleLayout"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_STYLES                            (MAP_CHAR_LEN("ParaStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_ADJUST                            (MAP_CHAR_LEN("ParaAdjust"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_REGISTER_MODE_ACTIVE              (MAP_CHAR_LEN("ParaRegisterModeActive"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_STYLE_NAME                        (MAP_CHAR_LEN("ParaStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LAST_LINE_ADJUST                  (MAP_CHAR_LEN("ParaLastLineAdjust"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LINE_NUMBER_COUNT                 (MAP_CHAR_LEN("ParaLineNumberCount"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_LINE_NUMBER_START_VALUE           (MAP_CHAR_LEN("ParaLineNumberStartValue"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_COLOR                             (MAP_CHAR_LEN("BackColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_BACK_COLOR                        (MAP_CHAR_LEN("ParaBackColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_WIDOWS                            (MAP_CHAR_LEN("ParaWidows"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_ORPHANS                           (MAP_CHAR_LEN("ParaOrphans"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_BACK_TRANSPARENT          (MAP_CHAR_LEN("ParaBackTransparent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_POSITION_END_OF_DOC                    (MAP_CHAR_LEN("PositionEndOfDoc"));
const SwPropNameLen __FAR_DATA  UNO_NAME_POSITION_PROTECTED                     (MAP_CHAR_LEN("PositionProtected"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ALTERNATIVE_TEXT                       (MAP_CHAR_LEN("AlternativeText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRIMARY_KEY                            (MAP_CHAR_LEN("PrimaryKey"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_TABLES                           (MAP_CHAR_LEN("PrintTables"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_GRAPHICS                         (MAP_CHAR_LEN("PrintGraphics"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_REVERSED                         (MAP_CHAR_LEN("PrintReversed"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_PROSPECT                         (MAP_CHAR_LEN("PrintProspect"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_CONTROLS                         (MAP_CHAR_LEN("PrintControls"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_DRAWINGS                         (MAP_CHAR_LEN("PrintDrawings"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_RIGHT_PAGES                      (MAP_CHAR_LEN("PrintRightPages"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_LEFT_PAGES                       (MAP_CHAR_LEN("PrintLeftPages"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_BLACK_FONTS                      (MAP_CHAR_LEN("PrintBlackFonts"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINTER_PAPER_TRAY                     (MAP_CHAR_LEN("PrinterPaperTray"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_PAGE_BACKGROUND                  (MAP_CHAR_LEN("PrintPageBackground"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_ANNOTATION_MODE                  (MAP_CHAR_LEN("PrintAnnotationMode"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_FAX_NAME                         (MAP_CHAR_LEN("PrintFaxName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT_PAPER_FROM_SETUP                 (MAP_CHAR_LEN("PrintPaperFromSetup"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REGISTER_MODE_ACTIVE                   (MAP_CHAR_LEN("RegisterModeActive"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RELATIVE_WIDTH                         (MAP_CHAR_LEN("RelativeWidth"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RELATIVE_HEIGHT                        (MAP_CHAR_LEN("RelativeHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REPEAT_HEADLINE                        (MAP_CHAR_LEN("RepeatHeadline"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_STYLES                          (MAP_CHAR_LEN("SearchStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_BACKWARDS                       (MAP_CHAR_LEN("SearchBackwards"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_SIMILARITY                      (MAP_CHAR_LEN("SearchSimilarity"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_IN_SELECTION                    (MAP_CHAR_LEN("SearchInSelection"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_CASE_SENSITIVE                  (MAP_CHAR_LEN("SearchCaseSensitive"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_SIMILARITY_ADD                  (MAP_CHAR_LEN("SearchSimilarityAdd"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_SIMILARITY_RELAX                (MAP_CHAR_LEN("SearchSimilarityRelax"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_SIMILARITY_REMOVE               (MAP_CHAR_LEN("SearchSimilarityRemove"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_REGULAR_EXPRESSION              (MAP_CHAR_LEN("SearchRegularExpression"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_SIMILARITY_EXCHANGE             (MAP_CHAR_LEN("SearchSimilarityExchange"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SECONDARY_KEY                          (MAP_CHAR_LEN("SecondaryKey"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEPARATOR_INTERVAL                     (MAP_CHAR_LEN("SeparatorInterval"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_BREAKS                            (MAP_CHAR_LEN("ShowBreaks"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_SPACES                            (MAP_CHAR_LEN("ShowSpaces"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_TABLES                            (MAP_CHAR_LEN("ShowTables"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_GRAPHICS                          (MAP_CHAR_LEN("ShowGraphics"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_DRAWINGS                          (MAP_CHAR_LEN("ShowDrawings"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_TABSTOPS                          (MAP_CHAR_LEN("ShowTabstops"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_VERT_RULER                        (MAP_CHAR_LEN("ShowVertRuler"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_PARA_BREAKS                       (MAP_CHAR_LEN("ShowParaBreaks"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_HIDDEN_TEXT                       (MAP_CHAR_LEN("ShowHiddenText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_ANNOTATIONS                       (MAP_CHAR_LEN("ShowAnnotations"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_SOFT_HYPHENS                      (MAP_CHAR_LEN("ShowSoftHyphens"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_VERT_SCROLL_BAR                   (MAP_CHAR_LEN("ShowVertScrollBar"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_HORI_SCROLL_BAR                   (MAP_CHAR_LEN("ShowHoriScrollBar"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_FIELD_COMMANDS                    (MAP_CHAR_LEN("ShowFieldCommands"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_TEXT_BOUNDARIES                   (MAP_CHAR_LEN("ShowTextBoundaries"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_PROTECTED_SPACES                  (MAP_CHAR_LEN("ShowProtectedSpaces"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_TABLE_BOUNDARIES                  (MAP_CHAR_LEN("ShowTableBoundaries"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_HIDDEN_PARAGRAPHS                 (MAP_CHAR_LEN("ShowHiddenParagraphs"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_INDEX_MARK_BACKGROUND             (MAP_CHAR_LEN("ShowIndexMarkBackground"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_FOOTNOTE_BACKGROUND               (MAP_CHAR_LEN("ShowFootnoteBackground"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_TEXT_FIELD_BACKGROUND             (MAP_CHAR_LEN("ShowTextFieldBackground"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT                (MAP_CHAR_LEN("IsSyncWidthToHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH                (MAP_CHAR_LEN("IsSyncHeightToWidth"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SIZE_PROTECTED                         (MAP_CHAR_LEN("SizeProtected"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SMOOTH_SCROLLING                       (MAP_CHAR_LEN("SmoothScrolling"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SOLID_MARK_HANDLES                     (MAP_CHAR_LEN("SolidMarkHandles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLES                                 (MAP_CHAR_LEN("Tables"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_FRAMES                            (MAP_CHAR_LEN("TextFrames"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_COLUMNS                           (MAP_CHAR_LEN("TextColumns"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BACK_TRANSPARENT               (MAP_CHAR_LEN("BackTransparent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_PP                                 (MAP_CHAR_LEN("UsePP"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USER_METRIC                            (MAP_CHAR_LEN("UserMetric"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ANCHOR_TYPE                            (MAP_CHAR_LEN("AnchorType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ANCHOR_TYPES                           (MAP_CHAR_LEN("AnchorTypes"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ANCHOR_PAGE_NO                         (MAP_CHAR_LEN("AnchorPageNo"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ANCHOR_FRAME                           (MAP_CHAR_LEN("AnchorFrame"));
const SwPropNameLen __FAR_DATA  UNO_NAME_AUTHOR                                 (MAP_CHAR_LEN("Author"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BREAK_TYPE                             (MAP_CHAR_LEN("BreakType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAIN_NEXT_NAME                        (MAP_CHAR_LEN("ChainNextName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAIN_PREV_NAME                        (MAP_CHAR_LEN("ChainPrevName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAPTER_FORMAT                         (MAP_CHAR_LEN("ChapterFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CLIENT_MAP                             (MAP_CHAR_LEN("ClientMap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CONDITION                              (MAP_CHAR_LEN("Condition"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CONTENT                                (MAP_CHAR_LEN("Content"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAR_CONTOURED                         (MAP_CHAR_LEN("CharContoured"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CONTOUR_OUTSIDE                        (MAP_CHAR_LEN("ContourOutside"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CONTENT_PROTECTED                      (MAP_CHAR_LEN("ContentProtected"));
const SwPropNameLen __FAR_DATA  UNO_NAME_COUNT_EMPTY_LINES                      (MAP_CHAR_LEN("CountEmptyLines"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RESTART_AT_EACH_PAGE                   (MAP_CHAR_LEN("RestartAtEachPage"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DATA_BASE_NAME                         (MAP_CHAR_LEN("DataBaseName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DATA_TABLE_NAME                        (MAP_CHAR_LEN("DataTableName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DATA_COMMAND_TYPE                      (MAP_CHAR_LEN("DataCommandType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DATA_COLUMN_NAME                       (MAP_CHAR_LEN("DataColumnName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_DATA_BASE_FORMAT                    (MAP_CHAR_LEN("DataBaseFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DATE                                   (MAP_CHAR_LEN("Date"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_DATE                                (MAP_CHAR_LEN("IsDate"));
const SwPropNameLen __FAR_DATA  UNO_NAME_EDIT_IN_READONLY                       (MAP_CHAR_LEN("EditInReadonly"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FALSE_CONTENT                          (MAP_CHAR_LEN("FalseContent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILE_FORMAT                            (MAP_CHAR_LEN("FileFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_FIXED                               (MAP_CHAR_LEN("IsFixed"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTNOTE_COUNTING                      (MAP_CHAR_LEN("FootnoteCounting"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FORMULA                                (MAP_CHAR_LEN("Formula"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FRAME_NAME                             (MAP_CHAR_LEN("FrameName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_NAME                           (MAP_CHAR_LEN("GraphicName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FULL_NAME                              (MAP_CHAR_LEN("FullName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEIGHT                                 (MAP_CHAR_LEN("Height"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_AUTO_HEIGHT                         (MAP_CHAR_LEN("IsAutoHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SIZE_TYPE                              (MAP_CHAR_LEN("SizeType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HINT                                   (MAP_CHAR_LEN("Hint"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HORI_ORIENT                            (MAP_CHAR_LEN("HoriOrient"));
const SwPropNameLen __FAR_DATA UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES             (MAP_CHAR_LEN("HoriMirroredOnEvenPages"));
const SwPropNameLen __FAR_DATA UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES              (MAP_CHAR_LEN("HoriMirroredOnOddPages"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HORI_ORIENT_RELATION                   (MAP_CHAR_LEN("HoriOrientRelation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HORI_ORIENT_POSITION                   (MAP_CHAR_LEN("HoriOrientPosition"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HYPER_LINK_U_R_L                       (MAP_CHAR_LEN("HyperLinkURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HYPER_LINK_TARGET                      (MAP_CHAR_LEN("HyperLinkTarget"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HYPER_LINK_NAME                        (MAP_CHAR_LEN("HyperLinkName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HYPER_LINK_EVENTS                      (MAP_CHAR_LEN("HyperLinkEvents"));
const SwPropNameLen __FAR_DATA  UNO_NAME_INFO_TYPE                              (MAP_CHAR_LEN("InfoType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_INFO_FORMAT                            (MAP_CHAR_LEN("InfoFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_INPUT                               (MAP_CHAR_LEN("Input"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEVEL                                  (MAP_CHAR_LEN("Level"));
const SwPropNameLen __FAR_DATA  UNO_NAME_INTERVAL                               (MAP_CHAR_LEN("Interval"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LINK_REGION                            (MAP_CHAR_LEN("LinkRegion"));
const SwPropNameLen __FAR_DATA  UNO_NAME_MACRO                                  (MAP_CHAR_LEN("Macro"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SPLIT                                  (MAP_CHAR_LEN("Split"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_SPLIT                             (MAP_CHAR_LEN("ParaSplit"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBER_FORMAT                          (MAP_CHAR_LEN("NumberFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_TYPE                         (MAP_CHAR_LEN("NumberingType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_RULES                        (MAP_CHAR_LEN("NumberingRules"));
const SwPropNameLen __FAR_DATA  UNO_NAME_OFFSET                                 (MAP_CHAR_LEN("Offset"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ON                                     (MAP_CHAR_LEN("On"));
const SwPropNameLen __FAR_DATA  UNO_NAME_OPAQUE                                 (MAP_CHAR_LEN("Opaque"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGE_TOGGLE                            (MAP_CHAR_LEN("PageToggle"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGE_DESC_NAME                         (MAP_CHAR_LEN("PageDescName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGE_NUMBER_OFFSET                     (MAP_CHAR_LEN("PageNumberOffset"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PLACEHOLDER                            (MAP_CHAR_LEN("PlaceHolder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PLACEHOLDER_TYPE                       (MAP_CHAR_LEN("PlaceHolderType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PRINT                                  (MAP_CHAR_LEN("Print"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REFERENCE_FIELD_PART                   (MAP_CHAR_LEN("ReferenceFieldPart"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REFERENCE_FIELD_SOURCE                 (MAP_CHAR_LEN("ReferenceFieldSource"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REGISTER_PARAGRAPH_STYLE               (MAP_CHAR_LEN("RegisterParagraphStyle"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SCRIPT_TYPE                            (MAP_CHAR_LEN("ScriptType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_ALL                             (MAP_CHAR_LEN("SearchAll"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEARCH_WORDS                           (MAP_CHAR_LEN("SearchWords"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SEQUENCE_VALUE                         (MAP_CHAR_LEN("SequenceValue"));
const SwPropNameLen __FAR_DATA UNO_NAME_IMAGE_MAP                               (MAP_CHAR_LEN("ImageMap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SERVER_MAP                             (MAP_CHAR_LEN("ServerMap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SET_NUMBER                             (MAP_CHAR_LEN("SetNumber"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHADOW_FORMAT                          (MAP_CHAR_LEN("ShadowFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_HORI_RULER                        (MAP_CHAR_LEN("ShowHoriRuler"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SIZE                                   (MAP_CHAR_LEN("Size"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ACTUAL_SIZE                            (MAP_CHAR_LEN("ActualSize"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SOURCE_NAME                            (MAP_CHAR_LEN("SourceName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_START_AT                               (MAP_CHAR_LEN("StartAt"));
const SwPropNameLen __FAR_DATA  UNO_NAME_STATISTIC_TYPE_ID                      (MAP_CHAR_LEN("StatisticTypeId"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SUB_TYPE                               (MAP_CHAR_LEN("SubType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SURROUND                               (MAP_CHAR_LEN("Surround"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_EXPRESSION                          (MAP_CHAR_LEN("IsExpression"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_SHOW_FORMULA                        (MAP_CHAR_LEN("IsShowFormula"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_WRAP                              (MAP_CHAR_LEN("TextWrap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SURROUND_CONTOUR                       (MAP_CHAR_LEN("SurroundContour"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SURROUND_ANCHORONLY                    (MAP_CHAR_LEN("SurroundAnchorOnly"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLE_NAME                             (MAP_CHAR_LEN("TableName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABSTOPS                               (MAP_CHAR_LEN("ParaTabStops"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TITLE                                  (MAP_CHAR_LEN("Title"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TOP_MARGIN                             (MAP_CHAR_LEN("TopMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BOTTOM_MARGIN                          (MAP_CHAR_LEN("BottomMargin"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TRUE_CONTENT                           (MAP_CHAR_LEN("TrueContent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_URL_CONTENT                            (MAP_CHAR_LEN("URLContent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USERTEXT                               (MAP_CHAR_LEN("UserText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USER_DATA_TYPE                         (MAP_CHAR_LEN("UserDataType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VALUE                                  (MAP_CHAR_LEN("Value"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VARIABLE_NAME                          (MAP_CHAR_LEN("VariableName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VARIABLE_SUBTYPE                       (MAP_CHAR_LEN("VariableSubtype"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VERT_ORIENT                            (MAP_CHAR_LEN("VertOrient"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VERT_MIRRORED                          (MAP_CHAR_LEN("VertMirrored"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VERT_ORIENT_POSITION                   (MAP_CHAR_LEN("VertOrientPosition"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VERT_ORIENT_RELATION                   (MAP_CHAR_LEN("VertOrientRelation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_VISIBLE                             (MAP_CHAR_LEN("IsVisible"));
const SwPropNameLen __FAR_DATA  UNO_NAME_WIDTH                                  (MAP_CHAR_LEN("Width"));
const SwPropNameLen __FAR_DATA  UNO_NAME_WORD_MODE                              (MAP_CHAR_LEN("CharWordMode"));
const SwPropNameLen __FAR_DATA  UNO_NAME_GRAPHIC_CROP                           (MAP_CHAR_LEN("GraphicCrop"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHARACTER_FORMAT_NONE                  (MAP_CHAR_LEN("CharacterFormatNone"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_POSITION                          (MAP_CHAR_LEN("TextPosition"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DOCUMENT_INDEX_MARK                    (MAP_CHAR_LEN("DocumentIndexMark"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DOCUMENT_INDEX                         (MAP_CHAR_LEN("DocumentIndex"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION             (MAP_CHAR_LEN("IsGlobalDocumentSection"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_FIELD                             (MAP_CHAR_LEN("TextField"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BOOKMARK                               (MAP_CHAR_LEN("Bookmark"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_TABLE                             (MAP_CHAR_LEN("TextTable"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CELL                                   (MAP_CHAR_LEN("Cell"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_FRAME                             (MAP_CHAR_LEN("TextFrame"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REFERENCE_MARK                         (MAP_CHAR_LEN("ReferenceMark"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_SECTION                           (MAP_CHAR_LEN("TextSection"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTNOTE                               (MAP_CHAR_LEN("Footnote"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ENDNOTE                                (MAP_CHAR_LEN("Endnote"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHART_ROW_AS_LABEL                     (MAP_CHAR_LEN("ChartRowAsLabel"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHART_COLUMN_AS_LABEL                  (MAP_CHAR_LEN("ChartColumnAsLabel"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLE_COLUMS                           (MAP_CHAR_LEN("TableColumns"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEFT_BORDER                            (MAP_CHAR_LEN("LeftBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RIGHT_BORDER                           (MAP_CHAR_LEN("RightBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TOP_BORDER                             (MAP_CHAR_LEN("TopBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BOTTOM_BORDER                          (MAP_CHAR_LEN("BottomBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BORDER_DISTANCE                        (MAP_CHAR_LEN("BorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEFT_BORDER_DISTANCE                   (MAP_CHAR_LEN("LeftBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RIGHT_BORDER_DISTANCE                  (MAP_CHAR_LEN("RightBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TOP_BORDER_DISTANCE                    (MAP_CHAR_LEN("TopBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_BOTTOM_BORDER_DISTANCE                 (MAP_CHAR_LEN("BottomBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLE_BORDER                           (MAP_CHAR_LEN("TableBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLE_COLUMN_SEPARATORS                (MAP_CHAR_LEN("TableColumnSeparators"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TABLE_COLUMN_RELATIVE_SUM              (MAP_CHAR_LEN("TableColumnRelativeSum"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_TEXT                            (MAP_CHAR_LEN("HeaderText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_TEXT_LEFT                       (MAP_CHAR_LEN("HeaderTextLeft"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_TEXT_RIGHT                      (MAP_CHAR_LEN("HeaderTextRight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_TEXT                            (MAP_CHAR_LEN("FooterText"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_TEXT_LEFT                       (MAP_CHAR_LEN("FooterTextLeft"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_TEXT_RIGHT                      (MAP_CHAR_LEN("FooterTextRight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_BACK_COLOR                      (MAP_CHAR_LEN("HeaderBackColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_GRAPHIC                         (MAP_CHAR_LEN("HeaderBackGraphic"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_BACK_TRANSPARENT        (MAP_CHAR_LEN("HeaderBackTransparent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_LEFT_BORDER                     (MAP_CHAR_LEN("HeaderLeftBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_RIGHT_BORDER                    (MAP_CHAR_LEN("HeaderRightBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_TOP_BORDER                      (MAP_CHAR_LEN("HeaderTopBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_BOTTOM_BORDER                   (MAP_CHAR_LEN("HeaderBottomBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_BORDER_DISTANCE                 (MAP_CHAR_LEN("HeaderBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_SHADOW_FORMAT                   (MAP_CHAR_LEN("HeaderShadowFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_BODY_DISTANCE                   (MAP_CHAR_LEN("HeaderBodyDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT               (MAP_CHAR_LEN("HeaderIsDynamicHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_IS_SHARED                       (MAP_CHAR_LEN("HeaderIsShared"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_HEIGHT                          (MAP_CHAR_LEN("HeaderHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADER_IS_ON                           (MAP_CHAR_LEN("HeaderIsOn"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_BACK_COLOR                      (MAP_CHAR_LEN("FooterBackColor"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_GRAPHIC                         (MAP_CHAR_LEN("FooterBackGraphic"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_BACK_TRANSPARENT                (MAP_CHAR_LEN("FooterBackTransparent"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_LEFT_BORDER                     (MAP_CHAR_LEN("FooterLeftBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_RIGHT_BORDER                    (MAP_CHAR_LEN("FooterRightBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_TOP_BORDER                      (MAP_CHAR_LEN("FooterTopBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_BOTTOM_BORDER                   (MAP_CHAR_LEN("FooterBottomBorder"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_BORDER_DISTANCE                 (MAP_CHAR_LEN("FooterBorderDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_SHADOW_FORMAT                   (MAP_CHAR_LEN("FooterShadowFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_BODY_DISTANCE                   (MAP_CHAR_LEN("FooterBodyDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT               (MAP_CHAR_LEN("FooterIsDynamicHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_IS_SHARED                       (MAP_CHAR_LEN("FooterIsShared"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_HEIGHT                          (MAP_CHAR_LEN("FooterHeight"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FOOTER_IS_ON                           (MAP_CHAR_LEN("FooterIsOn"));
const SwPropNameLen __FAR_DATA  UNO_NAME_OVERWRITE_STYLES                       (MAP_CHAR_LEN("OverwriteStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LOAD_NUMBERING_STYLES                  (MAP_CHAR_LEN("LoadNumberingStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LOAD_PAGE_STYLES                       (MAP_CHAR_LEN("LoadPageStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LOAD_FRAME_STYLES                      (MAP_CHAR_LEN("LoadFrameStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LOAD_TEXT_STYLES                       (MAP_CHAR_LEN("LoadTextStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILE_NAME                              (MAP_CHAR_LEN("FileName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILTER_NAME                            (MAP_CHAR_LEN("FilterName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILTER_OPTION                          (MAP_CHAR_LEN("FilterOption"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PASSWORD                               (MAP_CHAR_LEN("Password"));
const SwPropNameLen __FAR_DATA  UNO_NAME_COPY_COUNT                             (MAP_CHAR_LEN("CopyCount"));
const SwPropNameLen __FAR_DATA  UNO_NAME_COLLATE                                (MAP_CHAR_LEN("Collate"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SORT                                   (MAP_CHAR_LEN("Sort"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PAGES                                  (MAP_CHAR_LEN("Pages"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FIRST_LINE_OFFSET                      (MAP_CHAR_LEN("FirstLineOffset"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SYMBOL_TEXT_DISTANCE                   (MAP_CHAR_LEN("SymbolTextDistance"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USER_INDEX_NAME                        (MAP_CHAR_LEN("UserIndexName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_REVISION                               (MAP_CHAR_LEN("Revision"));
const SwPropNameLen __FAR_DATA  UNO_NAME_UNVISITED_CHAR_STYLE_NAME              (MAP_CHAR_LEN("UnvisitedCharStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_VISITED_CHAR_STYLE_NAME                (MAP_CHAR_LEN("VisitedCharStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARAGRAPH_COUNT                        (MAP_CHAR_LEN("ParagraphCount"));
const SwPropNameLen __FAR_DATA  UNO_NAME_WORD_COUNT                             (MAP_CHAR_LEN("WordCount"));
const SwPropNameLen __FAR_DATA  UNO_NAME_WORD_SEPARATOR                         (MAP_CHAR_LEN("WordSeparator"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHARACTER_COUNT                        (MAP_CHAR_LEN("CharacterCount"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ZOOM_VALUE                             (MAP_CHAR_LEN("ZoomValue"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ZOOM_TYPE                              (MAP_CHAR_LEN("ZoomType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_MARKS                      (MAP_CHAR_LEN("CreateFromMarks"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_OUTLINE                    (MAP_CHAR_LEN("CreateFromOutline"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARAGRAPH_STYLE_NAMES                  (MAP_CHAR_LEN("ParagraphStyleNames"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_CHAPTER                    (MAP_CHAR_LEN("CreateFromChapter"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_LABELS                     (MAP_CHAR_LEN("CreateFromLabels"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_ALPHABETICAL_SEPARATORS            (MAP_CHAR_LEN("UseAlphabeticalSeparators"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_KEY_AS_ENTRY                       (MAP_CHAR_LEN("UseKeyAsEntry"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_COMBINED_ENTRIES                   (MAP_CHAR_LEN("UseCombinedEntries"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_CASE_SENSITIVE                      (MAP_CHAR_LEN("IsCaseSensitive"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_P_P                                (MAP_CHAR_LEN("UsePP"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_DASH                               (MAP_CHAR_LEN("UseDash"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_UPPER_CASE                         (MAP_CHAR_LEN("UseUpperCase"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LABEL_CATEGORY                         (MAP_CHAR_LEN("LabelCategory"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LABEL_DISPLAY_TYPE                     (MAP_CHAR_LEN("LabelDisplayType"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USE_LEVEL_FROM_SOURCE                  (MAP_CHAR_LEN("UseLevelFromSource"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEVEL_FORMAT                           (MAP_CHAR_LEN("LevelFormat"));
const SwPropNameLen __FAR_DATA  UNO_NAME_LEVEL_PARAGRAPH_STYLES                 (MAP_CHAR_LEN("LevelParagraphStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RECALC_TAB_STOPS                       (MAP_CHAR_LEN("RecalcTabStops"));
const SwPropNameLen __FAR_DATA  UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME        (MAP_CHAR_LEN("MainEntryCharacterStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_TABLES                     (MAP_CHAR_LEN("CreateFromTables"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_TEXT_FRAMES                (MAP_CHAR_LEN("CreateFromTextFrames"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS            (MAP_CHAR_LEN("CreateFromGraphicObjects"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS           (MAP_CHAR_LEN("CreateFromEmbeddedObjects"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_STAR_MATH                  (MAP_CHAR_LEN("CreateFromStarMath"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_STAR_CHART                 (MAP_CHAR_LEN("CreateFromStarChart"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_STAR_CALC                  (MAP_CHAR_LEN("CreateFromStarCalc"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_STAR_DRAW                  (MAP_CHAR_LEN("CreateFromStarDraw"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS     (MAP_CHAR_LEN("CreateFromOtherEmbeddedObjects"));
const SwPropNameLen __FAR_DATA  UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L             (MAP_CHAR_LEN("IndexAutoMarkFileURL"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_COMMA_SEPARATED                     (MAP_CHAR_LEN("IsCommaSeparated"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_RELATIVE_TABSTOPS                   (MAP_CHAR_LEN("IsRelativeTabstops"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES     (MAP_CHAR_LEN("CreateFromLevelParagraphStyles"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_CHANGES                           (MAP_CHAR_LEN("ShowChanges"));
const SwPropNameLen __FAR_DATA  UNO_NAME_RECORD_CHANGES                         (MAP_CHAR_LEN("RecordChanges"));
const SwPropNameLen __FAR_DATA  UNO_LINK_DISPLAY_NAME                           (MAP_CHAR_LEN("LinkDisplayName"));
const SwPropNameLen __FAR_DATA  UNO_LINK_DISPLAY_BITMAP                         (MAP_CHAR_LEN("LinkDisplayBitmap"));
const SwPropNameLen __FAR_DATA  UNO_NAME_HEADING_STYLE_NAME                     (MAP_CHAR_LEN("HeadingStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_SHOW_ONLINE_LAYOUT                     (MAP_CHAR_LEN("ShowOnlineLayout"));
const SwPropNameLen __FAR_DATA  UNO_NAME_USER_DEFINED_ATTRIBUTES                (MAP_CHAR_LEN("UserDefinedAttributes"));
const SwPropNameLen __FAR_DATA  UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES           (MAP_CHAR_LEN("TextUserDefinedAttributes"));
const SwPropNameLen __FAR_DATA  UNO_NAME_FILE_PATH                              (MAP_CHAR_LEN("FilePath"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL           (MAP_CHAR_LEN("ParaChapterNumberingLevel"));
const SwPropNameLen __FAR_DATA  UNO_NAME_PARA_CONDITIONAL_STYLE_NAME            (MAP_CHAR_LEN("ParaConditionalStyleName"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CHAPTER_NUMBERING_LEVEL                (MAP_CHAR_LEN("ChapterNumberingLevel"));
const SwPropNameLen __FAR_DATA  UNO_NAME_NUMBERING_SEPARATOR                    (MAP_CHAR_LEN("NumberingSeparator"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_CONTINUOUS_NUMBERING                (MAP_CHAR_LEN("IsContinuousNumbering"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_AUTOMATIC                           (MAP_CHAR_LEN("IsAutomatic"));
const SwPropNameLen __FAR_DATA  UNO_NAME_IS_ABSOLUTE_MARGINS                    (MAP_CHAR_LEN("IsAbsoluteMargins"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CATEGORY                               (MAP_CHAR_LEN("Category"));
const SwPropNameLen __FAR_DATA  UNO_NAME_DEPENDENT_TEXT_FIELDS                  (MAP_CHAR_LEN("DependentTextFields"));
const SwPropNameLen __FAR_DATA  UNO_NAME_CURRENT_PRESENTATION                   (MAP_CHAR_LEN("CurrentPresentation"));
const SwPropNameLen __FAR_DATA  UNO_NAME_ADJUST                                 (MAP_CHAR_LEN("Adjust"));
const SwPropNameLen __FAR_DATA  UNO_NAME_INSTANCE_NAME                          (MAP_CHAR_LEN("InstanceName"));

const SwPropNameLen __FAR_DATA UNO_NAME_TEXT_PORTION_TYPE                       (MAP_CHAR_LEN("TextPortionType"));
const SwPropNameLen __FAR_DATA UNO_NAME_CONTROL_CHARACTER                       (MAP_CHAR_LEN("ControlCharacter"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_COLLAPSED                            (MAP_CHAR_LEN("IsCollapsed"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_START                                (MAP_CHAR_LEN("IsStart"));
const SwPropNameLen __FAR_DATA UNO_NAME_SEQUENCE_NUMBER                         (MAP_CHAR_LEN("SequenceNumber"));
const SwPropNameLen __FAR_DATA UNO_NAME_REFERENCE_ID                            (MAP_CHAR_LEN("ReferenceId"));

const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_LEFT_BORDER_DISTANCE             (MAP_CHAR_LEN("HeaderLeftBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE            (MAP_CHAR_LEN("HeaderRightBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_TOP_BORDER_DISTANCE              (MAP_CHAR_LEN("HeaderTopBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE           (MAP_CHAR_LEN("HeaderBottomBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_USER_DEFINED_ATTRIBUTES          (MAP_CHAR_LEN("HeaderUserDefinedAttributes"));

const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE             (MAP_CHAR_LEN("FooterLeftBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE            (MAP_CHAR_LEN("FooterRightBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_TOP_BORDER_DISTANCE              (MAP_CHAR_LEN("FooterTopBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE           (MAP_CHAR_LEN("FooterBottomBorderDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTER_USER_DEFINED_ATTRIBUTES          (MAP_CHAR_LEN("FooterUserDefinedAttributes"));


const SwPropNameLen __FAR_DATA UNO_NAME_PARA_IS_NUMBERING_RESTART               (MAP_CHAR_LEN("ParaIsNumberingRestart"));
const SwPropNameLen __FAR_DATA UNO_NAME_HIDE_FIELD_TIPS                         (MAP_CHAR_LEN("HideFieldTips"));
const SwPropNameLen __FAR_DATA UNO_NAME_PARA_SHADOW_FORMAT                      (MAP_CHAR_LEN("ParaShadowFormat"));

const SwPropNameLen __FAR_DATA UNO_NAME_CONTOUR_POLY_POLYGON                    (MAP_CHAR_LEN("ContourPolyPolygon"));

const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_WIDTH                    (MAP_CHAR_LEN("SeparatorLineWidth"));
const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_COLOR                    (MAP_CHAR_LEN("SeparatorLineColor"));
const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT          (MAP_CHAR_LEN("SeparatorLineRelativeHeight"));
const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT       (MAP_CHAR_LEN("SeparatorLineVerticalAlignment"));
const SwPropNameLen __FAR_DATA UNO_NAME_SEPARATOR_LINE_IS_ON                    (MAP_CHAR_LEN("SeparatorLineIsOn"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_SKIP_HIDDEN_TEXT                     (MAP_CHAR_LEN("IsSkipHiddenText"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_SKIP_PROTECTED_TEXT                  (MAP_CHAR_LEN("IsSkipProtectedText"));
const SwPropNameLen __FAR_DATA UNO_NAME_DOCUMENT_INDEX_MARKS                    (MAP_CHAR_LEN("DocumentIndexMarks"));

const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END     (MAP_CHAR_LEN("FootnoteIsCollectAtTextEnd"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING       (MAP_CHAR_LEN("FootnoteIsRestartNumbering"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT       (MAP_CHAR_LEN("FootnoteRestartNumberingAt"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING           (MAP_CHAR_LEN("FootnoteIsOwnNumbering"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_NUMBERING_TYPE             (MAP_CHAR_LEN("FootnoteNumberingType"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_NUMBERING_PREFIX           (MAP_CHAR_LEN("FootnoteNumberingPrefix"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX           (MAP_CHAR_LEN("FootnoteNumberingSuffix"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END      (MAP_CHAR_LEN("EndnoteIsCollectAtTextEnd"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING        (MAP_CHAR_LEN("EndnoteIsRestartNumbering"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT        (MAP_CHAR_LEN("EndnoteRestartNumberingAt"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_IS_OWN_NUMBERING            (MAP_CHAR_LEN("EndnoteIsOwnNumbering"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_NUMBERING_TYPE              (MAP_CHAR_LEN("EndnoteNumberingType"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_NUMBERING_PREFIX            (MAP_CHAR_LEN("EndnoteNumberingPrefix"));
const SwPropNameLen __FAR_DATA UNO_NAME_ENDNOTE_NUMBERING_SUFFIX            (MAP_CHAR_LEN("EndnoteNumberingSuffix"));

const SwPropNameLen __FAR_DATA UNO_NAME_BRACKET_BEFORE                      (MAP_CHAR_LEN("BracketBefore"));
const SwPropNameLen __FAR_DATA UNO_NAME_BRACKET_AFTER                       (MAP_CHAR_LEN("BracketAfter"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_NUMBER_ENTRIES                   (MAP_CHAR_LEN("IsNumberEntries"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_SORT_BY_POSITION                 (MAP_CHAR_LEN("IsSortByPosition"));
const SwPropNameLen __FAR_DATA UNO_NAME_SORT_KEYS                           (MAP_CHAR_LEN("SortKeys"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_SORT_ASCENDING                   (MAP_CHAR_LEN("IsSortAscending"));
const SwPropNameLen __FAR_DATA UNO_NAME_SORT_KEY                            (MAP_CHAR_LEN("SortKey"));
const SwPropNameLen __FAR_DATA UNO_NAME_FIELDS                              (MAP_CHAR_LEN("Fields"));
const SwPropNameLen __FAR_DATA UNO_NAME_DATE_TIME_VALUE                     (MAP_CHAR_LEN("DateTimeValue"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_ON                               (MAP_CHAR_LEN("IsOn"));
const SwPropNameLen __FAR_DATA UNO_NAME_Z_ORDER                             (MAP_CHAR_LEN("ZOrder"));

const SwPropNameLen __FAR_DATA UNO_NAME_CONTENT_SECTION                     (MAP_CHAR_LEN("ContentSection"));
const SwPropNameLen __FAR_DATA UNO_NAME_HEADER_SECTION                      (MAP_CHAR_LEN("HeaderSection"));

const SwPropNameLen __FAR_DATA UNO_NAME_PARA_IS_HANGING_PUNCTUATION         (MAP_CHAR_LEN("ParaIsHangingPunctuation"));
const SwPropNameLen __FAR_DATA UNO_NAME_PARA_IS_CHARACTER_DISTANCE          (MAP_CHAR_LEN("ParaIsCharacterDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_PARA_IS_FORBIDDEN_RULES             (MAP_CHAR_LEN("ParaIsForbiddenRules"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_MAIN_ENTRY                       (MAP_CHAR_LEN("IsMainEntry"));

const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_ROTATION                    (MAP_CHAR_LEN("GraphicRotation"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST_LUMINANCE                    (MAP_CHAR_LEN("AdjustLuminance"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST_CONTRAST                 (MAP_CHAR_LEN("AdjustContrast"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST_RED                         (MAP_CHAR_LEN("AdjustRed"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST_GREEN                       (MAP_CHAR_LEN("AdjustGreen"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADJUST_BLUE                        (MAP_CHAR_LEN("AdjustBlue"));
const SwPropNameLen __FAR_DATA UNO_NAME_GAMMA                       (MAP_CHAR_LEN("Gamma"));
const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_IS_INVERTED                  (MAP_CHAR_LEN("GraphicIsInverted"));
const SwPropNameLen __FAR_DATA UNO_NAME_TRANSPARENCY                (MAP_CHAR_LEN("Transparency"));
const SwPropNameLen __FAR_DATA UNO_NAME_GRAPHIC_COLOR_MODE                  (MAP_CHAR_LEN("GraphicColorMode"));

const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_AUTHOR                      (MAP_CHAR_LEN("RedlineAuthor"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_DATE_TIME                   (MAP_CHAR_LEN("RedlineDateTime"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_COMMENT                     (MAP_CHAR_LEN("RedlineComment"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_TYPE                        (MAP_CHAR_LEN("RedlineType"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_SUCCESSOR_DATA              (MAP_CHAR_LEN("RedlineSuccessorData"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_IDENTIFIER                  (MAP_CHAR_LEN("RedlineIdentifier"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_IN_HEADER_FOOTER                 (MAP_CHAR_LEN("IsInHeaderFooter"));

const SwPropNameLen __FAR_DATA UNO_NAME_START_REDLINE                       (MAP_CHAR_LEN("StartRedline"));
const SwPropNameLen __FAR_DATA UNO_NAME_END_REDLINE                         (MAP_CHAR_LEN("EndRedline"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_START                       (MAP_CHAR_LEN("RedlineStart"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_END                         (MAP_CHAR_LEN("RedlineEnd"));
const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_TEXT                        (MAP_CHAR_LEN("RedlineText"));

const SwPropNameLen __FAR_DATA UNO_NAME_REDLINE_DISPLAY_TYPE                (MAP_CHAR_LEN("RedlineDisplayType"));
const SwPropNameLen __FAR_DATA UNO_NAME_FORBIDDEN_CHARACTERS                (MAP_CHAR_LEN("ForbiddenCharacters"));

const SwPropNameLen __FAR_DATA UNO_NAME_RUBY_BASE_TEXT                      (MAP_CHAR_LEN("RubyBaseText"));
const SwPropNameLen __FAR_DATA UNO_NAME_RUBY_TEXT                           (MAP_CHAR_LEN("RubyText"));
const SwPropNameLen __FAR_DATA UNO_NAME_RUBY_ADJUST                         (MAP_CHAR_LEN("RubyAdjust"));
const SwPropNameLen __FAR_DATA UNO_NAME_RUBY_CHAR_STYLE_NAME                (MAP_CHAR_LEN("RubyCharStyleName"));

const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_HEIGHT                     (MAP_CHAR_LEN("FootnoteHeight"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_WEIGHT                (MAP_CHAR_LEN("FootnoteLineWeight"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_COLOR                 (MAP_CHAR_LEN("FootnoteLineColor"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH        (MAP_CHAR_LEN("FootnoteLineRelativeWidth"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_ADJUST                (MAP_CHAR_LEN("FootnoteLineAdjust"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE         (MAP_CHAR_LEN("FootnoteLineTextDistance"));
const SwPropNameLen __FAR_DATA UNO_NAME_FOOTNOTE_LINE_DISTANCE              (MAP_CHAR_LEN("FootnoteLineDistance"));

const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_ROTATION                       (MAP_CHAR_LEN("CharRotation"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE        (MAP_CHAR_LEN("CharRotationIsFitToLine"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_SCALE_WIDTH                    (MAP_CHAR_LEN("CharScaleWidth"));
const SwPropNameLen __FAR_DATA UNO_NAME_TAB_STOP_DISTANCE                   (MAP_CHAR_LEN("TabStopDistance"));

const SwPropNameLen __FAR_DATA UNO_NAME_IS_WIDTH_RELATIVE                   (MAP_CHAR_LEN("IsWidthRelative"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHAR_RELIEF                         (MAP_CHAR_LEN("CharRelief"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_HIDDEN                           (MAP_CHAR_LEN("IsHidden"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_CONDITION_TRUE                   (MAP_CHAR_LEN("IsConditionTrue"));

const SwPropNameLen __FAR_DATA UNO_NAME_LINK_UPDATE_MODE                    (MAP_CHAR_LEN("LinkUpdateMode"));
const SwPropNameLen __FAR_DATA UNO_NAME_FIELD_AUTO_UPDATE                   (MAP_CHAR_LEN("FieldAutoUpdate"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHART_AUTO_UPDATE                   (MAP_CHAR_LEN("ChartAutoUpdate"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADD_PARA_TABLE_SPACING              (MAP_CHAR_LEN("AddParaTableSpacing"));
const SwPropNameLen __FAR_DATA UNO_NAME_ADD_PARA_TABLE_SPACING_AT_START     (MAP_CHAR_LEN("AddParaTableSpacingAtStart"));
const SwPropNameLen __FAR_DATA UNO_NAME_PRINTER_NAME                        (MAP_CHAR_LEN("PrinterName"));
const SwPropNameLen __FAR_DATA UNO_NAME_IS_KERN_ASIAN_PUNCTUATION           (MAP_CHAR_LEN("IsKernAsianPunctuation"));
const SwPropNameLen __FAR_DATA UNO_NAME_CHARACTER_COMPRESSION_TYPE          (MAP_CHAR_LEN("CharacterCompressionType"));
const SwPropNameLen __FAR_DATA UNO_NAME_TWO_DIGIT_YEAR                      (MAP_CHAR_LEN("TwoDigitYear"));
const SwPropNameLen __FAR_DATA UNO_NAME_AUTOMATIC_CONTROL_FOCUS             (MAP_CHAR_LEN("AutomaticControlFocus"));
const SwPropNameLen __FAR_DATA UNO_NAME_APPLY_FORM_DESIGN_MODE              (MAP_CHAR_LEN("ApplyFormDesignMode"));
const SwPropNameLen __FAR_DATA UNO_NAME_APPLY_USER_DATA                     (MAP_CHAR_LEN("ApplyUserData"));
const SwPropNameLen __FAR_DATA UNO_NAME_SAVE_GLOBAL_DOCUMENT_LINKS          (MAP_CHAR_LEN("SaveGlobalDocumentLinks"));
const SwPropNameLen __FAR_DATA UNO_NAME_CURRENT_DATABASE_DATA_SOURCE        (MAP_CHAR_LEN("CurrentDatabaseDataSource"));
const SwPropNameLen __FAR_DATA UNO_NAME_CURRENT_DATABASE_COMMAND            (MAP_CHAR_LEN("CurrentDatabaseCommand"));
const SwPropNameLen __FAR_DATA UNO_NAME_CURRENT_DATABASE_COMMAND_TYPE       (MAP_CHAR_LEN("CurrentDatabaseCommandType"));
