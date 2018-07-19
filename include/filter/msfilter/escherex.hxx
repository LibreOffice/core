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

#ifndef INCLUDED_FILTER_MSFILTER_ESCHEREX_HXX
#define INCLUDED_FILTER_MSFILTER_ESCHEREX_HXX

#include <memory>
#include <vector>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <filter/msfilter/msfilterdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/GraphicObject.hxx>
#include <svx/svdtypes.hxx>
#include <svx/msdffdef.hxx>
#include <tools/gen.hxx>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <vcl/mapmod.hxx>
#include <o3tl/typed_flags_set.hxx>

class Color;

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace beans { class XPropertySet; }
    namespace drawing { struct EnhancedCustomShapeParameter; }
    namespace drawing { struct Hatch; }
} } }

namespace tools {
    class Polygon;
    class PolyPolygon;
}

        /*Record Name       FBT-Value   Instance                  Contents                                                          Wrd Exl PPt Ver*/
// In the Microsoft documentation the naming scheme is msofbt... instead of ESCHER_...
#define ESCHER_DggContainer     0xF000u /*                           per-document data                                                  X   X   X     */
#define ESCHER_Dgg              0xF006u /*                           an FDGG and several FIDCLs                                         X   X   X   0 */
#define ESCHER_OPT              0xF00Bu /* count of properties       the document-wide default shape properties                         X   X   X   3 */
#define ESCHER_SplitMenuColors  0xF11Eu /* count of colors           the colors in the top-level split menus                            X   X   X   0 */
#define ESCHER_BstoreContainer  0xF001u /* count of BLIPs            all images in the document (JPEGs, metafiles, etc.)                X   X   X     */
#define ESCHER_BSE              0xF007u /* BLIP type                 an FBSE (one per BLIP)                                             X   X   X   2 */
#define ESCHER_BlipFirst        0xF018u /*                           range of fbts reserved for various kinds of BLIPs                  X   X   X     */

#define ESCHER_DgContainer      0xF002u /*                           per-sheet/page/slide data                                          X   X   X     */
#define ESCHER_Dg               0xF008u /* drawing ID                an FDG                                                             X   X   X   0 */
#define ESCHER_SpgrContainer    0xF003u /*                           several SpContainers, the first of which is the group shape itself X   X   X     */
#define ESCHER_SpContainer      0xF004u /*                           a shape                                                            X   X   X     */
#define ESCHER_Spgr             0xF009u /*                           an FSPGR; only present if the shape is a group shape               X   X   X   1 */
#define ESCHER_Sp               0xF00Au /* shape type                an FSP                                                             X   X   X   2 */
//#define     ESCHER_OPT           0xF00Bu /* count of properties       a shape property table                                             X   X   X   3 */
#define ESCHER_ClientTextbox    0xF00Du /* host-defined              the text in the textbox, in host-defined format                    X   X   X     */
#define ESCHER_ChildAnchor      0xF00Fu /*                           a RECT, in units relative to the parent group                      X   X   X   0 */
#define ESCHER_ClientAnchor     0xF010u /* host-defined              the location of the shape, in a host-defined format                X   X   X     */
#define ESCHER_ClientData       0xF011u /* host-defined              host-specific data                                                 X   X   X     */
#define ESCHER_SolverContainer  0xF005u /* count of rules            the rules governing shapes                                         X   X   X     */
#define ESCHER_ConnectorRule    0xF012u /*                           an FConnectorRule                                                      X   X   1 */
#define ESCHER_UDefProp         0xF122u

enum class ShapeFlag : sal_uInt32
{
    NONE                = 0x000,
    Group               = 0x001,   /* shape is a group shape */
    Child               = 0x002,   /* shape is a child shape */
    Patriarch           = 0x004,   /* shape is the topmost group shape.
                                      Exactly one of these per drawing. */
    Deleted             = 0x008,   /* shape has been deleted */
    OLEShape            = 0x010,   /* shape is an OLE object */
    HaveMaster          = 0x020,   /* shape has a valid master in hspMaster property */
    FlipH               = 0x040,   /* shape is flipped horizontally */
    FlipV               = 0x080,   /* shape is flipped vertically */
    Connector           = 0x100,   /* shape is a connector shape */
    HaveAnchor          = 0x200,   /* shape has an anchor of some kind */
    Background          = 0x400,   /* shape is a background shape */
    HaveShapeProperty   = 0x800    /* shape has a shape type property */
};  /* 20 bits unused */
namespace o3tl {
    template<> struct typed_flags<ShapeFlag> : is_typed_flags<ShapeFlag, 0x00000FFF> {};
}

#define ESCHER_ShpInst_Min                          0
#define ESCHER_ShpInst_NotPrimitive                 ESCHER_ShpInst_Min
#define ESCHER_ShpInst_Rectangle                    1
#define ESCHER_ShpInst_RoundRectangle               2
#define ESCHER_ShpInst_Ellipse                      3
#define ESCHER_ShpInst_Arc                          19
#define ESCHER_ShpInst_Line                         20
#define ESCHER_ShpInst_StraightConnector1           32
#define ESCHER_ShpInst_BentConnector2               33
#define ESCHER_ShpInst_BentConnector3               34
#define ESCHER_ShpInst_CurvedConnector3             38
#define ESCHER_ShpInst_PictureFrame                 75
#define ESCHER_ShpInst_TextPlainText                136
#define ESCHER_ShpInst_TextDeflateInflateDeflate    167
#define ESCHER_ShpInst_TextSlantUp                  172
#define ESCHER_ShpInst_HostControl                  201
#define ESCHER_ShpInst_TextBox                      202
#define ESCHER_ShpInst_COUNT                        203
#define ESCHER_ShpInst_Max                          0x0FFF
#define ESCHER_ShpInst_Nil                          ESCHER_ShpInst_Max

enum ESCHER_BlibType
{                           // GEL provided types...
   ERROR = 0,               // An error occurred during loading
   UNKNOWN,                 // An unknown blip type
   EMF,                     // Windows Enhanced Metafile
   WMF,                     // Windows Metafile
   PICT,                    // Macintosh PICT
   PEG,                     // JFIF
   PNG,                     // PNG
   DIB,                     // Windows DIB
   FirstClient = 32,        // First client defined blip type
   LastClient  = 255        // Last client defined blip type
};


enum ESCHER_FillStyle
{
    ESCHER_FillSolid,       // Fill with a solid color
    ESCHER_FillPattern,     // Fill with a pattern (bitmap)
    ESCHER_FillTexture,     // A texture (pattern with its own color map)
    ESCHER_FillPicture,     // Center a picture in the shape
    ESCHER_FillShade,       // Shade from start to end points
    ESCHER_FillShadeCenter, // Shade from bounding rectangle to end point
    ESCHER_FillShadeShape,  // Shade from shape outline to end point
    ESCHER_FillShadeScale,
    ESCHER_FillShadeTitle,
    ESCHER_FillBackground
};

enum ESCHER_wMode
{
    ESCHER_wColor,          // only used for predefined shades
    ESCHER_wAutomatic,      // depends on object type
    ESCHER_wGrayScale,      // shades of gray only
    ESCHER_wLightGrayScale, // shades of light gray only
    ESCHER_wInverseGray,    // dark gray mapped to light gray, etc.
    ESCHER_wGrayOutline,    // pure gray and white
    ESCHER_wBlackTextLine,  // black text and lines, all else grayscale
    ESCHER_wHighContrast,   // pure black and white mode (no grays)
    ESCHER_wBlack,          // solid black   msobwWhite,          // solid white
    ESCHER_wDontShow,       // object not drawn
    ESCHER_wNumModes        // number of Black and white modes
};


enum ESCHER_ShapePath
{
    ESCHER_ShapeLines,          // A line of straight segments
    ESCHER_ShapeLinesClosed,    // A closed polygonal object
    ESCHER_ShapeCurves,         // A line of Bezier curve segments
    ESCHER_ShapeCurvesClosed,   // A closed shape with curved edges
    ESCHER_ShapeComplex         // pSegmentInfo must be non-empty
};


enum ESCHER_WrapMode
{
    ESCHER_WrapSquare,
    ESCHER_WrapByPoints,
    ESCHER_WrapNone,
    ESCHER_WrapTopBottom,
    ESCHER_WrapThrough
};


enum ESCHER_bwMode
{
    ESCHER_bwColor,             // only used for predefined shades
    ESCHER_bwAutomatic,         // depends on object type
    ESCHER_bwGrayScale,         // shades of gray only
    ESCHER_bwLightGrayScale,    // shades of light gray only
    ESCHER_bwInverseGray,       // dark gray mapped to light gray, etc.
    ESCHER_bwGrayOutline,       // pure gray and white
    ESCHER_bwBlackTextLine,     // black text and lines, all else grayscale
    ESCHER_bwHighContrast,      // pure black and white mode (no grays)
    ESCHER_bwBlack,             // solid black
    ESCHER_bwWhite,             // solid white
    ESCHER_bwDontShow,          // object not drawn
    ESCHER_bwNumModes           // number of Black and white modes
};


enum ESCHER_AnchorText
{
    ESCHER_AnchorTop,
    ESCHER_AnchorMiddle,
    ESCHER_AnchorBottom,
    ESCHER_AnchorTopCentered,
    ESCHER_AnchorMiddleCentered,
    ESCHER_AnchorBottomCentered,
    ESCHER_AnchorTopBaseline,
    ESCHER_AnchorBottomBaseline,
    ESCHER_AnchorTopCenteredBaseline,
    ESCHER_AnchorBottomCenteredBaseline
};

//  connector style
enum ESCHER_cxSTYLE
{
    ESCHER_cxstyleStraight = 0,
    ESCHER_cxstyleBent,
    ESCHER_cxstyleCurved,
    ESCHER_cxstyleNone
};

//  text flow
enum ESCHER_txfl
{
    ESCHER_txflHorzN,           // Horizontal non-@
    ESCHER_txflTtoBA,           // Top to Bottom @-font
    ESCHER_txflBtoT,            // Bottom to Top non-@
    ESCHER_txflTtoBN,           // Top to Bottom non-@
    ESCHER_txflHorzA,           // Horizontal @-font
    ESCHER_txflVertN            // Vertical, non-@
};


//  flags for pictures
enum ESCHER_BlipFlags
{
    ESCHER_BlipFlagDefault = 0,
    ESCHER_BlipFlagComment = 0,     // Blip name is a comment
    ESCHER_BlipFlagFile,            // Blip name is a file name
    ESCHER_BlipFlagURL,             // Blip name is a full URL
    ESCHER_BlipFlagType = 3,        // Mask to extract type
   /* Or the following flags with any of the above. */
    ESCHER_BlipFlagDoNotSave = 4,
    ESCHER_BlipFlagLinkToFile = 8
};

// dashed line style
enum ESCHER_LineDashing
{
    ESCHER_LineSolid,               // Solid (continuous) pen
    ESCHER_LineDashSys,             // PS_DASH system   dash style
    ESCHER_LineDotSys,              // PS_DOT system   dash style
    ESCHER_LineDashDotSys,          // PS_DASHDOT system dash style
    ESCHER_LineDashDotDotSys,       // PS_DASHDOTDOT system dash style
    ESCHER_LineDotGEL,              // square dot style
    ESCHER_LineDashGEL,             // dash style
    ESCHER_LineLongDashGEL,         // long dash style
    ESCHER_LineDashDotGEL,          // dash short dash
    ESCHER_LineLongDashDotGEL,      // long dash short dash
    ESCHER_LineLongDashDotDotGEL    // long dash short dash short dash
};

// line end effect
enum ESCHER_LineEnd
{
    ESCHER_LineNoEnd,
    ESCHER_LineArrowEnd,
    ESCHER_LineArrowStealthEnd,
    ESCHER_LineArrowDiamondEnd,
    ESCHER_LineArrowOvalEnd,
    ESCHER_LineArrowOpenEnd
};

// size of arrowhead
enum ESCHER_LineWidth
{
    ESCHER_LineNarrowArrow,
    ESCHER_LineMediumWidthArrow,
    ESCHER_LineWideArrow
};

// size of arrowhead
enum ESCHER_LineEndLength
{
    ESCHER_LineShortArrow,
    ESCHER_LineMediumLenArrow,
    ESCHER_LineLongArrow
};

// line join style.
enum ESCHER_LineJoin
{
    ESCHER_LineJoinBevel,     // Join edges by a straight line
    ESCHER_LineJoinMiter,     // Extend edges until they join
    ESCHER_LineJoinRound      // Draw an arc between the two edges
};

// line cap style (applies to ends of dash segments too).
enum ESCHER_LineCap
{
    ESCHER_LineEndCapRound,   // Rounded ends - the default
    ESCHER_LineEndCapSquare,  // Square protrudes by half line width
    ESCHER_LineEndCapFlat     // Line ends at end point
};

enum MSOPATHTYPE
{
    msopathLineTo,        // Draw a straight line (one point)
    msopathCurveTo,       // Draw a cubic Bezier curve (three points)
    msopathMoveTo,        // Move to a new point (one point)
    msopathClose,         // Close a sub - path (no points)
    msopathEnd,           // End a path (no points)
    msopathEscape,        // Escape code
    msopathClientEscape,  // Escape code interpreted by the client
    msopathInvalid        // Invalid - should never be found
};

// Shape Properties
// 1pt = 12700 EMU (English Metric Units)
// 1pt = 20 Twip = 20/1440" = 1/72"
// 1twip=635 EMU
// 1" = 12700*72 = 914400 EMU
// 1" = 25.4mm
// 1mm = 36000 EMU
// Transform
#define ESCHER_Prop_Rotation                      4  /*  Fixed Point 16.16 degrees                  */
// Protection
#define ESCHER_Prop_LockAgainstGrouping         127  /*  bool              Do not group this shape             */
// Text
#define ESCHER_Prop_lTxid                       128  /*  LONG              id for the text, value determined by the host            */
#define ESCHER_Prop_dxTextLeft                  129  /*  LONG              margins relative to shape's inscribed                    */
#define ESCHER_Prop_dyTextTop                   130  /*  LONG                text rectangle (in EMUs)                               */
#define ESCHER_Prop_dxTextRight                 131  /*  LONG                                                                       */
#define ESCHER_Prop_dyTextBottom                132  /*  LONG                                                                       */
#define ESCHER_Prop_WrapText                    133  /*  MSOWRAPMODE       Wrap text at shape margins                               */
#define ESCHER_Prop_AnchorText                  135  /*  ESCHER_AnchorText How to anchor the text                                   */
#define ESCHER_Prop_txflTextFlow                136  /*  MSOTXFL           Text flow                                                */
#define ESCHER_Prop_hspNext                     138  /*  MSOHSP            ID of the next shape (used by Word for linked textboxes) */
#define ESCHER_Prop_FitTextToShape              191  /*  bool              Size text to fit shape size                              */
// GeoText
#define ESCHER_Prop_gtextUNICODE                192  /*  WCHAR*            UNICODE text string       */
#define ESCHER_Prop_gtextSize                   195  /*  LONG              default point size        */
#define ESCHER_Prop_gtextFont                   197  /*  WCHAR*            font family name          */
// Blip
#define ESCHER_Prop_cropFromTop                 256  /*  LONG              16.16 fraction times total                          */
#define ESCHER_Prop_cropFromBottom              257  /*  LONG                image width or height,                            */
#define ESCHER_Prop_cropFromLeft                258  /*  LONG                as appropriate.                                   */
#define ESCHER_Prop_cropFromRight               259  /*  LONG                                                                  */
#define ESCHER_Prop_pib                         260  /*  IMsoBlip*         Blip to display                                     */
#define ESCHER_Prop_pibName                     261  /*  WCHAR*            Blip file name                                      */
#define ESCHER_Prop_pibFlags                    262  /*  MSOBLIPFLAGS      Blip flags                                          */
#define ESCHER_Prop_pictureContrast             264  /*  LONG              contrast setting                                    */
#define ESCHER_Prop_pictureBrightness           265  /*  LONG              brightness setting                                  */
#define ESCHER_Prop_pictureId                   267  /*  LONG              Host-defined ID for OLE objects (usually a pointer) */
#define ESCHER_Prop_pictureActive               319  /*  bool              Server is active (OLE objects only)                 */
// Geometry
#define ESCHER_Prop_geoLeft                     320  /*  LONG              Defines the G (geometry) coordinate space.  */
#define ESCHER_Prop_geoTop                      321  /*  LONG                                                          */
#define ESCHER_Prop_geoRight                    322  /*  LONG                                                          */
#define ESCHER_Prop_geoBottom                   323  /*  LONG                                                          */
#define ESCHER_Prop_shapePath                   324  /*  MSOSHAPEPATH                                                  */
#define ESCHER_Prop_pVertices                   325  /*  IMsoArray         An array of points, in G units.             */
#define ESCHER_Prop_pSegmentInfo                326  /*  IMsoArray                                                     */
#define ESCHER_Prop_adjustValue                 327  /*  LONG              Adjustment values corresponding to          */
#define ESCHER_Prop_adjust2Value                328  /*  LONG                the positions of the adjust handles       */
#define ESCHER_Prop_fFillOK                     383  /*  bool              OK to fill the shape through the UI or VBA? */
// FillStyle
#define ESCHER_Prop_fillType                    384  /*  ESCHER_FillStyle  Type of fill                                  */
#define ESCHER_Prop_fillColor                   385  /*  MSOCLR            Foreground color                              */
#define ESCHER_Prop_fillOpacity                 386  /*  LONG              Fixed 16.16                                   */
#define ESCHER_Prop_fillBackColor               387  /*  MSOCLR            Background color                              */
#define ESCHER_Prop_fillBackOpacity             388  /*  LONG              Shades only                                   */
#define ESCHER_Prop_fillBlip                    390  /*  IMsoBlip*         Pattern/texture                               */
#define ESCHER_Prop_fillAngle                   395  /*  LONG              Fade angle - degrees in 16.16                 */
#define ESCHER_Prop_fillFocus                   396  /*  LONG              Linear shaded fill focus percent              */
#define ESCHER_Prop_fillToLeft                  397  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToTop                   398  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToRight                 399  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToBottom                400  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillRectRight               403  /*  LONG                define how large the fade is going to be.   */
#define ESCHER_Prop_fillRectBottom              404  /*  LONG                                                            */
#define ESCHER_Prop_fNoFillHitTest              447  /*  bool              Hit test a shape as though filled             */
// LineStyle
#define ESCHER_Prop_lineColor                   448  /*  MSOCLR            Color of line                              */
#define ESCHER_Prop_lineOpacity                 449  /*  LONG              Not implemented                            */
#define ESCHER_Prop_lineBackColor               450  /*  MSOCLR            Background color                           */
#define ESCHER_Prop_lineWidth                   459  /*  LONG              A units; 1pt == 12700 EMUs                 */
#define ESCHER_Prop_lineStyle                   461  /*  MSOLINESTYLE      Draw parallel lines?                       */
#define ESCHER_Prop_lineDashing                 462  /*  MSOLINEDASHING    Can be overridden by:                      */
#define ESCHER_Prop_lineStartArrowhead          464  /*  MSOLINEEND        Arrow at start                             */
#define ESCHER_Prop_lineEndArrowhead            465  /*  MSOLINEEND        Arrow at end                               */
#define ESCHER_Prop_lineStartArrowWidth         466  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define ESCHER_Prop_lineStartArrowLength        467  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define ESCHER_Prop_lineEndArrowWidth           468  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define ESCHER_Prop_lineEndArrowLength          469  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define ESCHER_Prop_lineJoinStyle               470  /*  MSOLINEJOIN       How to join lines                          */
#define ESCHER_Prop_lineEndCapStyle             471  /*  MSOLINECAP        How to end lines                           */
#define ESCHER_Prop_fNoLineDrawDash             511  /*  bool              Draw a dashed line if no line              */
// ShadowStyle
#define ESCHER_Prop_shadowColor                 513  /*  MSOCLR            Foreground color          */
#define ESCHER_Prop_shadowOpacity               516  /*  LONG              Fixed 16.16               */
#define ESCHER_Prop_shadowOffsetX               517  /*  LONG              Offset shadow             */
#define ESCHER_Prop_shadowOffsetY               518  /*  LONG              Offset shadow             */
#define ESCHER_Prop_fshadowObscured             575  /*  bool              Excel5-style shadow       */
// PerspectiveStyle
// 3D Object
#define ESCHER_Prop_fc3DLightFace               703  /*  bool                                                                                                                             */
// 3D Style
// Shape
#define ESCHER_Prop_hspMaster                   769  /*  MSOHSP          master shape                                        */
#define ESCHER_Prop_cxstyle                     771  /*  MSOCXSTYLE      Type of connector                                   */
#define ESCHER_Prop_bWMode                      772  /*  ESCHERwMode     Settings for modifications to                       */
#define ESCHER_Prop_fBackground                 831  /*  bool            If sal_True, this is the background shape.              */
// Callout
// GroupShape
#define ESCHER_Prop_wzName                      896  /*  WCHAR*          Shape Name (present only if explicitly set)                                                            */
#define ESCHER_Prop_wzDescription               897  /*  WCHAR*          alternate text                                                                                         */
#define ESCHER_Prop_pihlShape                   898  /*  IHlink*         The hyperlink in the shape.                                                                            */
#define ESCHER_Prop_dxWrapDistLeft              900  /*  LONG            Left wrapping distance from text (Word)                                                                */
#define ESCHER_Prop_dyWrapDistTop               901  /*  LONG            Top wrapping distance from text (Word)                                                                 */
#define ESCHER_Prop_dxWrapDistRight             902  /*  LONG            Right wrapping distance from text (Word)                                                               */
#define ESCHER_Prop_dyWrapDistBottom            903  /*  LONG            Bottom wrapping distance from text (Word)                                                              */
#define ESCHER_Prop_tableProperties             927
#define ESCHER_Prop_tableRowProperties          928
#define ESCHER_Prop_fHidden                     958  /*  bool            Do not display                                                                                         */
#define ESCHER_Prop_fPrint                      959  /*  bool            Print this shape                                                                                       */


#define ESCHER_Persist_PrivateEntry         0x80000000
#define ESCHER_Persist_Dgg                  0x00010000
#define ESCHER_Persist_Dg                   0x00020000
#define ESCHER_Persist_CurrentPosition      0x00040000
#define ESCHER_Persist_Grouping_Snap        0x00050000
#define ESCHER_Persist_Grouping_Logic       0x00060000

const sal_uInt32 DFF_DGG_CLUSTER_SIZE       = 0x00000400;   /// Shape IDs per cluster in DGG atom.

namespace com { namespace sun { namespace star {
    namespace awt {
        struct Gradient;
    }
    namespace drawing {
        struct EnhancedCustomShapeAdjustmentValue;
        class XShape;
        class XShapes;
    }
}}}

struct MSFILTER_DLLPUBLIC EscherConnectorListEntry
{
    css::uno::Reference< css::drawing::XShape >   mXConnector;
    css::awt::Point                               maPointA;
    css::uno::Reference< css::drawing::XShape >   mXConnectToA;
    css::awt::Point                               maPointB;
    css::uno::Reference< css::drawing::XShape >   mXConnectToB;

    sal_uInt32      GetConnectorRule( bool bFirst );

                    EscherConnectorListEntry( const css::uno::Reference< css::drawing::XShape > & rC,
                                        const css::awt::Point& rPA,
                                        css::uno::Reference< css::drawing::XShape > const & rSA ,
                                        const css::awt::Point& rPB,
                                        css::uno::Reference< css::drawing::XShape > const & rSB ) :
                                            mXConnector ( rC ),
                                            maPointA    ( rPA ),
                                            mXConnectToA( rSA ),
                                            maPointB    ( rPB ),
                                            mXConnectToB( rSB ) {}

                    static sal_uInt32 GetClosestPoint( const tools::Polygon& rPoly, const css::awt::Point& rP );
};

struct MSFILTER_DLLPUBLIC EscherExContainer
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    EscherExContainer( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0 );
    ~EscherExContainer();
};

struct MSFILTER_DLLPUBLIC EscherExAtom
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    EscherExAtom( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0, const sal_uInt8 nVersion = 0 );
    ~EscherExAtom();
};

struct EscherPropertyValueHelper
{
    static bool GetPropertyValue(
        css::uno::Any& rAny,
        const css::uno::Reference< css::beans::XPropertySet > &,
        const OUString& rPropertyName,
        bool bTestPropertyAvailability = false
    );

    static css::beans::PropertyState GetPropertyState(
        const css::uno::Reference < css::beans::XPropertySet > &,
        const OUString& rPropertyName
    );
};


struct EscherPersistEntry
{
    sal_uInt32  mnID;
    sal_uInt32  mnOffset;

    EscherPersistEntry( sal_uInt32 nId, sal_uInt32 nOffset ) { mnID = nId; mnOffset = nOffset; };

};


class EscherBlibEntry
{

    friend class EscherGraphicProvider;
    friend class EscherEx;

protected:

    sal_uInt32      mnIdentifier[ 4 ];
    sal_uInt32      mnPictureOffset;        // offset to the graphic in PictureStreams
    sal_uInt32      mnSize;                 // size of real graphic

    sal_uInt32      mnRefCount;             // !! reference count
    sal_uInt32      mnSizeExtra;            // !! size of preceding header

    ESCHER_BlibType meBlibType;

    Size            maPrefSize;
    MapMode         maPrefMapMode;

    bool            mbIsEmpty;
    bool            mbIsNativeGraphicPossible;

public:

                    EscherBlibEntry(
                        sal_uInt32 nPictureOffset,
                        const GraphicObject& rObj,
                        const OString& rId,
                        const GraphicAttr* pAttr
                    );

                    ~EscherBlibEntry();

    void            WriteBlibEntry( SvStream& rSt, bool bWritePictureOffset, sal_uInt32 nResize = 0 );
    bool            IsEmpty() const { return mbIsEmpty; };

    bool            operator==( const EscherBlibEntry& ) const;
};


enum class EscherGraphicProviderFlags {
    NONE                    = 0,
    UseInstances            = 1,
};
namespace o3tl {
    template<> struct typed_flags<EscherGraphicProviderFlags> : is_typed_flags<EscherGraphicProviderFlags, 0x01> {};
}

class MSFILTER_DLLPUBLIC EscherGraphicProvider
{
    EscherGraphicProviderFlags
                            mnFlags;
    std::vector<std::unique_ptr<EscherBlibEntry>>
                            mvBlibEntrys;
    OUString                maBaseURI;

protected:

    sal_uInt32              ImplInsertBlib( EscherBlibEntry* p_EscherBlibEntry );

public:

    sal_uInt32  GetBlibStoreContainerSize( SvStream const * pMergePicStreamBSE = nullptr ) const;
    void        WriteBlibStoreContainer( SvStream& rStrm, SvStream* pMergePicStreamBSE = nullptr  );
    void        WriteBlibStoreEntry(SvStream& rStrm, sal_uInt32 nBlipId, sal_uInt32 nResize);
    sal_uInt32  GetBlibID(
                    SvStream& rPicOutStream,
                    GraphicObject const & pGraphicObject,
                    const css::awt::Rectangle* pVisArea = nullptr,
                    const GraphicAttr* pGrafikAttr = nullptr,
                    const bool ooxmlExport = false
                );
    bool        HasGraphics() const { return !mvBlibEntrys.empty(); };

    void        SetNewBlipStreamOffset( sal_Int32 nOffset );

    bool        GetPrefSize( const sal_uInt32 nBlibId, Size& rSize, MapMode& rMapMode );

    void        SetBaseURI( const OUString& rBaseURI ) { maBaseURI = rBaseURI; };
    const OUString& GetBaseURI() { return maBaseURI; };

    EscherGraphicProvider( EscherGraphicProviderFlags nFlags  = EscherGraphicProviderFlags::NONE );
    virtual ~EscherGraphicProvider();

    EscherGraphicProvider& operator=( EscherGraphicProvider const & ) = delete; // MSVC2015 workaround
    EscherGraphicProvider( EscherGraphicProvider const & ) = delete; // MSVC2015 workaround
};

struct EscherShapeListEntry;

class MSFILTER_DLLPUBLIC EscherSolverContainer
{
    ::std::vector< std::unique_ptr<EscherShapeListEntry> >     maShapeList;
    ::std::vector< std::unique_ptr<EscherConnectorListEntry> > maConnectorList;

public:

    sal_uInt32      GetShapeId(
                        const css::uno::Reference< css::drawing::XShape > & rShape
                    ) const;

    void            AddShape(
                        const css::uno::Reference< css::drawing::XShape > &,
                        sal_uInt32 nId
                    );

    void            AddConnector(
                        const css::uno::Reference< css::drawing::XShape > &,
                        const css::awt::Point& rA,
                        css::uno::Reference< css::drawing::XShape > const &,
                        const css::awt::Point& rB,
                        css::uno::Reference< css::drawing::XShape > const & rConB
                    );

    void            WriteSolver( SvStream& );

                    EscherSolverContainer();
                    ~EscherSolverContainer();

    EscherSolverContainer& operator=( EscherSolverContainer const & ) = delete; // MSVC2015 workaround
    EscherSolverContainer( EscherSolverContainer const & ) = delete; // MSVC2015 workaround
};


#define ESCHER_CREATEPOLYGON_LINE           1
#define ESCHER_CREATEPOLYGON_POLYLINE       2
#define ESCHER_CREATEPOLYGON_POLYPOLYGON    4

class SdrObjCustomShape;

struct EscherPropSortStruct
{
    std::vector<sal_uInt8>    nProp;
    sal_uInt32  nPropValue;
    sal_uInt16  nPropId;
};

typedef std::vector< EscherPropSortStruct > EscherProperties;

class MSFILTER_DLLPUBLIC EscherPropertyContainer
{
    EscherGraphicProvider*  pGraphicProvider;
    SvStream*               pPicOutStrm;
    tools::Rectangle*              pShapeBoundRect;

    sal_uInt32              nCountCount;
    sal_uInt32              nCountSize;

    std::vector<EscherPropSortStruct>
                            pSortStruct;

    bool                    bHasComplexData;


    static sal_uInt32 ImplGetColor( const sal_uInt32 rColor, bool bSwap = true );
    void        ImplCreateGraphicAttributes(
                    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                    sal_uInt32 nBlibId,
                    bool bCreateCroppingAttributes
                );
    bool        ImplCreateEmbeddedBmp(GraphicObject const & rGraphicObject);

    SAL_DLLPRIVATE explicit EscherPropertyContainer(
        EscherGraphicProvider * pGraphProv, SvStream * pPiOutStrm,
        tools::Rectangle * pBoundRect);

public:

    EscherPropertyContainer();
    EscherPropertyContainer(
        EscherGraphicProvider& rGraphicProvider,    // the PropertyContainer needs to know
        SvStream* pPicOutStrm,                      // the GraphicProvider to be able to write
        tools::Rectangle& rShapeBoundRect                  // FillBitmaps or GraphicObjects.
    );                                              // under some circumstances the ShapeBoundRect
                                                    // is adjusted this will happen when rotated
                                                    // GraphicObjects are saved to PowerPoint
    ~EscherPropertyContainer();

    void AddOpt(
        sal_uInt16 nPropID,
        bool bBlib,
        sal_uInt32 nSizeReduction,
        SvMemoryStream& rStream);

    void AddOpt(
        sal_uInt16 nPropertyID,
        const OUString& rString);

    void AddOpt(
        sal_uInt16 nPropertyID,
        sal_uInt32 nPropValue,
        bool bBlib = false);

    void AddOpt(
        sal_uInt16 nPropertyID,
        bool bBlib,
        sal_uInt32 nPropValue,
        const std::vector<sal_uInt8>& rProp);

    bool        GetOpt( sal_uInt16 nPropertyID, sal_uInt32& rPropValue ) const;

    bool        GetOpt( sal_uInt16 nPropertyID, EscherPropSortStruct& rPropValue ) const;

    EscherProperties GetOpts() const;

    void        Commit( SvStream& rSt, sal_uInt16 nVersion = 3, sal_uInt16 nRecType = ESCHER_OPT );

    void        CreateShapeProperties(
                    const css::uno::Reference< css::drawing::XShape > & rXShape
                );
    bool        CreateOLEGraphicProperties(
                    const css::uno::Reference< css::drawing::XShape > & rXOleObject
                );
    bool        CreateGraphicProperties(
                    const css::uno::Reference< css::drawing::XShape > & rXShape,
                    const GraphicObject& rGraphicObj
                );
    bool        CreateMediaGraphicProperties(
                    const css::uno::Reference< css::drawing::XShape > & rXMediaObject
                );

    /** Creates a complex ESCHER_Prop_fillBlip containing the BLIP directly (for Excel charts). */
    void        CreateEmbeddedBitmapProperties(
                    css::uno::Reference<css::awt::XBitmap> const & rxBitmap,
                    css::drawing::BitmapMode eBitmapMode
                );
    /** Creates a complex ESCHER_Prop_fillBlip containing a hatch style (for Excel charts). */
    void        CreateEmbeddedHatchProperties(
                    const css::drawing::Hatch& rHatch,
                    const Color& rBackColor,
                    bool bFillBackground
                );

                    // the GraphicProperties will only be created if a GraphicProvider and PicOutStrm is known
                    // DR: #99897# if no GraphicProvider is present, a complex ESCHER_Prop_fillBlip
                    //             will be created, containing the BLIP directly (e.g. for Excel charts).
    bool        CreateGraphicProperties(
                    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                    const OUString& rSource,
                    const bool bCreateFillBitmap,
                    const bool bCreateCroppingAttributes = false,
                    const bool bFillBitmapModeAllowed = true,
                    const bool bOOxmlExport = false
                );

    bool        CreateBlipPropertiesforOLEControl( const css::uno::Reference< css::beans::XPropertySet > & rXPropSet, const css::uno::Reference< css::drawing::XShape > & rXShape);

    bool        CreatePolygonProperties(
                    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                    sal_uInt32 nFlags,
                    bool bBezier,
                    css::awt::Rectangle& rGeoRect,
                    tools::Polygon const * pPolygon = nullptr
                );

    static sal_uInt32 GetGradientColor(
                    const css::awt::Gradient* pGradient,
                    sal_uInt32 nStartColor
                );

    void        CreateGradientProperties( const css::awt::Gradient & rGradient );
    void        CreateGradientProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &,
                    bool bTransparentGradient = false
                );

    void        CreateLineProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &,
                    bool bEdge
                );
    void        CreateFillProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &,
                    bool bEdge,
                    bool bTransparentGradient = false );
    void        CreateFillProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &,
                    bool bEdge,
                    const css::uno::Reference< css::drawing::XShape > & rXShape );
    void        CreateTextProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &,
                    sal_uInt32 nText,
                    const bool bIsCustomShape = false,
                    const bool bIsTextFrame = true
                );

    bool        CreateConnectorProperties(
                    const css::uno::Reference< css::drawing::XShape > & rXShape,
                    EscherSolverContainer& rSolver,
                    css::awt::Rectangle& rGeoRect,
                    sal_uInt16& rShapeType,
                    ShapeFlag& rShapeFlags
                );

                // Because shadow properties depends to the line and fillstyle, the CreateShadowProperties method should be called at last.
                // It's active only when at least a FillStyle or LineStyle is set.
    void        CreateShadowProperties(
                    const css::uno::Reference< css::beans::XPropertySet > &
                );

    sal_Int32   GetValueForEnhancedCustomShapeParameter( const css::drawing::EnhancedCustomShapeParameter& rParameter,
                            const std::vector< sal_Int32 >& rEquationOrder, bool bAdjustTrans = false );
        // creates all necessary CustomShape properties, this includes also Text-, Shadow-, Fill-, and LineProperties
    void        CreateCustomShapeProperties(
                    const MSO_SPT eShapeType,
                    const css::uno::Reference< css::drawing::XShape > &
                );
    bool        IsFontWork() const;

    // helper functions which are also used by the escher import
    static tools::PolyPolygon  GetPolyPolygon(
                            const css::uno::Reference< css::drawing::XShape > & rXShape
                        );
    static tools::PolyPolygon  GetPolyPolygon( const css::uno::Any& rSource );
    static MSO_SPT      GetCustomShapeType(
                            const css::uno::Reference< css::drawing::XShape > & rXShape,
                            ShapeFlag& nMirrorFlags,
                            OUString& rShapeType,
                            bool bOOXML = false
                        );

    // helper functions which are also used in ooxml export
    static bool         GetLineArrow(
                            const bool bLineStart,
                            const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                            ESCHER_LineEnd& reLineEnd,
                            sal_Int32& rnArrowLength,
                            sal_Int32& rnArrowWidth
                        );

    static bool IsDefaultObject(
        const SdrObjCustomShape& rSdrObjCustomShape,
        const MSO_SPT eShapeType);

    static void         LookForPolarHandles(
                            const MSO_SPT eShapeType,
                            sal_Int32& nAdjustmentsWhichNeedsToBeConverted
                        );
    static bool         GetAdjustmentValue( const css::drawing::EnhancedCustomShapeAdjustmentValue & rkProp, sal_Int32 nIndex, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, sal_Int32& nValue );
};


class MSFILTER_DLLPUBLIC EscherPersistTable
{

public:
    ::std::vector< std::unique_ptr<EscherPersistEntry> > maPersistTable;

    bool        PtIsID( sal_uInt32 nID );
    void        PtInsert( sal_uInt32 nID, sal_uInt32 nOfs );
    void        PtDelete( sal_uInt32 nID );
    sal_uInt32  PtGetOffsetByID( sal_uInt32 nID );
    void        PtReplace( sal_uInt32 nID, sal_uInt32 nOfs );
    void        PtReplaceOrInsert( sal_uInt32 nID, sal_uInt32 nOfs );

                EscherPersistTable();
    virtual     ~EscherPersistTable();

    EscherPersistTable& operator=( EscherPersistTable const & ) = delete; // MSVC2015 workaround
    EscherPersistTable( EscherPersistTable const & ) = delete; // MSVC2015 workaround
};


class EscherEx;

/// abstract base class for ESCHER_ClientTextbox, ESCHER_ClientData
class MSFILTER_DLLPUBLIC EscherExClientRecord_Base
{
public:
                                EscherExClientRecord_Base() {}
    virtual                     ~EscherExClientRecord_Base();

                                /// Application writes the record header
                                /// using rEx.AddAtom(...) followed by
                                /// record data written to rEx.GetStream()
    virtual void                WriteData( EscherEx& rEx ) const = 0;
};


/// abstract base class for ESCHER_ClientAnchor
class MSFILTER_DLLPUBLIC EscherExClientAnchor_Base
{
public:
                                EscherExClientAnchor_Base() {}
    virtual                     ~EscherExClientAnchor_Base();

                                /// Application writes the record header
                                /// using rEx.AddAtom(...) followed by
                                /// record data written to rEx.GetStream()
    virtual void                WriteData( EscherEx& rEx,
                                    const tools::Rectangle& rRect ) = 0;
};

class InteractionInfo
{
    std::unique_ptr<SvMemoryStream>       mpHyperlinkRecord;

public:
    InteractionInfo( SvMemoryStream* pStream )
    {
        mpHyperlinkRecord.reset( pStream );
    }
    const std::unique_ptr< SvMemoryStream >&  getHyperlinkRecord() { return mpHyperlinkRecord; }
};

class EscherExHostAppData
{
private:
        EscherExClientAnchor_Base*  pClientAnchor;
        EscherExClientRecord_Base*  pClientData;
        EscherExClientRecord_Base*  pClientTextbox;
        InteractionInfo*            pInteractionInfo;
        // ignore single shape if entire pages are written
        bool                        bDontWriteShape;

public:
        EscherExHostAppData() : pClientAnchor(nullptr), pClientData(nullptr),
                pClientTextbox(nullptr), pInteractionInfo(nullptr), bDontWriteShape(false)
        {}

        void SetInteractionInfo( InteractionInfo* p )
            { pInteractionInfo = p; }
        void SetClientAnchor( EscherExClientAnchor_Base* p )
            { pClientAnchor = p; }
        void SetClientData( EscherExClientRecord_Base* p )
            { pClientData = p; }
        void SetClientTextbox( EscherExClientRecord_Base* p )
            { pClientTextbox = p; }
        void SetDontWriteShape( bool b )
            { bDontWriteShape = b; }
        InteractionInfo* GetInteractionInfo() const
            { return pInteractionInfo; }
        EscherExClientAnchor_Base* GetClientAnchor() const
            { return pClientAnchor; }
        EscherExClientRecord_Base* GetClientTextbox() const
            { return pClientTextbox; }

        void WriteClientAnchor( EscherEx& rEx, const tools::Rectangle& rRect )
            { if( pClientAnchor )  pClientAnchor->WriteData( rEx, rRect ); }
        void WriteClientData( EscherEx& rEx )
            { if( pClientData ) pClientData->WriteData( rEx ); }
        void WriteClientTextbox( EscherEx& rEx )
            { if( pClientTextbox ) pClientTextbox->WriteData( rEx ); }

        bool DontWriteShape() const { return bDontWriteShape; }
};


/** Instance for global DFF data, shared through various instances of EscherEx. */
class MSFILTER_DLLPUBLIC EscherExGlobal : public EscherGraphicProvider
{
public:
    explicit            EscherExGlobal();
    virtual             ~EscherExGlobal() override;

    /** Returns a new drawing ID for a new drawing container (DGCONTAINER). */
    sal_uInt32          GenerateDrawingId();
    /** Creates and returns a new shape identifier, updates the internal shape
        counters and registers the identifier in the DGG cluster table.
        @param nDrawingId  Drawing identifier has to be passed to be able to
            generate shape identifiers for multiple drawings simultaniously. */
    sal_uInt32          GenerateShapeId( sal_uInt32 nDrawingId, bool bIsInSpgr );
    /** Returns the number of shapes in the current drawing, based on number of
        calls to the GenerateShapeId() function. */
    sal_uInt32          GetDrawingShapeCount( sal_uInt32 nDrawingId ) const;
    /** Returns the last shape identifier generated by the GenerateShapeId()
        function. */
    sal_uInt32          GetLastShapeId( sal_uInt32 nDrawingId ) const;

    /** Sets the flag indicating that the DGGCONTAINER exists. */
    void         SetDggContainer() { mbHasDggCont = true; }
    /** Sets the flag indicating that the DGGCONTAINER exists. */
    bool         HasDggContainer() const { return mbHasDggCont; }
    /** Returns the total size of the DGG atom (including header). */
    sal_uInt32          GetDggAtomSize() const;
    /** Writes the complete DGG atom to the passed stream (overwrites existing data!). */
    void                WriteDggAtom( SvStream& rStrm ) const;

    /** Called if a picture shall be written and no picture stream is set at
        class ImplEESdrWriter.

        On first invocation, this function calls the virtual member function
        ImplQueryPictureStream(). The return value will be cached internally
        for subsequent calls and for the GetPictureStream() function.
     */
    SvStream*           QueryPictureStream();

    /** Returns the picture stream if existing (queried), otherwise null. */
    SvStream*    GetPictureStream() { return mpPicStrm; }

private:
    /** Derived classes may implement to create a new stream used to store the
        picture data.

        The implementation has to take care about lifetime of the returned
        stream (it will not be destructed automatically). This function is
        called exactly once. The return value will be cached internally for
        repeated calls of the public QueryPictureStream() function.
     */
    virtual SvStream*   ImplQueryPictureStream();

private:
    struct ClusterEntry
    {
        sal_uInt32          mnDrawingId;        /// Identifier of drawing this cluster belongs to (one-based index into maDrawingInfos).
        sal_uInt32          mnNextShapeId;      /// Next free shape identifier in this cluster.
        explicit     ClusterEntry( sal_uInt32 nDrawingId ) : mnDrawingId( nDrawingId ), mnNextShapeId( 0 ) {}
    };
    typedef ::std::vector< ClusterEntry > ClusterTable;

    struct DrawingInfo
    {
        sal_uInt32          mnClusterId;        /// Currently used cluster (one-based index into maClusterTable).
        sal_uInt32          mnShapeCount;       /// Current number of shapes in this drawing.
        sal_uInt32          mnLastShapeId;      /// Last shape identifier generated for this drawing.
        explicit     DrawingInfo( sal_uInt32 nClusterId ) : mnClusterId( nClusterId ), mnShapeCount( 0 ), mnLastShapeId( 0 ) {}
    };
    typedef ::std::vector< DrawingInfo > DrawingInfoVector;

    ClusterTable        maClusterTable;     /// List with cluster IDs (used object IDs in drawings).
    DrawingInfoVector   maDrawingInfos;     /// Data about all used drawings.
    SvStream*           mpPicStrm;          /// Cached result of ImplQueryPictureStream().
    bool                mbHasDggCont;       /// True = the DGGCONTAINER has been initialized.
    bool                mbPicStrmQueried;   /// True = ImplQueryPictureStream() has been called.
};

class SdrObject;
class SdrPage;
class ImplEESdrWriter;

class MSFILTER_DLLPUBLIC EscherEx : public EscherPersistTable
{
    protected:
        std::shared_ptr<EscherExGlobal>           mxGlobal;
        ::std::unique_ptr< ImplEESdrWriter > mpImplEESdrWriter;
        SvStream*                   mpOutStrm;
        bool                        mbOwnsStrm;
        sal_uInt32                  mnStrmStartOfs;
        std::vector< sal_uInt32 >   mOffsets;
        std::vector< sal_uInt16 >   mRecTypes;

        sal_uInt32                  mnCurrentDg;
        sal_uInt32                  mnCountOfs;

        sal_uInt32                  mnGroupLevel;
        SdrLayerID                  mnHellLayerId;

        bool                        mbEscherSpgr;
        bool                        mbEscherDg;
        bool                        mbOOXML;
        OUString                    mEditAs;


        bool DoSeek( sal_uInt32 nKey );

public:
    explicit            EscherEx( const std::shared_ptr<EscherExGlobal>& rxGlobal, SvStream* pOutStrm, bool bOOXML = false );
    virtual             ~EscherEx() override;

    /** Creates and returns a new shape identifier, updates the internal shape
        counters and registers the identifier in the DGG cluster table. */
    virtual sal_uInt32   GenerateShapeId() { return mxGlobal->GenerateShapeId( mnCurrentDg, mbEscherSpgr ); }

    /** Returns the graphic provider from the global object that has been
        passed to the constructor.
     */
    EscherGraphicProvider& GetGraphicProvider() { return *mxGlobal; }

    /** Called if a picture shall be written and no picture stream is set at
        class ImplEESdrWriter.
     */
    SvStream*    QueryPictureStream() { return mxGlobal->QueryPictureStream(); }

                /// Inserts internal data into the EscherStream, this process
                /// may and has to be executed only once
                /// If pPicStreamMergeBSE is known, the BLIPs from this stream are being
                /// merged into the MsofbtBSE Records of the EscherStream like it's
                /// required for Excel (and maybe Word?)
        void Flush( SvStream* pPicStreamMergeBSE = nullptr );

    /** Inserts the passed number of bytes at the current position of the
        output stream.

        Inserts dummy bytes and moves all following stream data, and updates
        all internal stream offsets stored in the PersistTable and the affected
        container sizes, which makes this operation very expensive. (!)

        @param nBytes  The number of bytes to be inserted into the stream.

        An atom that currently ends
        exactly at the current stream position will not be expanded to
        include the inserted data (used to insert e.g. a new atom after an
        existing atom). Note that containers that end exactly at the
        current stream position are always expanded to include the inserted
        data.
     */
    void            InsertAtCurrentPos( sal_uInt32 nBytes );

    void            InsertPersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset ); // It is not being checked if this key is already in the PersistantTable
    void            ReplacePersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset );
    sal_uInt32      GetPersistOffset( sal_uInt32 nKey );
    bool            SeekToPersistOffset( sal_uInt32 nKey );
    void            InsertAtPersistOffset( sal_uInt32 nKey, sal_uInt32 nValue );   // nValue is being inserted into the Stream where it's appropriate (overwrite mode), without that the
                                                                                    // current StreamPosition changes
    void            SetEditAs( const OUString& rEditAs );
    const OUString& GetEditAs() { return mEditAs; }
    SvStream&       GetStream() const   { return *mpOutStrm; }
    sal_uLong       GetStreamPos() const    { return mpOutStrm->Tell(); }

                // features during the creation of the following Containers:

                //      ESCHER_DggContainer:    a EscherDgg Atom is automatically being created and managed
                //      ESCHER_DgContainer:     a EscherDg Atom is automatically being created and managed
                //      ESCHER_SpgrContainer:
                //      ESCHER_SpContainer:

    virtual void OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance = 0 );
    virtual void CloseContainer();

    void BeginAtom();
    void EndAtom( sal_uInt16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
    void AddAtom( sal_uInt32 nAtomSitze, sal_uInt16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
    void AddChildAnchor( const tools::Rectangle& rRectangle );
    void AddClientAnchor( const tools::Rectangle& rRectangle );

    virtual sal_uInt32 EnterGroup( const OUString& rShapeName, const tools::Rectangle* pBoundRect );
    sal_uInt32  EnterGroup( const tools::Rectangle* pBoundRect = nullptr );
    sal_uInt32  GetGroupLevel() const { return mnGroupLevel; };
    void SetGroupSnapRect( sal_uInt32 nGroupLevel, const tools::Rectangle& rRect );
    void SetGroupLogicRect( sal_uInt32 nGroupLevel, const tools::Rectangle& rRect );
    virtual void LeaveGroup();

                // a ESCHER_Sp is being written ( a ESCHER_DgContainer has to be opened for this purpose!)
    virtual void AddShape( sal_uInt32 nShpInstance, ShapeFlag nFlagIds, sal_uInt32 nShapeID = 0 );

    virtual void Commit( EscherPropertyContainer& rProps, const tools::Rectangle& rRect);

    static sal_uInt32  GetColor( const sal_uInt32 nColor );
    static sal_uInt32  GetColor( const Color& rColor );

                // ...Sdr... implemented in eschesdo.cxx

    void    AddSdrPage( const SdrPage& rPage );
    void    AddUnoShapes( const css::uno::Reference< css::drawing::XShapes >& rxShapes );

                /// returns the ShapeID
    sal_uInt32  AddSdrObject( const SdrObject& rObj, bool ooxmlExport = false );
    virtual void  AddSdrObjectVMLObject( const SdrObject& /*rObj*/)
    {
        // Required for Exporting VML shape
    }

                /// If objects are written through AddSdrObject the
                /// SolverContainer has to be written, and maybe some
                /// maintenance to be done.
    void    EndSdrObjectPage();

                /// Called before a shape is written, application supplies
                /// ClientRecords. May set AppData::bDontWriteShape so the
                /// shape is ignored.
    virtual EscherExHostAppData* StartShape(
                            const css::uno::Reference< css::drawing::XShape >& rShape,
                            const tools::Rectangle* pChildAnchor );

                /// Called after a shape is written to inform the application
                /// of the resulted shape type and ID.
    virtual void    EndShape( sal_uInt16 nShapeType, sal_uInt32 nShapeID );

                /// Called before an AdditionalText EnterGroup occurs.
                /// The current shape will be written in three parts:
                /// a group shape, the shape itself, and an extra textbox shape.
                /// The complete flow is:
                /// StartShape sets HostData1.
                /// EnterAdditionalTextGroup sets HostData2, App may modify
                ///   HostData1 and keep track of the change.
                /// The group shape is written with HostData2.
                /// Another StartShape with the same (!) object sets HostData3.
                /// The current shape is written with HostData3.
                /// EndShape is called for the current shape.
                /// Another StartShape with the same (!) object sets HostData4.
                /// The textbox shape is written with HostData4.
                /// EndShape is called for the textbox shape.
                /// EndShape is called for the group shape, this provides
                ///   the same functionality as an ordinary recursive group.
    virtual EscherExHostAppData*    EnterAdditionalTextGroup();

                /// Called if an ESCHER_Prop_lTxid shall be written
    virtual sal_uInt32  QueryTextID( const css::uno::Reference< css::drawing::XShape >&, sal_uInt32 nShapeId );
            // add an dummy rectangle shape into the escher stream
        sal_uInt32  AddDummyShape();

    static const SdrObject* GetSdrObject( const css::uno::Reference< css::drawing::XShape >& rXShape );

    void SetHellLayerId( SdrLayerID nId )       { mnHellLayerId = nId; }
    SdrLayerID GetHellLayerId() const           { return mnHellLayerId; }

private:
                        EscherEx( const EscherEx& ) = delete;
    EscherEx&           operator=( const EscherEx& ) = delete;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
