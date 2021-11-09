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

// service names
inline constexpr OUStringLiteral SC_SERVICENAME_VALBIND = u"com.sun.star.table.CellValueBinding";
inline constexpr OUStringLiteral SC_SERVICENAME_LISTCELLBIND = u"com.sun.star.table.ListPositionCellBinding";
inline constexpr OUStringLiteral SC_SERVICENAME_LISTSOURCE = u"com.sun.star.table.CellRangeListSource";
inline constexpr OUStringLiteral SC_SERVICENAME_CELLADDRESS = u"com.sun.star.table.CellAddressConversion";
inline constexpr OUStringLiteral SC_SERVICENAME_RANGEADDRESS = u"com.sun.star.table.CellRangeAddressConversion";

inline constexpr OUStringLiteral SC_SERVICENAME_FORMULAPARS = u"com.sun.star.sheet.FormulaParser";
inline constexpr OUStringLiteral SC_SERVICENAME_OPCODEMAPPER = u"com.sun.star.sheet.FormulaOpCodeMapper";

inline constexpr OUStringLiteral SC_SERVICENAME_CHDATAPROV = u"com.sun.star.chart2.data.DataProvider";
inline constexpr OUStringLiteral SC_SERVICENAME_CHRANGEHILIGHT = u"com.sun.star.chart2.data.RangeHighlightListener";
inline constexpr OUStringLiteral SC_SERVICENAME_CHART_PIVOTTABLE_DATAPROVIDER = u"com.sun.star.chart2.data.PivotTableDataProvider";

//  document
inline constexpr OUStringLiteral SC_UNO_AREALINKS = u"AreaLinks";
inline constexpr OUStringLiteral SC_UNO_DDELINKS = u"DDELinks";
inline constexpr OUStringLiteral SC_UNO_EXTERNALDOCLINKS = u"ExternalDocLinks";
inline constexpr OUStringLiteral SC_UNO_COLLABELRNG = u"ColumnLabelRanges";
inline constexpr OUStringLiteral SC_UNO_DATABASERNG = u"DatabaseRanges";
#define SC_UNO_UNNAMEDDBRNG         "UnnamedDatabaseRanges"
inline constexpr OUStringLiteral SC_UNO_NAMEDRANGES = u"NamedRanges";
inline constexpr OUStringLiteral SC_UNO_ROWLABELRNG = u"RowLabelRanges";
inline constexpr OUStringLiteral SC_UNO_SHEETLINKS = u"SheetLinks";
inline constexpr OUStringLiteral SC_UNO_FORBIDDEN = u"ForbiddenCharacters";
inline constexpr OUStringLiteral SC_UNO_HASDRAWPAGES = u"HasDrawPages";

//  CharacterProperties
inline constexpr OUStringLiteral SC_UNONAME_CCOLOR = u"CharColor";
inline constexpr OUStringLiteral SC_UNONAME_CHEIGHT = u"CharHeight";
inline constexpr OUStringLiteral SC_UNONAME_CUNDER = u"CharUnderline";
inline constexpr OUStringLiteral SC_UNONAME_CUNDLCOL = u"CharUnderlineColor";
inline constexpr OUStringLiteral SC_UNONAME_CUNDLHAS = u"CharUnderlineHasColor";
inline constexpr OUStringLiteral SC_UNONAME_COVER = u"CharOverline";
inline constexpr OUStringLiteral SC_UNONAME_COVRLCOL = u"CharOverlineColor";
inline constexpr OUStringLiteral SC_UNONAME_COVRLHAS = u"CharOverlineHasColor";
inline constexpr OUStringLiteral SC_UNONAME_CWEIGHT = u"CharWeight";
inline constexpr OUStringLiteral SC_UNONAME_CPOST = u"CharPosture";
inline constexpr OUStringLiteral SC_UNONAME_CCROSS = u"CharCrossedOut";
inline constexpr OUStringLiteral SC_UNONAME_CSTRIKE = u"CharStrikeout";
inline constexpr OUStringLiteral SC_UNONAME_CLOCAL = u"CharLocale";
inline constexpr OUStringLiteral SC_UNONAME_CSHADD = u"CharShadowed";
inline constexpr OUStringLiteral SC_UNONAME_CFONT = u"CharFont";
inline constexpr OUStringLiteral SC_UNONAME_COUTL = u"CharContoured";
inline constexpr OUStringLiteral SC_UNONAME_CEMPHAS = u"CharEmphasis";
inline constexpr OUStringLiteral SC_UNONAME_CFNAME = u"CharFontName";
inline constexpr OUStringLiteral SC_UNONAME_CFSTYLE = u"CharFontStyleName";
inline constexpr OUStringLiteral SC_UNONAME_CFFAMIL = u"CharFontFamily";
inline constexpr OUStringLiteral SC_UNONAME_CFCHARS = u"CharFontCharSet";
inline constexpr OUStringLiteral SC_UNONAME_CFPITCH = u"CharFontPitch";
inline constexpr OUStringLiteral SC_UNONAME_CRELIEF = u"CharRelief";
inline constexpr OUStringLiteral SC_UNONAME_CWORDMOD = u"CharWordMode";

inline constexpr OUStringLiteral SC_UNO_CJK_CFNAME = u"CharFontNameAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CFSTYLE = u"CharFontStyleNameAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CFFAMIL = u"CharFontFamilyAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CFCHARS = u"CharFontCharSetAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CFPITCH = u"CharFontPitchAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CHEIGHT = u"CharHeightAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CWEIGHT = u"CharWeightAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CPOST = u"CharPostureAsian";
inline constexpr OUStringLiteral SC_UNO_CJK_CLOCAL = u"CharLocaleAsian";

inline constexpr OUStringLiteral SC_UNO_CTL_CFNAME = u"CharFontNameComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CFSTYLE = u"CharFontStyleNameComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CFFAMIL = u"CharFontFamilyComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CFCHARS = u"CharFontCharSetComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CFPITCH = u"CharFontPitchComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CHEIGHT = u"CharHeightComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CWEIGHT = u"CharWeightComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CPOST = u"CharPostureComplex";
inline constexpr OUStringLiteral SC_UNO_CTL_CLOCAL = u"CharLocaleComplex";

//  CellProperties
inline constexpr OUStringLiteral SC_UNONAME_CELLSTYL = u"CellStyle";
inline constexpr OUStringLiteral SC_UNONAME_CELLBACK = u"CellBackColor";
inline constexpr OUStringLiteral SC_UNONAME_CELLTRAN = u"IsCellBackgroundTransparent";
inline constexpr OUStringLiteral SC_UNONAME_CELLPRO = u"CellProtection";
inline constexpr OUStringLiteral SC_UNONAME_CELLHJUS = u"HoriJustify";
inline constexpr OUStringLiteral SC_UNONAME_CELLVJUS = u"VertJustify";
inline constexpr OUStringLiteral SC_UNONAME_CELLHJUS_METHOD = u"HoriJustifyMethod";
inline constexpr OUStringLiteral SC_UNONAME_CELLVJUS_METHOD = u"VertJustifyMethod";
inline constexpr OUStringLiteral SC_UNONAME_CELLORI = u"Orientation";
inline constexpr OUStringLiteral SC_UNONAME_NUMFMT = u"NumberFormat";
inline constexpr OUStringLiteral SC_UNONAME_FORMATID = u"FormatID";
inline constexpr OUStringLiteral SC_UNONAME_SHADOW = u"ShadowFormat";
inline constexpr OUStringLiteral SC_UNONAME_TBLBORD = u"TableBorder";
inline constexpr OUStringLiteral SC_UNONAME_TBLBORD2 = u"TableBorder2";
inline constexpr OUStringLiteral SC_UNONAME_WRAP = u"IsTextWrapped";
inline constexpr OUStringLiteral SC_UNONAME_PINDENT = u"ParaIndent";
inline constexpr OUStringLiteral SC_UNONAME_PTMARGIN = u"ParaTopMargin";
inline constexpr OUStringLiteral SC_UNONAME_PBMARGIN = u"ParaBottomMargin";
inline constexpr OUStringLiteral SC_UNONAME_PLMARGIN = u"ParaLeftMargin";
inline constexpr OUStringLiteral SC_UNONAME_PRMARGIN = u"ParaRightMargin";
inline constexpr OUStringLiteral SC_UNONAME_ROTANG = u"RotateAngle";
inline constexpr OUStringLiteral SC_UNONAME_ROTREF = u"RotateReference";
inline constexpr OUStringLiteral SC_UNONAME_ASIANVERT = u"AsianVerticalMode";
inline constexpr OUStringLiteral SC_UNONAME_WRITING = u"WritingMode";
inline constexpr OUStringLiteral SC_UNONAME_HIDDEN = u"Hidden";

inline constexpr OUStringLiteral SC_UNONAME_BOTTBORDER = u"BottomBorder";
inline constexpr OUStringLiteral SC_UNONAME_LEFTBORDER = u"LeftBorder";
inline constexpr OUStringLiteral SC_UNONAME_RIGHTBORDER = u"RightBorder";
inline constexpr OUStringLiteral SC_UNONAME_TOPBORDER = u"TopBorder";

inline constexpr OUStringLiteral SC_UNONAME_BOTTBORDER2 = u"BottomBorder2";
inline constexpr OUStringLiteral SC_UNONAME_LEFTBORDER2 = u"LeftBorder2";
inline constexpr OUStringLiteral SC_UNONAME_RIGHTBORDER2 = u"RightBorder2";
inline constexpr OUStringLiteral SC_UNONAME_TOPBORDER2 = u"TopBorder2";

inline constexpr OUStringLiteral SC_UNONAME_DIAGONAL_TLBR = u"DiagonalTLBR";
inline constexpr OUStringLiteral SC_UNONAME_DIAGONAL_BLTR = u"DiagonalBLTR";

inline constexpr OUStringLiteral SC_UNONAME_DIAGONAL_TLBR2 = u"DiagonalTLBR2";
inline constexpr OUStringLiteral SC_UNONAME_DIAGONAL_BLTR2 = u"DiagonalBLTR2";

inline constexpr OUStringLiteral SC_UNONAME_SHRINK_TO_FIT = u"ShrinkToFit";

inline constexpr OUStringLiteral SC_UNONAME_PISHANG = u"ParaIsHangingPunctuation";
inline constexpr OUStringLiteral SC_UNONAME_PISCHDIST = u"ParaIsCharacterDistance";
inline constexpr OUStringLiteral SC_UNONAME_PISFORBID = u"ParaIsForbiddenRules";
inline constexpr OUStringLiteral SC_UNONAME_PISHYPHEN = u"ParaIsHyphenation";
inline constexpr OUStringLiteral SC_UNONAME_PADJUST = u"ParaAdjust";
inline constexpr OUStringLiteral SC_UNONAME_PLASTADJ = u"ParaLastLineAdjust";

inline constexpr OUStringLiteral SC_UNONAME_NUMRULES = u"NumberingRules";

//  Styles
inline constexpr OUStringLiteral SC_UNONAME_DISPNAME = u"DisplayName";

//  XStyleLoader
inline constexpr OUStringLiteral SC_UNONAME_OVERWSTL = u"OverwriteStyles";
inline constexpr OUStringLiteral SC_UNONAME_LOADCELL = u"LoadCellStyles";
inline constexpr OUStringLiteral SC_UNONAME_LOADPAGE = u"LoadPageStyles";

//  SheetCellRange
inline constexpr OUStringLiteral SC_UNONAME_POS = u"Position";
inline constexpr OUStringLiteral SC_UNONAME_SIZE = u"Size";
inline constexpr OUStringLiteral SC_UNONAME_ABSNAME = u"AbsoluteName";

//  column/row/sheet
inline constexpr OUStringLiteral SC_UNONAME_CELLHGT = u"Height";
inline constexpr OUStringLiteral SC_UNONAME_CELLWID = u"Width";
inline constexpr OUStringLiteral SC_UNONAME_CELLVIS = u"IsVisible";
inline constexpr OUStringLiteral SC_UNONAME_CELLFILT = u"IsFiltered";
inline constexpr OUStringLiteral SC_UNONAME_MANPAGE = u"IsManualPageBreak";
inline constexpr OUStringLiteral SC_UNONAME_NEWPAGE = u"IsStartOfNewPage";
inline constexpr OUStringLiteral SC_UNONAME_OHEIGHT = u"OptimalHeight";
inline constexpr OUStringLiteral SC_UNONAME_OWIDTH = u"OptimalWidth";
inline constexpr OUStringLiteral SC_UNONAME_PAGESTL = u"PageStyle";
inline constexpr OUStringLiteral SC_UNONAME_ISACTIVE = u"IsActive";
inline constexpr OUStringLiteral SC_UNONAME_BORDCOL = u"BorderColor";
inline constexpr OUStringLiteral SC_UNONAME_PROTECT = u"Protected";
inline constexpr OUStringLiteral SC_UNONAME_SHOWBORD = u"ShowBorder";
inline constexpr OUStringLiteral SC_UNONAME_PRINTBORD = u"PrintBorder";
inline constexpr OUStringLiteral SC_UNONAME_COPYBACK = u"CopyBack";
inline constexpr OUStringLiteral SC_UNONAME_COPYSTYL = u"CopyStyles";
inline constexpr OUStringLiteral SC_UNONAME_COPYFORM = u"CopyFormulas";
inline constexpr OUStringLiteral SC_UNONAME_TABLAYOUT = u"TableLayout";
inline constexpr OUStringLiteral SC_UNONAME_AUTOPRINT = u"AutomaticPrintArea";
inline constexpr OUStringLiteral SC_UNONAME_TABCOLOR = u"TabColor";
inline constexpr OUStringLiteral SC_UNONAME_CONDFORMAT = u"ConditionalFormats";

#define SC_UNONAME_VISFLAG          "VisibleFlag"

//  LinkTarget
inline constexpr OUStringLiteral SC_UNO_LINKDISPBIT = u"LinkDisplayBitmap";
inline constexpr OUStringLiteral SC_UNO_LINKDISPNAME = u"LinkDisplayName";

//  drawing objects
inline constexpr OUStringLiteral SC_UNONAME_IMAGEMAP = u"ImageMap";
inline constexpr OUStringLiteral SC_UNONAME_ANCHOR = u"Anchor";
inline constexpr OUStringLiteral SC_UNONAME_RESIZE_WITH_CELL = u"ResizeWithCell";
inline constexpr OUStringLiteral SC_UNONAME_HORIPOS = u"HoriOrientPosition";
inline constexpr OUStringLiteral SC_UNONAME_VERTPOS = u"VertOrientPosition";
inline constexpr OUStringLiteral SC_UNONAME_HYPERLINK = u"Hyperlink";
inline constexpr OUStringLiteral SC_UNONAME_MOVEPROTECT = u"MoveProtect";

//  other cell properties
inline constexpr OUStringLiteral SC_UNONAME_CHCOLHDR = u"ChartColumnAsLabel";
inline constexpr OUStringLiteral SC_UNONAME_CHROWHDR = u"ChartRowAsLabel";
inline constexpr OUStringLiteral SC_UNONAME_CONDFMT = u"ConditionalFormat";
inline constexpr OUStringLiteral SC_UNONAME_CONDLOC = u"ConditionalFormatLocal";
inline constexpr OUStringLiteral SC_UNONAME_CONDXML = u"ConditionalFormatXML";
inline constexpr OUStringLiteral SC_UNONAME_VALIDAT = u"Validation";
inline constexpr OUStringLiteral SC_UNONAME_VALILOC = u"ValidationLocal";
inline constexpr OUStringLiteral SC_UNONAME_VALIXML = u"ValidationXML";
inline constexpr OUStringLiteral SC_UNONAME_FORMLOC = u"FormulaLocal";
inline constexpr OUStringLiteral SC_UNONAME_FORMRT = u"FormulaResultType";
inline constexpr OUStringLiteral SC_UNONAME_FORMRT2 = u"FormulaResultType2";
inline constexpr OUStringLiteral SC_UNONAME_CELLCONTENTTYPE = u"CellContentType";

inline constexpr OUStringLiteral SC_UNONAME_USERDEF = u"UserDefinedAttributes";
inline constexpr OUStringLiteral SC_UNONAME_TEXTUSER = u"TextUserDefinedAttributes";

//  auto format
inline constexpr OUStringLiteral SC_UNONAME_INCBACK = u"IncludeBackground";
inline constexpr OUStringLiteral SC_UNONAME_INCBORD = u"IncludeBorder";
inline constexpr OUStringLiteral SC_UNONAME_INCFONT = u"IncludeFont";
inline constexpr OUStringLiteral SC_UNONAME_INCJUST = u"IncludeJustify";
inline constexpr OUStringLiteral SC_UNONAME_INCNUM = u"IncludeNumberFormat";
inline constexpr OUStringLiteral SC_UNONAME_INCWIDTH = u"IncludeWidthAndHeight";

//  function description
inline constexpr OUStringLiteral SC_UNONAME_ARGUMENTS = u"Arguments";
inline constexpr OUStringLiteral SC_UNONAME_CATEGORY = u"Category";
inline constexpr OUStringLiteral SC_UNONAME_DESCRIPTION = u"Description";
inline constexpr OUStringLiteral SC_UNONAME_ID = u"Id";
inline constexpr OUStringLiteral SC_UNONAME_NAME = u"Name";

//  application settings
inline constexpr OUStringLiteral SC_UNONAME_DOAUTOCP = u"DoAutoComplete";
inline constexpr OUStringLiteral SC_UNONAME_ENTERED = u"EnterEdit";
inline constexpr OUStringLiteral SC_UNONAME_EXPREF = u"ExpandReferences";
inline constexpr OUStringLiteral SC_UNONAME_EXTFMT = u"ExtendFormat";
inline constexpr OUStringLiteral SC_UNONAME_LINKUPD = u"LinkUpdateMode";
inline constexpr OUStringLiteral SC_UNONAME_MARKHDR = u"MarkHeader";
inline constexpr OUStringLiteral SC_UNONAME_METRIC = u"Metric";
inline constexpr OUStringLiteral SC_UNONAME_MOVEDIR = u"MoveDirection";
inline constexpr OUStringLiteral SC_UNONAME_MOVESEL = u"MoveSelection";
inline constexpr OUStringLiteral SC_UNONAME_RANGEFIN = u"RangeFinder";
inline constexpr OUStringLiteral SC_UNONAME_SCALE = u"Scale";
inline constexpr OUStringLiteral SC_UNONAME_STBFUNC = u"StatusBarFunction";
inline constexpr OUStringLiteral SC_UNONAME_ULISTS = u"UserLists";
inline constexpr OUStringLiteral SC_UNONAME_USETABCOL = u"UseTabCol";
inline constexpr OUStringLiteral SC_UNONAME_PRMETRICS = u"UsePrinterMetrics";
inline constexpr OUStringLiteral SC_UNONAME_PRALLSH = u"PrintAllSheets";
inline constexpr OUStringLiteral SC_UNONAME_PREMPTY = u"PrintEmptyPages";
inline constexpr OUStringLiteral SC_UNONAME_REPLWARN = u"ReplaceCellsWarning";

//  data pilot field
inline constexpr OUStringLiteral SC_UNONAME_FUNCTION = u"Function";
inline constexpr OUStringLiteral SC_UNONAME_FUNCTION2 = u"Function2";
inline constexpr OUStringLiteral SC_UNONAME_SUBTOTALS = u"Subtotals";
inline constexpr OUStringLiteral SC_UNONAME_SUBTOTALS2 = u"Subtotals2";
inline constexpr OUStringLiteral SC_UNONAME_SELPAGE = u"SelectedPage";
inline constexpr OUStringLiteral SC_UNONAME_USESELPAGE = u"UseSelectedPage";
inline constexpr OUStringLiteral SC_UNONAME_HASREFERENCE = u"HasReference";
inline constexpr OUStringLiteral SC_UNONAME_REFERENCE = u"Reference";
inline constexpr OUStringLiteral SC_UNONAME_HASAUTOSHOW = u"HasAutoShowInfo";
inline constexpr OUStringLiteral SC_UNONAME_AUTOSHOW = u"AutoShowInfo";
inline constexpr OUStringLiteral SC_UNONAME_HASSORTINFO = u"HasSortInfo";
inline constexpr OUStringLiteral SC_UNONAME_SORTINFO = u"SortInfo";
inline constexpr OUStringLiteral SC_UNONAME_HASLAYOUTINFO = u"HasLayoutInfo";
inline constexpr OUStringLiteral SC_UNONAME_LAYOUTINFO = u"LayoutInfo";
inline constexpr OUStringLiteral SC_UNONAME_ISGROUP = u"IsGroupField";
inline constexpr OUStringLiteral SC_UNONAME_GROUPINFO = u"GroupInfo";
inline constexpr OUStringLiteral SC_UNONAME_SHOWEMPTY = u"ShowEmpty";
inline constexpr OUStringLiteral SC_UNONAME_REPEATITEMLABELS = u"RepeatItemLabels";

//  data pilot item
inline constexpr OUStringLiteral SC_UNONAME_SHOWDETAIL = u"ShowDetail";
inline constexpr OUStringLiteral SC_UNONAME_ISHIDDEN = u"IsHidden";

//  database options
inline constexpr OUStringLiteral SC_UNONAME_CASE = u"CaseSensitive";
inline constexpr OUStringLiteral SC_UNONAME_DBNAME = u"DatabaseName";
inline constexpr OUStringLiteral SC_UNONAME_FORMATS = u"IncludeFormats";
inline constexpr OUStringLiteral SC_UNONAME_INSBRK = u"InsertPageBreaks";
inline constexpr OUStringLiteral SC_UNONAME_KEEPFORM = u"KeepFormats";
inline constexpr OUStringLiteral SC_UNONAME_MOVCELLS = u"MoveCells";
inline constexpr OUStringLiteral SC_UNONAME_ISUSER = u"IsUserDefined";
inline constexpr OUStringLiteral SC_UNONAME_ISNATIVE = u"IsNative";
#define SC_UNONAME_REGEXP           "RegularExpressions"
#define SC_UNONAME_WILDCARDS        "Wildcards"
inline constexpr OUStringLiteral SC_UNONAME_SAVEOUT = u"SaveOutputPosition";
inline constexpr OUStringLiteral SC_UNONAME_SKIPDUP = u"SkipDuplicates";
inline constexpr OUStringLiteral SC_UNONAME_SRCOBJ = u"SourceObject";
inline constexpr OUStringLiteral SC_UNONAME_SRCTYPE = u"SourceType";
inline constexpr OUStringLiteral SC_UNONAME_STRIPDAT = u"StripData";
inline constexpr OUStringLiteral SC_UNONAME_USEREGEX = u"UseRegularExpressions";
inline constexpr OUStringLiteral SC_UNONAME_ULIST = u"UserListEnabled";
inline constexpr OUStringLiteral SC_UNONAME_UINDEX = u"UserListIndex";
inline constexpr OUStringLiteral SC_UNONAME_BINDFMT = u"BindFormatsToContent";
inline constexpr OUStringLiteral SC_UNONAME_COPYOUT = u"CopyOutputData";
inline constexpr OUStringLiteral SC_UNONAME_ISCASE = u"IsCaseSensitive";
inline constexpr OUStringLiteral SC_UNONAME_ISULIST = u"IsUserListEnabled";
inline constexpr OUStringLiteral SC_UNONAME_OUTPOS = u"OutputPosition";
inline constexpr OUStringLiteral SC_UNONAME_CONTHDR = u"ContainsHeader";
inline constexpr OUStringLiteral SC_UNONAME_MAXFLD = u"MaxFieldCount";
inline constexpr OUStringLiteral SC_UNONAME_ORIENT = u"Orientation";
inline constexpr OUStringLiteral SC_UNONAME_ISSORTCOLUMNS = u"IsSortColumns";
inline constexpr OUStringLiteral SC_UNONAME_SORTFLD = u"SortFields";
inline constexpr OUStringLiteral SC_UNONAME_SORTASC = u"SortAscending";
inline constexpr OUStringLiteral SC_UNONAME_ENUSLIST = u"EnableUserSortList";
inline constexpr OUStringLiteral SC_UNONAME_USINDEX = u"UserSortListIndex";
inline constexpr OUStringLiteral SC_UNONAME_COLLLOC = u"CollatorLocale";
inline constexpr OUStringLiteral SC_UNONAME_COLLALG = u"CollatorAlgorithm";
inline constexpr OUStringLiteral SC_UNONAME_AUTOFLT = u"AutoFilter";
inline constexpr OUStringLiteral SC_UNONAME_FLTCRT = u"FilterCriteriaSource";
inline constexpr OUStringLiteral SC_UNONAME_USEFLTCRT = u"UseFilterCriteriaSource";
inline constexpr OUStringLiteral SC_UNONAME_ENABSORT = u"EnableSort";
inline constexpr OUStringLiteral SC_UNONAME_FROMSELECT = u"FromSelection";
inline constexpr OUStringLiteral SC_UNONAME_CONRES = u"ConnectionResource";
inline constexpr OUStringLiteral SC_UNONAME_TOKENINDEX = u"TokenIndex";
inline constexpr OUStringLiteral SC_UNONAME_ISSHAREDFMLA = u"IsSharedFormula";
inline constexpr OUStringLiteral SC_UNONAME_TOTALSROW = u"TotalsRow";

//  text fields
inline constexpr OUStringLiteral SC_UNONAME_ANCTYPE = u"AnchorType";
inline constexpr OUStringLiteral SC_UNONAME_ANCTYPES = u"AnchorTypes";
inline constexpr OUStringLiteral SC_UNONAME_TEXTWRAP = u"TextWrap";
inline constexpr OUStringLiteral SC_UNONAME_FILEFORM = u"FileFormat";
#define SC_UNONAME_TEXTFIELD_TYPE   "TextFieldType"

//  url field
inline constexpr OUStringLiteral SC_UNONAME_REPR = u"Representation";
inline constexpr OUStringLiteral SC_UNONAME_TARGET = u"TargetFrame";
inline constexpr OUStringLiteral SC_UNONAME_URL = u"URL";

// date time field
inline constexpr OUStringLiteral SC_UNONAME_ISDATE = u"IsDate";
inline constexpr OUStringLiteral SC_UNONAME_ISFIXED = u"IsFixed";
inline constexpr OUStringLiteral SC_UNONAME_DATETIME = u"DateTime";

// table field
inline constexpr OUStringLiteral SC_UNONAME_TABLEPOS = u"TablePosition";

//  conditional format
#define SC_UNONAME_OPERATOR         "Operator"
#define SC_UNONAME_FORMULA1         "Formula1"
#define SC_UNONAME_FORMULA2         "Formula2"
#define SC_UNONAME_SOURCEPOS        "SourcePosition"
#define SC_UNONAME_SOURCESTR        "SourcePositionAsString" // only for use in XML filter
#define SC_UNONAME_FORMULANMSP1     "FormulaNamespace1" // only for use in XML filter
#define SC_UNONAME_FORMULANMSP2     "FormulaNamespace2" // only for use in XML filter
#define SC_UNONAME_GRAMMAR1         "Grammar1" // only for use in XML filter
#define SC_UNONAME_GRAMMAR2         "Grammar2" // only for use in XML filter
#define SC_UNONAME_STYLENAME        "StyleName"

//  validation
inline constexpr OUStringLiteral SC_UNONAME_ERRALSTY = u"ErrorAlertStyle";
inline constexpr OUStringLiteral SC_UNONAME_ERRMESS = u"ErrorMessage";
inline constexpr OUStringLiteral SC_UNONAME_ERRTITLE = u"ErrorTitle";
inline constexpr OUStringLiteral SC_UNONAME_IGNOREBL = u"IgnoreBlankCells";
inline constexpr OUStringLiteral SC_UNONAME_INPMESS = u"InputMessage";
inline constexpr OUStringLiteral SC_UNONAME_INPTITLE = u"InputTitle";
inline constexpr OUStringLiteral SC_UNONAME_SHOWERR = u"ShowErrorMessage";
inline constexpr OUStringLiteral SC_UNONAME_SHOWINP = u"ShowInputMessage";
inline constexpr OUStringLiteral SC_UNONAME_SHOWLIST = u"ShowList";
inline constexpr OUStringLiteral SC_UNONAME_TYPE = u"Type";

//  links
inline constexpr OUStringLiteral SC_UNONAME_FILTER = u"Filter";
inline constexpr OUStringLiteral SC_UNONAME_FILTOPT = u"FilterOptions";
inline constexpr OUStringLiteral SC_UNONAME_LINKURL = u"Url";
inline constexpr OUStringLiteral SC_UNONAME_REFPERIOD = u"RefreshPeriod";
inline constexpr OUStringLiteral SC_UNONAME_REFDELAY = u"RefreshDelay"; // deprecated, don't use anymore

//  search descriptor
inline constexpr OUStringLiteral SC_UNO_SRCHBACK = u"SearchBackwards";
inline constexpr OUStringLiteral SC_UNO_SRCHBYROW = u"SearchByRow";
inline constexpr OUStringLiteral SC_UNO_SRCHCASE = u"SearchCaseSensitive";
inline constexpr OUStringLiteral SC_UNO_SRCHREGEXP = u"SearchRegularExpression";
inline constexpr OUStringLiteral SC_UNO_SRCHWILDCARD = u"SearchWildcard";
inline constexpr OUStringLiteral SC_UNO_SRCHSIM = u"SearchSimilarity";
inline constexpr OUStringLiteral SC_UNO_SRCHSIMADD = u"SearchSimilarityAdd";
inline constexpr OUStringLiteral SC_UNO_SRCHSIMEX = u"SearchSimilarityExchange";
inline constexpr OUStringLiteral SC_UNO_SRCHSIMREL = u"SearchSimilarityRelax";
inline constexpr OUStringLiteral SC_UNO_SRCHSIMREM = u"SearchSimilarityRemove";
inline constexpr OUStringLiteral SC_UNO_SRCHSTYLES = u"SearchStyles";
inline constexpr OUStringLiteral SC_UNO_SRCHTYPE = u"SearchType";
inline constexpr OUStringLiteral SC_UNO_SRCHWORDS = u"SearchWords";
#define SC_UNO_SRCHFILTERED         "SearchFiltered"
#define SC_UNO_SRCHFORMATTED        "SearchFormatted"

//  old (5.2) property names for page styles - for compatibility only!
inline constexpr OUStringLiteral OLD_UNO_PAGE_BACKCOLOR = u"BackgroundColor";
inline constexpr OUStringLiteral OLD_UNO_PAGE_BACKTRANS = u"IsBackgroundTransparent";
inline constexpr OUStringLiteral OLD_UNO_PAGE_HDRBACKCOL = u"HeaderBackgroundColor";
inline constexpr OUStringLiteral OLD_UNO_PAGE_HDRBACKTRAN = u"TransparentHeaderBackground";
inline constexpr OUStringLiteral OLD_UNO_PAGE_HDRSHARED = u"HeaderShared";
inline constexpr OUStringLiteral OLD_UNO_PAGE_HDRDYNAMIC = u"HeaderDynamic";
inline constexpr OUStringLiteral OLD_UNO_PAGE_HDRON = u"HeaderOn";
inline constexpr OUStringLiteral OLD_UNO_PAGE_FTRBACKCOL = u"FooterBackgroundColor";
inline constexpr OUStringLiteral OLD_UNO_PAGE_FTRBACKTRAN = u"TransparentFooterBackground";
inline constexpr OUStringLiteral OLD_UNO_PAGE_FTRSHARED = u"FooterShared";
inline constexpr OUStringLiteral OLD_UNO_PAGE_FTRDYNAMIC = u"FooterDynamic";
inline constexpr OUStringLiteral OLD_UNO_PAGE_FTRON = u"FooterOn";

//  page styles
inline constexpr OUStringLiteral SC_UNO_PAGE_BACKCOLOR = u"BackColor";
inline constexpr OUStringLiteral SC_UNO_PAGE_BACKTRANS = u"BackTransparent";
inline constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICFILT = u"BackGraphicFilter";
inline constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICLOC = u"BackGraphicLocation";
inline constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICURL = u"BackGraphicURL";
inline constexpr OUStringLiteral SC_UNO_PAGE_GRAPHIC = u"BackGraphic";
#define SC_UNO_PAGE_LEFTBORDER      SC_UNONAME_LEFTBORDER
#define SC_UNO_PAGE_RIGHTBORDER     SC_UNONAME_RIGHTBORDER
#define SC_UNO_PAGE_BOTTBORDER      SC_UNONAME_BOTTBORDER
#define SC_UNO_PAGE_TOPBORDER       SC_UNONAME_TOPBORDER
inline constexpr OUStringLiteral SC_UNO_PAGE_LEFTBRDDIST = u"LeftBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_RIGHTBRDDIST = u"RightBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_BOTTBRDDIST = u"BottomBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_TOPBRDDIST = u"TopBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_BORDERDIST = u"BorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_SHADOWFORM = u"ShadowFormat";
inline constexpr OUStringLiteral SC_UNO_PAGE_LEFTMARGIN = u"LeftMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_RIGHTMARGIN = u"RightMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_TOPMARGIN = u"TopMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_BOTTMARGIN = u"BottomMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_LANDSCAPE = u"IsLandscape";
inline constexpr OUStringLiteral SC_UNO_PAGE_NUMBERTYPE = u"NumberingType";
inline constexpr OUStringLiteral SC_UNO_PAGE_SYTLELAYOUT = u"PageStyleLayout";
inline constexpr OUStringLiteral SC_UNO_PAGE_PAPERTRAY = u"PrinterPaperTray";
inline constexpr OUStringLiteral SC_UNO_PAGE_SIZE = u"Size";
inline constexpr OUStringLiteral SC_UNO_PAGE_WIDTH = u"Width";
inline constexpr OUStringLiteral SC_UNO_PAGE_HEIGHT = u"Height";
inline constexpr OUStringLiteral SC_UNO_PAGE_CENTERHOR = u"CenterHorizontally";
inline constexpr OUStringLiteral SC_UNO_PAGE_CENTERVER = u"CenterVertically";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTANNOT = u"PrintAnnotations";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTGRID = u"PrintGrid";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTHEADER = u"PrintHeaders";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTCHARTS = u"PrintCharts";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTOBJS = u"PrintObjects";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTDRAW = u"PrintDrawing";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTDOWN = u"PrintDownFirst";
inline constexpr OUStringLiteral SC_UNO_PAGE_SCALEVAL = u"PageScale";
inline constexpr OUStringLiteral SC_UNO_PAGE_SCALETOPAG = u"ScaleToPages";
inline constexpr OUStringLiteral SC_UNO_PAGE_SCALETOX = u"ScaleToPagesX";
inline constexpr OUStringLiteral SC_UNO_PAGE_SCALETOY = u"ScaleToPagesY";
inline constexpr OUStringLiteral SC_UNO_PAGE_FIRSTPAGE = u"FirstPageNumber";
inline constexpr OUStringLiteral SC_UNO_PAGE_FIRSTHDRSHARED = u"FirstPageHeaderIsShared";
inline constexpr OUStringLiteral SC_UNO_PAGE_FIRSTFTRSHARED = u"FirstPageFooterIsShared";
inline constexpr OUStringLiteral SC_UNO_PAGE_LEFTHDRCONT = u"LeftPageHeaderContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_LEFTFTRCONT = u"LeftPageFooterContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_RIGHTHDRCON = u"RightPageHeaderContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_RIGHTFTRCON = u"RightPageFooterContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_FIRSTHDRCONT = u"FirstPageHeaderContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_FIRSTFTRCONT = u"FirstPageFooterContent";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTFORMUL = u"PrintFormulas";
inline constexpr OUStringLiteral SC_UNO_PAGE_PRINTZERO = u"PrintZeroValues";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBACKCOL = u"HeaderBackColor";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBACKTRAN = u"HeaderBackTransparent";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFFILT = u"HeaderBackGraphicFilter";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFLOC = u"HeaderBackGraphicLocation";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRGRF = u"HeaderBackGraphic";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFURL = u"HeaderBackGraphicURL";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTBOR = u"HeaderLeftBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTBOR = u"HeaderRightBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBOTTBOR = u"HeaderBottomBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRTOPBOR = u"HeaderTopBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTBDIS = u"HeaderLeftBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTBDIS = u"HeaderRightBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBOTTBDIS = u"HeaderBottomBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRTOPBDIS = u"HeaderTopBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBRDDIST = u"HeaderBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRSHADOW = u"HeaderShadowFormat";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTMAR = u"HeaderLeftMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTMAR = u"HeaderRightMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRBODYDIST = u"HeaderBodyDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRHEIGHT = u"HeaderHeight";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRON = u"HeaderIsOn";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRDYNAMIC = u"HeaderIsDynamicHeight";
inline constexpr OUStringLiteral SC_UNO_PAGE_HDRSHARED = u"HeaderIsShared";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBACKCOL = u"FooterBackColor";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBACKTRAN = u"FooterBackTransparent";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFFILT = u"FooterBackGraphicFilter";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFLOC = u"FooterBackGraphicLocation";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRGRF = u"FooterBackGraphic";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFURL = u"FooterBackGraphicURL";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTBOR = u"FooterLeftBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTBOR = u"FooterRightBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBOTTBOR = u"FooterBottomBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRTOPBOR = u"FooterTopBorder";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTBDIS = u"FooterLeftBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTBDIS = u"FooterRightBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBOTTBDIS = u"FooterBottomBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRTOPBDIS = u"FooterTopBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBRDDIST = u"FooterBorderDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRSHADOW = u"FooterShadowFormat";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTMAR = u"FooterLeftMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTMAR = u"FooterRightMargin";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRBODYDIST = u"FooterBodyDistance";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRHEIGHT = u"FooterHeight";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRON = u"FooterIsOn";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRDYNAMIC = u"FooterIsDynamicHeight";
inline constexpr OUStringLiteral SC_UNO_PAGE_FTRSHARED = u"FooterIsShared";

//  document settings
inline constexpr OUStringLiteral SC_UNO_CALCASSHOWN = u"CalcAsShown";
inline constexpr OUStringLiteral SC_UNO_DEFTABSTOP = u"DefaultTabStop";
inline constexpr OUStringLiteral SC_UNO_TABSTOPDIS = u"TabStopDistance"; // is the same like the before, but only the writer name
inline constexpr OUStringLiteral SC_UNO_IGNORECASE = u"IgnoreCase";
inline constexpr OUStringLiteral SC_UNO_ITERCOUNT = u"IterationCount";
inline constexpr OUStringLiteral SC_UNO_ITERENABLED = u"IsIterationEnabled";
inline constexpr OUStringLiteral SC_UNO_ITEREPSILON = u"IterationEpsilon";
inline constexpr OUStringLiteral SC_UNO_LOOKUPLABELS = u"LookUpLabels";
inline constexpr OUStringLiteral SC_UNO_MATCHWHOLE = u"MatchWholeCell";
inline constexpr OUStringLiteral SC_UNO_NULLDATE = u"NullDate";
inline constexpr OUStringLiteral SC_UNO_SPELLONLINE = u"SpellOnline";
inline constexpr OUStringLiteral SC_UNO_STANDARDDEC = u"StandardDecimals";
inline constexpr OUStringLiteral SC_UNO_REGEXENABLED = u"RegularExpressions";
inline constexpr OUStringLiteral SC_UNO_WILDCARDSENABLED = u"Wildcards";
inline constexpr OUStringLiteral SC_UNO_BASICLIBRARIES = u"BasicLibraries";
inline constexpr OUStringLiteral SC_UNO_DIALOGLIBRARIES = u"DialogLibraries";
inline constexpr OUStringLiteral SC_UNO_RUNTIMEUID = u"RuntimeUID";
inline constexpr OUStringLiteral SC_UNO_HASVALIDSIGNATURES = u"HasValidSignatures";
inline constexpr OUStringLiteral SC_UNO_ISLOADED = u"IsLoaded";
inline constexpr OUStringLiteral SC_UNO_ISUNDOENABLED = u"IsUndoEnabled";
inline constexpr OUStringLiteral SC_UNO_ISADJUSTHEIGHTENABLED = u"IsAdjustHeightEnabled";
inline constexpr OUStringLiteral SC_UNO_ISEXECUTELINKENABLED = u"IsExecuteLinkEnabled";
inline constexpr OUStringLiteral SC_UNO_ISCHANGEREADONLYENABLED = u"IsChangeReadOnlyEnabled";
inline constexpr OUStringLiteral SC_UNO_REFERENCEDEVICE = u"ReferenceDevice";
inline constexpr OUStringLiteral SC_UNO_CODENAME = u"CodeName";
inline constexpr OUStringLiteral SC_UNO_INTEROPGRABBAG = u"InteropGrabBag";
inline constexpr OUStringLiteral SC_UNO_RECORDCHANGES = u"RecordChanges";
inline constexpr OUStringLiteral SC_UNO_ISRECORDCHANGESPROTECTED = u"IsRecordChangesProtected";
inline constexpr OUStringLiteral SC_UNO_SYNTAXSTRINGREF = u"SyntaxStringRef";


//  document properties from FormModel
inline constexpr OUStringLiteral SC_UNO_APPLYFMDES = u"ApplyFormDesignMode";
inline constexpr OUStringLiteral SC_UNO_AUTOCONTFOC = u"AutomaticControlFocus";

//  view options
inline constexpr OUStringLiteral SC_UNO_COLROWHDR = u"HasColumnRowHeaders";
inline constexpr OUStringLiteral SC_UNO_GRIDCOLOR = u"GridColor";
inline constexpr OUStringLiteral SC_UNO_HIDESPELL = u"HideSpellMarks";                /* deprecated #i91949 */
inline constexpr OUStringLiteral SC_UNO_HORSCROLL = u"HasHorizontalScrollBar";
inline constexpr OUStringLiteral SC_UNO_OUTLSYMB = u"IsOutlineSymbolsSet";
inline constexpr OUStringLiteral SC_UNO_SHEETTABS = u"HasSheetTabs";
inline constexpr OUStringLiteral SC_UNO_SHOWANCHOR = u"ShowAnchor";
inline constexpr OUStringLiteral SC_UNO_SHOWCHARTS = u"ShowCharts";
inline constexpr OUStringLiteral SC_UNO_SHOWDRAW = u"ShowDrawing";
inline constexpr OUStringLiteral SC_UNO_SHOWFORM = u"ShowFormulas";
inline constexpr OUStringLiteral SC_UNO_SHOWGRID = u"ShowGrid";
inline constexpr OUStringLiteral SC_UNO_SHOWHELP = u"ShowHelpLines";
inline constexpr OUStringLiteral SC_UNO_SHOWNOTES = u"ShowNotes";
inline constexpr OUStringLiteral SC_UNO_SHOWOBJ = u"ShowObjects";
inline constexpr OUStringLiteral SC_UNO_SHOWPAGEBR = u"ShowPageBreaks";
inline constexpr OUStringLiteral SC_UNO_SHOWZERO = u"ShowZeroValues";
inline constexpr OUStringLiteral SC_UNO_VALUEHIGH = u"IsValueHighlightingEnabled";
inline constexpr OUStringLiteral SC_UNO_VERTSCROLL = u"HasVerticalScrollBar";
inline constexpr OUStringLiteral SC_UNO_SNAPTORASTER = u"IsSnapToRaster";
inline constexpr OUStringLiteral SC_UNO_RASTERVIS = u"RasterIsVisible";
inline constexpr OUStringLiteral SC_UNO_RASTERRESX = u"RasterResolutionX";
inline constexpr OUStringLiteral SC_UNO_RASTERRESY = u"RasterResolutionY";
inline constexpr OUStringLiteral SC_UNO_RASTERSUBX = u"RasterSubdivisionX";
inline constexpr OUStringLiteral SC_UNO_RASTERSUBY = u"RasterSubdivisionY";
inline constexpr OUStringLiteral SC_UNO_RASTERSYNC = u"IsRasterAxisSynchronized";
inline constexpr OUStringLiteral SC_UNO_AUTOCALC = u"AutoCalculate";
inline constexpr OUStringLiteral SC_UNO_PRINTERNAME = u"PrinterName";
inline constexpr OUStringLiteral SC_UNO_PRINTERSETUP = u"PrinterSetup";
inline constexpr OUStringLiteral SC_UNO_PRINTERPAPER = u"PrinterPaperFromSetup";
inline constexpr OUStringLiteral SC_UNO_APPLYDOCINF = u"ApplyUserData";
inline constexpr OUStringLiteral SC_UNO_SAVE_THUMBNAIL = u"SaveThumbnail";
inline constexpr OUStringLiteral SC_UNO_CHARCOMP = u"CharacterCompressionType";
inline constexpr OUStringLiteral SC_UNO_ASIANKERN = u"IsKernAsianPunctuation";
inline constexpr OUStringLiteral SC_UNO_VISAREA = u"VisibleArea";
inline constexpr OUStringLiteral SC_UNO_ZOOMTYPE = u"ZoomType";
inline constexpr OUStringLiteral SC_UNO_ZOOMVALUE = u"ZoomValue";
inline constexpr OUStringLiteral SC_UNO_UPDTEMPL = u"UpdateFromTemplate";
inline constexpr OUStringLiteral SC_UNO_FILTERED_RANGE_SELECTION = u"FilteredRangeSelection";
inline constexpr OUStringLiteral SC_UNO_VISAREASCREEN = u"VisibleAreaOnScreen";

/*Stampit enable/disable print cancel */
inline constexpr OUStringLiteral SC_UNO_ALLOWPRINTJOBCANCEL = u"AllowPrintJobCancel";

//  old (5.2) property names for view options - for compatibility only!
inline constexpr OUStringLiteral OLD_UNO_COLROWHDR = u"ColumnRowHeaders";
inline constexpr OUStringLiteral OLD_UNO_HORSCROLL = u"HorizontalScrollBar";
inline constexpr OUStringLiteral OLD_UNO_OUTLSYMB = u"OutlineSymbols";
inline constexpr OUStringLiteral OLD_UNO_SHEETTABS = u"SheetTabs";
inline constexpr OUStringLiteral OLD_UNO_VALUEHIGH = u"ValueHighlighting";
inline constexpr OUStringLiteral OLD_UNO_VERTSCROLL = u"VerticalScrollBar";

//  data pilot source
inline constexpr OUStringLiteral SC_UNO_DP_COLGRAND = u"ColumnGrand";
inline constexpr OUStringLiteral SC_UNO_DP_ROWGRAND = u"RowGrand";
inline constexpr OUStringLiteral SC_UNO_DP_ORIGINAL = u"Original";
inline constexpr OUStringLiteral SC_UNO_DP_ORIGINAL_POS = u"OriginalPosition";
inline constexpr OUStringLiteral SC_UNO_DP_ISDATALAYOUT = u"IsDataLayoutDimension";
inline constexpr OUStringLiteral SC_UNO_DP_ORIENTATION = u"Orientation";
inline constexpr OUStringLiteral SC_UNO_DP_POSITION = u"Position";
inline constexpr OUStringLiteral SC_UNO_DP_FUNCTION = u"Function";
inline constexpr OUStringLiteral SC_UNO_DP_FUNCTION2 = u"Function2";
inline constexpr OUStringLiteral SC_UNO_DP_USEDHIERARCHY = u"UsedHierarchy";
inline constexpr OUStringLiteral SC_UNO_DP_FILTER = u"Filter";
inline constexpr OUStringLiteral SC_UNO_DP_SUBTOTAL = u"SubTotals";
inline constexpr OUStringLiteral SC_UNO_DP_SUBTOTAL2 = u"SubTotals2";
inline constexpr OUStringLiteral SC_UNO_DP_SHOWEMPTY = u"ShowEmpty";
inline constexpr OUStringLiteral SC_UNO_DP_REPEATITEMLABELS = u"RepeatItemLabels";
inline constexpr OUStringLiteral SC_UNO_DP_ISVISIBLE = u"IsVisible";
inline constexpr OUStringLiteral SC_UNO_DP_SHOWDETAILS = u"ShowDetails";
inline constexpr OUStringLiteral SC_UNO_DP_IGNOREEMPTY = u"IgnoreEmptyRows";
inline constexpr OUStringLiteral SC_UNO_DP_REPEATEMPTY = u"RepeatIfEmpty";
inline constexpr OUStringLiteral SC_UNO_DP_DATADESC = u"DataDescription";
#define SC_UNO_DP_NUMBERFO             SC_UNONAME_NUMFMT
inline constexpr OUStringLiteral SC_UNO_DP_ROWFIELDCOUNT = u"RowFieldCount";
inline constexpr OUStringLiteral SC_UNO_DP_COLUMNFIELDCOUNT = u"ColumnFieldCount";
inline constexpr OUStringLiteral SC_UNO_DP_DATAFIELDCOUNT = u"DataFieldCount";
inline constexpr OUStringLiteral SC_UNO_DP_LAYOUTNAME = u"LayoutName";
inline constexpr OUStringLiteral SC_UNO_DP_FIELD_SUBTOTALNAME = u"FieldSubtotalName";
inline constexpr OUStringLiteral SC_UNO_DP_GRANDTOTAL_NAME = u"GrandTotalName";
inline constexpr OUStringLiteral SC_UNO_DP_HAS_HIDDEN_MEMBER = u"HasHiddenMember";
inline constexpr OUStringLiteral SC_UNO_DP_FLAGS = u"Flags";

inline constexpr OUStringLiteral SC_UNO_DP_REFVALUE = u"ReferenceValue";
inline constexpr OUStringLiteral SC_UNO_DP_SORTING = u"Sorting";
inline constexpr OUStringLiteral SC_UNO_DP_AUTOSHOW = u"AutoShow";
inline constexpr OUStringLiteral SC_UNO_DP_LAYOUT = u"Layout";

// data pilot descriptor
inline constexpr OUStringLiteral SC_UNO_DP_IGNORE_EMPTYROWS = u"IgnoreEmptyRows";
inline constexpr OUStringLiteral SC_UNO_DP_DRILLDOWN = u"DrillDownOnDoubleClick";
inline constexpr OUStringLiteral SC_UNO_DP_SHOWFILTER = u"ShowFilterButton";
inline constexpr OUStringLiteral SC_UNO_DP_IMPORTDESC = u"ImportDescriptor";
inline constexpr OUStringLiteral SC_UNO_DP_SOURCESERVICE = u"SourceServiceName";
inline constexpr OUStringLiteral SC_UNO_DP_SERVICEARG = u"ServiceArguments";

// properties in data pilot descriptor ServiceArguments
inline constexpr OUStringLiteral SC_UNO_DP_SOURCENAME = u"SourceName";
inline constexpr OUStringLiteral SC_UNO_DP_OBJECTNAME = u"ObjectName";
inline constexpr OUStringLiteral SC_UNO_DP_USERNAME = u"UserName";
inline constexpr OUStringLiteral SC_UNO_DP_PASSWORD = u"Password";

//  range selection
#define SC_UNONAME_INITVAL          "InitialValue"
inline constexpr OUStringLiteral SC_UNONAME_TITLE = u"Title";
#define SC_UNONAME_CLOSEONUP        "CloseOnMouseRelease"
#define SC_UNONAME_SINGLECELL       "SingleCellMode"
#define SC_UNONAME_MULTISEL         "MultiSelectionMode"

//  XRenderable
inline constexpr OUStringLiteral SC_UNONAME_PAGESIZE = u"PageSize";
#define SC_UNONAME_RENDERDEV        "RenderDevice"
inline constexpr OUStringLiteral SC_UNONAME_SOURCERANGE = u"SourceRange";
inline constexpr OUStringLiteral SC_UNONAME_INC_NP_AREA = u"PageIncludesNonprintableArea";
inline constexpr OUStringLiteral SC_UNONAME_CALCPAGESIZE = u"CalcPageContentSize";
inline constexpr OUStringLiteral SC_UNONAME_CALCPAGEPOS = u"CalcPagePos";

// CellValueBinding
inline constexpr OUStringLiteral SC_UNONAME_BOUNDCELL = u"BoundCell";
// CellRangeListSource
inline constexpr OUStringLiteral SC_UNONAME_CELLRANGE = u"CellRange";

// CellAddressConversion / CellRangeAddressConversion
inline constexpr OUStringLiteral SC_UNONAME_ADDRESS = u"Address";
inline constexpr OUStringLiteral SC_UNONAME_UIREPR = u"UserInterfaceRepresentation";
inline constexpr OUStringLiteral SC_UNONAME_PERSREPR = u"PersistentRepresentation";
inline constexpr OUStringLiteral SC_UNONAME_XLA1REPR = u"XLA1Representation";
inline constexpr OUStringLiteral SC_UNONAME_REFSHEET = u"ReferenceSheet";

// Security options
inline constexpr OUStringLiteral SC_UNO_LOADREADONLY = u"LoadReadonly";
inline constexpr OUStringLiteral SC_UNO_MODIFYPASSWORDINFO = u"ModifyPasswordInfo";
inline constexpr OUStringLiteral SC_UNO_MODIFYPASSWORDHASH = u"ModifyPasswordHash";

// FormulaParser
inline constexpr OUStringLiteral SC_UNO_COMPILEENGLISH = u"CompileEnglish";
inline constexpr OUStringLiteral SC_UNO_FORMULACONVENTION = u"FormulaConvention";
inline constexpr OUStringLiteral SC_UNO_IGNORELEADING = u"IgnoreLeadingSpaces";
inline constexpr OUStringLiteral SC_UNO_OPCODEMAP = u"OpCodeMap";
#define SC_UNO_EXTERNALLINKS        "ExternalLinks"
inline constexpr OUStringLiteral SC_UNO_COMPILEFAP = u"CompileFAP";
#define SC_UNO_REF_CONV_CHARTOOXML  "RefConventionChartOOXML"

// Chart2
inline constexpr OUStringLiteral SC_UNONAME_ROLE = u"Role";
inline constexpr OUStringLiteral SC_UNONAME_HIDDENVALUES = u"HiddenValues";
inline constexpr OUStringLiteral SC_UNONAME_INCLUDEHIDDENCELLS = u"IncludeHiddenCells";
inline constexpr OUStringLiteral SC_UNONAME_USE_INTERNAL_DATA_PROVIDER = u"UseInternalDataProvider";
#define SC_UNONAME_HAS_STRING_LABEL "HasStringLabel"
#define SC_UNONAME_TIME_BASED       "TimeBased"

// Solver
#define SC_UNONAME_TIMEOUT          "Timeout"
inline constexpr OUStringLiteral SC_UNO_SHAREDOC = u"IsDocumentShared";

// EventDescriptor
#define SC_UNO_EVENTTYPE            "EventType"
#define SC_UNO_SCRIPT               "Script"

// Named ranges
inline constexpr OUStringLiteral SC_UNO_MODIFY_BROADCAST = u"ModifyAndBroadcast";

inline constexpr OUStringLiteral SC_UNO_EMBED_FONTS = u"EmbedFonts";
inline constexpr OUStringLiteral SC_UNO_EMBED_ONLY_USED_FONTS = u"EmbedOnlyUsedFonts";
inline constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_LATIN = u"EmbedLatinScriptFonts";
inline constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_ASIAN = u"EmbedAsianScriptFonts";
inline constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_COMPLEX = u"EmbedComplexScriptFonts";

inline constexpr OUStringLiteral SC_UNO_ODS_LOCK_SOLAR_MUTEX = u"ODSLockSolarMutex";
inline constexpr OUStringLiteral SC_UNO_ODS_IMPORT_STYLES = u"ODSImportStyles";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
