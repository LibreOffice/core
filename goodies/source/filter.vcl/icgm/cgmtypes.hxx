/*************************************************************************
 *
 *  $RCSfile: cgmtypes.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#ifndef CGM_TYPES_HXX_
#define CGM_TYPES_HXX_

struct FloatPoint
{
        double          X;
        double          Y;
        FloatPoint(){};
        FloatPoint( const double& rX, const double& rY ) { X = rX, Y = rY; };
};

struct FloatRect
{
        double          Left;
        double          Top;
        double          Right;
        double          Bottom;
        FloatRect(){};
        FloatRect( const FloatPoint& rTopLeft, const FloatPoint& rBottomRight )
        {
            Left = rTopLeft.X;
            Top = rTopLeft.Y;
            Right = rBottomRight.X;
            Bottom = rBottomRight.Y;
        }
        void Justify()
        {
            double fTemp;
            if ( Left > Right )
            {
                fTemp = Left;
                Left = Right;
                Right = fTemp;
            }
            if ( Top > Bottom )
            {
                fTemp = Top;
                Top = Bottom;
                Bottom = fTemp;
            }
        }
};

struct HatchEntry
{
    int     HatchStyle;
    long    HatchDistance;
    long    HatchAngle;
};

#define ASF_LINETYPE            0x00000001UL
#define ASF_LINEWIDTH           0x00000002UL
#define ASF_LINECOLOR           0x00000004UL
#define ASF_MARKERTYPE          0x00000008UL
#define ASF_MARKERSIZE          0x00000010UL
#define ASF_MARKERCOLOR         0x00000020UL    // NS
#define ASF_FILLINTERIORSTYLE   0x00000040UL
#define ASF_HATCHINDEX          0x00000080UL
#define ASF_PATTERNINDEX        0x00000100UL
#define ASF_BITMAPINDEX         0x00000200UL    // NS
#define ASF_FILLCOLOR           0x00000400UL
#define ASF_EDGETYPE            0x00000800UL
#define ASF_EDGEWIDTH           0x00001000UL
#define ASF_EDGECOLOR           0x00002000UL
#define ASF_TEXTFONTINDEX       0x00004000UL
#define ASF_TEXTPRECISION       0x00008000UL
#define ASF_CHARACTEREXPANSION  0x00010000UL
#define ASF_CHARACTERSPACING    0x00020000UL
#define ASF_TEXTCOLOR           0x00040000UL

#define ACT4_GRADIENT_ACTION    0x00000001UL

enum RealPrecision      { RP_FLOAT = 0, RP_FIXED = 1 };

enum ScalingMode        { SM_ABSTRACT = 0, SM_METRIC = 1 };

enum VDCType            { VDC_INTEGER = 0, VDC_REAL = 1 };
enum DeviceViewPortMode { DVPM_FRACTION = 0, DVPM_METRIC = 1, DVPM_DEVICE = 2 };
enum DeviceViewPortMap  { DVPM_NOT_FORCED = 0, DVPM_FORCED = 1 };
enum DeviceViewPortMapH { DVPMH_LEFT = 0, DVPMH_CENTER = 1, CVPMH_RIGHT = 2 };
enum DeviceViewPortMapV { DVPMV_BOTTOM = 0, DVPMV_CENTER = 1, DVPMV_TOP = 2 };

enum ClipIndicator      { CI_OFF = 0, CI_ON = 1 };

enum ColorSelectionMode { CSM_INDEXED = 0, CSM_DIRECT = 1 };
enum ColorModel         { CM_RGB = 0, CM_CYMK = 1 };

enum CharacterCodingA   { CCA_BASIC_7 = 0, CCA_BASIC_8 = 1, CCA_EXT_7 = 2, CCA_EXT_8 = 3 };
enum CharSetType        { CST_CBYTE_94 = 0, CST_CBYTE_96 = 1, CST_MULT94 = 2, CST_MULT96 = 3, CST_CCOMPLETE = 4 };
enum TextPrecision      { TPR_STRING = 0, TPR_CHARACTER = 1, TPR_STROKE = 2, TPR_UNDEFINED = 0xffff };
enum TextPath           { TPR_RIGHT = 0, TPR_LEFT = 1, TPR_UP = 2, TPR_DOWN = 3 };
enum TextAlignmentH     { TAH_NORMAL = 0, TAH_LEFT = 1, TAH_CENTER = 2, TAH_RIGHT = 3, TAH_CONT = 4 };
enum TextAlignmentV     { TAV_NORMAL = 0, TAV_TOP = 1, TAV_CAP = 2, TAV_HALF = 3, TAV_BASE = 4, TAV_BOTTOM = 5, TAV_CONT = 6 };
enum UnderlineMode      { UM_OFF = 0, UM_LOW = 1, UM_HIGH = 2, UM_STRIKEOUT = 4, UM_OVERSCORE = 8 };
enum FinalFlag          { FF_NOT_FINAL = 0, FF_FINAL = 1 };

enum LineType           { LT_SOLID = 1, LT_DASH = 2, LT_DOT = 3, LT_DASHDOT = 4, LT_DASHDOTDOT = 5,     // Standart
                            LT_NONE = -4, LT_DOTDOTSPACE = -3, LT_LONGDASH = -2, LT_DASHDASHDOT = -1 }; // GDSF Styles
enum SpecMode           { SM_ABSOLUTE = 0, SM_SCALED = 1 };
enum LineCapType        { LCT_BUTT = 0, LCT_ROUND = 1, LCT_SQUARE = 2, LCT_TRIANGLE = 3, LCT_ARROW = 4, LCT_NONE = -1 };
enum LineJoinType       { LJT_MITER = 0, LJT_ROUND = 1, LJT_BEVEL = 2, LJT_NONE = -1 };


enum EdgeType           { ET_SOLID = 1, ET_DASH = 2, ET_DOT = 3, ET_DASHDOT = 4, ET_DASHDOTDOT = 5,     // Standart
                            ET_NONE = -4, ET_DOTDOTSPACE = -3, ET_LONGDASH = -2, ET_DASHDASHDOT = -1 }; // GDSF Styles
enum EdgeVisibility     { EV_OFF = 0, EV_ON = 1 };

enum MarkerType         { MT_POINT = 1, MT_PLUS = 2, MT_STAR = 3, MT_CIRCLE = 4, MT_CROSS = 5 };

enum Transparency       { T_OFF = 0, T_ON = 1 };

enum FillInteriorStyle  { FIS_HOLLOW = 0, FIS_SOLID = 1, FIS_PATTERN = 2, FIS_HATCH = 3, FIS_EMPTY = 4, FIS_GEOPATTERN = 5,
                            FIS_INTERPOLATED = 6, FIS_GRADIENT = 7 };




#endif
