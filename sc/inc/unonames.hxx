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

#include <rtl/ustring.hxx>

// service names
inline constexpr OUString SC_SERVICENAME_VALBIND = u"com.sun.star.table.CellValueBinding"_ustr;
inline constexpr OUString SC_SERVICENAME_LISTCELLBIND = u"com.sun.star.table.ListPositionCellBinding"_ustr;
inline constexpr OUString SC_SERVICENAME_LISTSOURCE = u"com.sun.star.table.CellRangeListSource"_ustr;
inline constexpr OUString SC_SERVICENAME_CELLADDRESS = u"com.sun.star.table.CellAddressConversion"_ustr;
inline constexpr OUString SC_SERVICENAME_RANGEADDRESS = u"com.sun.star.table.CellRangeAddressConversion"_ustr;

inline constexpr OUString SC_SERVICENAME_FORMULAPARS = u"com.sun.star.sheet.FormulaParser"_ustr;
inline constexpr OUString SC_SERVICENAME_OPCODEMAPPER = u"com.sun.star.sheet.FormulaOpCodeMapper"_ustr;

inline constexpr OUString SC_SERVICENAME_CHDATAPROV = u"com.sun.star.chart2.data.DataProvider"_ustr;
inline constexpr OUString SC_SERVICENAME_CHRANGEHILIGHT = u"com.sun.star.chart2.data.RangeHighlightListener"_ustr;
inline constexpr OUString SC_SERVICENAME_CHART_PIVOTTABLE_DATAPROVIDER = u"com.sun.star.chart2.data.PivotTableDataProvider"_ustr;

//  document
inline constexpr OUString SC_UNO_AREALINKS            = u"AreaLinks"_ustr;
inline constexpr OUString SC_UNO_DDELINKS             = u"DDELinks"_ustr;
inline constexpr OUString SC_UNO_EXTERNALDOCLINKS     = u"ExternalDocLinks"_ustr;
inline constexpr OUString SC_UNO_COLLABELRNG          = u"ColumnLabelRanges"_ustr;
inline constexpr OUString SC_UNO_DATABASERNG          = u"DatabaseRanges"_ustr;
inline constexpr OUString SC_UNO_UNNAMEDDBRNG         = u"UnnamedDatabaseRanges"_ustr;
inline constexpr OUString SC_UNO_NAMEDRANGES          = u"NamedRanges"_ustr;
inline constexpr OUString SC_UNO_ROWLABELRNG          = u"RowLabelRanges"_ustr;
inline constexpr OUString SC_UNO_SHEETLINKS           = u"SheetLinks"_ustr;
inline constexpr OUString SC_UNO_FORBIDDEN            = u"ForbiddenCharacters"_ustr;
inline constexpr OUString SC_UNO_HASDRAWPAGES         = u"HasDrawPages"_ustr;
inline constexpr OUString SC_UNO_THEME                = u"Theme"_ustr;

//  CharacterProperties
inline constexpr OUString SC_UNONAME_CCOLOR           = u"CharColor"_ustr;
inline constexpr OUString SC_UNONAME_CHAR_COMPLEX_COLOR = u"CharComplexColor"_ustr;
inline constexpr OUString SC_UNONAME_CHEIGHT          = u"CharHeight"_ustr;
inline constexpr OUString SC_UNONAME_CUNDER           = u"CharUnderline"_ustr;
inline constexpr OUString SC_UNONAME_CUNDLCOL         = u"CharUnderlineColor"_ustr;
inline constexpr OUString SC_UNONAME_CUNDLHAS         = u"CharUnderlineHasColor"_ustr;
inline constexpr OUString SC_UNONAME_COVER            = u"CharOverline"_ustr;
inline constexpr OUString SC_UNONAME_COVRLCOL         = u"CharOverlineColor"_ustr;
inline constexpr OUString SC_UNONAME_COVRLHAS         = u"CharOverlineHasColor"_ustr;
inline constexpr OUString SC_UNONAME_CWEIGHT          = u"CharWeight"_ustr;
inline constexpr OUString SC_UNONAME_CPOST            = u"CharPosture"_ustr;
inline constexpr OUString SC_UNONAME_CCROSS           = u"CharCrossedOut"_ustr;
inline constexpr OUString SC_UNONAME_CSTRIKE          = u"CharStrikeout"_ustr;
inline constexpr OUString SC_UNONAME_CLOCAL           = u"CharLocale"_ustr;
inline constexpr OUString SC_UNONAME_CSHADD           = u"CharShadowed"_ustr;
inline constexpr OUString SC_UNONAME_CFONT            = u"CharFont"_ustr;
inline constexpr OUString SC_UNONAME_COUTL            = u"CharContoured"_ustr;
inline constexpr OUString SC_UNONAME_CEMPHAS          = u"CharEmphasis"_ustr;
inline constexpr OUString SC_UNONAME_CFNAME           = u"CharFontName"_ustr;
inline constexpr OUString SC_UNONAME_CFSTYLE          = u"CharFontStyleName"_ustr;
inline constexpr OUString SC_UNONAME_CFFAMIL          = u"CharFontFamily"_ustr;
inline constexpr OUString SC_UNONAME_CFCHARS          = u"CharFontCharSet"_ustr;
inline constexpr OUString SC_UNONAME_CFPITCH          = u"CharFontPitch"_ustr;
inline constexpr OUString SC_UNONAME_CRELIEF          = u"CharRelief"_ustr;
inline constexpr OUString SC_UNONAME_CWORDMOD         = u"CharWordMode"_ustr;

inline constexpr OUString SC_UNO_CJK_CFNAME           = u"CharFontNameAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CFSTYLE          = u"CharFontStyleNameAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CFFAMIL          = u"CharFontFamilyAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CFCHARS          = u"CharFontCharSetAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CFPITCH          = u"CharFontPitchAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CHEIGHT          = u"CharHeightAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CWEIGHT          = u"CharWeightAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CPOST            = u"CharPostureAsian"_ustr;
inline constexpr OUString SC_UNO_CJK_CLOCAL           = u"CharLocaleAsian"_ustr;

inline constexpr OUString SC_UNO_CTL_CFNAME           = u"CharFontNameComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CFSTYLE          = u"CharFontStyleNameComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CFFAMIL          = u"CharFontFamilyComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CFCHARS          = u"CharFontCharSetComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CFPITCH          = u"CharFontPitchComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CHEIGHT          = u"CharHeightComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CWEIGHT          = u"CharWeightComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CPOST            = u"CharPostureComplex"_ustr;
inline constexpr OUString SC_UNO_CTL_CLOCAL           = u"CharLocaleComplex"_ustr;

//  CellProperties
inline constexpr OUString SC_UNONAME_CELLSTYL         = u"CellStyle"_ustr;
inline constexpr OUString SC_UNONAME_CELLBACK         = u"CellBackColor"_ustr;
inline constexpr OUString SC_UNONAME_CELL_BACKGROUND_COMPLEX_COLOR = u"CellBackgroundComplexColor"_ustr;
inline constexpr OUString SC_UNONAME_CELLTRAN         = u"IsCellBackgroundTransparent"_ustr;
inline constexpr OUString SC_UNONAME_CELLPRO          = u"CellProtection"_ustr;
inline constexpr OUString SC_UNONAME_CELLHJUS         = u"HoriJustify"_ustr;
inline constexpr OUString SC_UNONAME_CELLVJUS         = u"VertJustify"_ustr;
inline constexpr OUString SC_UNONAME_CELLHJUS_METHOD  = u"HoriJustifyMethod"_ustr;
inline constexpr OUString SC_UNONAME_CELLVJUS_METHOD  = u"VertJustifyMethod"_ustr;
inline constexpr OUString SC_UNONAME_CELLORI          = u"Orientation"_ustr;
inline constexpr OUString SC_UNONAME_NUMFMT           = u"NumberFormat"_ustr;
inline constexpr OUString SC_UNONAME_FORMATID         = u"FormatID"_ustr;
inline constexpr OUString SC_UNONAME_SHADOW           = u"ShadowFormat"_ustr;
inline constexpr OUString SC_UNONAME_TBLBORD          = u"TableBorder"_ustr;
inline constexpr OUString SC_UNONAME_TBLBORD2         = u"TableBorder2"_ustr;
inline constexpr OUString SC_UNONAME_WRAP             = u"IsTextWrapped"_ustr;
inline constexpr OUString SC_UNONAME_PINDENT          = u"ParaIndent"_ustr;
inline constexpr OUString SC_UNONAME_PTMARGIN         = u"ParaTopMargin"_ustr;
inline constexpr OUString SC_UNONAME_PBMARGIN         = u"ParaBottomMargin"_ustr;
inline constexpr OUString SC_UNONAME_PLMARGIN         = u"ParaLeftMargin"_ustr;
inline constexpr OUString SC_UNONAME_PRMARGIN         = u"ParaRightMargin"_ustr;
inline constexpr OUString SC_UNONAME_ROTANG           = u"RotateAngle"_ustr;
inline constexpr OUString SC_UNONAME_ROTREF           = u"RotateReference"_ustr;
inline constexpr OUString SC_UNONAME_ASIANVERT        = u"AsianVerticalMode"_ustr;
inline constexpr OUString SC_UNONAME_WRITING          = u"WritingMode"_ustr;
inline constexpr OUString SC_UNONAME_HIDDEN           = u"Hidden"_ustr;

inline constexpr OUString SC_UNONAME_BOTTBORDER       = u"BottomBorder"_ustr;
inline constexpr OUString SC_UNONAME_LEFTBORDER       = u"LeftBorder"_ustr;
inline constexpr OUString SC_UNONAME_RIGHTBORDER      = u"RightBorder"_ustr;
inline constexpr OUString SC_UNONAME_TOPBORDER        = u"TopBorder"_ustr;

inline constexpr OUString SC_UNONAME_BOTTBORDER2      = u"BottomBorder2"_ustr;
inline constexpr OUString SC_UNONAME_LEFTBORDER2      = u"LeftBorder2"_ustr;
inline constexpr OUString SC_UNONAME_RIGHTBORDER2     = u"RightBorder2"_ustr;
inline constexpr OUString SC_UNONAME_TOPBORDER2       = u"TopBorder2"_ustr;

inline constexpr OUString SC_UNONAME_BOTTOM_BORDER_COMPLEX_COLOR = u"BottomBorderComplexColor"_ustr;
inline constexpr OUString SC_UNONAME_LEFT_BORDER_COMPLEX_COLOR = u"LeftBorderComplexColor"_ustr;
inline constexpr OUString SC_UNONAME_RIGHT_BORDER_COMPLEX_COLOR = u"RightBorderComplexColor"_ustr;
inline constexpr OUString SC_UNONAME_TOP_BORDER_COMPLEX_COLOR = u"TopBorderComplexColor"_ustr;

inline constexpr OUString SC_UNONAME_DIAGONAL_TLBR    = u"DiagonalTLBR"_ustr;
inline constexpr OUString SC_UNONAME_DIAGONAL_BLTR    = u"DiagonalBLTR"_ustr;

inline constexpr OUString SC_UNONAME_DIAGONAL_TLBR2   = u"DiagonalTLBR2"_ustr;
inline constexpr OUString SC_UNONAME_DIAGONAL_BLTR2   = u"DiagonalBLTR2"_ustr;

inline constexpr OUString SC_UNONAME_SHRINK_TO_FIT    = u"ShrinkToFit"_ustr;

inline constexpr OUString SC_UNONAME_PISHANG          = u"ParaIsHangingPunctuation"_ustr;
inline constexpr OUString SC_UNONAME_PISCHDIST        = u"ParaIsCharacterDistance"_ustr;
inline constexpr OUString SC_UNONAME_PISFORBID        = u"ParaIsForbiddenRules"_ustr;
inline constexpr OUString SC_UNONAME_PISHYPHEN        = u"ParaIsHyphenation"_ustr;
inline constexpr OUString SC_UNONAME_PADJUST          = u"ParaAdjust"_ustr;
inline constexpr OUString SC_UNONAME_PLASTADJ         = u"ParaLastLineAdjust"_ustr;

inline constexpr OUString SC_UNONAME_NUMRULES         = u"NumberingRules"_ustr;

//  Styles
inline constexpr OUString SC_UNONAME_DISPNAME         = u"DisplayName"_ustr;

//  XStyleLoader
inline constexpr OUString SC_UNONAME_OVERWSTL         = u"OverwriteStyles"_ustr;
inline constexpr OUString SC_UNONAME_LOADCELL         = u"LoadCellStyles"_ustr;
inline constexpr OUString SC_UNONAME_LOADPAGE         = u"LoadPageStyles"_ustr;

//  SheetCellRange
inline constexpr OUString SC_UNONAME_POS              = u"Position"_ustr;
inline constexpr OUString SC_UNONAME_SIZE             = u"Size"_ustr;
inline constexpr OUString SC_UNONAME_ABSNAME          = u"AbsoluteName"_ustr;

//  column/row/sheet
inline constexpr OUString SC_UNONAME_CELLHGT          = u"Height"_ustr;
inline constexpr OUString SC_UNONAME_CELLWID          = u"Width"_ustr;
inline constexpr OUString SC_UNONAME_CELLVIS          = u"IsVisible"_ustr;
inline constexpr OUString SC_UNONAME_CELLFILT         = u"IsFiltered"_ustr;
inline constexpr OUString SC_UNONAME_MANPAGE          = u"IsManualPageBreak"_ustr;
inline constexpr OUString SC_UNONAME_NEWPAGE          = u"IsStartOfNewPage"_ustr;
inline constexpr OUString SC_UNONAME_OHEIGHT          = u"OptimalHeight"_ustr;
inline constexpr OUString SC_UNONAME_OWIDTH           = u"OptimalWidth"_ustr;
inline constexpr OUString SC_UNONAME_PAGESTL          = u"PageStyle"_ustr;
inline constexpr OUString SC_UNONAME_ISACTIVE         = u"IsActive"_ustr;
inline constexpr OUString SC_UNONAME_BORDCOL          = u"BorderColor"_ustr;
inline constexpr OUString SC_UNONAME_PROTECT          = u"Protected"_ustr;
inline constexpr OUString SC_UNONAME_SHOWBORD         = u"ShowBorder"_ustr;
inline constexpr OUString SC_UNONAME_PRINTBORD        = u"PrintBorder"_ustr;
inline constexpr OUString SC_UNONAME_COPYBACK         = u"CopyBack"_ustr;
inline constexpr OUString SC_UNONAME_COPYSTYL         = u"CopyStyles"_ustr;
inline constexpr OUString SC_UNONAME_COPYFORM         = u"CopyFormulas"_ustr;
inline constexpr OUString SC_UNONAME_TABLAYOUT        = u"TableLayout"_ustr;
inline constexpr OUString SC_UNONAME_AUTOPRINT        = u"AutomaticPrintArea"_ustr;
inline constexpr OUString SC_UNONAME_TABCOLOR         = u"TabColor"_ustr;
inline constexpr OUString SC_UNONAME_CONDFORMAT       = u"ConditionalFormats"_ustr;

inline constexpr OUString SC_UNONAME_VISFLAG          = u"VisibleFlag"_ustr;

//  LinkTarget
inline constexpr OUString SC_UNO_LINKDISPBIT          = u"LinkDisplayBitmap"_ustr;
inline constexpr OUString SC_UNO_LINKDISPNAME         = u"LinkDisplayName"_ustr;

//  drawing objects
inline constexpr OUString SC_UNONAME_IMAGEMAP         = u"ImageMap"_ustr;
inline constexpr OUString SC_UNONAME_ANCHOR           = u"Anchor"_ustr;
inline constexpr OUString SC_UNONAME_RESIZE_WITH_CELL = u"ResizeWithCell"_ustr;
inline constexpr OUString SC_UNONAME_HORIPOS          = u"HoriOrientPosition"_ustr;
inline constexpr OUString SC_UNONAME_VERTPOS          = u"VertOrientPosition"_ustr;
inline constexpr OUString SC_UNONAME_HYPERLINK        = u"Hyperlink"_ustr;
inline constexpr OUString SC_UNONAME_MOVEPROTECT      = u"MoveProtect"_ustr;
inline constexpr OUString SC_UNONAME_STYLE            = u"Style"_ustr;

//  other cell properties
inline constexpr OUString SC_UNONAME_CHCOLHDR         = u"ChartColumnAsLabel"_ustr;
inline constexpr OUString SC_UNONAME_CHROWHDR         = u"ChartRowAsLabel"_ustr;
inline constexpr OUString SC_UNONAME_CONDFMT          = u"ConditionalFormat"_ustr;
inline constexpr OUString SC_UNONAME_CONDLOC          = u"ConditionalFormatLocal"_ustr;
inline constexpr OUString SC_UNONAME_CONDXML          = u"ConditionalFormatXML"_ustr;
inline constexpr OUString SC_UNONAME_VALIDAT          = u"Validation"_ustr;
inline constexpr OUString SC_UNONAME_VALILOC          = u"ValidationLocal"_ustr;
inline constexpr OUString SC_UNONAME_VALIXML          = u"ValidationXML"_ustr;
inline constexpr OUString SC_UNONAME_FORMLOC          = u"FormulaLocal"_ustr;
inline constexpr OUString SC_UNONAME_FORMRT           = u"FormulaResultType"_ustr;
inline constexpr OUString SC_UNONAME_FORMRT2          = u"FormulaResultType2"_ustr;
inline constexpr OUString SC_UNONAME_CELLCONTENTTYPE  = u"CellContentType"_ustr;

inline constexpr OUString SC_UNONAME_USERDEF          = u"UserDefinedAttributes"_ustr;
inline constexpr OUString SC_UNONAME_TEXTUSER         = u"TextUserDefinedAttributes"_ustr;

//  auto format
inline constexpr OUString SC_UNONAME_INCBACK          = u"IncludeBackground"_ustr;
inline constexpr OUString SC_UNONAME_INCBORD          = u"IncludeBorder"_ustr;
inline constexpr OUString SC_UNONAME_INCFONT          = u"IncludeFont"_ustr;
inline constexpr OUString SC_UNONAME_INCJUST          = u"IncludeJustify"_ustr;
inline constexpr OUString SC_UNONAME_INCNUM           = u"IncludeNumberFormat"_ustr;
inline constexpr OUString SC_UNONAME_INCWIDTH         = u"IncludeWidthAndHeight"_ustr;

//  function description
inline constexpr OUString SC_UNONAME_ARGUMENTS        = u"Arguments"_ustr;
inline constexpr OUString SC_UNONAME_CATEGORY         = u"Category"_ustr;
inline constexpr OUString SC_UNONAME_DESCRIPTION      = u"Description"_ustr;
inline constexpr OUString SC_UNONAME_ID               = u"Id"_ustr;
inline constexpr OUString SC_UNONAME_NAME             = u"Name"_ustr;

//  application settings
inline constexpr OUString SC_UNONAME_DOAUTOCP         = u"DoAutoComplete"_ustr;
inline constexpr OUString SC_UNONAME_ENTERED          = u"EnterEdit"_ustr;
inline constexpr OUString SC_UNONAME_EXPREF           = u"ExpandReferences"_ustr;
inline constexpr OUString SC_UNONAME_EXTFMT           = u"ExtendFormat"_ustr;
inline constexpr OUString SC_UNONAME_LINKUPD          = u"LinkUpdateMode"_ustr;
inline constexpr OUString SC_UNONAME_MARKHDR          = u"MarkHeader"_ustr;
inline constexpr OUString SC_UNONAME_METRIC           = u"Metric"_ustr;
inline constexpr OUString SC_UNONAME_MOVEDIR          = u"MoveDirection"_ustr;
inline constexpr OUString SC_UNONAME_MOVESEL          = u"MoveSelection"_ustr;
inline constexpr OUString SC_UNONAME_RANGEFIN         = u"RangeFinder"_ustr;
inline constexpr OUString SC_UNONAME_SCALE            = u"Scale"_ustr;
inline constexpr OUString SC_UNONAME_STBFUNC          = u"StatusBarFunction"_ustr;
inline constexpr OUString SC_UNONAME_ULISTS           = u"UserLists"_ustr;
inline constexpr OUString SC_UNONAME_USETABCOL        = u"UseTabCol"_ustr;
inline constexpr OUString SC_UNONAME_PRMETRICS        = u"UsePrinterMetrics"_ustr;
inline constexpr OUString SC_UNONAME_PRALLSH          = u"PrintAllSheets"_ustr;
inline constexpr OUString SC_UNONAME_PREMPTY          = u"PrintEmptyPages"_ustr;
inline constexpr OUString SC_UNONAME_REPLWARN         = u"ReplaceCellsWarning"_ustr;

//  data pilot field
inline constexpr OUString SC_UNONAME_FUNCTION         = u"Function"_ustr;
inline constexpr OUString SC_UNONAME_FUNCTION2        = u"Function2"_ustr;
inline constexpr OUString SC_UNONAME_SUBTOTALS        = u"Subtotals"_ustr;
inline constexpr OUString SC_UNONAME_SUBTOTALS2       = u"Subtotals2"_ustr;
inline constexpr OUString SC_UNONAME_SELPAGE          = u"SelectedPage"_ustr;
inline constexpr OUString SC_UNONAME_USESELPAGE       = u"UseSelectedPage"_ustr;
inline constexpr OUString SC_UNONAME_HASREFERENCE     = u"HasReference"_ustr;
inline constexpr OUString SC_UNONAME_REFERENCE        = u"Reference"_ustr;
inline constexpr OUString SC_UNONAME_HASAUTOSHOW      = u"HasAutoShowInfo"_ustr;
inline constexpr OUString SC_UNONAME_AUTOSHOW         = u"AutoShowInfo"_ustr;
inline constexpr OUString SC_UNONAME_HASSORTINFO      = u"HasSortInfo"_ustr;
inline constexpr OUString SC_UNONAME_SORTINFO         = u"SortInfo"_ustr;
inline constexpr OUString SC_UNONAME_HASLAYOUTINFO    = u"HasLayoutInfo"_ustr;
inline constexpr OUString SC_UNONAME_LAYOUTINFO       = u"LayoutInfo"_ustr;
inline constexpr OUString SC_UNONAME_ISGROUP          = u"IsGroupField"_ustr;
inline constexpr OUString SC_UNONAME_GROUPINFO        = u"GroupInfo"_ustr;
inline constexpr OUString SC_UNONAME_SHOWEMPTY        = u"ShowEmpty"_ustr;
inline constexpr OUString SC_UNONAME_REPEATITEMLABELS = u"RepeatItemLabels"_ustr;

//  data pilot item
inline constexpr OUString SC_UNONAME_SHOWDETAIL       = u"ShowDetail"_ustr;
inline constexpr OUString SC_UNONAME_ISHIDDEN         = u"IsHidden"_ustr;

//  database options
inline constexpr OUString SC_UNONAME_CASE             = u"CaseSensitive"_ustr;
inline constexpr OUString SC_UNONAME_DBNAME           = u"DatabaseName"_ustr;
inline constexpr OUString SC_UNONAME_FORMATS          = u"IncludeFormats"_ustr;
inline constexpr OUString SC_UNONAME_INSBRK           = u"InsertPageBreaks"_ustr;
inline constexpr OUString SC_UNONAME_KEEPFORM         = u"KeepFormats"_ustr;
inline constexpr OUString SC_UNONAME_MOVCELLS         = u"MoveCells"_ustr;
inline constexpr OUString SC_UNONAME_ISUSER           = u"IsUserDefined"_ustr;
inline constexpr OUString SC_UNONAME_ISNATIVE         = u"IsNative"_ustr;
inline constexpr OUString SC_UNONAME_REGEXP           = u"RegularExpressions"_ustr;
inline constexpr OUString SC_UNONAME_WILDCARDS        = u"Wildcards"_ustr;
inline constexpr OUString SC_UNONAME_SAVEOUT          = u"SaveOutputPosition"_ustr;
inline constexpr OUString SC_UNONAME_SKIPDUP          = u"SkipDuplicates"_ustr;
inline constexpr OUString SC_UNONAME_SRCOBJ           = u"SourceObject"_ustr;
inline constexpr OUString SC_UNONAME_SRCTYPE          = u"SourceType"_ustr;
inline constexpr OUString SC_UNONAME_STRIPDAT         = u"StripData"_ustr;
inline constexpr OUString SC_UNONAME_USEREGEX         = u"UseRegularExpressions"_ustr;
inline constexpr OUString SC_UNONAME_ULIST            = u"UserListEnabled"_ustr;
inline constexpr OUString SC_UNONAME_UINDEX           = u"UserListIndex"_ustr;
inline constexpr OUString SC_UNONAME_BINDFMT          = u"BindFormatsToContent"_ustr;
inline constexpr OUString SC_UNONAME_COPYOUT          = u"CopyOutputData"_ustr;
inline constexpr OUString SC_UNONAME_ISCASE           = u"IsCaseSensitive"_ustr;
inline constexpr OUString SC_UNONAME_ISULIST          = u"IsUserListEnabled"_ustr;
inline constexpr OUString SC_UNONAME_OUTPOS           = u"OutputPosition"_ustr;
inline constexpr OUString SC_UNONAME_CONTHDR          = u"ContainsHeader"_ustr;
inline constexpr OUString SC_UNONAME_MAXFLD           = u"MaxFieldCount"_ustr;
inline constexpr OUString SC_UNONAME_ORIENT           = u"Orientation"_ustr;
inline constexpr OUString SC_UNONAME_ISSORTCOLUMNS    = u"IsSortColumns"_ustr;
inline constexpr OUString SC_UNONAME_SORTFLD          = u"SortFields"_ustr;
inline constexpr OUString SC_UNONAME_SORTASC          = u"SortAscending"_ustr;
inline constexpr OUString SC_UNONAME_ENUSLIST         = u"EnableUserSortList"_ustr;
inline constexpr OUString SC_UNONAME_USINDEX          = u"UserSortListIndex"_ustr;
inline constexpr OUString SC_UNONAME_COLLLOC          = u"CollatorLocale"_ustr;
inline constexpr OUString SC_UNONAME_COLLALG          = u"CollatorAlgorithm"_ustr;
inline constexpr OUString SC_UNONAME_AUTOFLT          = u"AutoFilter"_ustr;
inline constexpr OUString SC_UNONAME_FLTCRT           = u"FilterCriteriaSource"_ustr;
inline constexpr OUString SC_UNONAME_USEFLTCRT        = u"UseFilterCriteriaSource"_ustr;
inline constexpr OUString SC_UNONAME_ENABSORT         = u"EnableSort"_ustr;
inline constexpr OUString SC_UNONAME_FROMSELECT       = u"FromSelection"_ustr;
inline constexpr OUString SC_UNONAME_CONRES           = u"ConnectionResource"_ustr;
inline constexpr OUString SC_UNONAME_TOKENINDEX       = u"TokenIndex"_ustr;
inline constexpr OUString SC_UNONAME_ISSHAREDFMLA     = u"IsSharedFormula"_ustr;
inline constexpr OUString SC_UNONAME_TOTALSROW        = u"TotalsRow"_ustr;

//  text fields
inline constexpr OUString SC_UNONAME_ANCTYPE          = u"AnchorType"_ustr;
inline constexpr OUString SC_UNONAME_ANCTYPES         = u"AnchorTypes"_ustr;
inline constexpr OUString SC_UNONAME_TEXTWRAP         = u"TextWrap"_ustr;
inline constexpr OUString SC_UNONAME_FILEFORM         = u"FileFormat"_ustr;
inline constexpr OUString SC_UNONAME_TEXTFIELD_TYPE   = u"TextFieldType"_ustr;

//  url field
inline constexpr OUString SC_UNONAME_REPR             = u"Representation"_ustr;
inline constexpr OUString SC_UNONAME_TARGET           = u"TargetFrame"_ustr;
inline constexpr OUString SC_UNONAME_URL              = u"URL"_ustr;

// date time field
inline constexpr OUString SC_UNONAME_ISDATE           = u"IsDate"_ustr;
inline constexpr OUString SC_UNONAME_ISFIXED          = u"IsFixed"_ustr;
inline constexpr OUString SC_UNONAME_DATETIME         = u"DateTime"_ustr;

// table field
inline constexpr OUString SC_UNONAME_TABLEPOS         = u"TablePosition"_ustr;

//  conditional format
inline constexpr OUString SC_UNONAME_OPERATOR         = u"Operator"_ustr;
inline constexpr OUString SC_UNONAME_FORMULA1         = u"Formula1"_ustr;
inline constexpr OUString SC_UNONAME_FORMULA2         = u"Formula2"_ustr;
inline constexpr OUString SC_UNONAME_SOURCEPOS        = u"SourcePosition"_ustr;
inline constexpr OUString SC_UNONAME_SOURCESTR        = u"SourcePositionAsString"_ustr; // only for use in XML filter
inline constexpr OUString SC_UNONAME_FORMULANMSP1     = u"FormulaNamespace1"_ustr; // only for use in XML filter
inline constexpr OUString SC_UNONAME_FORMULANMSP2     = u"FormulaNamespace2"_ustr; // only for use in XML filter
inline constexpr OUString SC_UNONAME_GRAMMAR1         = u"Grammar1"_ustr; // only for use in XML filter
inline constexpr OUString SC_UNONAME_GRAMMAR2         = u"Grammar2"_ustr; // only for use in XML filter
inline constexpr OUString SC_UNONAME_STYLENAME        = u"StyleName"_ustr;

//  validation
inline constexpr OUString SC_UNONAME_ERRALSTY         = u"ErrorAlertStyle"_ustr;
inline constexpr OUString SC_UNONAME_ERRMESS          = u"ErrorMessage"_ustr;
inline constexpr OUString SC_UNONAME_ERRTITLE         = u"ErrorTitle"_ustr;
inline constexpr OUString SC_UNONAME_IGNOREBL         = u"IgnoreBlankCells"_ustr;
inline constexpr OUString SC_UNONAME_INPMESS          = u"InputMessage"_ustr;
inline constexpr OUString SC_UNONAME_INPTITLE         = u"InputTitle"_ustr;
inline constexpr OUString SC_UNONAME_SHOWERR          = u"ShowErrorMessage"_ustr;
inline constexpr OUString SC_UNONAME_SHOWINP          = u"ShowInputMessage"_ustr;
inline constexpr OUString SC_UNONAME_SHOWLIST         = u"ShowList"_ustr;
inline constexpr OUString SC_UNONAME_TYPE             = u"Type"_ustr;

//  links
inline constexpr OUString SC_UNONAME_FILTER           = u"Filter"_ustr;
inline constexpr OUString SC_UNONAME_FILTOPT          = u"FilterOptions"_ustr;
inline constexpr OUString SC_UNONAME_LINKURL          = u"Url"_ustr;
inline constexpr OUString SC_UNONAME_REFPERIOD        = u"RefreshPeriod"_ustr;
inline constexpr OUString SC_UNONAME_REFDELAY         = u"RefreshDelay"_ustr; // deprecated, don't use anymore

//  search descriptor
inline constexpr OUString SC_UNO_SRCHBACK             = u"SearchBackwards"_ustr;
inline constexpr OUString SC_UNO_SRCHBYROW            = u"SearchByRow"_ustr;
inline constexpr OUString SC_UNO_SRCHCASE             = u"SearchCaseSensitive"_ustr;
inline constexpr OUString SC_UNO_SRCHREGEXP           = u"SearchRegularExpression"_ustr;
inline constexpr OUString SC_UNO_SRCHWILDCARD         = u"SearchWildcard"_ustr;
inline constexpr OUString SC_UNO_SRCHSIM              = u"SearchSimilarity"_ustr;
inline constexpr OUString SC_UNO_SRCHSIMADD           = u"SearchSimilarityAdd"_ustr;
inline constexpr OUString SC_UNO_SRCHSIMEX            = u"SearchSimilarityExchange"_ustr;
inline constexpr OUString SC_UNO_SRCHSIMREL           = u"SearchSimilarityRelax"_ustr;
inline constexpr OUString SC_UNO_SRCHSIMREM           = u"SearchSimilarityRemove"_ustr;
inline constexpr OUString SC_UNO_SRCHSTYLES           = u"SearchStyles"_ustr;
inline constexpr OUString SC_UNO_SRCHTYPE             = u"SearchType"_ustr;
inline constexpr OUString SC_UNO_SRCHWORDS            = u"SearchWords"_ustr;
inline constexpr OUString SC_UNO_SRCHFILTERED         = u"SearchFiltered"_ustr;
inline constexpr OUString SC_UNO_SRCHFORMATTED        = u"SearchFormatted"_ustr;
inline constexpr OUString SC_UNO_SRCHWCESCCHAR        = u"WildcardEscapeCharacter"_ustr;

//  old (5.2) property names for page styles - for compatibility only!
inline constexpr OUString OLD_UNO_PAGE_BACKCOLOR      = u"BackgroundColor"_ustr;
inline constexpr OUString OLD_UNO_PAGE_BACKTRANS      = u"IsBackgroundTransparent"_ustr;
inline constexpr OUString OLD_UNO_PAGE_HDRBACKCOL     = u"HeaderBackgroundColor"_ustr;
inline constexpr OUString OLD_UNO_PAGE_HDRBACKTRAN    = u"TransparentHeaderBackground"_ustr;
inline constexpr OUString OLD_UNO_PAGE_HDRSHARED      = u"HeaderShared"_ustr;
inline constexpr OUString OLD_UNO_PAGE_HDRDYNAMIC     = u"HeaderDynamic"_ustr;
inline constexpr OUString OLD_UNO_PAGE_HDRON          = u"HeaderOn"_ustr;
inline constexpr OUString OLD_UNO_PAGE_FTRBACKCOL     = u"FooterBackgroundColor"_ustr;
inline constexpr OUString OLD_UNO_PAGE_FTRBACKTRAN    = u"TransparentFooterBackground"_ustr;
inline constexpr OUString OLD_UNO_PAGE_FTRSHARED      = u"FooterShared"_ustr;
inline constexpr OUString OLD_UNO_PAGE_FTRDYNAMIC     = u"FooterDynamic"_ustr;
inline constexpr OUString OLD_UNO_PAGE_FTRON          = u"FooterOn"_ustr;

//  page styles
inline constexpr OUString SC_UNO_PAGE_BACKCOLOR       = u"BackColor"_ustr;
inline constexpr OUString SC_UNO_PAGE_BACKTRANS       = u"BackTransparent"_ustr;
inline constexpr OUString SC_UNO_PAGE_GRAPHICFILT     = u"BackGraphicFilter"_ustr;
inline constexpr OUString SC_UNO_PAGE_GRAPHICLOC      = u"BackGraphicLocation"_ustr;
inline constexpr OUString SC_UNO_PAGE_GRAPHICURL      = u"BackGraphicURL"_ustr;
inline constexpr OUString SC_UNO_PAGE_GRAPHIC         = u"BackGraphic"_ustr;
#define SC_UNO_PAGE_LEFTBORDER      SC_UNONAME_LEFTBORDER
#define SC_UNO_PAGE_RIGHTBORDER     SC_UNONAME_RIGHTBORDER
#define SC_UNO_PAGE_BOTTBORDER      SC_UNONAME_BOTTBORDER
#define SC_UNO_PAGE_TOPBORDER       SC_UNONAME_TOPBORDER
inline constexpr OUString SC_UNO_PAGE_LEFTBRDDIST     = u"LeftBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_RIGHTBRDDIST    = u"RightBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_BOTTBRDDIST     = u"BottomBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_TOPBRDDIST      = u"TopBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_BORDERDIST      = u"BorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_SHADOWFORM      = u"ShadowFormat"_ustr;
inline constexpr OUString SC_UNO_PAGE_LEFTMARGIN      = u"LeftMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_RIGHTMARGIN     = u"RightMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_TOPMARGIN       = u"TopMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_BOTTMARGIN      = u"BottomMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_LANDSCAPE       = u"IsLandscape"_ustr;
inline constexpr OUString SC_UNO_PAGE_NUMBERTYPE      = u"NumberingType"_ustr;
inline constexpr OUString SC_UNO_PAGE_SYTLELAYOUT     = u"PageStyleLayout"_ustr;
inline constexpr OUString SC_UNO_PAGE_PAPERTRAY       = u"PrinterPaperTray"_ustr;
inline constexpr OUString SC_UNO_PAGE_SIZE            = u"Size"_ustr;
inline constexpr OUString SC_UNO_PAGE_WIDTH           = u"Width"_ustr;
inline constexpr OUString SC_UNO_PAGE_HEIGHT          = u"Height"_ustr;
inline constexpr OUString SC_UNO_PAGE_CENTERHOR       = u"CenterHorizontally"_ustr;
inline constexpr OUString SC_UNO_PAGE_CENTERVER       = u"CenterVertically"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTANNOT      = u"PrintAnnotations"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTGRID       = u"PrintGrid"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTHEADER     = u"PrintHeaders"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTCHARTS     = u"PrintCharts"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTOBJS       = u"PrintObjects"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTDRAW       = u"PrintDrawing"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTDOWN       = u"PrintDownFirst"_ustr;
inline constexpr OUString SC_UNO_PAGE_SCALEVAL        = u"PageScale"_ustr;
inline constexpr OUString SC_UNO_PAGE_SCALETOPAG      = u"ScaleToPages"_ustr;
inline constexpr OUString SC_UNO_PAGE_SCALETOX        = u"ScaleToPagesX"_ustr;
inline constexpr OUString SC_UNO_PAGE_SCALETOY        = u"ScaleToPagesY"_ustr;
inline constexpr OUString SC_UNO_PAGE_FIRSTPAGE       = u"FirstPageNumber"_ustr;
inline constexpr OUString SC_UNO_PAGE_FIRSTHDRSHARED  = u"FirstPageHeaderIsShared"_ustr;
inline constexpr OUString SC_UNO_PAGE_FIRSTFTRSHARED  = u"FirstPageFooterIsShared"_ustr;
inline constexpr OUString SC_UNO_PAGE_LEFTHDRCONT     = u"LeftPageHeaderContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_LEFTFTRCONT     = u"LeftPageFooterContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_RIGHTHDRCON     = u"RightPageHeaderContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_RIGHTFTRCON     = u"RightPageFooterContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_FIRSTHDRCONT    = u"FirstPageHeaderContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_FIRSTFTRCONT    = u"FirstPageFooterContent"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTFORMUL     = u"PrintFormulas"_ustr;
inline constexpr OUString SC_UNO_PAGE_PRINTZERO       = u"PrintZeroValues"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBACKCOL      = u"HeaderBackColor"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBACKTRAN     = u"HeaderBackTransparent"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRGRFFILT      = u"HeaderBackGraphicFilter"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRGRFLOC       = u"HeaderBackGraphicLocation"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRGRF          = u"HeaderBackGraphic"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRGRFURL       = u"HeaderBackGraphicURL"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRLEFTBOR      = u"HeaderLeftBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRRIGHTBOR     = u"HeaderRightBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBOTTBOR      = u"HeaderBottomBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRTOPBOR       = u"HeaderTopBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRLEFTBDIS     = u"HeaderLeftBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRRIGHTBDIS    = u"HeaderRightBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBOTTBDIS     = u"HeaderBottomBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRTOPBDIS      = u"HeaderTopBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBRDDIST      = u"HeaderBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRSHADOW       = u"HeaderShadowFormat"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRLEFTMAR      = u"HeaderLeftMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRRIGHTMAR     = u"HeaderRightMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRBODYDIST     = u"HeaderBodyDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRHEIGHT       = u"HeaderHeight"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRON           = u"HeaderIsOn"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRDYNAMIC      = u"HeaderIsDynamicHeight"_ustr;
inline constexpr OUString SC_UNO_PAGE_HDRSHARED       = u"HeaderIsShared"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBACKCOL      = u"FooterBackColor"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBACKTRAN     = u"FooterBackTransparent"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRGRFFILT      = u"FooterBackGraphicFilter"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRGRFLOC       = u"FooterBackGraphicLocation"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRGRF          = u"FooterBackGraphic"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRGRFURL       = u"FooterBackGraphicURL"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRLEFTBOR      = u"FooterLeftBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRRIGHTBOR     = u"FooterRightBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBOTTBOR      = u"FooterBottomBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRTOPBOR       = u"FooterTopBorder"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRLEFTBDIS     = u"FooterLeftBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRRIGHTBDIS    = u"FooterRightBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBOTTBDIS     = u"FooterBottomBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRTOPBDIS      = u"FooterTopBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBRDDIST      = u"FooterBorderDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRSHADOW       = u"FooterShadowFormat"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRLEFTMAR      = u"FooterLeftMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRRIGHTMAR     = u"FooterRightMargin"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRBODYDIST     = u"FooterBodyDistance"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRHEIGHT       = u"FooterHeight"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRON           = u"FooterIsOn"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRDYNAMIC      = u"FooterIsDynamicHeight"_ustr;
inline constexpr OUString SC_UNO_PAGE_FTRSHARED       = u"FooterIsShared"_ustr;

//  document settings
inline constexpr OUString SC_UNO_CALCASSHOWN              = u"CalcAsShown"_ustr;
inline constexpr OUString SC_UNO_DEFTABSTOP               = u"DefaultTabStop"_ustr;
inline constexpr OUString SC_UNO_TABSTOPDIS               = u"TabStopDistance"_ustr; // is the same like the before, but only the writer name
inline constexpr OUString SC_UNO_IGNORECASE               = u"IgnoreCase"_ustr;
inline constexpr OUString SC_UNO_ITERCOUNT                = u"IterationCount"_ustr;
inline constexpr OUString SC_UNO_ITERENABLED              = u"IsIterationEnabled"_ustr;
inline constexpr OUString SC_UNO_ITEREPSILON              = u"IterationEpsilon"_ustr;
inline constexpr OUString SC_UNO_LOOKUPLABELS             = u"LookUpLabels"_ustr;
inline constexpr OUString SC_UNO_MATCHWHOLE               = u"MatchWholeCell"_ustr;
inline constexpr OUString SC_UNO_NULLDATE                 = u"NullDate"_ustr;
inline constexpr OUString SC_UNO_SPELLONLINE              = u"SpellOnline"_ustr;
inline constexpr OUString SC_UNO_STANDARDDEC              = u"StandardDecimals"_ustr;
inline constexpr OUString SC_UNO_REGEXENABLED             = u"RegularExpressions"_ustr;
inline constexpr OUString SC_UNO_WILDCARDSENABLED         = u"Wildcards"_ustr;
inline constexpr OUString SC_UNO_BASICLIBRARIES           = u"BasicLibraries"_ustr;
inline constexpr OUString SC_UNO_DIALOGLIBRARIES          = u"DialogLibraries"_ustr;
inline constexpr OUString SC_UNO_RUNTIMEUID               = u"RuntimeUID"_ustr;
inline constexpr OUString SC_UNO_HASVALIDSIGNATURES       = u"HasValidSignatures"_ustr;
inline constexpr OUString SC_UNO_ALLOWLINKUPDATE          = u"AllowLinkUpdate"_ustr;
inline constexpr OUString SC_UNO_ISLOADED                 = u"IsLoaded"_ustr;
inline constexpr OUString SC_UNO_ISUNDOENABLED            = u"IsUndoEnabled"_ustr;
inline constexpr OUString SC_UNO_ISADJUSTHEIGHTENABLED    = u"IsAdjustHeightEnabled"_ustr;
inline constexpr OUString SC_UNO_ISEXECUTELINKENABLED     = u"IsExecuteLinkEnabled"_ustr;
inline constexpr OUString SC_UNO_ISCHANGEREADONLYENABLED  = u"IsChangeReadOnlyEnabled"_ustr;
inline constexpr OUString SC_UNO_REFERENCEDEVICE          = u"ReferenceDevice"_ustr;
inline constexpr OUString SC_UNO_CODENAME                 = u"CodeName"_ustr;
inline constexpr OUString SC_UNO_INTEROPGRABBAG           = u"InteropGrabBag"_ustr;
inline constexpr OUString SC_UNO_RECORDCHANGES            = u"RecordChanges"_ustr;
inline constexpr OUString SC_UNO_ISRECORDCHANGESPROTECTED = u"IsRecordChangesProtected"_ustr;
inline constexpr OUString SC_UNO_SYNTAXSTRINGREF          = u"SyntaxStringRef"_ustr;


//  document properties from FormModel
inline constexpr OUString SC_UNO_APPLYFMDES           = u"ApplyFormDesignMode"_ustr;
inline constexpr OUString SC_UNO_AUTOCONTFOC          = u"AutomaticControlFocus"_ustr;

//  view options
inline constexpr OUString SC_UNO_COLROWHDR            = u"HasColumnRowHeaders"_ustr;
inline constexpr OUString SC_UNO_GRIDCOLOR            = u"GridColor"_ustr;
inline constexpr OUString SC_UNO_HIDESPELL            = u"HideSpellMarks"_ustr;                /* deprecated #i91949 */
inline constexpr OUString SC_UNO_HORSCROLL            = u"HasHorizontalScrollBar"_ustr;
inline constexpr OUString SC_UNO_OUTLSYMB             = u"IsOutlineSymbolsSet"_ustr;
inline constexpr OUString SC_UNO_SHEETTABS            = u"HasSheetTabs"_ustr;
inline constexpr OUString SC_UNO_SHOWANCHOR           = u"ShowAnchor"_ustr;
inline constexpr OUString SC_UNO_SHOWCHARTS           = u"ShowCharts"_ustr;
inline constexpr OUString SC_UNO_SHOWDRAW             = u"ShowDrawing"_ustr;
inline constexpr OUString SC_UNO_SHOWFORM             = u"ShowFormulas"_ustr;
inline constexpr OUString SC_UNO_SHOWGRID             = u"ShowGrid"_ustr;
inline constexpr OUString SC_UNO_SHOWHELP             = u"ShowHelpLines"_ustr;
inline constexpr OUString SC_UNO_SHOWNOTES            = u"ShowNotes"_ustr;
inline constexpr OUString SC_UNO_SHOWNOTEAUTHOR       = u"ShowNoteAuthor"_ustr;
inline constexpr OUString SC_UNO_SHOWFORMULASMARKS    = u"ShowFormulasMarks"_ustr;
inline constexpr OUString SC_UNO_SHOWOBJ              = u"ShowObjects"_ustr;
inline constexpr OUString SC_UNO_SHOWPAGEBR           = u"ShowPageBreaks"_ustr;
inline constexpr OUString SC_UNO_SHOWZERO             = u"ShowZeroValues"_ustr;
inline constexpr OUString SC_UNO_VALUEHIGH            = u"IsValueHighlightingEnabled"_ustr;
inline constexpr OUString SC_UNO_VERTSCROLL           = u"HasVerticalScrollBar"_ustr;
inline constexpr OUString SC_UNO_SNAPTORASTER         = u"IsSnapToRaster"_ustr;
inline constexpr OUString SC_UNO_RASTERVIS            = u"RasterIsVisible"_ustr;
inline constexpr OUString SC_UNO_RASTERRESX           = u"RasterResolutionX"_ustr;
inline constexpr OUString SC_UNO_RASTERRESY           = u"RasterResolutionY"_ustr;
inline constexpr OUString SC_UNO_RASTERSUBX           = u"RasterSubdivisionX"_ustr;
inline constexpr OUString SC_UNO_RASTERSUBY           = u"RasterSubdivisionY"_ustr;
inline constexpr OUString SC_UNO_RASTERSYNC           = u"IsRasterAxisSynchronized"_ustr;
inline constexpr OUString SC_UNO_AUTOCALC             = u"AutoCalculate"_ustr;
inline constexpr OUString SC_UNO_PRINTERNAME          = u"PrinterName"_ustr;
inline constexpr OUString SC_UNO_PRINTERSETUP         = u"PrinterSetup"_ustr;
inline constexpr OUString SC_UNO_PRINTERPAPER         = u"PrinterPaperFromSetup"_ustr;
inline constexpr OUString SC_UNO_APPLYDOCINF          = u"ApplyUserData"_ustr;
inline constexpr OUString SC_UNO_SAVE_THUMBNAIL       = u"SaveThumbnail"_ustr;
inline constexpr OUString SC_UNO_CHARCOMP             = u"CharacterCompressionType"_ustr;
inline constexpr OUString SC_UNO_ASIANKERN            = u"IsKernAsianPunctuation"_ustr;
inline constexpr OUString SC_UNO_VISAREA              = u"VisibleArea"_ustr;
inline constexpr OUString SC_UNO_ZOOMTYPE             = u"ZoomType"_ustr;
inline constexpr OUString SC_UNO_ZOOMVALUE            = u"ZoomValue"_ustr;
inline constexpr OUString SC_UNO_UPDTEMPL             = u"UpdateFromTemplate"_ustr;
inline constexpr OUString SC_UNO_FILTERED_RANGE_SELECTION       = u"FilteredRangeSelection"_ustr;
inline constexpr OUString SC_UNO_VISAREASCREEN        = u"VisibleAreaOnScreen"_ustr;
inline constexpr OUString SC_UNO_FORMULABARHEIGHT     = u"FormulaBarHeight"_ustr;
inline constexpr OUString SC_UNO_IMAGE_PREFERRED_DPI  = u"ImagePreferredDPI"_ustr;

/*Stampit enable/disable print cancel */
inline constexpr OUString SC_UNO_ALLOWPRINTJOBCANCEL  = u"AllowPrintJobCancel"_ustr;

//  old (5.2) property names for view options - for compatibility only!
inline constexpr OUString OLD_UNO_COLROWHDR           = u"ColumnRowHeaders"_ustr;
inline constexpr OUString OLD_UNO_HORSCROLL           = u"HorizontalScrollBar"_ustr;
inline constexpr OUString OLD_UNO_OUTLSYMB            = u"OutlineSymbols"_ustr;
inline constexpr OUString OLD_UNO_SHEETTABS           = u"SheetTabs"_ustr;
inline constexpr OUString OLD_UNO_VALUEHIGH           = u"ValueHighlighting"_ustr;
inline constexpr OUString OLD_UNO_VERTSCROLL          = u"VerticalScrollBar"_ustr;

//  data pilot source
inline constexpr OUString SC_UNO_DP_COLGRAND             = u"ColumnGrand"_ustr;
inline constexpr OUString SC_UNO_DP_ROWGRAND             = u"RowGrand"_ustr;
inline constexpr OUString SC_UNO_DP_ORIGINAL             = u"Original"_ustr;
inline constexpr OUString SC_UNO_DP_ORIGINAL_POS         = u"OriginalPosition"_ustr;
inline constexpr OUString SC_UNO_DP_ISDATALAYOUT         = u"IsDataLayoutDimension"_ustr;
inline constexpr OUString SC_UNO_DP_ORIENTATION          = u"Orientation"_ustr;
inline constexpr OUString SC_UNO_DP_POSITION             = u"Position"_ustr;
inline constexpr OUString SC_UNO_DP_FUNCTION             = u"Function"_ustr;
inline constexpr OUString SC_UNO_DP_FUNCTION2            = u"Function2"_ustr;
inline constexpr OUString SC_UNO_DP_USEDHIERARCHY        = u"UsedHierarchy"_ustr;
inline constexpr OUString SC_UNO_DP_FILTER               = u"Filter"_ustr;
inline constexpr OUString SC_UNO_DP_SUBTOTAL             = u"SubTotals"_ustr;
inline constexpr OUString SC_UNO_DP_SUBTOTAL2            = u"SubTotals2"_ustr;
inline constexpr OUString SC_UNO_DP_SHOWEMPTY            = u"ShowEmpty"_ustr;
inline constexpr OUString SC_UNO_DP_REPEATITEMLABELS     = u"RepeatItemLabels"_ustr;
inline constexpr OUString SC_UNO_DP_ISVISIBLE            = u"IsVisible"_ustr;
inline constexpr OUString SC_UNO_DP_SHOWDETAILS          = u"ShowDetails"_ustr;
inline constexpr OUString SC_UNO_DP_IGNOREEMPTY          = u"IgnoreEmptyRows"_ustr;
inline constexpr OUString SC_UNO_DP_REPEATEMPTY          = u"RepeatIfEmpty"_ustr;
inline constexpr OUString SC_UNO_DP_DATADESC             = u"DataDescription"_ustr;
#define SC_UNO_DP_NUMBERFO             SC_UNONAME_NUMFMT
inline constexpr OUString SC_UNO_DP_ROWFIELDCOUNT        = u"RowFieldCount"_ustr;
inline constexpr OUString SC_UNO_DP_COLUMNFIELDCOUNT     = u"ColumnFieldCount"_ustr;
inline constexpr OUString SC_UNO_DP_DATAFIELDCOUNT       = u"DataFieldCount"_ustr;
inline constexpr OUString SC_UNO_DP_LAYOUTNAME           = u"LayoutName"_ustr;
inline constexpr OUString SC_UNO_DP_FIELD_SUBTOTALNAME   = u"FieldSubtotalName"_ustr;
inline constexpr OUString SC_UNO_DP_GRANDTOTAL_NAME      = u"GrandTotalName"_ustr;
inline constexpr OUString SC_UNO_DP_HAS_HIDDEN_MEMBER    = u"HasHiddenMember"_ustr;
inline constexpr OUString SC_UNO_DP_FLAGS                = u"Flags"_ustr;

inline constexpr OUString SC_UNO_DP_REFVALUE             = u"ReferenceValue"_ustr;
inline constexpr OUString SC_UNO_DP_SORTING              = u"Sorting"_ustr;
inline constexpr OUString SC_UNO_DP_AUTOSHOW             = u"AutoShow"_ustr;
inline constexpr OUString SC_UNO_DP_LAYOUT               = u"Layout"_ustr;

// data pilot descriptor
inline constexpr OUString SC_UNO_DP_IGNORE_EMPTYROWS     = u"IgnoreEmptyRows"_ustr;
inline constexpr OUString SC_UNO_DP_DRILLDOWN            = u"DrillDownOnDoubleClick"_ustr;
inline constexpr OUString SC_UNO_DP_SHOWFILTER           = u"ShowFilterButton"_ustr;
inline constexpr OUString SC_UNO_DP_IMPORTDESC           = u"ImportDescriptor"_ustr;
inline constexpr OUString SC_UNO_DP_SOURCESERVICE        = u"SourceServiceName"_ustr;
inline constexpr OUString SC_UNO_DP_SERVICEARG           = u"ServiceArguments"_ustr;

// properties in data pilot descriptor ServiceArguments
inline constexpr OUString SC_UNO_DP_SOURCENAME           = u"SourceName"_ustr;
inline constexpr OUString SC_UNO_DP_OBJECTNAME           = u"ObjectName"_ustr;
inline constexpr OUString SC_UNO_DP_USERNAME             = u"UserName"_ustr;
inline constexpr OUString SC_UNO_DP_PASSWORD             = u"Password"_ustr;

//  range selection
inline constexpr OUString SC_UNONAME_INITVAL          = u"InitialValue"_ustr;
inline constexpr OUString SC_UNONAME_TITLE            = u"Title"_ustr;
inline constexpr OUString SC_UNONAME_CLOSEONUP        = u"CloseOnMouseRelease"_ustr;
inline constexpr OUString SC_UNONAME_SINGLECELL       = u"SingleCellMode"_ustr;
inline constexpr OUString SC_UNONAME_MULTISEL         = u"MultiSelectionMode"_ustr;

//  XRenderable
inline constexpr OUString SC_UNONAME_PAGESIZE         = u"PageSize"_ustr;
inline constexpr OUString SC_UNONAME_RENDERDEV        = u"RenderDevice"_ustr;
inline constexpr OUString SC_UNONAME_SOURCERANGE      = u"SourceRange"_ustr;
inline constexpr OUString SC_UNONAME_INC_NP_AREA      = u"PageIncludesNonprintableArea"_ustr;
inline constexpr OUString SC_UNONAME_CALCPAGESIZE     = u"CalcPageContentSize"_ustr;
inline constexpr OUString SC_UNONAME_CALCPAGEPOS      = u"CalcPagePos"_ustr;

// CellValueBinding
inline constexpr OUString SC_UNONAME_BOUNDCELL        = u"BoundCell"_ustr;
// CellRangeListSource
inline constexpr OUString SC_UNONAME_CELLRANGE        = u"CellRange"_ustr;

// CellAddressConversion / CellRangeAddressConversion
inline constexpr OUString SC_UNONAME_ADDRESS          = u"Address"_ustr;
inline constexpr OUString SC_UNONAME_UIREPR           = u"UserInterfaceRepresentation"_ustr;
inline constexpr OUString SC_UNONAME_PERSREPR         = u"PersistentRepresentation"_ustr;
inline constexpr OUString SC_UNONAME_XLA1REPR         = u"XLA1Representation"_ustr;
inline constexpr OUString SC_UNONAME_REFSHEET         = u"ReferenceSheet"_ustr;

// Security options
inline constexpr OUString SC_UNO_LOADREADONLY         = u"LoadReadonly"_ustr;
inline constexpr OUString SC_UNO_MODIFYPASSWORDINFO   = u"ModifyPasswordInfo"_ustr;
inline constexpr OUString SC_UNO_MODIFYPASSWORDHASH   = u"ModifyPasswordHash"_ustr;

// FormulaParser
inline constexpr OUString SC_UNO_COMPILEENGLISH       = u"CompileEnglish"_ustr;
inline constexpr OUString SC_UNO_FORMULACONVENTION    = u"FormulaConvention"_ustr;
inline constexpr OUString SC_UNO_IGNORELEADING        = u"IgnoreLeadingSpaces"_ustr;
inline constexpr OUString SC_UNO_OPCODEMAP            = u"OpCodeMap"_ustr;
inline constexpr OUString SC_UNO_EXTERNALLINKS        = u"ExternalLinks"_ustr;
inline constexpr OUString SC_UNO_COMPILEFAP           = u"CompileFAP"_ustr;
inline constexpr OUString SC_UNO_REF_CONV_CHARTOOXML  = u"RefConventionChartOOXML"_ustr;

// Chart2
inline constexpr OUString SC_UNONAME_ROLE                 = u"Role"_ustr;
inline constexpr OUString SC_UNONAME_HIDDENVALUES         = u"HiddenValues"_ustr;
inline constexpr OUString SC_UNONAME_INCLUDEHIDDENCELLS   = u"IncludeHiddenCells"_ustr;
inline constexpr OUString SC_UNONAME_USE_INTERNAL_DATA_PROVIDER = u"UseInternalDataProvider"_ustr;
inline constexpr OUString SC_UNONAME_HAS_STRING_LABEL = u"HasStringLabel"_ustr;
inline constexpr OUString SC_UNONAME_TIME_BASED       = u"TimeBased"_ustr;

// Solver
inline constexpr OUString SC_UNONAME_TIMEOUT          = u"Timeout"_ustr;
inline constexpr OUString SC_UNO_SHAREDOC = u"IsDocumentShared"_ustr;

// EventDescriptor
inline constexpr OUString SC_UNO_EVENTTYPE            = u"EventType"_ustr;
inline constexpr OUString SC_UNO_SCRIPT               = u"Script"_ustr;

// Named ranges
inline constexpr OUString SC_UNO_MODIFY_BROADCAST = u"ModifyAndBroadcast"_ustr;

inline constexpr OUString SC_UNO_EMBED_FONTS = u"EmbedFonts"_ustr;
inline constexpr OUString SC_UNO_EMBED_ONLY_USED_FONTS = u"EmbedOnlyUsedFonts"_ustr;
inline constexpr OUString SC_UNO_EMBED_FONT_SCRIPT_LATIN = u"EmbedLatinScriptFonts"_ustr;
inline constexpr OUString SC_UNO_EMBED_FONT_SCRIPT_ASIAN = u"EmbedAsianScriptFonts"_ustr;
inline constexpr OUString SC_UNO_EMBED_FONT_SCRIPT_COMPLEX = u"EmbedComplexScriptFonts"_ustr;

inline constexpr OUString SC_UNO_ODS_LOCK_SOLAR_MUTEX = u"ODSLockSolarMutex"_ustr;
inline constexpr OUString SC_UNO_ODS_IMPORT_STYLES = u"ODSImportStyles"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
