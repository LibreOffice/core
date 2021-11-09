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

inline constexpr OUStringLiteral RID_STR_CLASS_FIXEDTEXT = u"Label";
inline constexpr OUStringLiteral RID_STR_CLASS_FIXEDLINE = u"Line";
inline constexpr OUStringLiteral RID_STR_CLASS_IMAGECONTROL = u"Graphic";
inline constexpr OUStringLiteral RID_STR_CLASS_FORMATTEDFIELD = u"FormattedField";

//= service names

inline constexpr OUStringLiteral SERVICE_FIXEDTEXT = u"com.sun.star.report.FixedText";
inline constexpr OUStringLiteral SERVICE_FORMATTEDFIELD = u"com.sun.star.report.FormattedField";
inline constexpr OUStringLiteral SERVICE_IMAGECONTROL = u"com.sun.star.report.ImageControl";
inline constexpr OUStringLiteral SERVICE_FORMATCONDITION = u"com.sun.star.report.FormatCondition";
inline constexpr OUStringLiteral SERVICE_FUNCTION = u"com.sun.star.report.Function";
inline constexpr OUStringLiteral SERVICE_REPORTDEFINITION = u"com.sun.star.report.ReportDefinition";
inline constexpr OUStringLiteral SERVICE_SHAPE = u"com.sun.star.report.Shape";
inline constexpr OUStringLiteral SERVICE_FIXEDLINE = u"com.sun.star.report.FixedLine";
inline constexpr OUStringLiteral SERVICE_SECTION = u"com.sun.star.report.Section";
inline constexpr OUStringLiteral SERVICE_GROUP = u"com.sun.star.report.Group";

//= property names

inline constexpr OUStringLiteral PROPERTY_VISIBLE = u"Visible";
inline constexpr OUStringLiteral PROPERTY_NAME = u"Name";
inline constexpr OUStringLiteral PROPERTY_HEIGHT = u"Height";
inline constexpr OUStringLiteral PROPERTY_BACKCOLOR = u"BackColor";
inline constexpr OUStringLiteral PROPERTY_BACKTRANSPARENT = u"BackTransparent";
inline constexpr OUStringLiteral PROPERTY_CONTROLBACKGROUND = u"ControlBackground";
inline constexpr OUStringLiteral PROPERTY_CONTROLBACKGROUNDTRANSPARENT = u"ControlBackgroundTransparent";
inline constexpr OUStringLiteral PROPERTY_FORCENEWPAGE = u"ForceNewPage";
inline constexpr OUStringLiteral PROPERTY_NEWROWORCOL = u"NewRowOrCol";
inline constexpr OUStringLiteral PROPERTY_KEEPTOGETHER = u"KeepTogether";
inline constexpr OUStringLiteral PROPERTY_CANGROW = u"CanGrow";
inline constexpr OUStringLiteral PROPERTY_CANSHRINK = u"CanShrink";
inline constexpr OUStringLiteral PROPERTY_REPEATSECTION = u"RepeatSection";
inline constexpr OUStringLiteral PROPERTY_GROUP = u"Group";
inline constexpr OUStringLiteral PROPERTY_REPORTDEFINITION = u"ReportDefinition";

inline constexpr OUStringLiteral PROPERTY_GROUPINTERVAL = u"GroupInterval";
inline constexpr OUStringLiteral PROPERTY_EXPRESSION = u"Expression";
inline constexpr OUStringLiteral PROPERTY_GROUPON = u"GroupOn";
inline constexpr OUStringLiteral PROPERTY_SORTASCENDING = u"SortAscending";

inline constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
inline constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
inline constexpr OUStringLiteral PROPERTY_CAPTION = u"Caption";
inline constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
inline constexpr OUStringLiteral PROPERTY_BACKGRAPHICLOCATION = u"BackGraphicLocation";
inline constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
inline constexpr OUStringLiteral PROPERTY_PAPERSIZE = u"Size";
inline constexpr OUStringLiteral PROPERTY_GROUPKEEPTOGETHER = u"GroupKeepTogether";
inline constexpr OUStringLiteral PROPERTY_PAGEHEADEROPTION = u"PageHeaderOption";
inline constexpr OUStringLiteral PROPERTY_PAGEFOOTEROPTION = u"PageFooterOption";
inline constexpr OUStringLiteral PROPERTY_COMMANDTYPE = u"CommandType";
inline constexpr OUStringLiteral PROPERTY_REPORTHEADERON = u"ReportHeaderOn";
inline constexpr OUStringLiteral PROPERTY_REPORTFOOTERON = u"ReportFooterOn";
inline constexpr OUStringLiteral PROPERTY_PAGEHEADERON = u"PageHeaderOn";
inline constexpr OUStringLiteral PROPERTY_PAGEFOOTERON = u"PageFooterOn";
inline constexpr OUStringLiteral PROPERTY_HEADERON = u"HeaderOn";
inline constexpr OUStringLiteral PROPERTY_FOOTERON = u"FooterOn";
inline constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
inline constexpr OUStringLiteral PROPERTY_POSITIONX = u"PositionX";
inline constexpr OUStringLiteral PROPERTY_POSITIONY = u"PositionY";
inline constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
inline constexpr OUStringLiteral PROPERTY_MINHEIGHT = u"MinHeight";
inline constexpr OUStringLiteral PROPERTY_DATAFIELD = u"DataField";
inline constexpr OUStringLiteral PROPERTY_PARAADJUST = u"ParaAdjust";
inline constexpr OUStringLiteral PROPERTY_FONTDESCRIPTOR = u"FontDescriptor";
inline constexpr OUStringLiteral PROPERTY_FONTDESCRIPTORASIAN = u"FontDescriptorAsian";
inline constexpr OUStringLiteral PROPERTY_FONTDESCRIPTORCOMPLEX = u"FontDescriptorComplex";
inline constexpr OUStringLiteral PROPERTY_CONTROLTEXTEMPHASISMARK = u"ControlTextEmphasis";
inline constexpr OUStringLiteral PROPERTY_CHARRELIEF = u"CharRelief";
inline constexpr OUStringLiteral PROPERTY_CHARCOLOR = u"CharColor";
inline constexpr OUStringLiteral PROPERTY_VERTICALALIGN = u"VerticalAlign";
#define PROPERTY_IMAGEPOSITION "ImagePosition"
inline constexpr OUStringLiteral PROPERTY_IMAGEURL = u"ImageURL";
inline constexpr OUStringLiteral PROPERTY_CHARUNDERLINECOLOR = u"CharUnderlineColor";
inline constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
#define PROPERTY_EFFECTIVEDEFAULT "EffectiveDefault"
#define PROPERTY_EFFECTIVEMAX "EffectiveMax"
#define PROPERTY_EFFECTIVEMIN "EffectiveMin"
inline constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
#define PROPERTY_MAXTEXTLEN "MaxTextLen"
#define PROPERTY_LISTSOURCE "ListSource"
inline constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
inline constexpr OUStringLiteral PROPERTY_CONTROLBORDER = u"ControlBorder";
inline constexpr OUStringLiteral PROPERTY_CONTROLBORDERCOLOR = u"ControlBorderColor";
inline constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
inline constexpr OUStringLiteral PROPERTY_BORDERCOLOR = u"BorderColor";
inline constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";

inline constexpr OUStringLiteral PROPERTY_LEFTMARGIN = u"LeftMargin";
inline constexpr OUStringLiteral PROPERTY_RIGHTMARGIN = u"RightMargin";
inline constexpr OUStringLiteral PROPERTY_TOPMARGIN = u"TopMargin";
inline constexpr OUStringLiteral PROPERTY_BOTTOMMARGIN = u"BottomMargin";

inline constexpr OUStringLiteral PROPERTY_PRINTREPEATEDVALUES = u"PrintRepeatedValues";
inline constexpr OUStringLiteral PROPERTY_CONDITIONALPRINTEXPRESSION = u"ConditionalPrintExpression";
inline constexpr OUStringLiteral PROPERTY_STARTNEWCOLUMN = u"StartNewColumn";
inline constexpr OUStringLiteral PROPERTY_RESETPAGENUMBER = u"ResetPageNumber";
inline constexpr OUStringLiteral PROPERTY_PRINTWHENGROUPCHANGE = u"PrintWhenGroupChange";
inline constexpr OUStringLiteral PROPERTY_STATE = u"State";
inline constexpr OUStringLiteral PROPERTY_TIME_STATE = u"TimeState";
inline constexpr OUStringLiteral PROPERTY_DATE_STATE = u"DateState";
#define PROPERTY_TRISTATE "TriState"
#define PROPERTY_LISTSOURCETYPE "ListSourceType"
inline constexpr OUStringLiteral PROPERTY_FONTCHARWIDTH = u"FontCharWidth";
inline constexpr OUStringLiteral PROPERTY_FONTCHARSET = u"CharFontCharSet";
inline constexpr OUStringLiteral PROPERTY_FONTFAMILY = u"CharFontFamily";
inline constexpr OUStringLiteral PROPERTY_CHARFONTHEIGHT = u"CharHeight";
inline constexpr OUStringLiteral PROPERTY_FONTHEIGHT = u"FontHeight";
inline constexpr OUStringLiteral PROPERTY_FONTKERNING = u"FontKerning";
inline constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
inline constexpr OUStringLiteral PROPERTY_FONTNAME = u"FontName";
inline constexpr OUStringLiteral PROPERTY_FONTORIENTATION = u"CharRotation";
inline constexpr OUStringLiteral PROPERTY_FONTPITCH = u"CharFontPitch";
#define PROPERTY_FONTSLANT        "FontSlant"
inline constexpr OUStringLiteral PROPERTY_CHARSTRIKEOUT = u"CharStrikeout";
inline constexpr OUStringLiteral PROPERTY_FONTSTRIKEOUT = u"FontStrikeout";
inline constexpr OUStringLiteral PROPERTY_FONTSTYLENAME = u"CharFontStyleName";
inline constexpr OUStringLiteral PROPERTY_FONTUNDERLINE = u"CharUnderline";
inline constexpr OUStringLiteral PROPERTY_FONTWEIGHT = u"CharWeight";
inline constexpr OUStringLiteral PROPERTY_FONTWIDTH = u"FontWidth";
#define PROPERTY_FONTWORDLINEMODE "FontWordLineMode"
inline constexpr OUStringLiteral PROPERTY_FONTTYPE = u"FontType";

inline constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";

inline constexpr OUStringLiteral PROPERTY_CHAREMPHASIS = u"CharEmphasis";
inline constexpr OUStringLiteral PROPERTY_CHARFONTNAME = u"CharFontName";
inline constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAME = u"CharFontStyleName";
inline constexpr OUStringLiteral PROPERTY_CHARFONTFAMILY = u"CharFontFamily";
inline constexpr OUStringLiteral PROPERTY_CHARFONTCHARSET = u"CharFontCharSet";
inline constexpr OUStringLiteral PROPERTY_CHARFONTPITCH = u"CharFontPitch";
inline constexpr OUStringLiteral PROPERTY_CHARHEIGHT = u"CharHeight";
inline constexpr OUStringLiteral PROPERTY_CHARUNDERLINE = u"CharUnderline";
inline constexpr OUStringLiteral PROPERTY_CHARWEIGHT = u"CharWeight";
inline constexpr OUStringLiteral PROPERTY_CHARPOSTURE = u"CharPosture";
inline constexpr OUStringLiteral PROPERTY_CHARWORDMODE = u"CharWordMode";
inline constexpr OUStringLiteral PROPERTY_CHARROTATION = u"CharRotation";
inline constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTH = u"CharScaleWidth";

// Asian
inline constexpr OUStringLiteral PROPERTY_CHAREMPHASISASIAN = u"CharEmphasisAsian";
inline constexpr OUStringLiteral PROPERTY_CHARFONTNAMEASIAN = u"CharFontNameAsian";
inline constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAMEASIAN = u"CharFontStyleNameAsian";
inline constexpr OUStringLiteral PROPERTY_CHARFONTFAMILYASIAN = u"CharFontFamilyAsian";
inline constexpr OUStringLiteral PROPERTY_CHARFONTCHARSETASIAN = u"CharFontCharSetAsian";
inline constexpr OUStringLiteral PROPERTY_CHARFONTPITCHASIAN = u"CharFontPitchAsian";
inline constexpr OUStringLiteral PROPERTY_CHARHEIGHTASIAN = u"CharHeightAsian";
inline constexpr OUStringLiteral PROPERTY_CHARUNDERLINEASIAN = u"CharUnderlineAsian";
inline constexpr OUStringLiteral PROPERTY_CHARWEIGHTASIAN = u"CharWeightAsian";
inline constexpr OUStringLiteral PROPERTY_CHARPOSTUREASIAN = u"CharPostureAsian";
inline constexpr OUStringLiteral PROPERTY_CHARWORDMODEASIAN = u"CharWordModeAsian";
inline constexpr OUStringLiteral PROPERTY_CHARROTATIONASIAN = u"CharRotationAsian";
inline constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTHASIAN = u"CharScaleWidthAsian";
inline constexpr OUStringLiteral PROPERTY_CHARLOCALEASIAN = u"CharLocaleAsian";

// Complex
inline constexpr OUStringLiteral PROPERTY_CHAREMPHASISCOMPLEX = u"CharEmphasisComplex";
inline constexpr OUStringLiteral PROPERTY_CHARFONTNAMECOMPLEX = u"CharFontNameComplex";
inline constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAMECOMPLEX = u"CharFontStyleNameComplex";
inline constexpr OUStringLiteral PROPERTY_CHARFONTFAMILYCOMPLEX = u"CharFontFamilyComplex";
inline constexpr OUStringLiteral PROPERTY_CHARFONTCHARSETCOMPLEX = u"CharFontCharSetComplex";
inline constexpr OUStringLiteral PROPERTY_CHARFONTPITCHCOMPLEX = u"CharFontPitchComplex";
inline constexpr OUStringLiteral PROPERTY_CHARHEIGHTCOMPLEX = u"CharHeightComplex";
inline constexpr OUStringLiteral PROPERTY_CHARUNDERLINECOMPLEX = u"CharUnderlineComplex";
inline constexpr OUStringLiteral PROPERTY_CHARWEIGHTCOMPLEX = u"CharWeightComplex";
inline constexpr OUStringLiteral PROPERTY_CHARPOSTURECOMPLEX = u"CharPostureComplex";
inline constexpr OUStringLiteral PROPERTY_CHARWORDMODECOMPLEX = u"CharWordModeComplex";
inline constexpr OUStringLiteral PROPERTY_CHARROTATIONCOMPLEX = u"CharRotationComplex";
inline constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTHCOMPLEX = u"CharScaleWidthComplex";
inline constexpr OUStringLiteral PROPERTY_CHARLOCALECOMPLEX = u"CharLocaleComplex";

inline constexpr OUStringLiteral PROPERTY_STATUSINDICATOR = u"StatusIndicator";
inline constexpr OUStringLiteral PROPERTY_SECTION = u"Section";
inline constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
inline constexpr OUStringLiteral PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing";

inline constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
inline constexpr OUStringLiteral PROPERTY_ACTIVECONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
inline constexpr OUStringLiteral PROPERTY_FORMULA = u"Formula";
inline constexpr OUStringLiteral PROPERTY_INITIALFORMULA = u"InitialFormula";
inline constexpr OUStringLiteral PROPERTY_PREEVALUATED = u"PreEvaluated";
inline constexpr OUStringLiteral PROPERTY_DEEPTRAVERSING = u"DeepTraversing";
inline constexpr OUStringLiteral PROPERTY_MIMETYPE = u"MimeType";
inline constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor";
inline constexpr OUStringLiteral PROPERTY_TEXT = u"Text";
inline constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
inline constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
inline constexpr OUStringLiteral PROPERTY_FONTRELIEF = u"FontRelief";
inline constexpr OUStringLiteral PROPERTY_FONTEMPHASISMARK = u"FontEmphasisMark";
inline constexpr OUStringLiteral PROPERTY_ZORDER = u"ZOrder";
inline constexpr OUStringLiteral PROPERTY_OPAQUE = u"Opaque";
inline constexpr OUStringLiteral PROPERTY_TRANSFORMATION = u"Transformation";
inline constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEENGINE = u"CustomShapeEngine";
inline constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEDATA = u"CustomShapeData";
inline constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEGEOMETRY = u"CustomShapeGeometry";

inline constexpr OUStringLiteral PROPERTY_NUMBERINGTYPE = u"NumberingType";
inline constexpr OUStringLiteral PROPERTY_PAGESTYLELAYOUT = u"PageStyleLayout";
inline constexpr OUStringLiteral PROPERTY_ISLANDSCAPE = u"IsLandscape";
inline constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
inline constexpr OUStringLiteral PROPERTY_TYPE = u"Type";

inline constexpr OUStringLiteral PROPERTY_PRESERVEIRI = u"PreserveIRI";
inline constexpr OUStringLiteral PROPERTY_SCALEMODE = u"ScaleMode";

inline constexpr OUStringLiteral PROPERTY_LINESTYLE = u"LineStyle";
inline constexpr OUStringLiteral PROPERTY_LINEDASH = u"LineDash";
inline constexpr OUStringLiteral PROPERTY_LINECOLOR = u"LineColor";
inline constexpr OUStringLiteral PROPERTY_LINETRANSPARENCE = u"LineTransparence";
inline constexpr OUStringLiteral PROPERTY_LINEWIDTH = u"LineWidth";

inline constexpr OUStringLiteral PROPERTY_CHARFLASH = u"CharFlash";
inline constexpr OUStringLiteral PROPERTY_CHARESCAPEMENTHEIGHT = u"CharEscapementHeight";
inline constexpr OUStringLiteral PROPERTY_CHARLOCALE = u"CharLocale";
inline constexpr OUStringLiteral PROPERTY_CHARESCAPEMENT = u"CharEscapement";
inline constexpr OUStringLiteral PROPERTY_CHARCASEMAP = u"CharCaseMap";
inline constexpr OUStringLiteral PROPERTY_CHARCOMBINEISON = u"CharCombineIsOn";
inline constexpr OUStringLiteral PROPERTY_CHARCOMBINEPREFIX = u"CharCombinePrefix";
inline constexpr OUStringLiteral PROPERTY_CHARCOMBINESUFFIX = u"CharCombineSuffix";
inline constexpr OUStringLiteral PROPERTY_CHARHIDDEN = u"CharHidden";
inline constexpr OUStringLiteral PROPERTY_CHARSHADOWED = u"CharShadowed";
inline constexpr OUStringLiteral PROPERTY_CHARCONTOURED = u"CharContoured";
inline constexpr OUStringLiteral PROPERTY_CHARAUTOKERNING = u"CharAutoKerning";
inline constexpr OUStringLiteral PROPERTY_CHARKERNING = u"CharKerning";
inline constexpr OUStringLiteral PROPERTY_HYPERLINKURL = u"HyperLinkURL";
inline constexpr OUStringLiteral PROPERTY_HYPERLINKTARGET = u"HyperLinkTarget";
inline constexpr OUStringLiteral PROPERTY_HYPERLINKNAME = u"HyperLinkName";
inline constexpr OUStringLiteral PROPERTY_VISITEDCHARSTYLENAME = u"VisitedCharStyleName";
inline constexpr OUStringLiteral PROPERTY_UNVISITEDCHARSTYLENAME = u"UnvisitedCharStyleName";
#define PROPERTY_ORDER                "Order"
inline constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
inline constexpr OUStringLiteral PROPERTY_MAXROWS = u"MaxRows";

inline constexpr OUStringLiteral PROPERTY_REPORTNAME = u"ReportName";
inline constexpr OUStringLiteral CFG_REPORTDESIGNER = u"SunReportBuilder";
inline constexpr OUStringLiteral DBREPORTHEADER = u"ReportHeader";
inline constexpr OUStringLiteral DBREPORTFOOTER = u"ReportFooter";
inline constexpr OUStringLiteral DBPAGEHEADER = u"PageHeader";
inline constexpr OUStringLiteral DBPAGEFOOTER = u"PageFooter";
inline constexpr OUStringLiteral DBGROUPHEADER = u"GroupHeader";
inline constexpr OUStringLiteral DBGROUPFOOTER = u"GroupFooter";
inline constexpr OUStringLiteral DBDETAIL = u"Detail";
inline constexpr OUStringLiteral REPORTCONTROLFORMAT = u"ReportControlFormat";
inline constexpr OUStringLiteral CURRENT_WINDOW = u"CurrentWindow";
inline constexpr OUStringLiteral PROPERTY_FONTCOLOR = u"FontColor";
inline constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
inline constexpr OUStringLiteral PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal";
inline constexpr OUStringLiteral PROPERTY_POSITION = u"Position";
inline constexpr OUStringLiteral PROPERTY_FORMATKEYDATE = u"FormatKeyDate";
inline constexpr OUStringLiteral PROPERTY_FORMATKEYTIME = u"FormatKeyTime";
inline constexpr OUStringLiteral DBOVERLAPPEDCONTROL = u"OverlappedControl";
inline constexpr OUStringLiteral PROPERTY_FORMULALIST = u"FormulaList";
inline constexpr OUStringLiteral PROPERTY_SCOPE = u"Scope";
inline constexpr OUStringLiteral PROPERTY_CHARTTYPE = u"ChartType";
inline constexpr OUStringLiteral PROPERTY_PREVIEW_COUNT = u"RowLimit";
inline constexpr OUStringLiteral PROPERTY_TITLE = u"Title";
inline constexpr OUStringLiteral PROPERTY_AREA = u"Area";
inline constexpr OUStringLiteral PROPERTY_FILLCOLOR = u"FillColor";
inline constexpr OUStringLiteral DBTEXTBOXBOUNDCONTENT = u"TextBoxBoundContent";

inline constexpr OUStringLiteral SERVICE_SETTINGSIMPORTER = u"com.sun.star.comp.Report.XMLOasisSettingsImporter";
inline constexpr OUStringLiteral SERVICE_STYLESIMPORTER = u"com.sun.star.comp.Report.XMLOasisStylesImporter";
inline constexpr OUStringLiteral SERVICE_CONTENTIMPORTER = u"com.sun.star.comp.Report.XMLOasisContentImporter";
inline constexpr OUStringLiteral SERVICE_METAIMPORTER = u"com.sun.star.comp.Report.XMLOasisMetaImporter";
inline constexpr OUStringLiteral PROPERTY_BORDER_LEFT = u"BorderLeft";
inline constexpr OUStringLiteral PROPERTY_BORDER_RIGHT = u"BorderRight";
inline constexpr OUStringLiteral PROPERTY_BORDER_TOP = u"BorderTop";
inline constexpr OUStringLiteral PROPERTY_BORDER_BOTTOM = u"BorderBottom";

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
