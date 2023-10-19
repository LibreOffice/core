/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <rtl/ustring.hxx>

// Dialog Controls

inline constexpr OUString RID_STR_CLASS_FIXEDTEXT = u"Label"_ustr;
inline constexpr OUString RID_STR_CLASS_FIXEDLINE = u"Line"_ustr;
inline constexpr OUString RID_STR_CLASS_IMAGECONTROL = u"Graphic"_ustr;
inline constexpr OUString RID_STR_CLASS_FORMATTEDFIELD = u"FormattedField"_ustr;

//= service names

inline constexpr OUString SERVICE_FIXEDTEXT = u"com.sun.star.report.FixedText"_ustr;
inline constexpr OUString SERVICE_FORMATTEDFIELD = u"com.sun.star.report.FormattedField"_ustr;
inline constexpr OUString SERVICE_IMAGECONTROL = u"com.sun.star.report.ImageControl"_ustr;
inline constexpr OUString SERVICE_FORMATCONDITION = u"com.sun.star.report.FormatCondition"_ustr;
inline constexpr OUString SERVICE_FUNCTION = u"com.sun.star.report.Function"_ustr;
inline constexpr OUString SERVICE_REPORTDEFINITION = u"com.sun.star.report.ReportDefinition"_ustr;
inline constexpr OUString SERVICE_SHAPE = u"com.sun.star.report.Shape"_ustr;
inline constexpr OUString SERVICE_FIXEDLINE = u"com.sun.star.report.FixedLine"_ustr;
inline constexpr OUString SERVICE_SECTION = u"com.sun.star.report.Section"_ustr;
inline constexpr OUString SERVICE_GROUP = u"com.sun.star.report.Group"_ustr;

//= property names

inline constexpr OUString PROPERTY_VISIBLE = u"Visible"_ustr;
inline constexpr OUString PROPERTY_NAME = u"Name"_ustr;
inline constexpr OUString PROPERTY_HEIGHT = u"Height"_ustr;
inline constexpr OUString PROPERTY_BACKCOLOR = u"BackColor"_ustr;
inline constexpr OUString PROPERTY_BACKTRANSPARENT = u"BackTransparent"_ustr;
inline constexpr OUString PROPERTY_CONTROLBACKGROUND = u"ControlBackground"_ustr;
inline constexpr OUString PROPERTY_CONTROLBACKGROUNDTRANSPARENT = u"ControlBackgroundTransparent"_ustr;
inline constexpr OUString PROPERTY_FORCENEWPAGE = u"ForceNewPage"_ustr;
inline constexpr OUString PROPERTY_NEWROWORCOL = u"NewRowOrCol"_ustr;
inline constexpr OUString PROPERTY_KEEPTOGETHER = u"KeepTogether"_ustr;
inline constexpr OUString PROPERTY_CANGROW = u"CanGrow"_ustr;
inline constexpr OUString PROPERTY_CANSHRINK = u"CanShrink"_ustr;
inline constexpr OUString PROPERTY_REPEATSECTION = u"RepeatSection"_ustr;
inline constexpr OUString PROPERTY_GROUP = u"Group"_ustr;
inline constexpr OUString PROPERTY_REPORTDEFINITION = u"ReportDefinition"_ustr;

inline constexpr OUString PROPERTY_GROUPINTERVAL = u"GroupInterval"_ustr;
inline constexpr OUString PROPERTY_EXPRESSION = u"Expression"_ustr;
inline constexpr OUString PROPERTY_GROUPON = u"GroupOn"_ustr;
inline constexpr OUString PROPERTY_SORTASCENDING = u"SortAscending"_ustr;

inline constexpr OUString PROPERTY_MASTERFIELDS = u"MasterFields"_ustr;
inline constexpr OUString PROPERTY_DETAILFIELDS = u"DetailFields"_ustr;
inline constexpr OUString PROPERTY_CAPTION = u"Caption"_ustr;
inline constexpr OUString PROPERTY_COMMAND = u"Command"_ustr;
inline constexpr OUString PROPERTY_BACKGRAPHICLOCATION = u"BackGraphicLocation"_ustr;
inline constexpr OUString PROPERTY_ORIENTATION = u"Orientation"_ustr;
inline constexpr OUString PROPERTY_PAPERSIZE = u"Size"_ustr;
inline constexpr OUString PROPERTY_GROUPKEEPTOGETHER = u"GroupKeepTogether"_ustr;
inline constexpr OUString PROPERTY_PAGEHEADEROPTION = u"PageHeaderOption"_ustr;
inline constexpr OUString PROPERTY_PAGEFOOTEROPTION = u"PageFooterOption"_ustr;
inline constexpr OUString PROPERTY_COMMANDTYPE = u"CommandType"_ustr;
inline constexpr OUString PROPERTY_REPORTHEADERON = u"ReportHeaderOn"_ustr;
inline constexpr OUString PROPERTY_REPORTFOOTERON = u"ReportFooterOn"_ustr;
inline constexpr OUString PROPERTY_PAGEHEADERON = u"PageHeaderOn"_ustr;
inline constexpr OUString PROPERTY_PAGEFOOTERON = u"PageFooterOn"_ustr;
inline constexpr OUString PROPERTY_HEADERON = u"HeaderOn"_ustr;
inline constexpr OUString PROPERTY_FOOTERON = u"FooterOn"_ustr;
inline constexpr OUString PROPERTY_WIDTH = u"Width"_ustr;
inline constexpr OUString PROPERTY_POSITIONX = u"PositionX"_ustr;
inline constexpr OUString PROPERTY_POSITIONY = u"PositionY"_ustr;
inline constexpr OUString PROPERTY_AUTOGROW = u"AutoGrow"_ustr;
inline constexpr OUString PROPERTY_MINHEIGHT = u"MinHeight"_ustr;
inline constexpr OUString PROPERTY_DATAFIELD = u"DataField"_ustr;
inline constexpr OUString PROPERTY_PARAADJUST = u"ParaAdjust"_ustr;
inline constexpr OUString PROPERTY_FONTDESCRIPTOR = u"FontDescriptor"_ustr;
inline constexpr OUString PROPERTY_FONTDESCRIPTORASIAN = u"FontDescriptorAsian"_ustr;
inline constexpr OUString PROPERTY_FONTDESCRIPTORCOMPLEX = u"FontDescriptorComplex"_ustr;
inline constexpr OUString PROPERTY_CONTROLTEXTEMPHASISMARK = u"ControlTextEmphasis"_ustr;
inline constexpr OUString PROPERTY_CHARRELIEF = u"CharRelief"_ustr;
inline constexpr OUString PROPERTY_CHARCOLOR = u"CharColor"_ustr;
inline constexpr OUString PROPERTY_VERTICALALIGN = u"VerticalAlign"_ustr;
inline constexpr OUString PROPERTY_IMAGEURL = u"ImageURL"_ustr;
inline constexpr OUString PROPERTY_CHARUNDERLINECOLOR = u"CharUnderlineColor"_ustr;
inline constexpr OUString PROPERTY_LABEL = u"Label"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVEDEFAULT = u"EffectiveDefault"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVEMAX = u"EffectiveMax"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVEMIN = u"EffectiveMin"_ustr;
inline constexpr OUString PROPERTY_FORMATKEY = u"FormatKey"_ustr;
inline constexpr OUString PROPERTY_MAXTEXTLEN = u"MaxTextLen"_ustr;
inline constexpr OUString PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier"_ustr;
inline constexpr OUString PROPERTY_CONTROLBORDER = u"ControlBorder"_ustr;
inline constexpr OUString PROPERTY_CONTROLBORDERCOLOR = u"ControlBorderColor"_ustr;
inline constexpr OUString PROPERTY_BORDER = u"Border"_ustr;
inline constexpr OUString PROPERTY_BORDERCOLOR = u"BorderColor"_ustr;
inline constexpr OUString PROPERTY_DEFAULTCONTROL = u"DefaultControl"_ustr;

inline constexpr OUString PROPERTY_LEFTMARGIN = u"LeftMargin"_ustr;
inline constexpr OUString PROPERTY_RIGHTMARGIN = u"RightMargin"_ustr;
inline constexpr OUString PROPERTY_TOPMARGIN = u"TopMargin"_ustr;
inline constexpr OUString PROPERTY_BOTTOMMARGIN = u"BottomMargin"_ustr;

inline constexpr OUString PROPERTY_PRINTREPEATEDVALUES = u"PrintRepeatedValues"_ustr;
inline constexpr OUString PROPERTY_CONDITIONALPRINTEXPRESSION = u"ConditionalPrintExpression"_ustr;
inline constexpr OUString PROPERTY_STARTNEWCOLUMN = u"StartNewColumn"_ustr;
inline constexpr OUString PROPERTY_RESETPAGENUMBER = u"ResetPageNumber"_ustr;
inline constexpr OUString PROPERTY_PRINTWHENGROUPCHANGE = u"PrintWhenGroupChange"_ustr;
inline constexpr OUString PROPERTY_STATE = u"State"_ustr;
inline constexpr OUString PROPERTY_TIME_STATE = u"TimeState"_ustr;
inline constexpr OUString PROPERTY_DATE_STATE = u"DateState"_ustr;
inline constexpr OUString PROPERTY_FONTCHARWIDTH    = u"FontCharWidth"_ustr;
inline constexpr OUString PROPERTY_FONTCHARSET      = u"CharFontCharSet"_ustr;
inline constexpr OUString PROPERTY_FONTFAMILY       = u"CharFontFamily"_ustr;
inline constexpr OUString PROPERTY_CHARFONTHEIGHT   = u"CharHeight"_ustr;
inline constexpr OUString PROPERTY_FONTHEIGHT       = u"FontHeight"_ustr;
inline constexpr OUString PROPERTY_FONTKERNING      = u"FontKerning"_ustr;
inline constexpr OUString PROPERTY_FONT             = u"FontDescriptor"_ustr;
inline constexpr OUString PROPERTY_FONTNAME         = u"FontName"_ustr;
inline constexpr OUString PROPERTY_FONTORIENTATION  = u"CharRotation"_ustr;
inline constexpr OUString PROPERTY_FONTPITCH        = u"CharFontPitch"_ustr;
inline constexpr OUString PROPERTY_CHARSTRIKEOUT    = u"CharStrikeout"_ustr;
inline constexpr OUString PROPERTY_FONTSTRIKEOUT    = u"FontStrikeout"_ustr;
inline constexpr OUString PROPERTY_FONTSTYLENAME    = u"CharFontStyleName"_ustr;
inline constexpr OUString PROPERTY_FONTUNDERLINE    = u"CharUnderline"_ustr;
inline constexpr OUString PROPERTY_FONTWEIGHT       = u"CharWeight"_ustr;
inline constexpr OUString PROPERTY_FONTWIDTH        = u"FontWidth"_ustr;
inline constexpr OUString PROPERTY_FONTTYPE         = u"FontType"_ustr;

inline constexpr OUString PROPERTY_ENABLED       = u"Enabled"_ustr;

inline constexpr OUString PROPERTY_CHAREMPHASIS     = u"CharEmphasis"_ustr;
inline constexpr OUString PROPERTY_CHARFONTNAME     = u"CharFontName"_ustr;
inline constexpr OUString PROPERTY_CHARFONTSTYLENAME = u"CharFontStyleName"_ustr;
inline constexpr OUString PROPERTY_CHARFONTFAMILY   = u"CharFontFamily"_ustr;
inline constexpr OUString PROPERTY_CHARFONTCHARSET  = u"CharFontCharSet"_ustr;
inline constexpr OUString PROPERTY_CHARFONTPITCH    = u"CharFontPitch"_ustr;
inline constexpr OUString PROPERTY_CHARHEIGHT       = u"CharHeight"_ustr;
inline constexpr OUString PROPERTY_CHARUNDERLINE    = u"CharUnderline"_ustr;
inline constexpr OUString PROPERTY_CHARWEIGHT       = u"CharWeight"_ustr;
inline constexpr OUString PROPERTY_CHARPOSTURE      = u"CharPosture"_ustr;
inline constexpr OUString PROPERTY_CHARWORDMODE     = u"CharWordMode"_ustr;
inline constexpr OUString PROPERTY_CHARROTATION     = u"CharRotation"_ustr;
inline constexpr OUString PROPERTY_CHARSCALEWIDTH   = u"CharScaleWidth"_ustr;

// Asian
inline constexpr OUString PROPERTY_CHAREMPHASISASIAN        = u"CharEmphasisAsian"_ustr;
inline constexpr OUString PROPERTY_CHARFONTNAMEASIAN        = u"CharFontNameAsian"_ustr;
inline constexpr OUString PROPERTY_CHARFONTSTYLENAMEASIAN   = u"CharFontStyleNameAsian"_ustr;
inline constexpr OUString PROPERTY_CHARFONTFAMILYASIAN      = u"CharFontFamilyAsian"_ustr;
inline constexpr OUString PROPERTY_CHARFONTCHARSETASIAN     = u"CharFontCharSetAsian"_ustr;
inline constexpr OUString PROPERTY_CHARFONTPITCHASIAN       = u"CharFontPitchAsian"_ustr;
inline constexpr OUString PROPERTY_CHARHEIGHTASIAN          = u"CharHeightAsian"_ustr;
inline constexpr OUString PROPERTY_CHARUNDERLINEASIAN       = u"CharUnderlineAsian"_ustr;
inline constexpr OUString PROPERTY_CHARWEIGHTASIAN          = u"CharWeightAsian"_ustr;
inline constexpr OUString PROPERTY_CHARPOSTUREASIAN         = u"CharPostureAsian"_ustr;
inline constexpr OUString PROPERTY_CHARWORDMODEASIAN        = u"CharWordModeAsian"_ustr;
inline constexpr OUString PROPERTY_CHARROTATIONASIAN        = u"CharRotationAsian"_ustr;
inline constexpr OUString PROPERTY_CHARSCALEWIDTHASIAN      = u"CharScaleWidthAsian"_ustr;
inline constexpr OUString PROPERTY_CHARLOCALEASIAN          = u"CharLocaleAsian"_ustr;

// Complex
inline constexpr OUString PROPERTY_CHAREMPHASISCOMPLEX      = u"CharEmphasisComplex"_ustr;
inline constexpr OUString PROPERTY_CHARFONTNAMECOMPLEX      = u"CharFontNameComplex"_ustr;
inline constexpr OUString PROPERTY_CHARFONTSTYLENAMECOMPLEX = u"CharFontStyleNameComplex"_ustr;
inline constexpr OUString PROPERTY_CHARFONTFAMILYCOMPLEX    = u"CharFontFamilyComplex"_ustr;
inline constexpr OUString PROPERTY_CHARFONTCHARSETCOMPLEX   = u"CharFontCharSetComplex"_ustr;
inline constexpr OUString PROPERTY_CHARFONTPITCHCOMPLEX     = u"CharFontPitchComplex"_ustr;
inline constexpr OUString PROPERTY_CHARHEIGHTCOMPLEX        = u"CharHeightComplex"_ustr;
inline constexpr OUString PROPERTY_CHARUNDERLINECOMPLEX     = u"CharUnderlineComplex"_ustr;
inline constexpr OUString PROPERTY_CHARWEIGHTCOMPLEX        = u"CharWeightComplex"_ustr;
inline constexpr OUString PROPERTY_CHARPOSTURECOMPLEX       = u"CharPostureComplex"_ustr;
inline constexpr OUString PROPERTY_CHARWORDMODECOMPLEX      = u"CharWordModeComplex"_ustr;
inline constexpr OUString PROPERTY_CHARROTATIONCOMPLEX      = u"CharRotationComplex"_ustr;
inline constexpr OUString PROPERTY_CHARSCALEWIDTHCOMPLEX    = u"CharScaleWidthComplex"_ustr;
inline constexpr OUString PROPERTY_CHARLOCALECOMPLEX        = u"CharLocaleComplex"_ustr;

inline constexpr OUString PROPERTY_STATUSINDICATOR  = u"StatusIndicator"_ustr;
inline constexpr OUString PROPERTY_SECTION          = u"Section"_ustr;
inline constexpr OUString PROPERTY_FILTER           = u"Filter"_ustr;
inline constexpr OUString PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing"_ustr;

inline constexpr OUString PROPERTY_MULTILINE        = u"MultiLine"_ustr;
inline constexpr OUString PROPERTY_ACTIVECONNECTION = u"ActiveConnection"_ustr;
inline constexpr OUString PROPERTY_DATASOURCENAME   = u"DataSourceName"_ustr;
inline constexpr OUString PROPERTY_FORMULA          = u"Formula"_ustr;
inline constexpr OUString PROPERTY_INITIALFORMULA   = u"InitialFormula"_ustr;
inline constexpr OUString PROPERTY_PREEVALUATED     = u"PreEvaluated"_ustr;
inline constexpr OUString PROPERTY_DEEPTRAVERSING   = u"DeepTraversing"_ustr;
inline constexpr OUString PROPERTY_MIMETYPE         = u"MimeType"_ustr;
inline constexpr OUString PROPERTY_BACKGROUNDCOLOR  = u"BackgroundColor"_ustr;
inline constexpr OUString PROPERTY_TEXT             = u"Text"_ustr;
inline constexpr OUString PROPERTY_TEXTCOLOR        = u"TextColor"_ustr;
inline constexpr OUString PROPERTY_TEXTLINECOLOR   = u"TextLineColor"_ustr;
inline constexpr OUString PROPERTY_FONTRELIEF   = u"FontRelief"_ustr;
inline constexpr OUString PROPERTY_FONTEMPHASISMARK = u"FontEmphasisMark"_ustr;
inline constexpr OUString PROPERTY_ZORDER           = u"ZOrder"_ustr;
inline constexpr OUString PROPERTY_OPAQUE           = u"Opaque"_ustr;
inline constexpr OUString PROPERTY_TRANSFORMATION    = u"Transformation"_ustr;
inline constexpr OUString PROPERTY_CUSTOMSHAPEENGINE = u"CustomShapeEngine"_ustr;
inline constexpr OUString PROPERTY_CUSTOMSHAPEDATA   = u"CustomShapeData"_ustr;
inline constexpr OUString PROPERTY_CUSTOMSHAPEGEOMETRY = u"CustomShapeGeometry"_ustr;

inline constexpr OUString PROPERTY_NUMBERINGTYPE     = u"NumberingType"_ustr;
inline constexpr OUString PROPERTY_PAGESTYLELAYOUT   = u"PageStyleLayout"_ustr;
inline constexpr OUString PROPERTY_ISLANDSCAPE       = u"IsLandscape"_ustr;
inline constexpr OUString PROPERTY_ALIGN = u"Align"_ustr;
inline constexpr OUString PROPERTY_TYPE  = u"Type"_ustr;

inline constexpr OUString PROPERTY_PRESERVEIRI  = u"PreserveIRI"_ustr;
inline constexpr OUString PROPERTY_SCALEMODE  = u"ScaleMode"_ustr;

inline constexpr OUString PROPERTY_LINESTYLE  = u"LineStyle"_ustr;
inline constexpr OUString PROPERTY_LINEDASH  = u"LineDash"_ustr;
inline constexpr OUString PROPERTY_LINECOLOR  = u"LineColor"_ustr;
inline constexpr OUString PROPERTY_LINETRANSPARENCE  = u"LineTransparence"_ustr;
inline constexpr OUString PROPERTY_LINEWIDTH  = u"LineWidth"_ustr;

inline constexpr OUString PROPERTY_CHARFLASH            = u"CharFlash"_ustr;
inline constexpr OUString PROPERTY_CHARESCAPEMENTHEIGHT = u"CharEscapementHeight"_ustr;
inline constexpr OUString PROPERTY_CHARLOCALE           = u"CharLocale"_ustr;
inline constexpr OUString PROPERTY_CHARESCAPEMENT       = u"CharEscapement"_ustr;
inline constexpr OUString PROPERTY_CHARCASEMAP          = u"CharCaseMap"_ustr;
inline constexpr OUString PROPERTY_CHARCOMBINEISON      = u"CharCombineIsOn"_ustr;
inline constexpr OUString PROPERTY_CHARCOMBINEPREFIX    = u"CharCombinePrefix"_ustr;
inline constexpr OUString PROPERTY_CHARCOMBINESUFFIX    = u"CharCombineSuffix"_ustr;
inline constexpr OUString PROPERTY_CHARHIDDEN           = u"CharHidden"_ustr;
inline constexpr OUString PROPERTY_CHARSHADOWED         = u"CharShadowed"_ustr;
inline constexpr OUString PROPERTY_CHARCONTOURED        = u"CharContoured"_ustr;
inline constexpr OUString PROPERTY_CHARAUTOKERNING      = u"CharAutoKerning"_ustr;
inline constexpr OUString PROPERTY_CHARKERNING          = u"CharKerning"_ustr;
inline constexpr OUString PROPERTY_HYPERLINKURL = u"HyperLinkURL"_ustr;
inline constexpr OUString PROPERTY_HYPERLINKTARGET = u"HyperLinkTarget"_ustr;
inline constexpr OUString PROPERTY_HYPERLINKNAME = u"HyperLinkName"_ustr;
inline constexpr OUString PROPERTY_VISITEDCHARSTYLENAME = u"VisitedCharStyleName"_ustr;
inline constexpr OUString PROPERTY_UNVISITEDCHARSTYLENAME = u"UnvisitedCharStyleName"_ustr;
inline constexpr OUString PROPERTY_APPLYFILTER          = u"ApplyFilter"_ustr;
inline constexpr OUString PROPERTY_MAXROWS              = u"MaxRows"_ustr;

inline constexpr OUString PROPERTY_REPORTNAME = u"ReportName"_ustr;
inline constexpr OUString CFG_REPORTDESIGNER = u"SunReportBuilder"_ustr;
inline constexpr OUString DBREPORTHEADER = u"ReportHeader"_ustr;
inline constexpr OUString DBREPORTFOOTER = u"ReportFooter"_ustr;
inline constexpr OUString DBPAGEHEADER = u"PageHeader"_ustr;
inline constexpr OUString DBPAGEFOOTER = u"PageFooter"_ustr;
inline constexpr OUString DBGROUPHEADER = u"GroupHeader"_ustr;
inline constexpr OUString DBGROUPFOOTER = u"GroupFooter"_ustr;
inline constexpr OUString DBDETAIL = u"Detail"_ustr;
inline constexpr OUString REPORTCONTROLFORMAT = u"ReportControlFormat"_ustr;
inline constexpr OUString CURRENT_WINDOW = u"CurrentWindow"_ustr;
inline constexpr OUString PROPERTY_FONTCOLOR = u"FontColor"_ustr;
inline constexpr OUString PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull"_ustr;
inline constexpr OUString PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal"_ustr;
inline constexpr OUString PROPERTY_POSITION = u"Position"_ustr;
inline constexpr OUString PROPERTY_FORMATKEYDATE = u"FormatKeyDate"_ustr;
inline constexpr OUString PROPERTY_FORMATKEYTIME = u"FormatKeyTime"_ustr;
inline constexpr OUString DBOVERLAPPEDCONTROL = u"OverlappedControl"_ustr;
inline constexpr OUString PROPERTY_FORMULALIST = u"FormulaList"_ustr;
inline constexpr OUString PROPERTY_SCOPE = u"Scope"_ustr;
inline constexpr OUString PROPERTY_CHARTTYPE = u"ChartType"_ustr;
inline constexpr OUString PROPERTY_PREVIEW_COUNT = u"RowLimit"_ustr;
inline constexpr OUString PROPERTY_TITLE = u"Title"_ustr;
inline constexpr OUString PROPERTY_AREA = u"Area"_ustr;
inline constexpr OUString PROPERTY_FILLCOLOR = u"FillColor"_ustr;
inline constexpr OUString DBTEXTBOXBOUNDCONTENT = u"TextBoxBoundContent"_ustr;

inline constexpr OUString SERVICE_SETTINGSIMPORTER = u"com.sun.star.comp.Report.XMLOasisSettingsImporter"_ustr;
inline constexpr OUString SERVICE_STYLESIMPORTER = u"com.sun.star.comp.Report.XMLOasisStylesImporter"_ustr;
inline constexpr OUString SERVICE_CONTENTIMPORTER = u"com.sun.star.comp.Report.XMLOasisContentImporter"_ustr;
inline constexpr OUString SERVICE_METAIMPORTER = u"com.sun.star.comp.Report.XMLOasisMetaImporter"_ustr;
inline constexpr OUString PROPERTY_BORDERLEFT = u"BorderLeft"_ustr;
inline constexpr OUString PROPERTY_BORDERRIGHT = u"BorderRight"_ustr;
inline constexpr OUString PROPERTY_BORDERTOP = u"BorderTop"_ustr;
inline constexpr OUString PROPERTY_BORDERBOTTOM = u"BorderBottom"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
