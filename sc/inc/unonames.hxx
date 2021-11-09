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
constexpr OUStringLiteral SC_SERVICENAME_VALBIND = u"com.sun.star.table.CellValueBinding";
constexpr OUStringLiteral SC_SERVICENAME_LISTCELLBIND = u"com.sun.star.table.ListPositionCellBinding";
constexpr OUStringLiteral SC_SERVICENAME_LISTSOURCE = u"com.sun.star.table.CellRangeListSource";
constexpr OUStringLiteral SC_SERVICENAME_CELLADDRESS = u"com.sun.star.table.CellAddressConversion";
constexpr OUStringLiteral SC_SERVICENAME_RANGEADDRESS = u"com.sun.star.table.CellRangeAddressConversion";

constexpr OUStringLiteral SC_SERVICENAME_FORMULAPARS = u"com.sun.star.sheet.FormulaParser";
constexpr OUStringLiteral SC_SERVICENAME_OPCODEMAPPER = u"com.sun.star.sheet.FormulaOpCodeMapper";

constexpr OUStringLiteral SC_SERVICENAME_CHDATAPROV = u"com.sun.star.chart2.data.DataProvider";
constexpr OUStringLiteral SC_SERVICENAME_CHRANGEHILIGHT = u"com.sun.star.chart2.data.RangeHighlightListener";
constexpr OUStringLiteral SC_SERVICENAME_CHART_PIVOTTABLE_DATAPROVIDER = u"com.sun.star.chart2.data.PivotTableDataProvider";

//  document
constexpr OUStringLiteral SC_UNO_AREALINKS = u"AreaLinks";
constexpr OUStringLiteral SC_UNO_DDELINKS = u"DDELinks";
constexpr OUStringLiteral SC_UNO_EXTERNALDOCLINKS = u"ExternalDocLinks";
constexpr OUStringLiteral SC_UNO_COLLABELRNG = u"ColumnLabelRanges";
constexpr OUStringLiteral SC_UNO_DATABASERNG = u"DatabaseRanges";
#define SC_UNO_UNNAMEDDBRNG         "UnnamedDatabaseRanges"
constexpr OUStringLiteral SC_UNO_NAMEDRANGES = u"NamedRanges";
constexpr OUStringLiteral SC_UNO_ROWLABELRNG = u"RowLabelRanges";
constexpr OUStringLiteral SC_UNO_SHEETLINKS = u"SheetLinks";
constexpr OUStringLiteral SC_UNO_FORBIDDEN = u"ForbiddenCharacters";
constexpr OUStringLiteral SC_UNO_HASDRAWPAGES = u"HasDrawPages";

//  CharacterProperties
constexpr OUStringLiteral SC_UNONAME_CCOLOR = u"CharColor";
constexpr OUStringLiteral SC_UNONAME_CHEIGHT = u"CharHeight";
constexpr OUStringLiteral SC_UNONAME_CUNDER = u"CharUnderline";
constexpr OUStringLiteral SC_UNONAME_CUNDLCOL = u"CharUnderlineColor";
constexpr OUStringLiteral SC_UNONAME_CUNDLHAS = u"CharUnderlineHasColor";
constexpr OUStringLiteral SC_UNONAME_COVER = u"CharOverline";
constexpr OUStringLiteral SC_UNONAME_COVRLCOL = u"CharOverlineColor";
constexpr OUStringLiteral SC_UNONAME_COVRLHAS = u"CharOverlineHasColor";
constexpr OUStringLiteral SC_UNONAME_CWEIGHT = u"CharWeight";
constexpr OUStringLiteral SC_UNONAME_CPOST = u"CharPosture";
constexpr OUStringLiteral SC_UNONAME_CCROSS = u"CharCrossedOut";
constexpr OUStringLiteral SC_UNONAME_CSTRIKE = u"CharStrikeout";
constexpr OUStringLiteral SC_UNONAME_CLOCAL = u"CharLocale";
constexpr OUStringLiteral SC_UNONAME_CSHADD = u"CharShadowed";
constexpr OUStringLiteral SC_UNONAME_CFONT = u"CharFont";
constexpr OUStringLiteral SC_UNONAME_COUTL = u"CharContoured";
constexpr OUStringLiteral SC_UNONAME_CEMPHAS = u"CharEmphasis";
constexpr OUStringLiteral SC_UNONAME_CFNAME = u"CharFontName";
constexpr OUStringLiteral SC_UNONAME_CFSTYLE = u"CharFontStyleName";
constexpr OUStringLiteral SC_UNONAME_CFFAMIL = u"CharFontFamily";
constexpr OUStringLiteral SC_UNONAME_CFCHARS = u"CharFontCharSet";
constexpr OUStringLiteral SC_UNONAME_CFPITCH = u"CharFontPitch";
constexpr OUStringLiteral SC_UNONAME_CRELIEF = u"CharRelief";
constexpr OUStringLiteral SC_UNONAME_CWORDMOD = u"CharWordMode";

constexpr OUStringLiteral SC_UNO_CJK_CFNAME = u"CharFontNameAsian";
constexpr OUStringLiteral SC_UNO_CJK_CFSTYLE = u"CharFontStyleNameAsian";
constexpr OUStringLiteral SC_UNO_CJK_CFFAMIL = u"CharFontFamilyAsian";
constexpr OUStringLiteral SC_UNO_CJK_CFCHARS = u"CharFontCharSetAsian";
constexpr OUStringLiteral SC_UNO_CJK_CFPITCH = u"CharFontPitchAsian";
constexpr OUStringLiteral SC_UNO_CJK_CHEIGHT = u"CharHeightAsian";
constexpr OUStringLiteral SC_UNO_CJK_CWEIGHT = u"CharWeightAsian";
constexpr OUStringLiteral SC_UNO_CJK_CPOST = u"CharPostureAsian";
constexpr OUStringLiteral SC_UNO_CJK_CLOCAL = u"CharLocaleAsian";

constexpr OUStringLiteral SC_UNO_CTL_CFNAME = u"CharFontNameComplex";
constexpr OUStringLiteral SC_UNO_CTL_CFSTYLE = u"CharFontStyleNameComplex";
constexpr OUStringLiteral SC_UNO_CTL_CFFAMIL = u"CharFontFamilyComplex";
constexpr OUStringLiteral SC_UNO_CTL_CFCHARS = u"CharFontCharSetComplex";
constexpr OUStringLiteral SC_UNO_CTL_CFPITCH = u"CharFontPitchComplex";
constexpr OUStringLiteral SC_UNO_CTL_CHEIGHT = u"CharHeightComplex";
constexpr OUStringLiteral SC_UNO_CTL_CWEIGHT = u"CharWeightComplex";
constexpr OUStringLiteral SC_UNO_CTL_CPOST = u"CharPostureComplex";
constexpr OUStringLiteral SC_UNO_CTL_CLOCAL = u"CharLocaleComplex";

//  CellProperties
constexpr OUStringLiteral SC_UNONAME_CELLSTYL = u"CellStyle";
constexpr OUStringLiteral SC_UNONAME_CELLBACK = u"CellBackColor";
constexpr OUStringLiteral SC_UNONAME_CELLTRAN = u"IsCellBackgroundTransparent";
constexpr OUStringLiteral SC_UNONAME_CELLPRO = u"CellProtection";
constexpr OUStringLiteral SC_UNONAME_CELLHJUS = u"HoriJustify";
constexpr OUStringLiteral SC_UNONAME_CELLVJUS = u"VertJustify";
constexpr OUStringLiteral SC_UNONAME_CELLHJUS_METHOD = u"HoriJustifyMethod";
constexpr OUStringLiteral SC_UNONAME_CELLVJUS_METHOD = u"VertJustifyMethod";
constexpr OUStringLiteral SC_UNONAME_CELLORI = u"Orientation";
constexpr OUStringLiteral SC_UNONAME_NUMFMT = u"NumberFormat";
constexpr OUStringLiteral SC_UNONAME_FORMATID = u"FormatID";
constexpr OUStringLiteral SC_UNONAME_SHADOW = u"ShadowFormat";
constexpr OUStringLiteral SC_UNONAME_TBLBORD = u"TableBorder";
constexpr OUStringLiteral SC_UNONAME_TBLBORD2 = u"TableBorder2";
constexpr OUStringLiteral SC_UNONAME_WRAP = u"IsTextWrapped";
constexpr OUStringLiteral SC_UNONAME_PINDENT = u"ParaIndent";
constexpr OUStringLiteral SC_UNONAME_PTMARGIN = u"ParaTopMargin";
constexpr OUStringLiteral SC_UNONAME_PBMARGIN = u"ParaBottomMargin";
constexpr OUStringLiteral SC_UNONAME_PLMARGIN = u"ParaLeftMargin";
constexpr OUStringLiteral SC_UNONAME_PRMARGIN = u"ParaRightMargin";
constexpr OUStringLiteral SC_UNONAME_ROTANG = u"RotateAngle";
constexpr OUStringLiteral SC_UNONAME_ROTREF = u"RotateReference";
constexpr OUStringLiteral SC_UNONAME_ASIANVERT = u"AsianVerticalMode";
constexpr OUStringLiteral SC_UNONAME_WRITING = u"WritingMode";
constexpr OUStringLiteral SC_UNONAME_HIDDEN = u"Hidden";

constexpr OUStringLiteral SC_UNONAME_BOTTBORDER = u"BottomBorder";
constexpr OUStringLiteral SC_UNONAME_LEFTBORDER = u"LeftBorder";
constexpr OUStringLiteral SC_UNONAME_RIGHTBORDER = u"RightBorder";
constexpr OUStringLiteral SC_UNONAME_TOPBORDER = u"TopBorder";

constexpr OUStringLiteral SC_UNONAME_BOTTBORDER2 = u"BottomBorder2";
constexpr OUStringLiteral SC_UNONAME_LEFTBORDER2 = u"LeftBorder2";
constexpr OUStringLiteral SC_UNONAME_RIGHTBORDER2 = u"RightBorder2";
constexpr OUStringLiteral SC_UNONAME_TOPBORDER2 = u"TopBorder2";

constexpr OUStringLiteral SC_UNONAME_DIAGONAL_TLBR = u"DiagonalTLBR";
constexpr OUStringLiteral SC_UNONAME_DIAGONAL_BLTR = u"DiagonalBLTR";

constexpr OUStringLiteral SC_UNONAME_DIAGONAL_TLBR2 = u"DiagonalTLBR2";
constexpr OUStringLiteral SC_UNONAME_DIAGONAL_BLTR2 = u"DiagonalBLTR2";

constexpr OUStringLiteral SC_UNONAME_SHRINK_TO_FIT = u"ShrinkToFit";

constexpr OUStringLiteral SC_UNONAME_PISHANG = u"ParaIsHangingPunctuation";
constexpr OUStringLiteral SC_UNONAME_PISCHDIST = u"ParaIsCharacterDistance";
constexpr OUStringLiteral SC_UNONAME_PISFORBID = u"ParaIsForbiddenRules";
constexpr OUStringLiteral SC_UNONAME_PISHYPHEN = u"ParaIsHyphenation";
constexpr OUStringLiteral SC_UNONAME_PADJUST = u"ParaAdjust";
constexpr OUStringLiteral SC_UNONAME_PLASTADJ = u"ParaLastLineAdjust";

constexpr OUStringLiteral SC_UNONAME_NUMRULES = u"NumberingRules";

//  Styles
constexpr OUStringLiteral SC_UNONAME_DISPNAME = u"DisplayName";

//  XStyleLoader
#define SC_UNONAME_OVERWSTL         "OverwriteStyles"
#define SC_UNONAME_LOADCELL         "LoadCellStyles"
#define SC_UNONAME_LOADPAGE         "LoadPageStyles"

//  SheetCellRange
constexpr OUStringLiteral SC_UNONAME_POS = u"Position";
constexpr OUStringLiteral SC_UNONAME_SIZE = u"Size";
constexpr OUStringLiteral SC_UNONAME_ABSNAME = u"AbsoluteName";

//  column/row/sheet
constexpr OUStringLiteral SC_UNONAME_CELLHGT = u"Height";
constexpr OUStringLiteral SC_UNONAME_CELLWID = u"Width";
constexpr OUStringLiteral SC_UNONAME_CELLVIS = u"IsVisible";
constexpr OUStringLiteral SC_UNONAME_CELLFILT = u"IsFiltered";
constexpr OUStringLiteral SC_UNONAME_MANPAGE = u"IsManualPageBreak";
constexpr OUStringLiteral SC_UNONAME_NEWPAGE = u"IsStartOfNewPage";
constexpr OUStringLiteral SC_UNONAME_OHEIGHT = u"OptimalHeight";
constexpr OUStringLiteral SC_UNONAME_OWIDTH = u"OptimalWidth";
constexpr OUStringLiteral SC_UNONAME_PAGESTL = u"PageStyle";
constexpr OUStringLiteral SC_UNONAME_ISACTIVE = u"IsActive";
constexpr OUStringLiteral SC_UNONAME_BORDCOL = u"BorderColor";
constexpr OUStringLiteral SC_UNONAME_PROTECT = u"Protected";
constexpr OUStringLiteral SC_UNONAME_SHOWBORD = u"ShowBorder";
constexpr OUStringLiteral SC_UNONAME_PRINTBORD = u"PrintBorder";
constexpr OUStringLiteral SC_UNONAME_COPYBACK = u"CopyBack";
constexpr OUStringLiteral SC_UNONAME_COPYSTYL = u"CopyStyles";
constexpr OUStringLiteral SC_UNONAME_COPYFORM = u"CopyFormulas";
constexpr OUStringLiteral SC_UNONAME_TABLAYOUT = u"TableLayout";
constexpr OUStringLiteral SC_UNONAME_AUTOPRINT = u"AutomaticPrintArea";
constexpr OUStringLiteral SC_UNONAME_TABCOLOR = u"TabColor";
constexpr OUStringLiteral SC_UNONAME_CONDFORMAT = u"ConditionalFormats";

#define SC_UNONAME_VISFLAG          "VisibleFlag"

//  LinkTarget
constexpr OUStringLiteral SC_UNO_LINKDISPBIT = u"LinkDisplayBitmap";
constexpr OUStringLiteral SC_UNO_LINKDISPNAME = u"LinkDisplayName";

//  drawing objects
constexpr OUStringLiteral SC_UNONAME_IMAGEMAP = u"ImageMap";
constexpr OUStringLiteral SC_UNONAME_ANCHOR = u"Anchor";
constexpr OUStringLiteral SC_UNONAME_RESIZE_WITH_CELL = u"ResizeWithCell";
constexpr OUStringLiteral SC_UNONAME_HORIPOS = u"HoriOrientPosition";
constexpr OUStringLiteral SC_UNONAME_VERTPOS = u"VertOrientPosition";
constexpr OUStringLiteral SC_UNONAME_HYPERLINK = u"Hyperlink";
constexpr OUStringLiteral SC_UNONAME_MOVEPROTECT = u"MoveProtect";

//  other cell properties
constexpr OUStringLiteral SC_UNONAME_CHCOLHDR = u"ChartColumnAsLabel";
constexpr OUStringLiteral SC_UNONAME_CHROWHDR = u"ChartRowAsLabel";
constexpr OUStringLiteral SC_UNONAME_CONDFMT = u"ConditionalFormat";
constexpr OUStringLiteral SC_UNONAME_CONDLOC = u"ConditionalFormatLocal";
constexpr OUStringLiteral SC_UNONAME_CONDXML = u"ConditionalFormatXML";
constexpr OUStringLiteral SC_UNONAME_VALIDAT = u"Validation";
constexpr OUStringLiteral SC_UNONAME_VALILOC = u"ValidationLocal";
constexpr OUStringLiteral SC_UNONAME_VALIXML = u"ValidationXML";
constexpr OUStringLiteral SC_UNONAME_FORMLOC = u"FormulaLocal";
constexpr OUStringLiteral SC_UNONAME_FORMRT = u"FormulaResultType";
constexpr OUStringLiteral SC_UNONAME_FORMRT2 = u"FormulaResultType2";
constexpr OUStringLiteral SC_UNONAME_CELLCONTENTTYPE = u"CellContentType";

constexpr OUStringLiteral SC_UNONAME_USERDEF = u"UserDefinedAttributes";
constexpr OUStringLiteral SC_UNONAME_TEXTUSER = u"TextUserDefinedAttributes";

//  auto format
constexpr OUStringLiteral SC_UNONAME_INCBACK = u"IncludeBackground";
constexpr OUStringLiteral SC_UNONAME_INCBORD = u"IncludeBorder";
constexpr OUStringLiteral SC_UNONAME_INCFONT = u"IncludeFont";
constexpr OUStringLiteral SC_UNONAME_INCJUST = u"IncludeJustify";
constexpr OUStringLiteral SC_UNONAME_INCNUM = u"IncludeNumberFormat";
constexpr OUStringLiteral SC_UNONAME_INCWIDTH = u"IncludeWidthAndHeight";

//  function description
#define SC_UNONAME_ARGUMENTS        "Arguments"
#define SC_UNONAME_CATEGORY         "Category"
#define SC_UNONAME_DESCRIPTION      "Description"
#define SC_UNONAME_ID               "Id"
#define SC_UNONAME_NAME             "Name"

//  application settings
constexpr OUStringLiteral SC_UNONAME_DOAUTOCP = u"DoAutoComplete";
constexpr OUStringLiteral SC_UNONAME_ENTERED = u"EnterEdit";
constexpr OUStringLiteral SC_UNONAME_EXPREF = u"ExpandReferences";
constexpr OUStringLiteral SC_UNONAME_EXTFMT = u"ExtendFormat";
constexpr OUStringLiteral SC_UNONAME_LINKUPD = u"LinkUpdateMode";
constexpr OUStringLiteral SC_UNONAME_MARKHDR = u"MarkHeader";
constexpr OUStringLiteral SC_UNONAME_METRIC = u"Metric";
constexpr OUStringLiteral SC_UNONAME_MOVEDIR = u"MoveDirection";
constexpr OUStringLiteral SC_UNONAME_MOVESEL = u"MoveSelection";
constexpr OUStringLiteral SC_UNONAME_RANGEFIN = u"RangeFinder";
constexpr OUStringLiteral SC_UNONAME_SCALE = u"Scale";
constexpr OUStringLiteral SC_UNONAME_STBFUNC = u"StatusBarFunction";
constexpr OUStringLiteral SC_UNONAME_ULISTS = u"UserLists";
constexpr OUStringLiteral SC_UNONAME_USETABCOL = u"UseTabCol";
constexpr OUStringLiteral SC_UNONAME_PRMETRICS = u"UsePrinterMetrics";
constexpr OUStringLiteral SC_UNONAME_PRALLSH = u"PrintAllSheets";
constexpr OUStringLiteral SC_UNONAME_PREMPTY = u"PrintEmptyPages";
constexpr OUStringLiteral SC_UNONAME_REPLWARN = u"ReplaceCellsWarning";

//  data pilot field
constexpr OUStringLiteral SC_UNONAME_FUNCTION = u"Function";
constexpr OUStringLiteral SC_UNONAME_FUNCTION2 = u"Function2";
constexpr OUStringLiteral SC_UNONAME_SUBTOTALS = u"Subtotals";
constexpr OUStringLiteral SC_UNONAME_SUBTOTALS2 = u"Subtotals2";
constexpr OUStringLiteral SC_UNONAME_SELPAGE = u"SelectedPage";
constexpr OUStringLiteral SC_UNONAME_USESELPAGE = u"UseSelectedPage";
constexpr OUStringLiteral SC_UNONAME_HASREFERENCE = u"HasReference";
constexpr OUStringLiteral SC_UNONAME_REFERENCE = u"Reference";
constexpr OUStringLiteral SC_UNONAME_HASAUTOSHOW = u"HasAutoShowInfo";
constexpr OUStringLiteral SC_UNONAME_AUTOSHOW = u"AutoShowInfo";
constexpr OUStringLiteral SC_UNONAME_HASSORTINFO = u"HasSortInfo";
constexpr OUStringLiteral SC_UNONAME_SORTINFO = u"SortInfo";
constexpr OUStringLiteral SC_UNONAME_HASLAYOUTINFO = u"HasLayoutInfo";
constexpr OUStringLiteral SC_UNONAME_LAYOUTINFO = u"LayoutInfo";
constexpr OUStringLiteral SC_UNONAME_ISGROUP = u"IsGroupField";
constexpr OUStringLiteral SC_UNONAME_GROUPINFO = u"GroupInfo";
constexpr OUStringLiteral SC_UNONAME_SHOWEMPTY = u"ShowEmpty";
constexpr OUStringLiteral SC_UNONAME_REPEATITEMLABELS = u"RepeatItemLabels";

//  data pilot item
constexpr OUStringLiteral SC_UNONAME_SHOWDETAIL = u"ShowDetail";
constexpr OUStringLiteral SC_UNONAME_ISHIDDEN = u"IsHidden";

//  database options
constexpr OUStringLiteral SC_UNONAME_CASE = u"CaseSensitive";
#define SC_UNONAME_DBNAME           "DatabaseName"
constexpr OUStringLiteral SC_UNONAME_FORMATS = u"IncludeFormats";
constexpr OUStringLiteral SC_UNONAME_INSBRK = u"InsertPageBreaks";
constexpr OUStringLiteral SC_UNONAME_KEEPFORM = u"KeepFormats";
constexpr OUStringLiteral SC_UNONAME_MOVCELLS = u"MoveCells";
constexpr OUStringLiteral SC_UNONAME_ISUSER = u"IsUserDefined";
#define SC_UNONAME_ISNATIVE         "IsNative"
#define SC_UNONAME_REGEXP           "RegularExpressions"
#define SC_UNONAME_WILDCARDS        "Wildcards"
constexpr OUStringLiteral SC_UNONAME_SAVEOUT = u"SaveOutputPosition";
constexpr OUStringLiteral SC_UNONAME_SKIPDUP = u"SkipDuplicates";
#define SC_UNONAME_SRCOBJ           "SourceObject"
#define SC_UNONAME_SRCTYPE          "SourceType"
constexpr OUStringLiteral SC_UNONAME_STRIPDAT = u"StripData";
constexpr OUStringLiteral SC_UNONAME_USEREGEX = u"UseRegularExpressions";
constexpr OUStringLiteral SC_UNONAME_ULIST = u"UserListEnabled";
constexpr OUStringLiteral SC_UNONAME_UINDEX = u"UserListIndex";
constexpr OUStringLiteral SC_UNONAME_BINDFMT = u"BindFormatsToContent";
constexpr OUStringLiteral SC_UNONAME_COPYOUT = u"CopyOutputData";
constexpr OUStringLiteral SC_UNONAME_ISCASE = u"IsCaseSensitive";
#define SC_UNONAME_ISULIST          "IsUserListEnabled"
constexpr OUStringLiteral SC_UNONAME_OUTPOS = u"OutputPosition";
constexpr OUStringLiteral SC_UNONAME_CONTHDR = u"ContainsHeader";
constexpr OUStringLiteral SC_UNONAME_MAXFLD = u"MaxFieldCount";
constexpr OUStringLiteral SC_UNONAME_ORIENT = u"Orientation";
#define SC_UNONAME_ISSORTCOLUMNS    "IsSortColumns"
#define SC_UNONAME_SORTFLD          "SortFields"
constexpr OUStringLiteral SC_UNONAME_SORTASC = u"SortAscending";
constexpr OUStringLiteral SC_UNONAME_ENUSLIST = u"EnableUserSortList";
constexpr OUStringLiteral SC_UNONAME_USINDEX = u"UserSortListIndex";
#define SC_UNONAME_COLLLOC          "CollatorLocale"
#define SC_UNONAME_COLLALG          "CollatorAlgorithm"
constexpr OUStringLiteral SC_UNONAME_AUTOFLT = u"AutoFilter";
constexpr OUStringLiteral SC_UNONAME_FLTCRT = u"FilterCriteriaSource";
constexpr OUStringLiteral SC_UNONAME_USEFLTCRT = u"UseFilterCriteriaSource";
constexpr OUStringLiteral SC_UNONAME_ENABSORT = u"EnableSort";
constexpr OUStringLiteral SC_UNONAME_FROMSELECT = u"FromSelection";
#define SC_UNONAME_CONRES           "ConnectionResource"
constexpr OUStringLiteral SC_UNONAME_TOKENINDEX = u"TokenIndex";
constexpr OUStringLiteral SC_UNONAME_ISSHAREDFMLA = u"IsSharedFormula";
constexpr OUStringLiteral SC_UNONAME_TOTALSROW = u"TotalsRow";

//  text fields
constexpr OUStringLiteral SC_UNONAME_ANCTYPE = u"AnchorType";
constexpr OUStringLiteral SC_UNONAME_ANCTYPES = u"AnchorTypes";
constexpr OUStringLiteral SC_UNONAME_TEXTWRAP = u"TextWrap";
constexpr OUStringLiteral SC_UNONAME_FILEFORM = u"FileFormat";
#define SC_UNONAME_TEXTFIELD_TYPE   "TextFieldType"

//  url field
constexpr OUStringLiteral SC_UNONAME_REPR = u"Representation";
constexpr OUStringLiteral SC_UNONAME_TARGET = u"TargetFrame";
constexpr OUStringLiteral SC_UNONAME_URL = u"URL";

// date time field
constexpr OUStringLiteral SC_UNONAME_ISDATE = u"IsDate";
constexpr OUStringLiteral SC_UNONAME_ISFIXED = u"IsFixed";
constexpr OUStringLiteral SC_UNONAME_DATETIME = u"DateTime";

// table field
constexpr OUStringLiteral SC_UNONAME_TABLEPOS = u"TablePosition";

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
constexpr OUStringLiteral SC_UNONAME_ERRALSTY = u"ErrorAlertStyle";
constexpr OUStringLiteral SC_UNONAME_ERRMESS = u"ErrorMessage";
constexpr OUStringLiteral SC_UNONAME_ERRTITLE = u"ErrorTitle";
constexpr OUStringLiteral SC_UNONAME_IGNOREBL = u"IgnoreBlankCells";
constexpr OUStringLiteral SC_UNONAME_INPMESS = u"InputMessage";
constexpr OUStringLiteral SC_UNONAME_INPTITLE = u"InputTitle";
constexpr OUStringLiteral SC_UNONAME_SHOWERR = u"ShowErrorMessage";
constexpr OUStringLiteral SC_UNONAME_SHOWINP = u"ShowInputMessage";
constexpr OUStringLiteral SC_UNONAME_SHOWLIST = u"ShowList";
constexpr OUStringLiteral SC_UNONAME_TYPE = u"Type";

//  links
constexpr OUStringLiteral SC_UNONAME_FILTER = u"Filter";
constexpr OUStringLiteral SC_UNONAME_FILTOPT = u"FilterOptions";
constexpr OUStringLiteral SC_UNONAME_LINKURL = u"Url";
constexpr OUStringLiteral SC_UNONAME_REFPERIOD = u"RefreshPeriod";
constexpr OUStringLiteral SC_UNONAME_REFDELAY = u"RefreshDelay"; // deprecated, don't use anymore

//  search descriptor
constexpr OUStringLiteral SC_UNO_SRCHBACK = u"SearchBackwards";
constexpr OUStringLiteral SC_UNO_SRCHBYROW = u"SearchByRow";
constexpr OUStringLiteral SC_UNO_SRCHCASE = u"SearchCaseSensitive";
constexpr OUStringLiteral SC_UNO_SRCHREGEXP = u"SearchRegularExpression";
constexpr OUStringLiteral SC_UNO_SRCHWILDCARD = u"SearchWildcard";
constexpr OUStringLiteral SC_UNO_SRCHSIM = u"SearchSimilarity";
constexpr OUStringLiteral SC_UNO_SRCHSIMADD = u"SearchSimilarityAdd";
constexpr OUStringLiteral SC_UNO_SRCHSIMEX = u"SearchSimilarityExchange";
constexpr OUStringLiteral SC_UNO_SRCHSIMREL = u"SearchSimilarityRelax";
constexpr OUStringLiteral SC_UNO_SRCHSIMREM = u"SearchSimilarityRemove";
constexpr OUStringLiteral SC_UNO_SRCHSTYLES = u"SearchStyles";
constexpr OUStringLiteral SC_UNO_SRCHTYPE = u"SearchType";
constexpr OUStringLiteral SC_UNO_SRCHWORDS = u"SearchWords";
#define SC_UNO_SRCHFILTERED         "SearchFiltered"
#define SC_UNO_SRCHFORMATTED        "SearchFormatted"

//  old (5.2) property names for page styles - for compatibility only!
#define OLD_UNO_PAGE_BACKCOLOR      "BackgroundColor"
#define OLD_UNO_PAGE_BACKTRANS      "IsBackgroundTransparent"
#define OLD_UNO_PAGE_HDRBACKCOL     "HeaderBackgroundColor"
#define OLD_UNO_PAGE_HDRBACKTRAN    "TransparentHeaderBackground"
#define OLD_UNO_PAGE_HDRSHARED      "HeaderShared"
#define OLD_UNO_PAGE_HDRDYNAMIC     "HeaderDynamic"
#define OLD_UNO_PAGE_HDRON          "HeaderOn"
#define OLD_UNO_PAGE_FTRBACKCOL     "FooterBackgroundColor"
#define OLD_UNO_PAGE_FTRBACKTRAN    "TransparentFooterBackground"
#define OLD_UNO_PAGE_FTRSHARED      "FooterShared"
#define OLD_UNO_PAGE_FTRDYNAMIC     "FooterDynamic"
#define OLD_UNO_PAGE_FTRON          "FooterOn"

//  page styles
constexpr OUStringLiteral SC_UNO_PAGE_BACKCOLOR = u"BackColor";
constexpr OUStringLiteral SC_UNO_PAGE_BACKTRANS = u"BackTransparent";
constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICFILT = u"BackGraphicFilter";
constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICLOC = u"BackGraphicLocation";
constexpr OUStringLiteral SC_UNO_PAGE_GRAPHICURL = u"BackGraphicURL";
constexpr OUStringLiteral SC_UNO_PAGE_GRAPHIC = u"BackGraphic";
#define SC_UNO_PAGE_LEFTBORDER      SC_UNONAME_LEFTBORDER
#define SC_UNO_PAGE_RIGHTBORDER     SC_UNONAME_RIGHTBORDER
#define SC_UNO_PAGE_BOTTBORDER      SC_UNONAME_BOTTBORDER
#define SC_UNO_PAGE_TOPBORDER       SC_UNONAME_TOPBORDER
constexpr OUStringLiteral SC_UNO_PAGE_LEFTBRDDIST = u"LeftBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_RIGHTBRDDIST = u"RightBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_BOTTBRDDIST = u"BottomBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_TOPBRDDIST = u"TopBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_BORDERDIST = u"BorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_SHADOWFORM = u"ShadowFormat";
constexpr OUStringLiteral SC_UNO_PAGE_LEFTMARGIN = u"LeftMargin";
constexpr OUStringLiteral SC_UNO_PAGE_RIGHTMARGIN = u"RightMargin";
constexpr OUStringLiteral SC_UNO_PAGE_TOPMARGIN = u"TopMargin";
constexpr OUStringLiteral SC_UNO_PAGE_BOTTMARGIN = u"BottomMargin";
constexpr OUStringLiteral SC_UNO_PAGE_LANDSCAPE = u"IsLandscape";
constexpr OUStringLiteral SC_UNO_PAGE_NUMBERTYPE = u"NumberingType";
constexpr OUStringLiteral SC_UNO_PAGE_SYTLELAYOUT = u"PageStyleLayout";
constexpr OUStringLiteral SC_UNO_PAGE_PAPERTRAY = u"PrinterPaperTray";
constexpr OUStringLiteral SC_UNO_PAGE_SIZE = u"Size";
constexpr OUStringLiteral SC_UNO_PAGE_WIDTH = u"Width";
constexpr OUStringLiteral SC_UNO_PAGE_HEIGHT = u"Height";
constexpr OUStringLiteral SC_UNO_PAGE_CENTERHOR = u"CenterHorizontally";
constexpr OUStringLiteral SC_UNO_PAGE_CENTERVER = u"CenterVertically";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTANNOT = u"PrintAnnotations";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTGRID = u"PrintGrid";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTHEADER = u"PrintHeaders";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTCHARTS = u"PrintCharts";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTOBJS = u"PrintObjects";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTDRAW = u"PrintDrawing";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTDOWN = u"PrintDownFirst";
constexpr OUStringLiteral SC_UNO_PAGE_SCALEVAL = u"PageScale";
constexpr OUStringLiteral SC_UNO_PAGE_SCALETOPAG = u"ScaleToPages";
constexpr OUStringLiteral SC_UNO_PAGE_SCALETOX = u"ScaleToPagesX";
constexpr OUStringLiteral SC_UNO_PAGE_SCALETOY = u"ScaleToPagesY";
constexpr OUStringLiteral SC_UNO_PAGE_FIRSTPAGE = u"FirstPageNumber";
constexpr OUStringLiteral SC_UNO_PAGE_FIRSTHDRSHARED = u"FirstPageHeaderIsShared";
constexpr OUStringLiteral SC_UNO_PAGE_FIRSTFTRSHARED = u"FirstPageFooterIsShared";
constexpr OUStringLiteral SC_UNO_PAGE_LEFTHDRCONT = u"LeftPageHeaderContent";
constexpr OUStringLiteral SC_UNO_PAGE_LEFTFTRCONT = u"LeftPageFooterContent";
constexpr OUStringLiteral SC_UNO_PAGE_RIGHTHDRCON = u"RightPageHeaderContent";
constexpr OUStringLiteral SC_UNO_PAGE_RIGHTFTRCON = u"RightPageFooterContent";
constexpr OUStringLiteral SC_UNO_PAGE_FIRSTHDRCONT = u"FirstPageHeaderContent";
constexpr OUStringLiteral SC_UNO_PAGE_FIRSTFTRCONT = u"FirstPageFooterContent";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTFORMUL = u"PrintFormulas";
constexpr OUStringLiteral SC_UNO_PAGE_PRINTZERO = u"PrintZeroValues";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBACKCOL = u"HeaderBackColor";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBACKTRAN = u"HeaderBackTransparent";
constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFFILT = u"HeaderBackGraphicFilter";
constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFLOC = u"HeaderBackGraphicLocation";
constexpr OUStringLiteral SC_UNO_PAGE_HDRGRF = u"HeaderBackGraphic";
constexpr OUStringLiteral SC_UNO_PAGE_HDRGRFURL = u"HeaderBackGraphicURL";
constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTBOR = u"HeaderLeftBorder";
constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTBOR = u"HeaderRightBorder";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBOTTBOR = u"HeaderBottomBorder";
constexpr OUStringLiteral SC_UNO_PAGE_HDRTOPBOR = u"HeaderTopBorder";
constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTBDIS = u"HeaderLeftBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTBDIS = u"HeaderRightBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBOTTBDIS = u"HeaderBottomBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRTOPBDIS = u"HeaderTopBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBRDDIST = u"HeaderBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRSHADOW = u"HeaderShadowFormat";
constexpr OUStringLiteral SC_UNO_PAGE_HDRLEFTMAR = u"HeaderLeftMargin";
constexpr OUStringLiteral SC_UNO_PAGE_HDRRIGHTMAR = u"HeaderRightMargin";
constexpr OUStringLiteral SC_UNO_PAGE_HDRBODYDIST = u"HeaderBodyDistance";
constexpr OUStringLiteral SC_UNO_PAGE_HDRHEIGHT = u"HeaderHeight";
constexpr OUStringLiteral SC_UNO_PAGE_HDRON = u"HeaderIsOn";
constexpr OUStringLiteral SC_UNO_PAGE_HDRDYNAMIC = u"HeaderIsDynamicHeight";
constexpr OUStringLiteral SC_UNO_PAGE_HDRSHARED = u"HeaderIsShared";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBACKCOL = u"FooterBackColor";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBACKTRAN = u"FooterBackTransparent";
constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFFILT = u"FooterBackGraphicFilter";
constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFLOC = u"FooterBackGraphicLocation";
constexpr OUStringLiteral SC_UNO_PAGE_FTRGRF = u"FooterBackGraphic";
constexpr OUStringLiteral SC_UNO_PAGE_FTRGRFURL = u"FooterBackGraphicURL";
constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTBOR = u"FooterLeftBorder";
constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTBOR = u"FooterRightBorder";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBOTTBOR = u"FooterBottomBorder";
constexpr OUStringLiteral SC_UNO_PAGE_FTRTOPBOR = u"FooterTopBorder";
constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTBDIS = u"FooterLeftBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTBDIS = u"FooterRightBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBOTTBDIS = u"FooterBottomBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRTOPBDIS = u"FooterTopBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBRDDIST = u"FooterBorderDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRSHADOW = u"FooterShadowFormat";
constexpr OUStringLiteral SC_UNO_PAGE_FTRLEFTMAR = u"FooterLeftMargin";
constexpr OUStringLiteral SC_UNO_PAGE_FTRRIGHTMAR = u"FooterRightMargin";
constexpr OUStringLiteral SC_UNO_PAGE_FTRBODYDIST = u"FooterBodyDistance";
constexpr OUStringLiteral SC_UNO_PAGE_FTRHEIGHT = u"FooterHeight";
constexpr OUStringLiteral SC_UNO_PAGE_FTRON = u"FooterIsOn";
constexpr OUStringLiteral SC_UNO_PAGE_FTRDYNAMIC = u"FooterIsDynamicHeight";
constexpr OUStringLiteral SC_UNO_PAGE_FTRSHARED = u"FooterIsShared";

//  document settings
constexpr OUStringLiteral SC_UNO_CALCASSHOWN = u"CalcAsShown";
constexpr OUStringLiteral SC_UNO_DEFTABSTOP = u"DefaultTabStop";
constexpr OUStringLiteral SC_UNO_TABSTOPDIS = u"TabStopDistance"; // is the same like the before, but only the writer name
constexpr OUStringLiteral SC_UNO_IGNORECASE = u"IgnoreCase";
constexpr OUStringLiteral SC_UNO_ITERCOUNT = u"IterationCount";
constexpr OUStringLiteral SC_UNO_ITERENABLED = u"IsIterationEnabled";
constexpr OUStringLiteral SC_UNO_ITEREPSILON = u"IterationEpsilon";
constexpr OUStringLiteral SC_UNO_LOOKUPLABELS = u"LookUpLabels";
constexpr OUStringLiteral SC_UNO_MATCHWHOLE = u"MatchWholeCell";
constexpr OUStringLiteral SC_UNO_NULLDATE = u"NullDate";
constexpr OUStringLiteral SC_UNO_SPELLONLINE = u"SpellOnline";
constexpr OUStringLiteral SC_UNO_STANDARDDEC = u"StandardDecimals";
constexpr OUStringLiteral SC_UNO_REGEXENABLED = u"RegularExpressions";
constexpr OUStringLiteral SC_UNO_WILDCARDSENABLED = u"Wildcards";
constexpr OUStringLiteral SC_UNO_BASICLIBRARIES = u"BasicLibraries";
constexpr OUStringLiteral SC_UNO_DIALOGLIBRARIES = u"DialogLibraries";
constexpr OUStringLiteral SC_UNO_RUNTIMEUID = u"RuntimeUID";
constexpr OUStringLiteral SC_UNO_HASVALIDSIGNATURES = u"HasValidSignatures";
constexpr OUStringLiteral SC_UNO_ISLOADED = u"IsLoaded";
constexpr OUStringLiteral SC_UNO_ISUNDOENABLED = u"IsUndoEnabled";
constexpr OUStringLiteral SC_UNO_ISADJUSTHEIGHTENABLED = u"IsAdjustHeightEnabled";
constexpr OUStringLiteral SC_UNO_ISEXECUTELINKENABLED = u"IsExecuteLinkEnabled";
constexpr OUStringLiteral SC_UNO_ISCHANGEREADONLYENABLED = u"IsChangeReadOnlyEnabled";
constexpr OUStringLiteral SC_UNO_REFERENCEDEVICE = u"ReferenceDevice";
constexpr OUStringLiteral SC_UNO_CODENAME = u"CodeName";
constexpr OUStringLiteral SC_UNO_INTEROPGRABBAG = u"InteropGrabBag";
constexpr OUStringLiteral SC_UNO_RECORDCHANGES = u"RecordChanges";
constexpr OUStringLiteral SC_UNO_ISRECORDCHANGESPROTECTED = u"IsRecordChangesProtected";
constexpr OUStringLiteral SC_UNO_SYNTAXSTRINGREF = u"SyntaxStringRef";


//  document properties from FormModel
constexpr OUStringLiteral SC_UNO_APPLYFMDES = u"ApplyFormDesignMode";
constexpr OUStringLiteral SC_UNO_AUTOCONTFOC = u"AutomaticControlFocus";

//  view options
constexpr OUStringLiteral SC_UNO_COLROWHDR = u"HasColumnRowHeaders";
constexpr OUStringLiteral SC_UNO_GRIDCOLOR = u"GridColor";
constexpr OUStringLiteral SC_UNO_HIDESPELL = u"HideSpellMarks";                /* deprecated #i91949 */
constexpr OUStringLiteral SC_UNO_HORSCROLL = u"HasHorizontalScrollBar";
constexpr OUStringLiteral SC_UNO_OUTLSYMB = u"IsOutlineSymbolsSet";
constexpr OUStringLiteral SC_UNO_SHEETTABS = u"HasSheetTabs";
constexpr OUStringLiteral SC_UNO_SHOWANCHOR = u"ShowAnchor";
constexpr OUStringLiteral SC_UNO_SHOWCHARTS = u"ShowCharts";
constexpr OUStringLiteral SC_UNO_SHOWDRAW = u"ShowDrawing";
constexpr OUStringLiteral SC_UNO_SHOWFORM = u"ShowFormulas";
constexpr OUStringLiteral SC_UNO_SHOWGRID = u"ShowGrid";
constexpr OUStringLiteral SC_UNO_SHOWHELP = u"ShowHelpLines";
constexpr OUStringLiteral SC_UNO_SHOWNOTES = u"ShowNotes";
constexpr OUStringLiteral SC_UNO_SHOWOBJ = u"ShowObjects";
constexpr OUStringLiteral SC_UNO_SHOWPAGEBR = u"ShowPageBreaks";
constexpr OUStringLiteral SC_UNO_SHOWZERO = u"ShowZeroValues";
constexpr OUStringLiteral SC_UNO_VALUEHIGH = u"IsValueHighlightingEnabled";
constexpr OUStringLiteral SC_UNO_VERTSCROLL = u"HasVerticalScrollBar";
constexpr OUStringLiteral SC_UNO_SNAPTORASTER = u"IsSnapToRaster";
constexpr OUStringLiteral SC_UNO_RASTERVIS = u"RasterIsVisible";
constexpr OUStringLiteral SC_UNO_RASTERRESX = u"RasterResolutionX";
constexpr OUStringLiteral SC_UNO_RASTERRESY = u"RasterResolutionY";
constexpr OUStringLiteral SC_UNO_RASTERSUBX = u"RasterSubdivisionX";
constexpr OUStringLiteral SC_UNO_RASTERSUBY = u"RasterSubdivisionY";
constexpr OUStringLiteral SC_UNO_RASTERSYNC = u"IsRasterAxisSynchronized";
constexpr OUStringLiteral SC_UNO_AUTOCALC = u"AutoCalculate";
constexpr OUStringLiteral SC_UNO_PRINTERNAME = u"PrinterName";
constexpr OUStringLiteral SC_UNO_PRINTERSETUP = u"PrinterSetup";
constexpr OUStringLiteral SC_UNO_PRINTERPAPER = u"PrinterPaperFromSetup";
constexpr OUStringLiteral SC_UNO_APPLYDOCINF = u"ApplyUserData";
constexpr OUStringLiteral SC_UNO_SAVE_THUMBNAIL = u"SaveThumbnail";
constexpr OUStringLiteral SC_UNO_CHARCOMP = u"CharacterCompressionType";
constexpr OUStringLiteral SC_UNO_ASIANKERN = u"IsKernAsianPunctuation";
constexpr OUStringLiteral SC_UNO_VISAREA = u"VisibleArea";
constexpr OUStringLiteral SC_UNO_ZOOMTYPE = u"ZoomType";
constexpr OUStringLiteral SC_UNO_ZOOMVALUE = u"ZoomValue";
constexpr OUStringLiteral SC_UNO_UPDTEMPL = u"UpdateFromTemplate";
constexpr OUStringLiteral SC_UNO_FILTERED_RANGE_SELECTION = u"FilteredRangeSelection";
constexpr OUStringLiteral SC_UNO_VISAREASCREEN = u"VisibleAreaOnScreen";

/*Stampit enable/disable print cancel */
constexpr OUStringLiteral SC_UNO_ALLOWPRINTJOBCANCEL = u"AllowPrintJobCancel";

//  old (5.2) property names for view options - for compatibility only!
constexpr OUStringLiteral OLD_UNO_COLROWHDR = u"ColumnRowHeaders";
constexpr OUStringLiteral OLD_UNO_HORSCROLL = u"HorizontalScrollBar";
constexpr OUStringLiteral OLD_UNO_OUTLSYMB = u"OutlineSymbols";
constexpr OUStringLiteral OLD_UNO_SHEETTABS = u"SheetTabs";
constexpr OUStringLiteral OLD_UNO_VALUEHIGH = u"ValueHighlighting";
constexpr OUStringLiteral OLD_UNO_VERTSCROLL = u"VerticalScrollBar";

//  data pilot source
constexpr OUStringLiteral SC_UNO_DP_COLGRAND = u"ColumnGrand";
constexpr OUStringLiteral SC_UNO_DP_ROWGRAND = u"RowGrand";
constexpr OUStringLiteral SC_UNO_DP_ORIGINAL = u"Original";
constexpr OUStringLiteral SC_UNO_DP_ORIGINAL_POS = u"OriginalPosition";
constexpr OUStringLiteral SC_UNO_DP_ISDATALAYOUT = u"IsDataLayoutDimension";
constexpr OUStringLiteral SC_UNO_DP_ORIENTATION = u"Orientation";
constexpr OUStringLiteral SC_UNO_DP_POSITION = u"Position";
constexpr OUStringLiteral SC_UNO_DP_FUNCTION = u"Function";
constexpr OUStringLiteral SC_UNO_DP_FUNCTION2 = u"Function2";
constexpr OUStringLiteral SC_UNO_DP_USEDHIERARCHY = u"UsedHierarchy";
constexpr OUStringLiteral SC_UNO_DP_FILTER = u"Filter";
constexpr OUStringLiteral SC_UNO_DP_SUBTOTAL = u"SubTotals";
constexpr OUStringLiteral SC_UNO_DP_SUBTOTAL2 = u"SubTotals2";
constexpr OUStringLiteral SC_UNO_DP_SHOWEMPTY = u"ShowEmpty";
constexpr OUStringLiteral SC_UNO_DP_REPEATITEMLABELS = u"RepeatItemLabels";
constexpr OUStringLiteral SC_UNO_DP_ISVISIBLE = u"IsVisible";
constexpr OUStringLiteral SC_UNO_DP_SHOWDETAILS = u"ShowDetails";
constexpr OUStringLiteral SC_UNO_DP_IGNOREEMPTY = u"IgnoreEmptyRows";
constexpr OUStringLiteral SC_UNO_DP_REPEATEMPTY = u"RepeatIfEmpty";
constexpr OUStringLiteral SC_UNO_DP_DATADESC = u"DataDescription";
#define SC_UNO_DP_NUMBERFO             SC_UNONAME_NUMFMT
constexpr OUStringLiteral SC_UNO_DP_ROWFIELDCOUNT = u"RowFieldCount";
constexpr OUStringLiteral SC_UNO_DP_COLUMNFIELDCOUNT = u"ColumnFieldCount";
constexpr OUStringLiteral SC_UNO_DP_DATAFIELDCOUNT = u"DataFieldCount";
constexpr OUStringLiteral SC_UNO_DP_LAYOUTNAME = u"LayoutName";
constexpr OUStringLiteral SC_UNO_DP_FIELD_SUBTOTALNAME = u"FieldSubtotalName";
constexpr OUStringLiteral SC_UNO_DP_GRANDTOTAL_NAME = u"GrandTotalName";
constexpr OUStringLiteral SC_UNO_DP_HAS_HIDDEN_MEMBER = u"HasHiddenMember";
constexpr OUStringLiteral SC_UNO_DP_FLAGS = u"Flags";

constexpr OUStringLiteral SC_UNO_DP_REFVALUE = u"ReferenceValue";
constexpr OUStringLiteral SC_UNO_DP_SORTING = u"Sorting";
constexpr OUStringLiteral SC_UNO_DP_AUTOSHOW = u"AutoShow";
constexpr OUStringLiteral SC_UNO_DP_LAYOUT = u"Layout";

// data pilot descriptor
constexpr OUStringLiteral SC_UNO_DP_IGNORE_EMPTYROWS = u"IgnoreEmptyRows";
constexpr OUStringLiteral SC_UNO_DP_DRILLDOWN = u"DrillDownOnDoubleClick";
constexpr OUStringLiteral SC_UNO_DP_SHOWFILTER = u"ShowFilterButton";
constexpr OUStringLiteral SC_UNO_DP_IMPORTDESC = u"ImportDescriptor";
constexpr OUStringLiteral SC_UNO_DP_SOURCESERVICE = u"SourceServiceName";
constexpr OUStringLiteral SC_UNO_DP_SERVICEARG = u"ServiceArguments";

// properties in data pilot descriptor ServiceArguments
#define SC_UNO_DP_SOURCENAME           "SourceName"
#define SC_UNO_DP_OBJECTNAME           "ObjectName"
#define SC_UNO_DP_USERNAME             "UserName"
#define SC_UNO_DP_PASSWORD             "Password"

//  range selection
#define SC_UNONAME_INITVAL          "InitialValue"
constexpr OUStringLiteral SC_UNONAME_TITLE = u"Title";
#define SC_UNONAME_CLOSEONUP        "CloseOnMouseRelease"
#define SC_UNONAME_SINGLECELL       "SingleCellMode"
#define SC_UNONAME_MULTISEL         "MultiSelectionMode"

//  XRenderable
#define SC_UNONAME_PAGESIZE         "PageSize"
#define SC_UNONAME_RENDERDEV        "RenderDevice"
#define SC_UNONAME_SOURCERANGE      "SourceRange"
#define SC_UNONAME_INC_NP_AREA      "PageIncludesNonprintableArea"
#define SC_UNONAME_CALCPAGESIZE     "CalcPageContentSize"
#define SC_UNONAME_CALCPAGEPOS      "CalcPagePos"

// CellValueBinding
constexpr OUStringLiteral SC_UNONAME_BOUNDCELL = u"BoundCell";
// CellRangeListSource
constexpr OUStringLiteral SC_UNONAME_CELLRANGE = u"CellRange";

// CellAddressConversion / CellRangeAddressConversion
#define SC_UNONAME_ADDRESS          "Address"
#define SC_UNONAME_UIREPR           "UserInterfaceRepresentation"
#define SC_UNONAME_PERSREPR         "PersistentRepresentation"
#define SC_UNONAME_XLA1REPR         "XLA1Representation"
#define SC_UNONAME_REFSHEET         "ReferenceSheet"

// Security options
constexpr OUStringLiteral SC_UNO_LOADREADONLY = u"LoadReadonly";
constexpr OUStringLiteral SC_UNO_MODIFYPASSWORDINFO = u"ModifyPasswordInfo";
constexpr OUStringLiteral SC_UNO_MODIFYPASSWORDHASH = u"ModifyPasswordHash";

// FormulaParser
constexpr OUStringLiteral SC_UNO_COMPILEENGLISH = u"CompileEnglish";
constexpr OUStringLiteral SC_UNO_FORMULACONVENTION = u"FormulaConvention";
constexpr OUStringLiteral SC_UNO_IGNORELEADING = u"IgnoreLeadingSpaces";
constexpr OUStringLiteral SC_UNO_OPCODEMAP = u"OpCodeMap";
#define SC_UNO_EXTERNALLINKS        "ExternalLinks"
constexpr OUStringLiteral SC_UNO_COMPILEFAP = u"CompileFAP";
#define SC_UNO_REF_CONV_CHARTOOXML  "RefConventionChartOOXML"

// Chart2
#define SC_UNONAME_ROLE                 "Role"
#define SC_UNONAME_HIDDENVALUES         "HiddenValues"
#define SC_UNONAME_INCLUDEHIDDENCELLS   "IncludeHiddenCells"
#define SC_UNONAME_USE_INTERNAL_DATA_PROVIDER "UseInternalDataProvider"
#define SC_UNONAME_HAS_STRING_LABEL "HasStringLabel"
#define SC_UNONAME_TIME_BASED       "TimeBased"

// Solver
#define SC_UNONAME_TIMEOUT          "Timeout"
constexpr OUStringLiteral SC_UNO_SHAREDOC = u"IsDocumentShared";

// EventDescriptor
#define SC_UNO_EVENTTYPE            "EventType"
#define SC_UNO_SCRIPT               "Script"

// Named ranges
constexpr OUStringLiteral SC_UNO_MODIFY_BROADCAST = u"ModifyAndBroadcast";

constexpr OUStringLiteral SC_UNO_EMBED_FONTS = u"EmbedFonts";
constexpr OUStringLiteral SC_UNO_EMBED_ONLY_USED_FONTS = u"EmbedOnlyUsedFonts";
constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_LATIN = u"EmbedLatinScriptFonts";
constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_ASIAN = u"EmbedAsianScriptFonts";
constexpr OUStringLiteral SC_UNO_EMBED_FONT_SCRIPT_COMPLEX = u"EmbedComplexScriptFonts";

constexpr OUStringLiteral SC_UNO_ODS_LOCK_SOLAR_MUTEX = u"ODSLockSolarMutex";
constexpr OUStringLiteral SC_UNO_ODS_IMPORT_STYLES = u"ODSImportStyles";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
