/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlchart.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:03:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLCHART_HXX
#define SC_XLCHART_HXX

#include <map>

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace lang { class XMultiServiceFactory; }
} } }

// Property names =============================================================

#define SERVICE_CHART_LINE                  CREATE_OUSTRING( "com.sun.star.chart.LineDiagram" )
#define SERVICE_CHART_AREA                  CREATE_OUSTRING( "com.sun.star.chart.AreaDiagram" )
#define SERVICE_CHART_BAR                   CREATE_OUSTRING( "com.sun.star.chart.BarDiagram" )
#define SERVICE_CHART_PIE                   CREATE_OUSTRING( "com.sun.star.chart.PieDiagram" )
#define SERVICE_CHART_DONUT                 CREATE_OUSTRING( "com.sun.star.chart.DonutDiagram" )
#define SERVICE_CHART_NET                   CREATE_OUSTRING( "com.sun.star.chart.NetDiagram" )
#define SERVICE_CHART_XY                    CREATE_OUSTRING( "com.sun.star.chart.XYDiagram" )
#define SERVICE_CHART_STOCK                 CREATE_OUSTRING( "com.sun.star.chart.StockDiagram" )

#define EXC_CHPROP_ALIGNMENT                CREATE_OUSTRING( "Alignment" )
#define EXC_CHPROP_ATTRIBUTEDDP             CREATE_OUSTRING( "AttributedDataPoints" )
#define EXC_CHPROP_AUTOMAX                  CREATE_OUSTRING( "AutoMax" )
#define EXC_CHPROP_AUTOMIN                  CREATE_OUSTRING( "AutoMin" )
#define EXC_CHPROP_AUTOORIG                 CREATE_OUSTRING( "AutoOrigin" )
#define EXC_CHPROP_AUTOSTHELP               CREATE_OUSTRING( "AutoStepHelp" )
#define EXC_CHPROP_AUTOSTMAIN               CREATE_OUSTRING( "AutoStepMain" )
#define EXC_CHPROP_AXIS                     CREATE_OUSTRING( "Axis" )
#define EXC_CHPROP_CHARCOLOR                CREATE_OUSTRING( "CharColor" )
#define EXC_CHPROP_CHARCONTOURED            CREATE_OUSTRING( "CharContoured" )
#define EXC_CHPROP_CHARCROSSEDOUT           CREATE_OUSTRING( "CharCrossedOut" )
#define EXC_CHPROP_CHARFONTCHARSET          CREATE_OUSTRING( "CharFontCharSet" )
#define EXC_CHPROP_CHARFONTFAMILY           CREATE_OUSTRING( "CharFontFamily" )
#define EXC_CHPROP_CHARFONTNAME             CREATE_OUSTRING( "CharFontName" )
#define EXC_CHPROP_CHARHEIGHT               CREATE_OUSTRING( "CharHeight" )
#define EXC_CHPROP_CHARPOSTURE              CREATE_OUSTRING( "CharPosture" )
#define EXC_CHPROP_CHARSHADOWED             CREATE_OUSTRING( "CharShadowed" )
#define EXC_CHPROP_CHARUNDERL               CREATE_OUSTRING( "CharUnderline" )
#define EXC_CHPROP_CHARWEIGHT               CREATE_OUSTRING( "CharWeight" )
#define EXC_CHPROP_CONSTANTERRORHIGH        CREATE_OUSTRING( "ConstantErrorHigh" )
#define EXC_CHPROP_CONSTANTERRORLOW         CREATE_OUSTRING( "ConstantErrorLow" )
#define EXC_CHPROP_DATACAPTION              CREATE_OUSTRING( "DataCaption" )
#define EXC_CHPROP_DATAERRORPROPS           CREATE_OUSTRING( "DataErrorProperties" )
#define EXC_CHPROP_DATAREGRESSIONPROPS      CREATE_OUSTRING( "DataRegressionProperties" )
#define EXC_CHPROP_DATAROWSOURCE            CREATE_OUSTRING( "DataRowSource" )
#define EXC_CHPROP_DEEP                     CREATE_OUSTRING( "Deep" )
#define EXC_CHPROP_DIM3D                    CREATE_OUSTRING( "Dim3D" )
#define EXC_CHPROP_ERRORCATEGORY            CREATE_OUSTRING( "ErrorCategory" )
#define EXC_CHPROP_ERRORINDICATOR           CREATE_OUSTRING( "ErrorIndicator" )
#define EXC_CHPROP_FILLCOLOR                CREATE_OUSTRING( "FillColor" )
#define EXC_CHPROP_FILLSTYLE                CREATE_OUSTRING( "FillStyle" )
#define EXC_CHPROP_GAPWIDTH                 CREATE_OUSTRING( "GapWidth" )
#define EXC_CHPROP_HASLEGEND                CREATE_OUSTRING( "HasLegend" )
#define EXC_CHPROP_HASMAINTIT               CREATE_OUSTRING( "HasMainTitle" )
#define EXC_CHPROP_HASSECYAXIS              CREATE_OUSTRING( "HasSecondaryYAxis" )
#define EXC_CHPROP_HASSECYAXISDESCR         CREATE_OUSTRING( "HasSecondaryYAxisDescription" )
#define EXC_CHPROP_HASXAXIS                 CREATE_OUSTRING( "HasXAxis" )
#define EXC_CHPROP_HASXAXISDESCR            CREATE_OUSTRING( "HasXAxisDescription" )
#define EXC_CHPROP_HASXAXISHELPGRID         CREATE_OUSTRING( "HasXAxisHelpGrid" )
#define EXC_CHPROP_HASXAXISGRID             CREATE_OUSTRING( "HasXAxisGrid" )
#define EXC_CHPROP_HASXAXISTIT              CREATE_OUSTRING( "HasXAxisTitle" )
#define EXC_CHPROP_HASYAXIS                 CREATE_OUSTRING( "HasYAxis" )
#define EXC_CHPROP_HASYAXISDESCR            CREATE_OUSTRING( "HasYAxisDescription" )
#define EXC_CHPROP_HASYAXISHELPGRID         CREATE_OUSTRING( "HasYAxisHelpGrid" )
#define EXC_CHPROP_HASYAXISGRID             CREATE_OUSTRING( "HasYAxisGrid" )
#define EXC_CHPROP_HASYAXISTIT              CREATE_OUSTRING( "HasYAxisTitle" )
#define EXC_CHPROP_HASZAXIS                 CREATE_OUSTRING( "HasZAxis" )
#define EXC_CHPROP_HASZAXISDESCR            CREATE_OUSTRING( "HasZAxisDescription" )
#define EXC_CHPROP_HASZAXISHELPGRID         CREATE_OUSTRING( "HasZAxisHelpGrid" )
#define EXC_CHPROP_HASZAXISGRID             CREATE_OUSTRING( "HasZAxisGrid" )
#define EXC_CHPROP_HASZAXISTIT              CREATE_OUSTRING( "HasZAxisTitle" )
#define EXC_CHPROP_HELPMARKS                CREATE_OUSTRING( "HelpMarks" )
#define EXC_CHPROP_LINECOLOR                CREATE_OUSTRING( "LineColor" )
#define EXC_CHPROP_LINEDASH                 CREATE_OUSTRING( "LineDash" )
#define EXC_CHPROP_LINES                    CREATE_OUSTRING( "Lines" )
#define EXC_CHPROP_LINESTYLE                CREATE_OUSTRING( "LineStyle" )
#define EXC_CHPROP_LINETRANSPARENCE         CREATE_OUSTRING( "LineTransparence" )
#define EXC_CHPROP_LINEWIDTH                CREATE_OUSTRING( "LineWidth" )
#define EXC_CHPROP_LINKNUMFMT               CREATE_OUSTRING( "LinkNumberFormatToSource" )
#define EXC_CHPROP_LOG                      CREATE_OUSTRING( "Logarithmic" )
#define EXC_CHPROP_MARKS                    CREATE_OUSTRING( "Marks" )
#define EXC_CHPROP_MAX                      CREATE_OUSTRING( "Max" )
#define EXC_CHPROP_MIN                      CREATE_OUSTRING( "Min" )
#define EXC_CHPROP_NUMFMT                   CREATE_OUSTRING( "NumberFormat" )
#define EXC_CHPROP_NULLDATE                 CREATE_OUSTRING( "NullDate" )
#define EXC_CHPROP_ORIGIN                   CREATE_OUSTRING( "Origin" )
#define EXC_CHPROP_OVERLAP                  CREATE_OUSTRING( "Overlap" )
#define EXC_CHPROP_PERCENT                  CREATE_OUSTRING( "Percent" )
#define EXC_CHPROP_PERCENTAGEERROR          CREATE_OUSTRING( "PercentageError" )
#define EXC_CHPROP_REGRESSIONCURVES         CREATE_OUSTRING( "RegressionCurves" )
#define EXC_CHPROP_SEGMENTOFFSET            CREATE_OUSTRING( "SegmentOffset" )
#define EXC_CHPROP_SOLIDTYPE                CREATE_OUSTRING( "SolidType" )
#define EXC_CHPROP_SPLINETYPE               CREATE_OUSTRING( "SplineType" )
#define EXC_CHPROP_STACKED                  CREATE_OUSTRING( "Stacked" )
#define EXC_CHPROP_STEPHELP                 CREATE_OUSTRING( "StepHelp" )
#define EXC_CHPROP_STEPMAIN                 CREATE_OUSTRING( "StepMain" )
#define EXC_CHPROP_STRING                   CREATE_OUSTRING( "String" )
#define EXC_CHPROP_SYMBOLSIZE               CREATE_OUSTRING( "SymbolSize" )
#define EXC_CHPROP_SYMBOLTYPE               CREATE_OUSTRING( "SymbolType" )
#define EXC_CHPROP_TEXTBREAK                CREATE_OUSTRING( "TextBreak" )
#define EXC_CHPROP_TEXTOVERLAP              CREATE_OUSTRING( "TextCanOverlap" )
#define EXC_CHPROP_TEXTROTATION             CREATE_OUSTRING( "TextRotation" )
#define EXC_CHPROP_TRANSLATEDCOLS           CREATE_OUSTRING( "TranslatedColumns" )
#define EXC_CHPROP_TRANSLATEDROWS           CREATE_OUSTRING( "TranslatedRows" )
#define EXC_CHPROP_UPDOWN                   CREATE_OUSTRING( "UpDown" )
#define EXC_CHPROP_VERTICAL                 CREATE_OUSTRING( "Vertical" )
#define EXC_CHPROP_VOLUME                   CREATE_OUSTRING( "Volume" )

// service names
#define SERVICE_DRAWING_BITMAPTABLE         CREATE_OUSTRING( "com.sun.star.drawing.BitmapTable" )
#define SERVICE_DRAWING_DASHTABLE           CREATE_OUSTRING( "com.sun.star.drawing.DashTable" )
#define SERVICE_DRAWING_GRADIENTTABLE       CREATE_OUSTRING( "com.sun.star.drawing.GradientTable" )

// Constants and Enumerations =================================================

const sal_Size EXC_CHART_PROGRESS_SIZE          = 10;
const sal_uInt16 EXC_CHART_AUTOROTATION         = 0xFFFF;   /// Automatic rotation, e.g. axis labels.

// ----------------------------------------------------------------------------

/** Enumerates possible orientations for a source range. */
enum XclChOrientation
{
    EXC_CHORIENT_EMPTY,             /// No linked cell found, or invalid link type.
    EXC_CHORIENT_SINGLE,            /// Single cell linked.
    EXC_CHORIENT_VERTICAL,          /// Vertical (all cells in one column).
    EXC_CHORIENT_HORIZONTAL,        /// Horizontal (all cells in one row).
    EXC_CHORIENT_COMPLEX            /// Range too complex (multiple rows/columns).
};

// (0x1002) CHCHART -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHART                 = 0x1002;

// (0x1003) CHSERIES ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERIES                = 0x1003;

const sal_uInt16 EXC_CHSERIES_DATE              = 0;
const sal_uInt16 EXC_CHSERIES_NUMERIC           = 1;
const sal_uInt16 EXC_CHSERIES_SEQUENCE          = 2;
const sal_uInt16 EXC_CHSERIES_TEXT              = 3;

const sal_uInt16 EXC_CHSERIES_MAXSERIES         = 255;      /// Maximum valid series index.
const sal_uInt16 EXC_CHSERIES_INVALID           = 0xFFFF;   /// Invalid series index (for internal use).

// (0x1006) CHDATAFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHDATAFORMAT            = 0x1006;

const sal_uInt16 EXC_CHDATAFORMAT_MAXPOINT      = 31999;    /// Maximum valid point index.
const sal_uInt16 EXC_CHDATAFORMAT_DEFAULT       = 0xFFFD;   /// As format index: global default for an axes set.
const sal_uInt16 EXC_CHDATAFORMAT_UNKNOWN       = 0xFFFE;   /// As point index: unknown format, don't use.
const sal_uInt16 EXC_CHDATAFORMAT_ALLPOINTS     = 0xFFFF;   /// As point index: default for a series.

const sal_uInt16 EXC_CHDATAFORMAT_OLDCOLORS     = 0x0001;

// (0x1007) CHLINEFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHLINEFORMAT            = 0x1007;

const sal_uInt16 EXC_CHLINEFORMAT_SOLID         = 0;
const sal_uInt16 EXC_CHLINEFORMAT_DASH          = 1;
const sal_uInt16 EXC_CHLINEFORMAT_DOT           = 2;
const sal_uInt16 EXC_CHLINEFORMAT_DASHDOT       = 3;
const sal_uInt16 EXC_CHLINEFORMAT_DASHDOTDOT    = 4;
const sal_uInt16 EXC_CHLINEFORMAT_NONE          = 5;
const sal_uInt16 EXC_CHLINEFORMAT_DARKTRANS     = 6;
const sal_uInt16 EXC_CHLINEFORMAT_MEDTRANS      = 7;
const sal_uInt16 EXC_CHLINEFORMAT_LIGHTTRANS    = 8;

const sal_Int16 EXC_CHLINEFORMAT_HAIR           = -1;
const sal_Int16 EXC_CHLINEFORMAT_SINGLE         = 0;
const sal_Int16 EXC_CHLINEFORMAT_DOUBLE         = 1;
const sal_Int16 EXC_CHLINEFORMAT_TRIPLE         = 2;

const sal_uInt16 EXC_CHLINEFORMAT_AUTO          = 0x0001;
const sal_uInt16 EXC_CHLINEFORMAT_SHOWAXIS      = 0x0004;

// (0x1009) CHMARKERFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHMARKERFORMAT          = 0x1009;

const sal_uInt16 EXC_CHMARKERFORMAT_NOSYMBOL    = 0;
const sal_uInt16 EXC_CHMARKERFORMAT_SQUARE      = 1;
const sal_uInt16 EXC_CHMARKERFORMAT_DIAMOND     = 2;
const sal_uInt16 EXC_CHMARKERFORMAT_TRIANGLE    = 3;
const sal_uInt16 EXC_CHMARKERFORMAT_CROSS       = 4;
const sal_uInt16 EXC_CHMARKERFORMAT_STAR        = 5;
const sal_uInt16 EXC_CHMARKERFORMAT_DOWJ        = 6;
const sal_uInt16 EXC_CHMARKERFORMAT_STDDEV      = 7;
const sal_uInt16 EXC_CHMARKERFORMAT_CIRCLE      = 8;
const sal_uInt16 EXC_CHMARKERFORMAT_PLUS        = 9;

const sal_uInt16 EXC_CHMARKERFORMAT_SYMBOLCOUNT = 9;

const sal_uInt32 EXC_CHMARKERFORMAT_HAIRSIZE    = 60;
const sal_uInt32 EXC_CHMARKERFORMAT_SINGLESIZE  = 100;
const sal_uInt32 EXC_CHMARKERFORMAT_DOUBLESIZE  = 140;
const sal_uInt32 EXC_CHMARKERFORMAT_TRIPLESIZE  = 180;

const sal_uInt16 EXC_CHMARKERFORMAT_AUTO        = 0x0001;
const sal_uInt16 EXC_CHMARKERFORMAT_NOFILL      = 0x0010;
const sal_uInt16 EXC_CHMARKERFORMAT_NOLINE      = 0x0020;

// (0x100A) CHAREAFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHAREAFORMAT            = 0x100A;

const sal_uInt16 EXC_CHAREAFORMAT_NONE          = 0;
const sal_uInt16 EXC_CHAREAFORMAT_SOLID         = 1;

const sal_uInt16 EXC_CHAREAFORMAT_AUTO          = 0x0001;
const sal_uInt16 EXC_CHAREAFORMAT_INVERTNEG     = 0x0002;

// (0x100B) CHPIEFORMAT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIEFORMAT             = 0x100B;

// (0x100C) CHATTACHEDLABEL ---------------------------------------------------

const sal_uInt16 EXC_ID_CHATTACHEDLABEL         = 0x100C;

const sal_uInt16 EXC_CHATTLABEL_SHOWVALUE       = 0x0001;
const sal_uInt16 EXC_CHATTLABEL_SHOWPERCENT     = 0x0002;
const sal_uInt16 EXC_CHATTLABEL_SHOWCATEGPERC   = 0x0004;
const sal_uInt16 EXC_CHATTLABEL_SMOOTHED        = 0x0008;   /// Smoothed line.
const sal_uInt16 EXC_CHATTLABEL_SHOWCATEG       = 0x0010;
const sal_uInt16 EXC_CHATTLABEL_SHOWBUBBLE      = 0x0020;

// (0x100D) CHSTRING ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHSTRING                = 0x100D;

// (0x1014) CHCHARTGROUP ------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHARTGROUP            = 0x1014;

const sal_uInt16 EXC_CHCHARTGROUP_VARIED        = 0x0001;   /// Varied colors for points.

// (0x1015) CHLEGEND ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHLEGEND                = 0x1015;

const sal_uInt8 EXC_CHLEGEND_BOTTOM             = 0;
const sal_uInt8 EXC_CHLEGEND_CORNER             = 1;
const sal_uInt8 EXC_CHLEGEND_TOP                = 2;
const sal_uInt8 EXC_CHLEGEND_RIGHT              = 3;
const sal_uInt8 EXC_CHLEGEND_LEFT               = 4;
const sal_uInt8 EXC_CHLEGEND_NOTDOCKED          = 7;

const sal_uInt8 EXC_CHLEGEND_CLOSE              = 0;
const sal_uInt8 EXC_CHLEGEND_MEDIUM             = 1;
const sal_uInt8 EXC_CHLEGEND_OPEN               = 2;

const sal_uInt16 EXC_CHLEGEND_DOCKED            = 0x0001;
const sal_uInt16 EXC_CHLEGEND_AUTOSERIES        = 0x0002;
const sal_uInt16 EXC_CHLEGEND_AUTOPOSX          = 0x0004;
const sal_uInt16 EXC_CHLEGEND_AUTOPOSY          = 0x0008;
const sal_uInt16 EXC_CHLEGEND_STACKED           = 0x0010;
const sal_uInt16 EXC_CHLEGEND_DATATABLE         = 0x0020;

// (0x1017) CHBAR, CHCOLUMN ---------------------------------------------------

const sal_uInt16 EXC_ID_CHBAR                   = 0x1017;
const sal_uInt16 EXC_ID_CHCOLUMN                = 0xFF17;   /// Column chart - for internal use only.

const sal_uInt16 EXC_CHBAR_HORIZONTAL           = 0x0001;
const sal_uInt16 EXC_CHBAR_STACKED              = 0x0002;
const sal_uInt16 EXC_CHBAR_PERCENT              = 0x0004;
const sal_uInt16 EXC_CHBAR_SHADOW               = 0x0008;

// (0x1018, 0x101A) CHLINE, CHAREA --------------------------------------------

const sal_uInt16 EXC_ID_CHLINE                  = 0x1018;
const sal_uInt16 EXC_ID_CHAREA                  = 0x101A;
const sal_uInt16 EXC_ID_CHSTOCK                 = 0xFF18;   /// Stock chart - for internal use only.

const sal_uInt16 EXC_CHLINE_STACKED             = 0x0001;
const sal_uInt16 EXC_CHLINE_PERCENT             = 0x0002;
const sal_uInt16 EXC_CHLINE_SHADOW              = 0x0004;

// (0x1019) CHPIE -------------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIE                   = 0x1019;
const sal_uInt16 EXC_ID_CHDONUT                 = 0xFF19;   /// Donut chart - for internal use only.

const sal_uInt16 EXC_CHPIE_SHADOW               = 0x0001;
const sal_uInt16 EXC_CHPIE_LINES                = 0x0002;

// (0x101B) CHSCATTER ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHSCATTER               = 0x101B;
const sal_uInt16 EXC_ID_CHBUBBLES               = 0xFF1B;   /// Bubble chart - for internal use only.

const sal_uInt16 EXC_CHSCATTER_AREA             = 1;        /// Bubble area refers to value.
const sal_uInt16 EXC_CHSCATTER_WIDTH            = 2;        /// Bubble width refers to value.

const sal_uInt16 EXC_CHSCATTER_BUBBLES          = 0x0001;
const sal_uInt16 EXC_CHSCATTER_SHOWNEG          = 0x0002;
const sal_uInt16 EXC_CHSCATTER_SHADOW           = 0x0004;

// (0x001C) CHCHARTLINE -------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHARTLINE             = 0x101C;

const sal_uInt16 EXC_CHCHARTLINE_DROP           = 0;        /// Drop lines.
const sal_uInt16 EXC_CHCHARTLINE_HILO           = 1;        /// Hi-lo lines.
const sal_uInt16 EXC_CHCHARTLINE_SERIES         = 2;        /// Series lines.

// (0x101D) CHAXIS ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXIS                  = 0x101D;

const sal_uInt16 EXC_CHAXIS_X                   = 0;
const sal_uInt16 EXC_CHAXIS_Y                   = 1;
const sal_uInt16 EXC_CHAXIS_Z                   = 2;

// (0x101E) CHTICK ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHTICK                  = 0x101E;

const sal_uInt8 EXC_CHTICK_INSIDE               = 0x01;
const sal_uInt8 EXC_CHTICK_OUTSIDE              = 0x02;

const sal_uInt8 EXC_CHTICK_NOLABEL              = 0;
const sal_uInt8 EXC_CHTICK_LOW                  = 1;        /// Below diagram/right of diagram.
const sal_uInt8 EXC_CHTICK_HIGH                 = 2;        /// Above diagram/left of diagram.
const sal_uInt8 EXC_CHTICK_NEXT                 = 3;        /// Next to axis.

const sal_uInt8 EXC_CHTICK_TRANSPARENT          = 1;
const sal_uInt8 EXC_CHTICK_OPAQUE               = 2;

const sal_uInt16 EXC_CHTICK_AUTOCOLOR           = 0x0001;
const sal_uInt16 EXC_CHTICK_AUTOFILL            = 0x0002;
const sal_uInt16 EXC_CHTICK_AUTOROT             = 0x0020;

// (0x101F) CHVALUERANGE ------------------------------------------------------

const sal_uInt16 EXC_ID_CHVALUERANGE            = 0x101F;

const sal_uInt16 EXC_CHVALUERANGE_AUTOMIN       = 0x0001;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMAX       = 0x0002;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMAJOR     = 0x0004;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMINOR     = 0x0008;
const sal_uInt16 EXC_CHVALUERANGE_AUTOCROSS     = 0x0010;
const sal_uInt16 EXC_CHVALUERANGE_LOGSCALE      = 0x0020;
const sal_uInt16 EXC_CHVALUERANGE_REVERSE       = 0x0040;   /// Axis direction reversed.
const sal_uInt16 EXC_CHVALUERANGE_MAXCROSS      = 0x0080;   /// Other axis crosses at own maximum.
const sal_uInt16 EXC_CHVALUERANGE_BIT8          = 0x0100;   /// This bit is always set in BIFF5+.

// (0x1020) CHLABELRANGE -----------------------------------------------------

const sal_uInt16 EXC_ID_CHLABELRANGE            = 0x1020;

const sal_uInt16 EXC_CHLABELRANGE_BETWEEN       = 0x0001;   /// Axis between categories.
const sal_uInt16 EXC_CHLABELRANGE_MAXCROSS      = 0x0002;   /// Other axis crosses at own maximum.
const sal_uInt16 EXC_CHLABELRANGE_REVERSE       = 0x0004;   /// Axis direction reversed.

// (0x1021) CHAXISLINE --------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXISLINE              = 0x1021;

const sal_uInt16 EXC_CHAXISLINE_AXISLINE        = 0;        /// Axis line itself.
const sal_uInt16 EXC_CHAXISLINE_MAJORGRID       = 1;        /// Major grid line.
const sal_uInt16 EXC_CHAXISLINE_MINORGRID       = 2;        /// Minor grid line.
const sal_uInt16 EXC_CHAXISLINE_WALLS           = 3;        /// Walls (X, Z axis), floor (Y axis).

// (0x1024) CHDEFAULTTEXT -----------------------------------------------------

const sal_uInt16 EXC_ID_CHDEFAULTTEXT           = 0x1024;

const sal_uInt16 EXC_CHDEFTEXT_TEXTLABEL        = 0;        /// Default for text data labels (not used?).
const sal_uInt16 EXC_CHDEFTEXT_NUMLABEL         = 1;        /// Default for numeric data labels (not used?).
const sal_uInt16 EXC_CHDEFTEXT_TITLE            = 2;        /// Default text for chart title.
const sal_uInt16 EXC_CHDEFTEXT_LEGEND           = 2;        /// Default text for legend.
const sal_uInt16 EXC_CHDEFTEXT_AXIS             = 3;        /// Default text for axis title and labels.
const sal_uInt16 EXC_CHDEFTEXT_LABEL            = 3;        /// Default text for data point labels.
const sal_uInt16 EXC_CHDEFTEXT_NONE             = 0xFFFF;   /// No default text available.

// (0x1025) CHTEXT ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHTEXT                  = 0x1025;

const sal_uInt8 EXC_CHTEXT_ALIGN_TOPLEFT        = 1;        /// Horizontal: left, vertical: top.
const sal_uInt8 EXC_CHTEXT_ALIGN_CENTER         = 2;
const sal_uInt8 EXC_CHTEXT_ALIGN_BOTTOMRIGHT    = 3;        /// Horizontal: right, vertical: bottom.
const sal_uInt8 EXC_CHTEXT_ALIGN_JUSTIFY        = 4;
const sal_uInt8 EXC_CHTEXT_ALIGN_DISTRIBUTE     = 5;

const sal_uInt16 EXC_CHTEXT_TRANSPARENT         = 1;
const sal_uInt16 EXC_CHTEXT_OPAQUE              = 2;

const sal_uInt16 EXC_CHTEXT_AUTOCOLOR           = 0x0001;
const sal_uInt16 EXC_CHTEXT_SHOWSYMBOL          = 0x0002;   /// Legend symbol for data point caption.
const sal_uInt16 EXC_CHTEXT_SHOWVALUE           = 0x0004;   /// Data point caption is the value.
const sal_uInt16 EXC_CHTEXT_VERTICAL            = 0x0008;
const sal_uInt16 EXC_CHTEXT_STORED              = 0x0010;
const sal_uInt16 EXC_CHTEXT_GENERATED           = 0x0020;
const sal_uInt16 EXC_CHTEXT_DELETED             = 0x0040;   /// Text object is removed.
const sal_uInt16 EXC_CHTEXT_AUTOMODE            = 0x0080;
const sal_uInt16 EXC_CHTEXT_SHOWCATEGPERC       = 0x0800;   /// Data point caption is category and percent.
const sal_uInt16 EXC_CHTEXT_SHOWPERCENT         = 0x1000;   /// Data point caption as percent.
const sal_uInt16 EXC_CHTEXT_SHOWBUBBLE          = 0x2000;   /// Show bubble size.
const sal_uInt16 EXC_CHTEXT_SHOWCATEG           = 0x4000;   /// Data point caption is category name.

const sal_uInt16 EXC_CHTEXT_POS_DEFAULT         = 0;
const sal_uInt16 EXC_CHTEXT_POS_OUTSIDE         = 1;
const sal_uInt16 EXC_CHTEXT_POS_INSIDE          = 2;
const sal_uInt16 EXC_CHTEXT_POS_CENTER          = 3;
const sal_uInt16 EXC_CHTEXT_POS_AXIS            = 4;
const sal_uInt16 EXC_CHTEXT_POS_ABOVE           = 5;
const sal_uInt16 EXC_CHTEXT_POS_BELOW           = 6;
const sal_uInt16 EXC_CHTEXT_POS_LEFT            = 7;
const sal_uInt16 EXC_CHTEXT_POS_RIGHT           = 8;
const sal_uInt16 EXC_CHTEXT_POS_AUTO            = 9;
const sal_uInt16 EXC_CHTEXT_POS_MOVED           = 10;

// (0x1026) CHFONT ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHFONT                  = 0x1026;

// (0x1027) CHOBJECTLINK ------------------------------------------------------

const sal_uInt16 EXC_ID_CHOBJECTLINK            = 0x1027;

// link targets
const sal_uInt16 EXC_CHOBJLINK_NONE             = 0;        /// No link target.
const sal_uInt16 EXC_CHOBJLINK_TITLE            = 1;        /// Chart title.
const sal_uInt16 EXC_CHOBJLINK_YAXIS            = 2;        /// Value axis (Y axis).
const sal_uInt16 EXC_CHOBJLINK_XAXIS            = 3;        /// Category axis (X axis).
const sal_uInt16 EXC_CHOBJLINK_DATA             = 4;        /// Data series/point.
const sal_uInt16 EXC_CHOBJLINK_ZAXIS            = 7;        /// Series axis (Z axis).
const sal_uInt16 EXC_CHOBJLINK_AXISUNIT         = 12;       /// Unit name for axis labels.
const sal_uInt16 EXC_CHOBJLINK_BACKGROUND       = 0xFF00;   /// Chart background (internal use only).
const sal_uInt16 EXC_CHOBJLINK_DIAGRAM          = 0xFF01;   /// Diagram wall/floor (internal use only).
const sal_uInt16 EXC_CHOBJLINK_LEGEND           = 0xFF02;   /// Legend object (internal use only).

// (0x1032) CHFRAME -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHFRAME                 = 0x1032;

const sal_uInt16 EXC_CHFRAME_STANDARD           = 0;
const sal_uInt16 EXC_CHFRAME_SHADOW             = 4;

const sal_uInt16 EXC_CHFRAME_AUTOSIZE           = 0x0001;
const sal_uInt16 EXC_CHFRAME_AUTOPOS            = 0x0002;

// (0x1033, 0x1034) CHBEGIN, CHEND --------------------------------------------

const sal_uInt16 EXC_ID_CHBEGIN                 = 0x1033;
const sal_uInt16 EXC_ID_CHEND                   = 0x1034;

// (0x1035) CHPLOTFRAME -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPLOTFRAME             = 0x1035;

// (0x103A) CHCHART3D ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHART3D               = 0x103A;

const sal_uInt16 EXC_CHCHART3D_PERSP            = 0x0001;
const sal_uInt16 EXC_CHCHART3D_CLUSTER          = 0x0002;   /// false = Z axis, true = clustered/stacked.
const sal_uInt16 EXC_CHCHART3D_AUTOSCALE        = 0x0004;
const sal_uInt16 EXC_CHCHART3D_BIT4             = 0x0010;   /// This bit is always set in BIFF5+.
const sal_uInt16 EXC_CHCHART3D_2DWALLS          = 0x0020;

// (0x103C) CHPICFORMAT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPICFORMAT             = 0x103C;

const sal_uInt16 EXC_CHPICFORMAT_STRETCH        = 1;        /// Bitmap stretched to area.
const sal_uInt16 EXC_CHPICFORMAT_STACK          = 2;        /// Bitmap stacked.
const sal_uInt16 EXC_CHPICFORMAT_SCALE          = 3;        /// Bitmap scaled to axis scale.

const sal_uInt16 EXC_CHPICFORMAT_WINDOWS        = 0x0001;
const sal_uInt16 EXC_CHPICFORMAT_MACOS          = 0x0002;
const sal_uInt16 EXC_CHPICFORMAT_FORMATONLY     = 0x0100;

// (0x103D) CHDROPBAR ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHDROPBAR               = 0x103D;

const sal_uInt16 EXC_CHDROPBAR_UP               = 0;
const sal_uInt16 EXC_CHDROPBAR_DOWN             = 1;
const sal_uInt16 EXC_CHDROPBAR_NONE             = 0xFFFF;

// (0x103E, 0x1040) CHRADARLINE, CHRADARAREA ----------------------------------

const sal_uInt16 EXC_ID_CHRADARLINE             = 0x103E;
const sal_uInt16 EXC_ID_CHRADARAREA             = 0x1040;

const sal_uInt16 EXC_CHRADAR_AXISLABELS         = 0x0001;
const sal_uInt16 EXC_CHRADAR_SHADOW             = 0x0002;

// (0x103F) CHSURFACE ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHSURFACE               = 0x103F;

const sal_uInt16 EXC_CHSURFACE_FILLED           = 0x0001;
const sal_uInt16 EXC_CHSURFACE_SHADING          = 0x0002;

// (0x1041) CHAXESSET ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXESSET               = 0x1041;

const sal_uInt16 EXC_CHAXESSET_PRIMARY          = 0;
const sal_uInt16 EXC_CHAXESSET_SECONDARY        = 1;
const sal_uInt16 EXC_CHAXESSET_NONE             = 0xFFFF;   /// For internal use.

// (0x1044) CHPROPERTIES ------------------------------------------------------

const sal_uInt16 EXC_ID_CHPROPERTIES            = 0x1044;

const sal_uInt16 EXC_CHPROPS_CHANGED            = 0x0001;

const sal_uInt8 EXC_CHPROPS_EMPTY_SKIP          = 0;        /// Skip empty values.
const sal_uInt8 EXC_CHPROPS_EMPTY_ZERO          = 1;        /// Plot empty values as zero.
const sal_uInt8 EXC_CHPROPS_EMPTY_INTERPOLATE   = 2;        /// Interpolate empty values.

// (0x1045) CHSERGROUP --------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERGROUP              = 0x1045;

const sal_uInt16 EXC_CHSERGROUP_NONE            = 0xFFFF;   /// For internal use: no chart group.

// (0x1048, 0x0858) CHPIVOTREF ------------------------------------------------

const sal_uInt16 EXC_ID5_CHPIVOTREF             = 0x1048;
const sal_uInt16 EXC_ID8_CHPIVOTREF             = 0x0858;

// (0x104A) CHSERPARENT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERPARENT             = 0x104A;

// (0x104B) CHSERTRENDLINE ----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERTRENDLINE          = 0x104B;

const sal_uInt8 EXC_CHSERTREND_POLYNOMIAL       = 0;    /// If order is 1, trend line is linear.
const sal_uInt8 EXC_CHSERTREND_EXPONENTIAL      = 1;
const sal_uInt8 EXC_CHSERTREND_LOGARITHMIC      = 2;
const sal_uInt8 EXC_CHSERTREND_POWER            = 3;
const sal_uInt8 EXC_CHSERTREND_MOVING_AVG       = 4;

// (0x104E) CHFORMAT ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHFORMAT                = 0x104E;

// (0x104F) CHFRAMEPOS --------------------------------------------------------

const sal_uInt16 EXC_ID_CHFRAMEPOS              = 0x104F;

const sal_uInt16 EXC_CHFRAMEPOS_ANY             = 2;
const sal_uInt16 EXC_CHFRAMEPOS_LEGEND          = 5;

const sal_uInt16 EXC_CHFRAMEPOS_MANUALSIZE      = 1;
const sal_uInt16 EXC_CHFRAMEPOS_AUTOSIZE        = 2;

// (0x1050) CHFORMATRUNS ------------------------------------------------------

const sal_uInt16 EXC_ID_CHFORMATRUNS            = 0x1050;

// (0x1051) CHSOURCELINK ------------------------------------------------------

const sal_uInt16 EXC_ID_CHSOURCELINK            = 0x1051;

const sal_uInt8 EXC_CHSRCLINK_TITLE             = 0;
const sal_uInt8 EXC_CHSRCLINK_VALUES            = 1;
const sal_uInt8 EXC_CHSRCLINK_CATEGORY          = 2;
const sal_uInt8 EXC_CHSRCLINK_BUBBLES           = 3;

const sal_uInt8 EXC_CHSRCLINK_DEFAULT           = 0;
const sal_uInt8 EXC_CHSRCLINK_DIRECTLY          = 1;
const sal_uInt8 EXC_CHSRCLINK_WORKSHEET         = 2;

const sal_uInt16 EXC_CHSRCLINK_NUMFMT           = 0x0001;

// (0x105B) CHSERERRORBAR -----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERERRORBAR           = 0x105B;

const sal_uInt8 EXC_CHSERERR_NONE               = 0;    /// For internal use only.
const sal_uInt8 EXC_CHSERERR_XPLUS              = 1;
const sal_uInt8 EXC_CHSERERR_XMINUS             = 2;
const sal_uInt8 EXC_CHSERERR_YPLUS              = 3;
const sal_uInt8 EXC_CHSERERR_YMINUS             = 4;
const sal_uInt8 EXC_CHSERERR_XBOTH              = 5;    /// For internal use only.
const sal_uInt8 EXC_CHSERERR_YBOTH              = 6;    /// For internal use only.

const sal_uInt8 EXC_CHSERERR_PERCENT            = 1;
const sal_uInt8 EXC_CHSERERR_FIXED              = 2;
const sal_uInt8 EXC_CHSERERR_STDDEV             = 3;
const sal_uInt8 EXC_CHSERERR_CUSTOM             = 4;
const sal_uInt8 EXC_CHSERERR_STDERR             = 5;

const sal_uInt8 EXC_CHSERERR_END_BLANK          = 0;    /// Line end: blank.
const sal_uInt8 EXC_CHSERERR_END_TSHAPE         = 1;    /// Line end: t-shape.

// (0x105D) CHSERIESFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERIESFORMAT          = 0x105D;

const sal_uInt16 EXC_CHSERIESFORMAT_SMOOTHED    = 0x0001;
const sal_uInt16 EXC_CHSERIESFORMAT_BUBBLE3D    = 0x0002;
const sal_uInt16 EXC_CHSERIESFORMAT_SHADOW      = 0x0004;

// (0x105F) CH3DDATAFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CH3DDATAFORMAT          = 0x105F;

const sal_uInt8 EXC_CH3DDATAFORMAT_RECT         = 0;        /// Rectangular base.
const sal_uInt8 EXC_CH3DDATAFORMAT_CIRC         = 1;        /// Circular base.

const sal_uInt8 EXC_CH3DDATAFORMAT_STRAIGHT     = 0;        /// Straight to top.
const sal_uInt8 EXC_CH3DDATAFORMAT_SHARP        = 1;        /// Sharp top.
const sal_uInt8 EXC_CH3DDATAFORMAT_TRUNC        = 2;        /// Shart top, truncated.

// (0x1061) CHPIEEXT ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIEEXT                = 0x1061;

// (0x1066) CHESCHERFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHESCHERFORMAT          = 0x1066;

// Other record IDs -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHWRAPPEDRECORD         = 0x0851;
const sal_uInt16 EXC_ID_CHUNITPROPERTIES        = 0x0857;
const sal_uInt16 EXC_ID_CHEXTRANGE              = 0x1062;
const sal_uInt16 EXC_ID_CHPLOTGROWTH            = 0x1064;
const sal_uInt16 EXC_ID_CHUNKNOWN               = 0xFFFF;

// ============================================================================
// Structs and classes
// ============================================================================

// Common =====================================================================

struct XclChRectangle
{
    sal_Int32           mnX;                /// X position of the object in 1/4000 of chart width.
    sal_Int32           mnY;                /// Y position of the object in 1/4000 of chart height.
    sal_Int32           mnWidth;            /// Width of the object in 1/4000 of chart width.
    sal_Int32           mnHeight;           /// Height of the object in 1/4000 of chart height.

    explicit            XclChRectangle();
};

// ----------------------------------------------------------------------------

/** Specifies the position of a data series or data point. */
struct XclChDataPointPos
{
    sal_uInt16          mnSeriesIdx;        /// Series index of series or a data point.
    sal_uInt16          mnPointIdx;         /// Index of a data point inside a series.

    explicit            XclChDataPointPos();
};

bool operator==( const XclChDataPointPos& rL, const XclChDataPointPos& rR );
bool operator<( const XclChDataPointPos& rL, const XclChDataPointPos& rR );

// Formatting =================================================================

struct XclChFramePos
{
    XclChRectangle      maRect;             /// Object dependent position data.
    sal_uInt16          mnObjType;          /// Object type.
    sal_uInt16          mnSizeMode;         /// Size mode (manual, automatic).

    explicit            XclChFramePos();
};

// ----------------------------------------------------------------------------

struct XclChLineFormat
{
    Color               maColor;            /// Line color.
    sal_uInt16          mnPattern;          /// Line pattern (solid, dashed, ...).
    sal_Int16           mnWeight;           /// Line weight (hairline, single, ...).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnColorIdx;         /// Palette index of the line color (BIFF8+).

    explicit            XclChLineFormat();
};

// ----------------------------------------------------------------------------

struct XclChAreaFormat
{
    Color               maForeColor;        /// Pattern color.
    Color               maBackColor;        /// Pattern background color.
    sal_uInt16          mnPattern;          /// Pattern (none, solid).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnForeColorIdx;     /// Palette index of pattern color (BIFF8+).
    sal_uInt16          mnBackColorIdx;     /// Palette index of pattern background color (BIFF8+).

    explicit            XclChAreaFormat();
};

// ----------------------------------------------------------------------------

struct XclChPicFormat
{
    sal_uInt16          mnBmpMode;          /// Bitmap mode, e.g. stretched, stacked.
    sal_uInt16          mnFormat;           /// Image data format (WMF, BMP).
    sal_uInt16          mnFlags;            /// Additional flags.
    double              mfScale;            /// Picture scaling (units).

    explicit            XclChPicFormat();
};

// ----------------------------------------------------------------------------

struct XclChMarkerFormat
{
    Color               maLineColor;        /// Border line color.
    Color               maFillColor;        /// Fill color.
    sal_uInt32          mnMarkerSize;       /// Size of a marker
    sal_uInt16          mnMarkerType;       /// Marker type (none, diamond, ...).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnLineColorIdx;     /// Palette index of border line color (BIFF8).
    sal_uInt16          mnFillColorIdx;     /// Palette index of fill color (BIFF8).

    explicit            XclChMarkerFormat();
};

// ----------------------------------------------------------------------------

struct XclCh3dDataFormat
{
    sal_uInt8           mnBase;             /// Base form.
    sal_uInt8           mnTop;              /// Top egde mode.

    explicit            XclCh3dDataFormat();
};

// ----------------------------------------------------------------------------

struct XclChFrame
{
    sal_uInt16          mnFormat;           /// Format type of the frame.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChFrame();
};

// Text =======================================================================

struct XclChObjectLink
{
    XclChDataPointPos   maPointPos;         /// Position of the data point.
    sal_uInt16          mnTarget;           /// Target of the link.

    explicit            XclChObjectLink();
};

// ----------------------------------------------------------------------------

struct XclChText
{
    XclChRectangle      maRect;             /// Position of the text object.
    Color               maTextColor;        /// Text color.
    sal_uInt8           mnHAlign;           /// Horizontal alignment.
    sal_uInt8           mnVAlign;           /// Vertical alignment.
    sal_uInt16          mnBackMode;         /// Background mode: transparent, opaque.
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnTextColorIdx;     /// Palette index to text color (BIFF8+).
    sal_uInt16          mnPlacement;        /// Text object placement (BIFF8+).
    sal_uInt16          mnRotation;         /// Text object rotation (BIFF8+).

    explicit            XclChText();
};

// Linked source data =========================================================

struct XclChSourceLink
{
    sal_uInt8           mnDestType;         /// Type of the destination (title, values, ...).
    sal_uInt8           mnLinkType;         /// Link type (directly, linked to worksheet, ...).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnNumFmtIdx;        /// Number format index.

    explicit            XclChSourceLink();
};

// ----------------------------------------------------------------------------

struct XclChDataFormat
{
    XclChDataPointPos   maPointPos;         /// Position of the data point or series.
    sal_uInt16          mnFormatIdx;        /// Formatting index for automatic colors.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChDataFormat();
};

// ----------------------------------------------------------------------------

struct XclChSerTrendLine
{
    double              mfIntercept;        /// Forced intercept.
    double              mfForecastFor;      /// Counter to forecast forward.
    double              mfForecastBack;     /// Counter to forecast backward.
    sal_uInt8           mnLineType;         /// Type of the trend line.
    sal_uInt8           mnOrder;            /// Polynomial order or moving average counter.
    sal_uInt8           mnShowEquation;     /// 1 = Show equation.
    sal_uInt8           mnShowRSquared;     /// 1 = Show R-squared.

    explicit            XclChSerTrendLine();
};

// ----------------------------------------------------------------------------

struct XclChSerErrorBar
{
    double              mfValue;            /// Fixed value for several source types.
    sal_uInt16          mnValueCount;       /// Number of custom error values.
    sal_uInt8           mnBarType;          /// Type of the error bar (X/Y).
    sal_uInt8           mnSourceType;       /// Type of source values.
    sal_uInt8           mnLineEnd;          /// Type of the line ends.

    explicit            XclChSerErrorBar();
};

// ----------------------------------------------------------------------------

struct XclChSeries
{
    sal_uInt16          mnCategType;        /// Data type for category entries.
    sal_uInt16          mnValueType;        /// Data type for value entries.
    sal_uInt16          mnBubbleType;       /// Data type for bubble entries.
    sal_uInt16          mnCategCount;       /// Number of category entries.
    sal_uInt16          mnValueCount;       /// Number of value entries.
    sal_uInt16          mnBubbleCount;      /// Number of bubble entries.

    explicit            XclChSeries();
};

// Chart structure ============================================================

struct XclChType
{
    sal_Int16           mnOverlap;          /// Bar overlap width (CHBAR).
    sal_Int16           mnGap;              /// Gap between bars (CHBAR).
    sal_uInt16          mnRotation;         /// Rotation angle of first pie (CHPIE).
    sal_uInt16          mnPieHole;          /// Hole size in donut chart (CHPIE).
    sal_uInt16          mnBubbleSize;       /// Bubble size in bubble chart (CHSCATTER).
    sal_uInt16          mnBubbleType;       /// Bubble type in bubble chart (CHSCATTER).
    sal_uInt16          mnFlags;            /// Additional flags (all chart types).

    explicit            XclChType();
};

// ----------------------------------------------------------------------------

struct XclChChart3d
{
    sal_uInt16          mnRotation;         /// Rotation (0...360deg).
    sal_Int16           mnElevation;        /// Elevation (-90...+90deg).
    sal_uInt16          mnDist;             /// View distance to chart (0...100).
    sal_uInt16          mnHeight;           /// Height relative to width and depth.
    sal_uInt16          mnDepth;            /// Depth of points relative to width.
    sal_uInt16          mnGap;              /// Space between series.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChChart3d();
};

// ----------------------------------------------------------------------------

struct XclChLegend
{
    XclChRectangle      maRect;             /// Position of the legend.
    sal_uInt8           mnDockMode;         /// Docking mode.
    sal_uInt8           mnSpacing;          /// Spacing between elements.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChLegend();
};

// ----------------------------------------------------------------------------

struct XclChChartGroup
{
    XclChRectangle      maRect;             /// Position (not used).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnGroupIdx;         /// Chart group index.

    explicit            XclChChartGroup();
};

// ----------------------------------------------------------------------------

struct XclChProperties
{
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt8           mnEmptyMode;        /// Plotting mode for empty cells.

    explicit            XclChProperties();
};

// Axes =======================================================================

struct XclChLabelRange
{
    sal_uInt16          mnCross;            /// Crossing position of other axis.
    sal_uInt16          mnLabelFreq;        /// Frequency of labels.
    sal_uInt16          mnTickFreq;         /// Frequency of ticks.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChLabelRange();
};

// ----------------------------------------------------------------------------

struct XclChValueRange
{
    double              mfMin;              /// Minimum value on axis.
    double              mfMax;              /// Maximum value on axis.
    double              mfMajorStep;        /// Distance for major grid lines.
    double              mfMinorStep;        /// Distance for minor grid lines.
    double              mfCross;            /// Crossing position of other axis.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChValueRange();
};

// ----------------------------------------------------------------------------

struct XclChTick
{
    XclChRectangle      maRect;             /// Position (not used).
    Color               maTextColor;        /// Tick labels color.
    sal_uInt8           mnMajor;            /// Type of tick marks of major grid.
    sal_uInt8           mnMinor;            /// Type of tick marks of minor grid.
    sal_uInt8           mnLabelPos;         /// Position of labels relative to axis.
    sal_uInt8           mnBackMode;         /// Background mode: transparent, opaque.
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnTextColorIdx;     /// Palette index to tick labels color (BIFF8+).
    sal_uInt16          mnRotation;         /// Tick labels angle (BIFF8+).

    explicit            XclChTick();
};

// ----------------------------------------------------------------------------

struct XclChAxis
{
    XclChRectangle      maRect;             /// Position (not used).
    sal_uInt16          mnType;             /// Axis type.

    explicit            XclChAxis();
};

// ----------------------------------------------------------------------------

struct XclChAxesSet
{
    XclChRectangle      maRect;             /// Position of the axes set.
    sal_uInt16          mnAxesSetId;        /// Primary/secondary axes set.

    explicit            XclChAxesSet();
};

// Static helper functions ====================================================

/** Contains static helper functions for the chart filters. */
class XclChartHelper
{
public:
    /** Returns true, if series are shown as lines or areas. */
    static bool         IsLineChartType( sal_uInt16 nTypeId );
    /** Returns true, if series are shown as bars. */
    static bool         IsBarChartType( sal_uInt16 nTypeId );
    /** Returns true, if series are shown as radar net. */
    static bool         IsRadarChartType( sal_uInt16 nTypeId );
    /** Returns true, if series are shown as pies. */
    static bool         IsPieChartType( sal_uInt16 nTypeId );
    /** Returns true, if series are shown in an XY diagram. */
    static bool         IsScatterChartType( sal_uInt16 nTypeId );

    /** Returns true, if series are shown in a polar coordinate system. */
    static bool         HasPolarCoordSystem( sal_uInt16 nTypeId );
    /** Returns true, if series are shown with lines and symbols and without fill area. */
    static bool         HasLinearSeries( sal_uInt16 nTypeId );
    /** Returns true, if data points are grouped into categories. */
    static bool         HasCategoryAxis( sal_uInt16 nTypeId );
    /** Returns true, if the X and Y axes are swapped (e.g. horizontal bar chart). */
    static bool         HasSwappedAxesSet( sal_uInt16 nTypeId );
};

// Property helpers ===========================================================

class XclChObjectTable
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >     XNameContainerRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    XServiceFactoryRef;

public:
    explicit            XclChObjectTable( XServiceFactoryRef xFactory,
                            const ::rtl::OUString& rServiceName, const ::rtl::OUString& rObjNameBase );

    /** Insertes a named formatting object into the chart document. */
    ::rtl::OUString     InsertObject( const ::com::sun::star::uno::Any& rObj );

private:
    XServiceFactoryRef  mxFactory;              /// Factory to create the container.
    XNameContainerRef   mxContainer;            /// Container for the objects.
    ::rtl::OUString     maServiceName;          /// Service name to create the container.
    ::rtl::OUString     maObjNameBase;          /// Base of names for inserted objects.
    sal_Int32           mnIndex;                /// Index to create unique identifiers.
};

// ----------------------------------------------------------------------------

/** Specifies the type of a formatting. This results in different property names. */
enum XclChPropertyMode
{
    EXC_CHPROPMODE_COMMON,          /// Common objects, no special handling.
    EXC_CHPROPMODE_LINEARSERIES,    /// Specific to data series drawn as lines.
    EXC_CHPROPMODE_FILLEDSERIES     /// Specific to data series drawn as areas.
};

// ----------------------------------------------------------------------------

class SfxItemSet;
struct XclFontData;

/** Helper class for usage of property sets. */
class XclChPropSetHelper
{
public:
    explicit            XclChPropSetHelper();

    /** Writes all line properties to the passed property set. */
    void                WriteLineProperties(
                            ScfPropertySet& rPropSet,
                            XclChObjectTable& rDashTable,
                            const XclChLineFormat& rLineFmt,
                            XclChPropertyMode ePropMode );
    /** Writes solid area properties to the passed property set. */
    void                WriteAreaProperties(
                            ScfPropertySet& rPropSet,
                            const XclChAreaFormat& rAreaFmt,
                            XclChPropertyMode ePropMode );
    /** Writes gradient or bitmap area properties to the passed property set. */
    void                WriteEscherProperties(
                            ScfPropertySet& rPropSet,
                            XclChObjectTable& rGradientTable,
                            XclChObjectTable& rBitmapTable,
                            const SfxItemSet& rItemSet,
                            const XclChPicFormat& rPicFmt );
    /** Writes all marker properties to the passed property set. */
    void                WriteMarkerProperties(
                            ScfPropertySet& rPropSet,
                            const XclChMarkerFormat& rMarkerFmt );

private:
    ScfPropSetHelper    maLineHlp;          /// Properties for solid or dashed lines.
    ScfPropSetHelper    maAreaHlp;          /// Properties for solid areas.
    ScfPropSetHelper    maGradientHlp;      /// Properties for gradients.
    ScfPropSetHelper    maBitmapHlp;        /// Properties for bitmaps.
    ScfPropSetHelper    maMarkerHlp;        /// Properties for symbols.
};

// ============================================================================

#endif

