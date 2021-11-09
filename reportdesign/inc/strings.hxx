/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_REPORTDESIGN_INC_STRINGS_HXX
#define INCLUDED_REPORTDESIGN_INC_STRINGS_HXX

// Dialog Controls

#define RID_STR_CLASS_FIXEDTEXT         "Label"
#define RID_STR_CLASS_FIXEDLINE         "Line"
#define RID_STR_CLASS_IMAGECONTROL      "Graphic"
#define RID_STR_CLASS_FORMATTEDFIELD    "FormattedField"

//= service names

constexpr OUStringLiteral SERVICE_FIXEDTEXT = u"com.sun.star.report.FixedText";
constexpr OUStringLiteral SERVICE_FORMATTEDFIELD = u"com.sun.star.report.FormattedField";
constexpr OUStringLiteral SERVICE_IMAGECONTROL = u"com.sun.star.report.ImageControl";
constexpr OUStringLiteral SERVICE_FORMATCONDITION = u"com.sun.star.report.FormatCondition";
constexpr OUStringLiteral SERVICE_FUNCTION = u"com.sun.star.report.Function";
constexpr OUStringLiteral SERVICE_REPORTDEFINITION = u"com.sun.star.report.ReportDefinition";
constexpr OUStringLiteral SERVICE_SHAPE = u"com.sun.star.report.Shape";
constexpr OUStringLiteral SERVICE_FIXEDLINE = u"com.sun.star.report.FixedLine";
constexpr OUStringLiteral SERVICE_SECTION = u"com.sun.star.report.Section";
constexpr OUStringLiteral SERVICE_GROUP = u"com.sun.star.report.Group";

//= property names

constexpr OUStringLiteral PROPERTY_VISIBLE = u"Visible";
constexpr OUStringLiteral PROPERTY_NAME = u"Name";
constexpr OUStringLiteral PROPERTY_HEIGHT = u"Height";
constexpr OUStringLiteral PROPERTY_BACKCOLOR = u"BackColor";
constexpr OUStringLiteral PROPERTY_BACKTRANSPARENT = u"BackTransparent";
constexpr OUStringLiteral PROPERTY_CONTROLBACKGROUND = u"ControlBackground";
constexpr OUStringLiteral PROPERTY_CONTROLBACKGROUNDTRANSPARENT = u"ControlBackgroundTransparent";
constexpr OUStringLiteral PROPERTY_FORCENEWPAGE = u"ForceNewPage";
constexpr OUStringLiteral PROPERTY_NEWROWORCOL = u"NewRowOrCol";
constexpr OUStringLiteral PROPERTY_KEEPTOGETHER = u"KeepTogether";
constexpr OUStringLiteral PROPERTY_CANGROW = u"CanGrow";
constexpr OUStringLiteral PROPERTY_CANSHRINK = u"CanShrink";
constexpr OUStringLiteral PROPERTY_REPEATSECTION = u"RepeatSection";
constexpr OUStringLiteral PROPERTY_GROUP = u"Group";
constexpr OUStringLiteral PROPERTY_REPORTDEFINITION = u"ReportDefinition";

constexpr OUStringLiteral PROPERTY_GROUPINTERVAL = u"GroupInterval";
constexpr OUStringLiteral PROPERTY_EXPRESSION = u"Expression";
constexpr OUStringLiteral PROPERTY_GROUPON = u"GroupOn";
constexpr OUStringLiteral PROPERTY_SORTASCENDING = u"SortAscending";

constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
constexpr OUStringLiteral PROPERTY_CAPTION = u"Caption";
constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
constexpr OUStringLiteral PROPERTY_BACKGRAPHICLOCATION = u"BackGraphicLocation";
constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
constexpr OUStringLiteral PROPERTY_PAPERSIZE = u"Size";
constexpr OUStringLiteral PROPERTY_GROUPKEEPTOGETHER = u"GroupKeepTogether";
constexpr OUStringLiteral PROPERTY_PAGEHEADEROPTION = u"PageHeaderOption";
constexpr OUStringLiteral PROPERTY_PAGEFOOTEROPTION = u"PageFooterOption";
constexpr OUStringLiteral PROPERTY_COMMANDTYPE = u"CommandType";
constexpr OUStringLiteral PROPERTY_REPORTHEADERON = u"ReportHeaderOn";
constexpr OUStringLiteral PROPERTY_REPORTFOOTERON = u"ReportFooterOn";
constexpr OUStringLiteral PROPERTY_PAGEHEADERON = u"PageHeaderOn";
constexpr OUStringLiteral PROPERTY_PAGEFOOTERON = u"PageFooterOn";
constexpr OUStringLiteral PROPERTY_HEADERON = u"HeaderOn";
constexpr OUStringLiteral PROPERTY_FOOTERON = u"FooterOn";
constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
constexpr OUStringLiteral PROPERTY_POSITIONX = u"PositionX";
constexpr OUStringLiteral PROPERTY_POSITIONY = u"PositionY";
constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
constexpr OUStringLiteral PROPERTY_MINHEIGHT = u"MinHeight";
constexpr OUStringLiteral PROPERTY_DATAFIELD = u"DataField";
constexpr OUStringLiteral PROPERTY_PARAADJUST = u"ParaAdjust";
constexpr OUStringLiteral PROPERTY_FONTDESCRIPTOR = u"FontDescriptor";
constexpr OUStringLiteral PROPERTY_FONTDESCRIPTORASIAN = u"FontDescriptorAsian";
constexpr OUStringLiteral PROPERTY_FONTDESCRIPTORCOMPLEX = u"FontDescriptorComplex";
constexpr OUStringLiteral PROPERTY_CONTROLTEXTEMPHASISMARK = u"ControlTextEmphasis";
constexpr OUStringLiteral PROPERTY_CHARRELIEF = u"CharRelief";
constexpr OUStringLiteral PROPERTY_CHARCOLOR = u"CharColor";
constexpr OUStringLiteral PROPERTY_VERTICALALIGN = u"VerticalAlign";
#define PROPERTY_IMAGEPOSITION "ImagePosition"
constexpr OUStringLiteral PROPERTY_IMAGEURL = u"ImageURL";
constexpr OUStringLiteral PROPERTY_CHARUNDERLINECOLOR = u"CharUnderlineColor";
constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
#define PROPERTY_EFFECTIVEDEFAULT "EffectiveDefault"
#define PROPERTY_EFFECTIVEMAX "EffectiveMax"
#define PROPERTY_EFFECTIVEMIN "EffectiveMin"
constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
#define PROPERTY_MAXTEXTLEN "MaxTextLen"
#define PROPERTY_LISTSOURCE "ListSource"
constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
constexpr OUStringLiteral PROPERTY_CONTROLBORDER = u"ControlBorder";
constexpr OUStringLiteral PROPERTY_CONTROLBORDERCOLOR = u"ControlBorderColor";
constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
#define PROPERTY_BORDERCOLOR "BorderColor"
constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";

constexpr OUStringLiteral PROPERTY_LEFTMARGIN = u"LeftMargin";
constexpr OUStringLiteral PROPERTY_RIGHTMARGIN = u"RightMargin";
constexpr OUStringLiteral PROPERTY_TOPMARGIN = u"TopMargin";
constexpr OUStringLiteral PROPERTY_BOTTOMMARGIN = u"BottomMargin";

constexpr OUStringLiteral PROPERTY_PRINTREPEATEDVALUES = u"PrintRepeatedValues";
constexpr OUStringLiteral PROPERTY_CONDITIONALPRINTEXPRESSION = u"ConditionalPrintExpression";
constexpr OUStringLiteral PROPERTY_STARTNEWCOLUMN = u"StartNewColumn";
constexpr OUStringLiteral PROPERTY_RESETPAGENUMBER = u"ResetPageNumber";
constexpr OUStringLiteral PROPERTY_PRINTWHENGROUPCHANGE = u"PrintWhenGroupChange";
constexpr OUStringLiteral PROPERTY_STATE = u"State";
constexpr OUStringLiteral PROPERTY_TIME_STATE = u"TimeState";
constexpr OUStringLiteral PROPERTY_DATE_STATE = u"DateState";
#define PROPERTY_TRISTATE "TriState"
#define PROPERTY_LISTSOURCETYPE "ListSourceType"
constexpr OUStringLiteral PROPERTY_FONTCHARWIDTH = u"FontCharWidth";
constexpr OUStringLiteral PROPERTY_FONTCHARSET = u"CharFontCharSet";
constexpr OUStringLiteral PROPERTY_FONTFAMILY = u"CharFontFamily";
constexpr OUStringLiteral PROPERTY_CHARFONTHEIGHT = u"CharHeight";
#define PROPERTY_FONTHEIGHT       "FontHeight"
constexpr OUStringLiteral PROPERTY_FONTKERNING = u"FontKerning";
constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
constexpr OUStringLiteral PROPERTY_FONTNAME = u"FontName";
constexpr OUStringLiteral PROPERTY_FONTORIENTATION = u"CharRotation";
constexpr OUStringLiteral PROPERTY_FONTPITCH = u"CharFontPitch";
#define PROPERTY_FONTSLANT        "FontSlant"
constexpr OUStringLiteral PROPERTY_CHARSTRIKEOUT = u"CharStrikeout";
#define PROPERTY_FONTSTRIKEOUT    "FontStrikeout"
constexpr OUStringLiteral PROPERTY_FONTSTYLENAME = u"CharFontStyleName";
constexpr OUStringLiteral PROPERTY_FONTUNDERLINE = u"CharUnderline";
constexpr OUStringLiteral PROPERTY_FONTWEIGHT = u"CharWeight";
constexpr OUStringLiteral PROPERTY_FONTWIDTH = u"FontWidth";
#define PROPERTY_FONTWORDLINEMODE "FontWordLineMode"
constexpr OUStringLiteral PROPERTY_FONTTYPE = u"FontType";

constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";

constexpr OUStringLiteral PROPERTY_CHAREMPHASIS = u"CharEmphasis";
constexpr OUStringLiteral PROPERTY_CHARFONTNAME = u"CharFontName";
constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAME = u"CharFontStyleName";
constexpr OUStringLiteral PROPERTY_CHARFONTFAMILY = u"CharFontFamily";
constexpr OUStringLiteral PROPERTY_CHARFONTCHARSET = u"CharFontCharSet";
constexpr OUStringLiteral PROPERTY_CHARFONTPITCH = u"CharFontPitch";
constexpr OUStringLiteral PROPERTY_CHARHEIGHT = u"CharHeight";
constexpr OUStringLiteral PROPERTY_CHARUNDERLINE = u"CharUnderline";
constexpr OUStringLiteral PROPERTY_CHARWEIGHT = u"CharWeight";
constexpr OUStringLiteral PROPERTY_CHARPOSTURE = u"CharPosture";
constexpr OUStringLiteral PROPERTY_CHARWORDMODE = u"CharWordMode";
constexpr OUStringLiteral PROPERTY_CHARROTATION = u"CharRotation";
constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTH = u"CharScaleWidth";

// Asian
constexpr OUStringLiteral PROPERTY_CHAREMPHASISASIAN = u"CharEmphasisAsian";
constexpr OUStringLiteral PROPERTY_CHARFONTNAMEASIAN = u"CharFontNameAsian";
constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAMEASIAN = u"CharFontStyleNameAsian";
constexpr OUStringLiteral PROPERTY_CHARFONTFAMILYASIAN = u"CharFontFamilyAsian";
constexpr OUStringLiteral PROPERTY_CHARFONTCHARSETASIAN = u"CharFontCharSetAsian";
constexpr OUStringLiteral PROPERTY_CHARFONTPITCHASIAN = u"CharFontPitchAsian";
constexpr OUStringLiteral PROPERTY_CHARHEIGHTASIAN = u"CharHeightAsian";
constexpr OUStringLiteral PROPERTY_CHARUNDERLINEASIAN = u"CharUnderlineAsian";
constexpr OUStringLiteral PROPERTY_CHARWEIGHTASIAN = u"CharWeightAsian";
constexpr OUStringLiteral PROPERTY_CHARPOSTUREASIAN = u"CharPostureAsian";
constexpr OUStringLiteral PROPERTY_CHARWORDMODEASIAN = u"CharWordModeAsian";
constexpr OUStringLiteral PROPERTY_CHARROTATIONASIAN = u"CharRotationAsian";
constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTHASIAN = u"CharScaleWidthAsian";
constexpr OUStringLiteral PROPERTY_CHARLOCALEASIAN = u"CharLocaleAsian";

// Complex
constexpr OUStringLiteral PROPERTY_CHAREMPHASISCOMPLEX = u"CharEmphasisComplex";
constexpr OUStringLiteral PROPERTY_CHARFONTNAMECOMPLEX = u"CharFontNameComplex";
constexpr OUStringLiteral PROPERTY_CHARFONTSTYLENAMECOMPLEX = u"CharFontStyleNameComplex";
constexpr OUStringLiteral PROPERTY_CHARFONTFAMILYCOMPLEX = u"CharFontFamilyComplex";
constexpr OUStringLiteral PROPERTY_CHARFONTCHARSETCOMPLEX = u"CharFontCharSetComplex";
constexpr OUStringLiteral PROPERTY_CHARFONTPITCHCOMPLEX = u"CharFontPitchComplex";
constexpr OUStringLiteral PROPERTY_CHARHEIGHTCOMPLEX = u"CharHeightComplex";
constexpr OUStringLiteral PROPERTY_CHARUNDERLINECOMPLEX = u"CharUnderlineComplex";
constexpr OUStringLiteral PROPERTY_CHARWEIGHTCOMPLEX = u"CharWeightComplex";
constexpr OUStringLiteral PROPERTY_CHARPOSTURECOMPLEX = u"CharPostureComplex";
constexpr OUStringLiteral PROPERTY_CHARWORDMODECOMPLEX = u"CharWordModeComplex";
constexpr OUStringLiteral PROPERTY_CHARROTATIONCOMPLEX = u"CharRotationComplex";
constexpr OUStringLiteral PROPERTY_CHARSCALEWIDTHCOMPLEX = u"CharScaleWidthComplex";
constexpr OUStringLiteral PROPERTY_CHARLOCALECOMPLEX = u"CharLocaleComplex";

constexpr OUStringLiteral PROPERTY_STATUSINDICATOR = u"StatusIndicator";
constexpr OUStringLiteral PROPERTY_SECTION = u"Section";
constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
constexpr OUStringLiteral PROPERTY_ESCAPEPROCESSING = u"EscapeProcessing";

constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
constexpr OUStringLiteral PROPERTY_ACTIVECONNECTION = u"ActiveConnection";
constexpr OUStringLiteral PROPERTY_DATASOURCENAME = u"DataSourceName";
constexpr OUStringLiteral PROPERTY_FORMULA = u"Formula";
constexpr OUStringLiteral PROPERTY_INITIALFORMULA = u"InitialFormula";
constexpr OUStringLiteral PROPERTY_PREEVALUATED = u"PreEvaluated";
constexpr OUStringLiteral PROPERTY_DEEPTRAVERSING = u"DeepTraversing";
constexpr OUStringLiteral PROPERTY_MIMETYPE = u"MimeType";
constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor";
constexpr OUStringLiteral PROPERTY_TEXT = u"Text";
constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
#define PROPERTY_TEXTLINECOLOR   "TextLineColor"
#define PROPERTY_FONTRELIEF   "FontRelief"
#define PROPERTY_FONTEMPHASISMARK "FontEmphasisMark"
constexpr OUStringLiteral PROPERTY_ZORDER = u"ZOrder";
constexpr OUStringLiteral PROPERTY_OPAQUE = u"Opaque";
constexpr OUStringLiteral PROPERTY_TRANSFORMATION = u"Transformation";
constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEENGINE = u"CustomShapeEngine";
constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEDATA = u"CustomShapeData";
constexpr OUStringLiteral PROPERTY_CUSTOMSHAPEGEOMETRY = u"CustomShapeGeometry";

constexpr OUStringLiteral PROPERTY_NUMBERINGTYPE = u"NumberingType";
constexpr OUStringLiteral PROPERTY_PAGESTYLELAYOUT = u"PageStyleLayout";
constexpr OUStringLiteral PROPERTY_ISLANDSCAPE = u"IsLandscape";
#define PROPERTY_ALIGN "Align"
constexpr OUStringLiteral PROPERTY_TYPE = u"Type";

constexpr OUStringLiteral PROPERTY_PRESERVEIRI = u"PreserveIRI";
constexpr OUStringLiteral PROPERTY_SCALEMODE = u"ScaleMode";

constexpr OUStringLiteral PROPERTY_LINESTYLE = u"LineStyle";
constexpr OUStringLiteral PROPERTY_LINEDASH = u"LineDash";
constexpr OUStringLiteral PROPERTY_LINECOLOR = u"LineColor";
constexpr OUStringLiteral PROPERTY_LINETRANSPARENCE = u"LineTransparence";
constexpr OUStringLiteral PROPERTY_LINEWIDTH = u"LineWidth";

constexpr OUStringLiteral PROPERTY_CHARFLASH = u"CharFlash";
constexpr OUStringLiteral PROPERTY_CHARESCAPEMENTHEIGHT = u"CharEscapementHeight";
constexpr OUStringLiteral PROPERTY_CHARLOCALE = u"CharLocale";
constexpr OUStringLiteral PROPERTY_CHARESCAPEMENT = u"CharEscapement";
constexpr OUStringLiteral PROPERTY_CHARCASEMAP = u"CharCaseMap";
constexpr OUStringLiteral PROPERTY_CHARCOMBINEISON = u"CharCombineIsOn";
constexpr OUStringLiteral PROPERTY_CHARCOMBINEPREFIX = u"CharCombinePrefix";
constexpr OUStringLiteral PROPERTY_CHARCOMBINESUFFIX = u"CharCombineSuffix";
constexpr OUStringLiteral PROPERTY_CHARHIDDEN = u"CharHidden";
constexpr OUStringLiteral PROPERTY_CHARSHADOWED = u"CharShadowed";
constexpr OUStringLiteral PROPERTY_CHARCONTOURED = u"CharContoured";
constexpr OUStringLiteral PROPERTY_CHARAUTOKERNING = u"CharAutoKerning";
constexpr OUStringLiteral PROPERTY_CHARKERNING = u"CharKerning";
constexpr OUStringLiteral PROPERTY_HYPERLINKURL = u"HyperLinkURL";
constexpr OUStringLiteral PROPERTY_HYPERLINKTARGET = u"HyperLinkTarget";
constexpr OUStringLiteral PROPERTY_HYPERLINKNAME = u"HyperLinkName";
constexpr OUStringLiteral PROPERTY_VISITEDCHARSTYLENAME = u"VisitedCharStyleName";
constexpr OUStringLiteral PROPERTY_UNVISITEDCHARSTYLENAME = u"UnvisitedCharStyleName";
#define PROPERTY_ORDER                "Order"
constexpr OUStringLiteral PROPERTY_APPLYFILTER = u"ApplyFilter";
constexpr OUStringLiteral PROPERTY_MAXROWS = u"MaxRows";

constexpr OUStringLiteral PROPERTY_REPORTNAME = u"ReportName";
constexpr OUStringLiteral CFG_REPORTDESIGNER = u"SunReportBuilder";
constexpr OUStringLiteral DBREPORTHEADER = u"ReportHeader";
constexpr OUStringLiteral DBREPORTFOOTER = u"ReportFooter";
constexpr OUStringLiteral DBPAGEHEADER = u"PageHeader";
constexpr OUStringLiteral DBPAGEFOOTER = u"PageFooter";
constexpr OUStringLiteral DBGROUPHEADER = u"GroupHeader";
constexpr OUStringLiteral DBGROUPFOOTER = u"GroupFooter";
constexpr OUStringLiteral DBDETAIL = u"Detail";
constexpr OUStringLiteral REPORTCONTROLFORMAT = u"ReportControlFormat";
constexpr OUStringLiteral CURRENT_WINDOW = u"CurrentWindow";
constexpr OUStringLiteral PROPERTY_FONTCOLOR = u"FontColor";
#define PROPERTY_EMPTY_IS_NULL "ConvertEmptyToNull"
#define PROPERTY_FILTERPROPOSAL "UseFilterValueProposal"
constexpr OUStringLiteral PROPERTY_POSITION = u"Position";
constexpr OUStringLiteral PROPERTY_FORMATKEYDATE = u"FormatKeyDate";
constexpr OUStringLiteral PROPERTY_FORMATKEYTIME = u"FormatKeyTime";
constexpr OUStringLiteral DBOVERLAPPEDCONTROL = u"OverlappedControl";
constexpr OUStringLiteral PROPERTY_FORMULALIST = u"FormulaList";
constexpr OUStringLiteral PROPERTY_SCOPE = u"Scope";
constexpr OUStringLiteral PROPERTY_CHARTTYPE = u"ChartType";
constexpr OUStringLiteral PROPERTY_PREVIEW_COUNT = u"RowLimit";
constexpr OUStringLiteral PROPERTY_TITLE = u"Title";
constexpr OUStringLiteral PROPERTY_AREA = u"Area";
constexpr OUStringLiteral PROPERTY_FILLCOLOR = u"FillColor";
constexpr OUStringLiteral DBTEXTBOXBOUNDCONTENT = u"TextBoxBoundContent";

constexpr OUStringLiteral SERVICE_SETTINGSIMPORTER = u"com.sun.star.comp.Report.XMLOasisSettingsImporter";
constexpr OUStringLiteral SERVICE_STYLESIMPORTER = u"com.sun.star.comp.Report.XMLOasisStylesImporter";
constexpr OUStringLiteral SERVICE_CONTENTIMPORTER = u"com.sun.star.comp.Report.XMLOasisContentImporter";
#define SERVICE_IMPORTFILTER "com.sun.star.document.ImportFilter"
constexpr OUStringLiteral SERVICE_METAIMPORTER = u"com.sun.star.comp.Report.XMLOasisMetaImporter";
constexpr OUStringLiteral PROPERTY_BORDERLEFT = u"BorderLeft";
constexpr OUStringLiteral PROPERTY_BORDERRIGHT = u"BorderRight";
constexpr OUStringLiteral PROPERTY_BORDERTOP = u"BorderTop";
constexpr OUStringLiteral PROPERTY_BORDERBOTTOM = u"BorderBottom";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
