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

#ifndef SC_UNONAMES_HXX
#define SC_UNONAMES_HXX

// service names
#define SC_SERVICENAME_VALBIND          "com.sun.star.table.CellValueBinding"
#define SC_SERVICENAME_LISTCELLBIND     "com.sun.star.table.ListPositionCellBinding"
#define SC_SERVICENAME_LISTSOURCE       "com.sun.star.table.CellRangeListSource"
#define SC_SERVICENAME_CELLADDRESS      "com.sun.star.table.CellAddressConversion"
#define SC_SERVICENAME_RANGEADDRESS     "com.sun.star.table.CellRangeAddressConversion"

#define SC_SERVICENAME_FORMULAPARS      "com.sun.star.sheet.FormulaParser"
#define SC_SERVICENAME_OPCODEMAPPER     "com.sun.star.sheet.FormulaOpCodeMapper"

#define SC_SERVICENAME_CHDATAPROV       "com.sun.star.chart2.data.DataProvider"
#define SC_SERVICENAME_CHRANGEHILIGHT   "com.sun.star.chart2.data.RangeHighlightListener"

//  document
#define SC_UNO_AREALINKS            "AreaLinks"
#define SC_UNO_DDELINKS             "DDELinks"
#define SC_UNO_EXTERNALDOCLINKS     "ExternalDocLinks"
#define SC_UNO_COLLABELRNG          "ColumnLabelRanges"
#define SC_UNO_DATABASERNG          "DatabaseRanges"
#define SC_UNO_NAMEDRANGES          "NamedRanges"
#define SC_UNO_ROWLABELRNG          "RowLabelRanges"
#define SC_UNO_SHEETLINKS           "SheetLinks"
#define SC_UNO_FORBIDDEN            "ForbiddenCharacters"
#define SC_UNO_HASDRAWPAGES         "HasDrawPages"

//  CharacterProperties
#define SC_UNONAME_CCOLOR           "CharColor"
#define SC_UNONAME_CHEIGHT          "CharHeight"
#define SC_UNONAME_CUNDER           "CharUnderline"
#define SC_UNONAME_CUNDLCOL         "CharUnderlineColor"
#define SC_UNONAME_CUNDLHAS         "CharUnderlineHasColor"
#define SC_UNONAME_COVER            "CharOverline"
#define SC_UNONAME_COVRLCOL         "CharOverlineColor"
#define SC_UNONAME_COVRLHAS         "CharOverlineHasColor"
#define SC_UNONAME_CWEIGHT          "CharWeight"
#define SC_UNONAME_CPOST            "CharPosture"
#define SC_UNONAME_CCROSS           "CharCrossedOut"
#define SC_UNONAME_CSTRIKE          "CharStrikeout"
#define SC_UNONAME_CLOCAL           "CharLocale"
#define SC_UNONAME_CSHADD           "CharShadowed"
#define SC_UNONAME_CFONT            "CharFont"
#define SC_UNONAME_COUTL            "CharContoured"
#define SC_UNONAME_CEMPHAS          "CharEmphasis"
#define SC_UNONAME_CFNAME           "CharFontName"
#define SC_UNONAME_CFSTYLE          "CharFontStyleName"
#define SC_UNONAME_CFFAMIL          "CharFontFamily"
#define SC_UNONAME_CFCHARS          "CharFontCharSet"
#define SC_UNONAME_CFPITCH          "CharFontPitch"
#define SC_UNONAME_CRELIEF          "CharRelief"
#define SC_UNONAME_CWORDMOD         "CharWordMode"

#define SC_UNO_CJK_CFNAME           "CharFontNameAsian"
#define SC_UNO_CJK_CFSTYLE          "CharFontStyleNameAsian"
#define SC_UNO_CJK_CFFAMIL          "CharFontFamilyAsian"
#define SC_UNO_CJK_CFCHARS          "CharFontCharSetAsian"
#define SC_UNO_CJK_CFPITCH          "CharFontPitchAsian"
#define SC_UNO_CJK_CHEIGHT          "CharHeightAsian"
#define SC_UNO_CJK_CWEIGHT          "CharWeightAsian"
#define SC_UNO_CJK_CPOST            "CharPostureAsian"
#define SC_UNO_CJK_CLOCAL           "CharLocaleAsian"

#define SC_UNO_CTL_CFNAME           "CharFontNameComplex"
#define SC_UNO_CTL_CFSTYLE          "CharFontStyleNameComplex"
#define SC_UNO_CTL_CFFAMIL          "CharFontFamilyComplex"
#define SC_UNO_CTL_CFCHARS          "CharFontCharSetComplex"
#define SC_UNO_CTL_CFPITCH          "CharFontPitchComplex"
#define SC_UNO_CTL_CHEIGHT          "CharHeightComplex"
#define SC_UNO_CTL_CWEIGHT          "CharWeightComplex"
#define SC_UNO_CTL_CPOST            "CharPostureComplex"
#define SC_UNO_CTL_CLOCAL           "CharLocaleComplex"

//  CellProperties
#define SC_UNONAME_CELLSTYL         "CellStyle"
#define SC_UNONAME_CELLBACK         "CellBackColor"
#define SC_UNONAME_CELLTRAN         "IsCellBackgroundTransparent"
#define SC_UNONAME_CELLPRO          "CellProtection"
#define SC_UNONAME_CELLHJUS         "HoriJustify"
#define SC_UNONAME_CELLVJUS         "VertJustify"
#define SC_UNONAME_CELLHJUS_METHOD  "HoriJustifyMethod"
#define SC_UNONAME_CELLVJUS_METHOD  "VertJustifyMethod"
#define SC_UNONAME_CELLORI          "Orientation"
#define SC_UNONAME_NUMFMT           "NumberFormat"
#define SC_UNONAME_SHADOW           "ShadowFormat"
#define SC_UNONAME_TBLBORD          "TableBorder"
#define SC_UNONAME_WRAP             "IsTextWrapped"
#define SC_UNONAME_PINDENT          "ParaIndent"
#define SC_UNONAME_PTMARGIN         "ParaTopMargin"
#define SC_UNONAME_PBMARGIN         "ParaBottomMargin"
#define SC_UNONAME_PLMARGIN         "ParaLeftMargin"
#define SC_UNONAME_PRMARGIN         "ParaRightMargin"
#define SC_UNONAME_ROTANG           "RotateAngle"
#define SC_UNONAME_ROTREF           "RotateReference"
#define SC_UNONAME_ASIANVERT        "AsianVerticalMode"
#define SC_UNONAME_WRITING          "WritingMode"

#define SC_UNONAME_BOTTBORDER       "BottomBorder"
#define SC_UNONAME_LEFTBORDER       "LeftBorder"
#define SC_UNONAME_RIGHTBORDER      "RightBorder"
#define SC_UNONAME_TOPBORDER        "TopBorder"

#define SC_UNONAME_DIAGONAL_TLBR    "DiagonalTLBR"
#define SC_UNONAME_DIAGONAL_BLTR    "DiagonalBLTR"

#define SC_UNONAME_SHRINK_TO_FIT    "ShrinkToFit"

#define SC_UNONAME_PISHANG          "ParaIsHangingPunctuation"
#define SC_UNONAME_PISCHDIST        "ParaIsCharacterDistance"
#define SC_UNONAME_PISFORBID        "ParaIsForbiddenRules"
#define SC_UNONAME_PISHYPHEN        "ParaIsHyphenation"
#define SC_UNONAME_PADJUST          "ParaAdjust"
#define SC_UNONAME_PLASTADJ         "ParaLastLineAdjust"

#define SC_UNONAME_NUMRULES         "NumberingRules"

//  Styles
#define SC_UNONAME_DISPNAME         "DisplayName"

//  XStyleLoader
#define SC_UNONAME_OVERWSTL         "OverwriteStyles"
#define SC_UNONAME_LOADCELL         "LoadCellStyles"
#define SC_UNONAME_LOADPAGE         "LoadPageStyles"

//  SheetCellRange
#define SC_UNONAME_POS              "Position"
#define SC_UNONAME_SIZE             "Size"
#define SC_UNONAME_ABSNAME          "AbsoluteName"

//  column/row/sheet
#define SC_UNONAME_CELLHGT          "Height"
#define SC_UNONAME_CELLWID          "Width"
#define SC_UNONAME_CELLVIS          "IsVisible"
#define SC_UNONAME_CELLFILT         "IsFiltered"
#define SC_UNONAME_MANPAGE          "IsManualPageBreak"
#define SC_UNONAME_NEWPAGE          "IsStartOfNewPage"
#define SC_UNONAME_OHEIGHT          "OptimalHeight"
#define SC_UNONAME_OWIDTH           "OptimalWidth"
#define SC_UNONAME_PAGESTL          "PageStyle"
#define SC_UNONAME_ISACTIVE         "IsActive"
#define SC_UNONAME_BORDCOL          "BorderColor"
#define SC_UNONAME_PROTECT          "Protected"
#define SC_UNONAME_SHOWBORD         "ShowBorder"
#define SC_UNONAME_PRINTBORD        "PrintBorder"
#define SC_UNONAME_COPYBACK         "CopyBack"
#define SC_UNONAME_COPYSTYL         "CopyStyles"
#define SC_UNONAME_COPYFORM         "CopyFormulas"
#define SC_UNONAME_TABLAYOUT        "TableLayout"
#define SC_UNONAME_AUTOPRINT        "AutomaticPrintArea"
#define SC_UNONAME_TABCOLOR         "TabColor"

//  LinkTarget
#define SC_UNO_LINKDISPBIT          "LinkDisplayBitmap"
#define SC_UNO_LINKDISPNAME         "LinkDisplayName"

//  drawing objects
#define SC_UNONAME_IMAGEMAP         "ImageMap"
#define SC_UNONAME_ANCHOR           "Anchor"
#define SC_UNONAME_HORIPOS          "HoriOrientPosition"
#define SC_UNONAME_VERTPOS          "VertOrientPosition"
//     #i66550 HLINK_FOR_SHAPES
#define SC_UNONAME_HYPERLINK        "Hyperlink"
#define SC_UNONAME_MOVEPROTECT      "MoveProtect"
#define SC_UNONAME_SIZEPROTECT      "SizeProtect"

//  other cell properties
#define SC_UNONAME_CHCOLHDR         "ChartColumnAsLabel"
#define SC_UNONAME_CHROWHDR         "ChartRowAsLabel"
#define SC_UNONAME_CONDFMT          "ConditionalFormat"
#define SC_UNONAME_CONDLOC          "ConditionalFormatLocal"
#define SC_UNONAME_CONDXML          "ConditionalFormatXML"
#define SC_UNONAME_VALIDAT          "Validation"
#define SC_UNONAME_VALILOC          "ValidationLocal"
#define SC_UNONAME_VALIXML          "ValidationXML"
#define SC_UNONAME_FORMLOC          "FormulaLocal"
#define SC_UNONAME_FORMRT           "FormulaResultType"

#define SC_UNONAME_USERDEF          "UserDefinedAttributes"
#define SC_UNONAME_TEXTUSER         "TextUserDefinedAttributes"

//  auto format
#define SC_UNONAME_INCBACK          "IncludeBackground"
#define SC_UNONAME_INCBORD          "IncludeBorder"
#define SC_UNONAME_INCFONT          "IncludeFont"
#define SC_UNONAME_INCJUST          "IncludeJustify"
#define SC_UNONAME_INCNUM           "IncludeNumberFormat"
#define SC_UNONAME_INCWIDTH         "IncludeWidthAndHeight"

//  function description
#define SC_UNONAME_ARGUMENTS        "Arguments"
#define SC_UNONAME_CATEGORY         "Category"
#define SC_UNONAME_DESCRIPTION      "Description"
#define SC_UNONAME_ID               "Id"
#define SC_UNONAME_NAME             "Name"

//  application settings
#define SC_UNONAME_DOAUTOCP         "DoAutoComplete"
#define SC_UNONAME_ENTERED          "EnterEdit"
#define SC_UNONAME_EXPREF           "ExpandReferences"
#define SC_UNONAME_EXTFMT           "ExtendFormat"
#define SC_UNONAME_LINKUPD          "LinkUpdateMode"
#define SC_UNONAME_MARKHDR          "MarkHeader"
#define SC_UNONAME_METRIC           "Metric"
#define SC_UNONAME_MOVEDIR          "MoveDirection"
#define SC_UNONAME_MOVESEL          "MoveSelection"
#define SC_UNONAME_RANGEFIN         "RangeFinder"
#define SC_UNONAME_SCALE            "Scale"
#define SC_UNONAME_STBFUNC          "StatusBarFunction"
#define SC_UNONAME_ULISTS           "UserLists"
#define SC_UNONAME_USETABCOL        "UseTabCol"
#define SC_UNONAME_PRMETRICS        "UsePrinterMetrics"
#define SC_UNONAME_PRALLSH          "PrintAllSheets"
#define SC_UNONAME_PREMPTY          "PrintEmptyPages"
#define SC_UNONAME_REPLWARN         "ReplaceCellsWarning"

//  data pilot field
#define SC_UNONAME_FUNCTION         "Function"
#define SC_UNONAME_SUBTOTALS        "Subtotals"
#define SC_UNONAME_SELPAGE          "SelectedPage"
#define SC_UNONAME_USESELPAGE       "UseSelectedPage"
#define SC_UNONAME_HASREFERENCE     "HasReference"
#define SC_UNONAME_REFERENCE        "Reference"
#define SC_UNONAME_HASAUTOSHOW      "HasAutoShowInfo"
#define SC_UNONAME_AUTOSHOW         "AutoShowInfo"
#define SC_UNONAME_HASSORTINFO      "HasSortInfo"
#define SC_UNONAME_SORTINFO         "SortInfo"
#define SC_UNONAME_HASLAYOUTINFO    "HasLayoutInfo"
#define SC_UNONAME_LAYOUTINFO       "LayoutInfo"
#define SC_UNONAME_ISGROUP          "IsGroupField"
#define SC_UNONAME_GROUPINFO        "GroupInfo"
#define SC_UNONAME_SHOWEMPTY        "ShowEmpty"

//  data pilot item
#define SC_UNONAME_SHOWDETAIL       "ShowDetail"
#define SC_UNONAME_ISHIDDEN         "IsHidden"

//  database options
#define SC_UNONAME_CASE             "CaseSensitive"
#define SC_UNONAME_DBNAME           "DatabaseName"
#define SC_UNONAME_FORMATS          "IncludeFormats"
#define SC_UNONAME_INSBRK           "InsertPageBreaks"
#define SC_UNONAME_KEEPFORM         "KeepFormats"
#define SC_UNONAME_MOVCELLS         "MoveCells"
#define SC_UNONAME_ISUSER           "IsUserDefined"
#define SC_UNONAME_ISNATIVE         "IsNative"
#define SC_UNONAME_REGEXP           "RegularExpressions"
#define SC_UNONAME_SAVEOUT          "SaveOutputPosition"
#define SC_UNONAME_SKIPDUP          "SkipDuplicates"
#define SC_UNONAME_SRCOBJ           "SourceObject"
#define SC_UNONAME_SRCTYPE          "SourceType"
#define SC_UNONAME_STRIPDAT         "StripData"
#define SC_UNONAME_UNIQUE           "UniqueOnly"
#define SC_UNONAME_USEREGEX         "UseRegularExpressions"
#define SC_UNONAME_ULIST            "UserListEnabled"
#define SC_UNONAME_UINDEX           "UserListIndex"
#define SC_UNONAME_BINDFMT          "BindFormatsToContent"
#define SC_UNONAME_COPYOUT          "CopyOutputData"
#define SC_UNONAME_ISCASE           "IsCaseSensitive"
#define SC_UNONAME_ISULIST          "IsUserListEnabled"
#define SC_UNONAME_OUTPOS           "OutputPosition"
#define SC_UNONAME_CONTHDR          "ContainsHeader"
#define SC_UNONAME_MAXFLD           "MaxFieldCount"
#define SC_UNONAME_ORIENT           "Orientation"
#define SC_UNONAME_ISSORTCOLUMNS    "IsSortColumns"
#define SC_UNONAME_SORTFLD          "SortFields"
#define SC_UNONAME_SORTASC          "SortAscending"
#define SC_UNONAME_ENUSLIST         "EnableUserSortList"
#define SC_UNONAME_USINDEX          "UserSortListIndex"
#define SC_UNONAME_COLLLOC          "CollatorLocale"
#define SC_UNONAME_COLLALG          "CollatorAlgorithm"
#define SC_UNONAME_AUTOFLT          "AutoFilter"
#define SC_UNONAME_FLTCRT           "FilterCriteriaSource"
#define SC_UNONAME_USEFLTCRT        "UseFilterCriteriaSource"
#define SC_UNONAME_ENABSORT         "EnableSort"
#define SC_UNONAME_FROMSELECT       "FromSelection"
#define SC_UNONAME_CONRES           "ConnectionResource"
#define SC_UNONAME_TOKENINDEX       "TokenIndex"
#define SC_UNONAME_ISSHAREDFMLA     "IsSharedFormula"

//  text fields
#define SC_UNONAME_ANCTYPE          "AnchorType"
#define SC_UNONAME_ANCTYPES         "AnchorTypes"
#define SC_UNONAME_TEXTWRAP         "TextWrap"
#define SC_UNONAME_FILEFORM         "FileFormat"

//  url field
#define SC_UNONAME_REPR             "Representation"
#define SC_UNONAME_TARGET           "TargetFrame"
#define SC_UNONAME_URL              "URL"

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
#define SC_UNONAME_ERRALSTY         "ErrorAlertStyle"
#define SC_UNONAME_ERRMESS          "ErrorMessage"
#define SC_UNONAME_ERRTITLE         "ErrorTitle"
#define SC_UNONAME_IGNOREBL         "IgnoreBlankCells"
#define SC_UNONAME_INPMESS          "InputMessage"
#define SC_UNONAME_INPTITLE         "InputTitle"
#define SC_UNONAME_SHOWERR          "ShowErrorMessage"
#define SC_UNONAME_SHOWINP          "ShowInputMessage"
#define SC_UNONAME_SHOWLIST         "ShowList"
#define SC_UNONAME_TYPE             "Type"

//  links
#define SC_UNONAME_FILTER           "Filter"
#define SC_UNONAME_FILTOPT          "FilterOptions"
#define SC_UNONAME_LINKURL          "Url"
#define SC_UNONAME_REFPERIOD        "RefreshPeriod"
#define SC_UNONAME_REFDELAY         "RefreshDelay" // deprecated, don't use anymore

//  search descriptor
#define SC_UNO_SRCHBACK             "SearchBackwards"
#define SC_UNO_SRCHBYROW            "SearchByRow"
#define SC_UNO_SRCHCASE             "SearchCaseSensitive"
#define SC_UNO_SRCHREGEXP           "SearchRegularExpression"
#define SC_UNO_SRCHSIM              "SearchSimilarity"
#define SC_UNO_SRCHSIMADD           "SearchSimilarityAdd"
#define SC_UNO_SRCHSIMEX            "SearchSimilarityExchange"
#define SC_UNO_SRCHSIMREL           "SearchSimilarityRelax"
#define SC_UNO_SRCHSIMREM           "SearchSimilarityRemove"
#define SC_UNO_SRCHSTYLES           "SearchStyles"
#define SC_UNO_SRCHTYPE             "SearchType"
#define SC_UNO_SRCHWORDS            "SearchWords"
#define SC_UNO_SRCHFILTERED         "SearchFiltered"

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
#define SC_UNO_PAGE_BACKCOLOR       "BackColor"
#define SC_UNO_PAGE_BACKTRANS       "BackTransparent"
#define SC_UNO_PAGE_GRAPHICFILT     "BackGraphicFilter"
#define SC_UNO_PAGE_GRAPHICLOC      "BackGraphicLocation"
#define SC_UNO_PAGE_GRAPHICURL      "BackGraphicURL"
#define SC_UNO_PAGE_LEFTBORDER      SC_UNONAME_LEFTBORDER
#define SC_UNO_PAGE_RIGHTBORDER     SC_UNONAME_RIGHTBORDER
#define SC_UNO_PAGE_BOTTBORDER      SC_UNONAME_BOTTBORDER
#define SC_UNO_PAGE_TOPBORDER       SC_UNONAME_TOPBORDER
#define SC_UNO_PAGE_LEFTBRDDIST     "LeftBorderDistance"
#define SC_UNO_PAGE_RIGHTBRDDIST    "RightBorderDistance"
#define SC_UNO_PAGE_BOTTBRDDIST     "BottomBorderDistance"
#define SC_UNO_PAGE_TOPBRDDIST      "TopBorderDistance"
#define SC_UNO_PAGE_BORDERDIST      "BorderDistance"
#define SC_UNO_PAGE_SHADOWFORM      "ShadowFormat"
#define SC_UNO_PAGE_LEFTMARGIN      "LeftMargin"
#define SC_UNO_PAGE_RIGHTMARGIN     "RightMargin"
#define SC_UNO_PAGE_TOPMARGIN       "TopMargin"
#define SC_UNO_PAGE_BOTTMARGIN      "BottomMargin"
#define SC_UNO_PAGE_LANDSCAPE       "IsLandscape"
#define SC_UNO_PAGE_NUMBERTYPE      "NumberingType"
#define SC_UNO_PAGE_SYTLELAYOUT     "PageStyleLayout"
#define SC_UNO_PAGE_PAPERTRAY       "PrinterPaperTray"
#define SC_UNO_PAGE_SIZE            "Size"
#define SC_UNO_PAGE_WIDTH           "Width"
#define SC_UNO_PAGE_HEIGHT          "Height"
#define SC_UNO_PAGE_CENTERHOR       "CenterHorizontally"
#define SC_UNO_PAGE_CENTERVER       "CenterVertically"
#define SC_UNO_PAGE_PRINTANNOT      "PrintAnnotations"
#define SC_UNO_PAGE_PRINTGRID       "PrintGrid"
#define SC_UNO_PAGE_PRINTHEADER     "PrintHeaders"
#define SC_UNO_PAGE_PRINTCHARTS     "PrintCharts"
#define SC_UNO_PAGE_PRINTOBJS       "PrintObjects"
#define SC_UNO_PAGE_PRINTDRAW       "PrintDrawing"
#define SC_UNO_PAGE_PRINTDOWN       "PrintDownFirst"
#define SC_UNO_PAGE_SCALEVAL        "PageScale"
#define SC_UNO_PAGE_SCALETOPAG      "ScaleToPages"
#define SC_UNO_PAGE_SCALETOX        "ScaleToPagesX"
#define SC_UNO_PAGE_SCALETOY        "ScaleToPagesY"
#define SC_UNO_PAGE_FIRSTPAGE       "FirstPageNumber"
#define SC_UNO_PAGE_LEFTHDRCONT     "LeftPageHeaderContent"
#define SC_UNO_PAGE_LEFTFTRCONT     "LeftPageFooterContent"
#define SC_UNO_PAGE_RIGHTHDRCON     "RightPageHeaderContent"
#define SC_UNO_PAGE_RIGHTFTRCON     "RightPageFooterContent"
#define SC_UNO_PAGE_PRINTFORMUL     "PrintFormulas"
#define SC_UNO_PAGE_PRINTZERO       "PrintZeroValues"
#define SC_UNO_PAGE_HDRBACKCOL      "HeaderBackColor"
#define SC_UNO_PAGE_HDRBACKTRAN     "HeaderBackTransparent"
#define SC_UNO_PAGE_HDRGRFFILT      "HeaderBackGraphicFilter"
#define SC_UNO_PAGE_HDRGRFLOC       "HeaderBackGraphicLocation"
#define SC_UNO_PAGE_HDRGRFURL       "HeaderBackGraphicURL"
#define SC_UNO_PAGE_HDRLEFTBOR      "HeaderLeftBorder"
#define SC_UNO_PAGE_HDRRIGHTBOR     "HeaderRightBorder"
#define SC_UNO_PAGE_HDRBOTTBOR      "HeaderBottomBorder"
#define SC_UNO_PAGE_HDRTOPBOR       "HeaderTopBorder"
#define SC_UNO_PAGE_HDRLEFTBDIS     "HeaderLeftBorderDistance"
#define SC_UNO_PAGE_HDRRIGHTBDIS    "HeaderRightBorderDistance"
#define SC_UNO_PAGE_HDRBOTTBDIS     "HeaderBottomBorderDistance"
#define SC_UNO_PAGE_HDRTOPBDIS      "HeaderTopBorderDistance"
#define SC_UNO_PAGE_HDRBRDDIST      "HeaderBorderDistance"
#define SC_UNO_PAGE_HDRSHADOW       "HeaderShadowFormat"
#define SC_UNO_PAGE_HDRLEFTMAR      "HeaderLeftMargin"
#define SC_UNO_PAGE_HDRRIGHTMAR     "HeaderRightMargin"
#define SC_UNO_PAGE_HDRBODYDIST     "HeaderBodyDistance"
#define SC_UNO_PAGE_HDRHEIGHT       "HeaderHeight"
#define SC_UNO_PAGE_HDRON           "HeaderIsOn"
#define SC_UNO_PAGE_HDRDYNAMIC      "HeaderIsDynamicHeight"
#define SC_UNO_PAGE_HDRSHARED       "HeaderIsShared"
#define SC_UNO_PAGE_FTRBACKCOL      "FooterBackColor"
#define SC_UNO_PAGE_FTRBACKTRAN     "FooterBackTransparent"
#define SC_UNO_PAGE_FTRGRFFILT      "FooterBackGraphicFilter"
#define SC_UNO_PAGE_FTRGRFLOC       "FooterBackGraphicLocation"
#define SC_UNO_PAGE_FTRGRFURL       "FooterBackGraphicURL"
#define SC_UNO_PAGE_FTRLEFTBOR      "FooterLeftBorder"
#define SC_UNO_PAGE_FTRRIGHTBOR     "FooterRightBorder"
#define SC_UNO_PAGE_FTRBOTTBOR      "FooterBottomBorder"
#define SC_UNO_PAGE_FTRTOPBOR       "FooterTopBorder"
#define SC_UNO_PAGE_FTRLEFTBDIS     "FooterLeftBorderDistance"
#define SC_UNO_PAGE_FTRRIGHTBDIS    "FooterRightBorderDistance"
#define SC_UNO_PAGE_FTRBOTTBDIS     "FooterBottomBorderDistance"
#define SC_UNO_PAGE_FTRTOPBDIS      "FooterTopBorderDistance"
#define SC_UNO_PAGE_FTRBRDDIST      "FooterBorderDistance"
#define SC_UNO_PAGE_FTRSHADOW       "FooterShadowFormat"
#define SC_UNO_PAGE_FTRLEFTMAR      "FooterLeftMargin"
#define SC_UNO_PAGE_FTRRIGHTMAR     "FooterRightMargin"
#define SC_UNO_PAGE_FTRBODYDIST     "FooterBodyDistance"
#define SC_UNO_PAGE_FTRHEIGHT       "FooterHeight"
#define SC_UNO_PAGE_FTRON           "FooterIsOn"
#define SC_UNO_PAGE_FTRDYNAMIC      "FooterIsDynamicHeight"
#define SC_UNO_PAGE_FTRSHARED       "FooterIsShared"

//  document settings
#define SC_UNO_CALCASSHOWN              "CalcAsShown"
#define SC_UNO_DEFTABSTOP               "DefaultTabStop"
#define SC_UNO_TABSTOPDIS               "TabStopDistance" // is the same like the before, but only the writer name
#define SC_UNO_IGNORECASE               "IgnoreCase"
#define SC_UNO_ITERCOUNT                "IterationCount"
#define SC_UNO_ITERENABLED              "IsIterationEnabled"
#define SC_UNO_ITEREPSILON              "IterationEpsilon"
#define SC_UNO_LOOKUPLABELS             "LookUpLabels"
#define SC_UNO_MATCHWHOLE               "MatchWholeCell"
#define SC_UNO_NULLDATE                 "NullDate"
#define SC_UNO_SPELLONLINE              "SpellOnline"
#define SC_UNO_STANDARDDEC              "StandardDecimals"
#define SC_UNO_REGEXENABLED             "RegularExpressions"
#define SC_UNO_BASICLIBRARIES           "BasicLibraries"
#define SC_UNO_DIALOGLIBRARIES          "DialogLibraries"
#define SC_UNO_RUNTIMEUID               "RuntimeUID"
#define SC_UNO_HASVALIDSIGNATURES       "HasValidSignatures"
#define SC_UNO_ISLOADED                 "IsLoaded"
#define SC_UNO_ISUNDOENABLED            "IsUndoEnabled"
#define SC_UNO_ISADJUSTHEIGHTENABLED    "IsAdjustHeightEnabled"
#define SC_UNO_ISEXECUTELINKENABLED     "IsExecuteLinkEnabled"
#define SC_UNO_ISCHANGEREADONLYENABLED  "IsChangeReadOnlyEnabled"
#define SC_UNO_REFERENCEDEVICE          "ReferenceDevice"
#define SC_UNO_CODENAME                 "CodeName"

//  document properties from FormModel
#define SC_UNO_APPLYFMDES           "ApplyFormDesignMode"
#define SC_UNO_AUTOCONTFOC          "AutomaticControlFocus"

//  view options
#define SC_UNO_COLROWHDR            "HasColumnRowHeaders"
#define SC_UNO_GRIDCOLOR            "GridColor"
#define SC_UNO_HIDESPELL            "HideSpellMarks"                /* deprecated #i91949 */
#define SC_UNO_HORSCROLL            "HasHorizontalScrollBar"
#define SC_UNO_OUTLSYMB             "IsOutlineSymbolsSet"
#define SC_UNO_SHEETTABS            "HasSheetTabs"
#define SC_UNO_SHOWANCHOR           "ShowAnchor"
#define SC_UNO_SHOWCHARTS           "ShowCharts"
#define SC_UNO_SHOWDRAW             "ShowDrawing"
#define SC_UNO_SHOWFORM             "ShowFormulas"
#define SC_UNO_SHOWGRID             "ShowGrid"
#define SC_UNO_SHOWHELP             "ShowHelpLines"
#define SC_UNO_SHOWNOTES            "ShowNotes"
#define SC_UNO_SHOWOBJ              "ShowObjects"
#define SC_UNO_SHOWPAGEBR           "ShowPageBreaks"
#define SC_UNO_SHOWZERO             "ShowZeroValues"
#define SC_UNO_SHOWSOLID            "SolidHandles"
#define SC_UNO_VALUEHIGH            "IsValueHighlightingEnabled"
#define SC_UNO_VERTSCROLL           "HasVerticalScrollBar"
#define SC_UNO_SNAPTORASTER         "IsSnapToRaster"
#define SC_UNO_RASTERVIS            "RasterIsVisible"
#define SC_UNO_RASTERRESX           "RasterResolutionX"
#define SC_UNO_RASTERRESY           "RasterResolutionY"
#define SC_UNO_RASTERSUBX           "RasterSubdivisionX"
#define SC_UNO_RASTERSUBY           "RasterSubdivisionY"
#define SC_UNO_RASTERSYNC           "IsRasterAxisSynchronized"
#define SC_UNO_AUTOCALC             "AutoCalculate"
#define SC_UNO_PRINTERNAME          "PrinterName"
#define SC_UNO_PRINTERSETUP         "PrinterSetup"
#define SC_UNO_APPLYDOCINF          "ApplyUserData"
#define SC_UNO_CHARCOMP             "CharacterCompressionType"
#define SC_UNO_ASIANKERN            "IsKernAsianPunctuation"
#define SC_UNO_VISAREA              "VisibleArea"
#define SC_UNO_ZOOMTYPE             "ZoomType"
#define SC_UNO_ZOOMVALUE            "ZoomValue"
#define SC_UNO_UPDTEMPL             "UpdateFromTemplate"
#define SC_UNO_FILTERED_RANGE_SELECTION       "FilteredRangeSelection"
#define SC_UNO_VISAREASCREEN        "VisibleAreaOnScreen"

/*Stampit enable/disable print cancel */
#define SC_UNO_ALLOWPRINTJOBCANCEL  "AllowPrintJobCancel"

//  old (5.2) property names for view options - for compatibility only!
#define OLD_UNO_COLROWHDR           "ColumnRowHeaders"
#define OLD_UNO_HORSCROLL           "HorizontalScrollBar"
#define OLD_UNO_OUTLSYMB            "OutlineSymbols"
#define OLD_UNO_SHEETTABS           "SheetTabs"
#define OLD_UNO_VALUEHIGH           "ValueHighlighting"
#define OLD_UNO_VERTSCROLL          "VerticalScrollBar"

//  data pilot source
#define SC_UNO_COLGRAND             "ColumnGrand"
#define SC_UNO_ROWGRAND             "RowGrand"
#define SC_UNO_ORIGINAL             "Original"
#define SC_UNO_ISDATALA             "IsDataLayoutDimension"
#define SC_UNO_ORIENTAT             "Orientation"
#define SC_UNO_POSITION             "Position"
#define SC_UNO_FUNCTION             "Function"
#define SC_UNO_USEDHIER             "UsedHierarchy"
#define SC_UNO_FILTER               "Filter"
#define SC_UNO_SUBTOTAL             "SubTotals"
#define SC_UNO_SHOWEMPT             "ShowEmpty"
#define SC_UNO_ISVISIBL             "IsVisible"
#define SC_UNO_SHOWDETA             "ShowDetails"
#define SC_UNO_IGNOREEM             "IgnoreEmptyRows"
#define SC_UNO_REPEATIF             "RepeatIfEmpty"
#define SC_UNO_DATADESC             "DataDescription"
#define SC_UNO_NUMBERFO             "NumberFormat"
#define SC_UNO_ROWFIELDCOUNT        "RowFieldCount"
#define SC_UNO_COLUMNFIELDCOUNT     "ColumnFieldCount"
#define SC_UNO_DATAFIELDCOUNT       "DataFieldCount"
#define SC_UNO_LAYOUTNAME           "LayoutName"
#define SC_UNO_FIELD_SUBTOTALNAME   "FieldSubtotalName"
#define SC_UNO_GRANDTOTAL_NAME      "GrandTotalName"
#define SC_UNO_HAS_HIDDEN_MEMBER    "HasHiddenMember"
#define SC_UNO_FLAGS                "Flags"

//  (preliminary:)
#define SC_UNO_REFVALUE             "ReferenceValue"
#define SC_UNO_SORTING              "Sorting"
#define SC_UNO_AUTOSHOW             "AutoShow"
#define SC_UNO_LAYOUT               "Layout"

// data pilot descriptor
#define SC_UNO_IGNEMPROWS           "IgnoreEmptyRows"
#define SC_UNO_RPTEMPTY             "RepeatIfEmpty"
#define SC_UNO_DRILLDOWN            "DrillDownOnDoubleClick"
#define SC_UNO_SHOWFILT             "ShowFilterButton"
#define SC_UNO_IMPORTDESC           "ImportDescriptor"
#define SC_UNO_SOURCESERV           "SourceServiceName"
#define SC_UNO_SERVICEARG           "ServiceArguments"

// properties in data pilot descriptor ServiceArguments
#define SC_UNO_SOURCENAME           "SourceName"
#define SC_UNO_OBJECTNAME           "ObjectName"
#define SC_UNO_USERNAME             "UserName"
#define SC_UNO_PASSWORD             "Password"

//  range selection
#define SC_UNONAME_INITVAL          "InitialValue"
#define SC_UNONAME_TITLE            "Title"
#define SC_UNONAME_CLOSEONUP        "CloseOnMouseRelease"
#define SC_UNONAME_SINGLECELL       "SingleCellMode"
#define SC_UNONAME_MULTISEL         "MultiSelectionMode"

//  XRenderable
#define SC_UNONAME_PAGESIZE         "PageSize"
#define SC_UNONAME_RENDERDEV        "RenderDevice"
#define SC_UNONAME_SOURCERANGE      "SourceRange"
#define SC_UNONAME_INC_NP_AREA      "PageIncludesNonprintableArea"

// CellValueBinding
#define SC_UNONAME_BOUNDCELL        "BoundCell"
// CellRangeListSource
#define SC_UNONAME_CELLRANGE        "CellRange"

// CellAddressConversion / CellRangeAddressConversion
#define SC_UNONAME_ADDRESS          "Address"
#define SC_UNONAME_UIREPR           "UserInterfaceRepresentation"
#define SC_UNONAME_PERSREPR         "PersistentRepresentation"
#define SC_UNONAME_XLA1REPR         "XLA1Representation"
#define SC_UNONAME_REFSHEET         "ReferenceSheet"

// Security options
#define SC_UNO_LOADREADONLY         "LoadReadonly"
#define SC_UNO_MODIFYPASSWORDINFO   "ModifyPasswordInfo"

// FormulaParser
#define SC_UNO_COMPILEENGLISH       "CompileEnglish"
#define SC_UNO_FORMULACONVENTION    "FormulaConvention"
#define SC_UNO_IGNORELEADING        "IgnoreLeadingSpaces"
#define SC_UNO_OPCODEMAP            "OpCodeMap"
#define SC_UNO_EXTERNALLINKS        "ExternalLinks"
#define SC_UNO_COMPILEFAP           "CompileFAP"

// Chart2
#define SC_UNONAME_ROLE                 "Role"
#define SC_UNONAME_HIDDENVALUES         "HiddenValues"
#define SC_UNONAME_INCLUDEHIDDENCELLS   "IncludeHiddenCells"
#define SC_UNONAME_HIDDENVALUES     "HiddenValues"

// Solver
#define SC_UNONAME_TIMEOUT          "Timeout"
#define SC_UNO_SHAREDOC             "IsDocumentShared"

// EventDescriptor
#define SC_UNO_EVENTTYPE            "EventType"
#define SC_UNO_SCRIPT               "Script"

// Named ranges
#define SC_UNO_MODIFY_BROADCAST     "ModifyAndBroadcast"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
