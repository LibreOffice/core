/*************************************************************************
 *
 *  $RCSfile: classx.cxx,v $
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

#include <main.hxx>
#include <outact.hxx>

// ---------------------------------------------------------------

#define ImplSetUnderlineMode()                                  \
                                                                \
    sal_uInt32 nMode = ImplGetUI16();                               \
    switch ( nMode )                                            \
    {                                                           \
        case 1 : pElement->eUnderlineMode = UM_LOW; break;      \
        case 2 : pElement->eUnderlineMode = UM_HIGH; break;     \
        case 4 : pElement->eUnderlineMode = UM_STRIKEOUT; break;\
        case 8 : pElement->eUnderlineMode = UM_OVERSCORE; break;\
        default: pElement->eUnderlineMode = UM_OFF; break;      \
    }                                                           \
    pElement->nUnderlineColor = ImplGetBitmapColor();           \
    if ( mnMode & CGM_IMPORT_IM )                               \
        mnParaSize = mnElementSize;

// ---------------------------------------------------------------

void CGM::ImplDoClass6()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1, "Escape" )
        {
//          if ( mnMode & CGM_IMPORT_IM )           // IMAGE MARK ONLY ????
            {
                long    nIdentifier = ImplGetI( pElement->nIntegerPrecision );
                switch ( nIdentifier )
                {
                    case 0 : ComOut( CGM_DESCRIPTION, "inquire function support" ) break;
                    case -1 : ComOut( CGM_DESCRIPTION, "set underline mode" )
                    {
                        ImplSetUnderlineMode()
                    }
                    break;
                    case -2 : ComOut( CGM_DESCRIPTION, "set script mode" ) break;
                    case -3 : ComOut( CGM_DESCRIPTION, "set shadow mode" ) break;
                    case -6 : ComOut( CGM_DESCRIPTION, "inquire origin offset" ) break;
                    case -8 : ComOut( CGM_DESCRIPTION, "set media size" ) break;
                    case -10 : ComOut( CGM_DESCRIPTION, "set character mode" ) break;
                    case -14 : ComOut( CGM_DESCRIPTION, "resolution mode" ) break;
                    case -17 : ComOut( CGM_DESCRIPTION, "line cap" ) break;
                    case -18 : ComOut( CGM_DESCRIPTION, "line join" ) break;
                    case -19 : ComOut( CGM_DESCRIPTION, "edge join" ) break;
                    case -30 : ComOut( CGM_DESCRIPTION, "media type" ) break;
                    case -31 : ComOut( CGM_DESCRIPTION, "number of copies" ) break;
                    case -32 : ComOut( CGM_DESCRIPTION, "orientation" ) break;
                    case -33 : ComOut( CGM_DESCRIPTION, "device color representation" ) break;
                    case -34 : ComOut( CGM_DESCRIPTION, "device font list" ) break;
                    case -35 : ComOut( CGM_DESCRIPTION, "color reversal mode" ) break;
                    case -36 : ComOut( CGM_DESCRIPTION, "line cap attributes" ) break;
                    case -37 : ComOut( CGM_DESCRIPTION, "begin effect" ) break;
                    case -38 : ComOut( CGM_DESCRIPTION, "end effect" ) break;
                    case -39 : ComOut( CGM_DESCRIPTION, "begin effect definition" ) break;
                    case -40 : ComOut( CGM_DESCRIPTION, "end effect definition" ) break;
                    case -41 : ComOut( CGM_DESCRIPTION, "end style definition" ) break;
                    case -42 : ComOut( CGM_DESCRIPTION, "begin eps data" ) break;
                    case -43 : ComOut( CGM_DESCRIPTION, "eps data" ) break;
                    case -44 : ComOut( CGM_DESCRIPTION, "end eps data" ) break;
                    case -45 : ComOut( CGM_DESCRIPTION, "set background style" ) break;
                    case -46 : ComOut( CGM_DESCRIPTION, "set eps mode" ) break;
                    case -47 : ComOut( CGM_DESCRIPTION, "fill mode" ) break;
                    case -48 : ComOut( CGM_DESCRIPTION, "begin symbol" ) break;
                    case -49 : ComOut( CGM_DESCRIPTION, "end symbol" ) break;
                    case -50 : ComOut( CGM_DESCRIPTION, "begin layer" ) break;
                    case -51 : ComOut( CGM_DESCRIPTION, "end layer" ) break;
                    case -52 : ComOut( CGM_DESCRIPTION, "layer visibility" ) break;
                    case -53 : ComOut( CGM_DESCRIPTION, "inquire foreign data" ) break;
                    case -54 : ComOut( CGM_DESCRIPTION, "set text offset" ) break;
                    case -55 : ComOut( CGM_DESCRIPTION, "begin group" ) break;
                    case -56 : ComOut( CGM_DESCRIPTION, "end group" ) break;
                    case -100 : ComOut( CGM_DESCRIPTION, "begin patch" ) break;
                    case -101 : ComOut( CGM_DESCRIPTION, "end patch" ) break;
                    case -102 : ComOut( CGM_DESCRIPTION, "begin block text region" ) break;
                    case -103 : ComOut( CGM_DESCRIPTION, "end block text region" ) break;
                    case -120 : ComOut( CGM_DESCRIPTION, "region margins" ) break;
                    case -121 : ComOut( CGM_DESCRIPTION, "region expansions" ) break;
                    case -122 : ComOut( CGM_DESCRIPTION, "region anchor" ) break;
                    case -123 : ComOut( CGM_DESCRIPTION, "paragraph horizontal align" ) break;
                    case -124 : ComOut( CGM_DESCRIPTION, "paragraph vertical align" ) break;
                    case -125 : ComOut( CGM_DESCRIPTION, "region line flow" ) break;
                    case -130 : ComOut( CGM_DESCRIPTION, "paragraph spacing" ) break;
                    case -131 : ComOut( CGM_DESCRIPTION, "paragraph identation" ) break;
                    case -132 : ComOut( CGM_DESCRIPTION, "paragraph tabs" ) break;
                    case -133 : ComOut( CGM_DESCRIPTION, "paragraph bullet" ) break;
                    case -134 : ComOut( CGM_DESCRIPTION, "paragraph bulet level" ) break;
                    case -135 : ComOut( CGM_DESCRIPTION, "line horuzontal align" ) break;
                    case -136 : ComOut( CGM_DESCRIPTION, "line vertical align" ) break;
                    case -137 : ComOut( CGM_DESCRIPTION, "line spacing" ) break;
                    case -138 : ComOut( CGM_DESCRIPTION, "word wrap" ) break;
                    case -150 : ComOut( CGM_DESCRIPTION, "forward advance distance" ) break;
                    case -151 : ComOut( CGM_DESCRIPTION, "word spacing" ) break;
                    case -152 : ComOut( CGM_DESCRIPTION, "external leading" ) break;
                    case -160 : ComOut( CGM_DESCRIPTION, "set gradient offset" ) break;
                    case -161 : ComOut( CGM_DESCRIPTION, "set gradient edge" ) break;
                    case -162 : ComOut( CGM_DESCRIPTION, "set gradient angle" ) break;
                    case -163 : ComOut( CGM_DESCRIPTION, "set gradient description" ) break;
                    case -164 : ComOut( CGM_DESCRIPTION, "set gradient style" ) break;
                    case -165 : ComOut( CGM_DESCRIPTION, "set back ground style" ) break;
                    case -170 : ComOut( CGM_DESCRIPTION, "geometric pattern draw style" ) break;
                    case -190 : ComOut( CGM_DESCRIPTION, "set character width" ) break;
                    case -191 : ComOut( CGM_DESCRIPTION, "hyperlink definitions" ) break;
                    case -192 : ComOut( CGM_DESCRIPTION, "set color name for pantone" ) break;
                    case -32746 : ComOut( CGM_DESCRIPTION, "set text font" ) break;
                    case -32747 : ComOut( CGM_DESCRIPTION, "font selection mode" ) break;
                    case -32752 : ComOut( CGM_DESCRIPTION, "connecting edge" ) break;
                    case -32753 : ComOut( CGM_DESCRIPTION, "set drawing mode" ) break;
                    case -32754 : ComOut( CGM_DESCRIPTION, "inquire clip rectangle" ) break;
                    case -32755 : ComOut( CGM_DESCRIPTION, "protection region indicator" ) break;
                    case -32756 : ComOut( CGM_DESCRIPTION, "end protection region" ) break;
                    case -32757 : ComOut( CGM_DESCRIPTION, "begin protection region" ) break;
                    case -32758 : ComOut( CGM_DESCRIPTION, "geometric pattern definition" ) break;
                    case -32759 : ComOut( CGM_DESCRIPTION, "hatch style definition" ) break;
                    case -32760 : ComOut( CGM_DESCRIPTION, "close figure" ) break;
                    case -32761 : ComOut( CGM_DESCRIPTION, "end figure" )
                    {
                        mpOutAct->EndFigure();
                        mbFigure = sal_False;
                    }
                    break;
                    case -32762 : ComOut( CGM_DESCRIPTION, "begin figure" )
                    {
                        mbFigure = sal_True;
                        mpOutAct->BeginFigure();
                    }
                    break;
                    case -32763 : ComOut( CGM_DESCRIPTION, "pop transformatin" ) break;
                    case -32764 : ComOut( CGM_DESCRIPTION, "push transformation" ) break;
                    case -32765 : ComOut( CGM_DESCRIPTION, "copy segment" ) break;
                    case -32766 : ComOut( CGM_DESCRIPTION, "endseg" ) break;
                    case -32767 : ComOut( CGM_DESCRIPTION, "begin segment" ) break;
                    default : ComOut( CGM_DESCRIPTION, "????????????????????????????????" ) break;
                }
                mnParaSize = mnElementSize;

            }
        }
        break;
        case 0x02 : ComOut( CGM_LEVEL1, "Get Escape" ) break;
        case 0x11 : ComOut( CGM_GDSF_ONLY, "Set Underline Mode" )
        {
            ImplSetUnderlineMode();
        }
        break;
        case 0x12 : ComOut( CGM_GDSF_ONLY, "Set Script Mode" ) break;
        case 0x13 : ComOut( CGM_GDSF_ONLY, "Set Shadow Mode" ) break;
        case 0x18 : ComOut( CGM_GDSF_ONLY, "Set Media Size" ) break;
        case 0x20 : ComOut( CGM_GDSF_ONLY, "Set Character Mode" ) break;
        case 0x24 : ComOut( CGM_GDSF_ONLY, "Resolution Mode" ) break;
        case 0x27 : ComOut( CGM_GDSF_ONLY, "Line Cap" ) break;
        case 0x28 : ComOut( CGM_GDSF_ONLY, "Line Join" ) break;
        case 0x29 : ComOut( CGM_GDSF_ONLY, "Edge Join" ) break;
        case 0x40 : ComOut( CGM_GDSF_ONLY, "Media Type" ) break;
        case 0x41 : ComOut( CGM_GDSF_ONLY, "Number of Copies" ) break;
        case 0x42 : ComOut( CGM_GDSF_ONLY, "Origin" ) break;
        case 0x45 : ComOut( CGM_GDSF_ONLY, "Color Reversal Mode" ) break;
        case 0x46 : ComOut( CGM_GDSF_ONLY, "Line Cap Attributes" ) break;
        case 0x49 : ComOut( CGM_GDSF_ONLY, "Begin Effect Definition" ) break;
        case 0x50 : ComOut( CGM_GDSF_ONLY, "End Effect Definition" ) break;
        case 0x51 : ComOut( CGM_GDSF_ONLY, "Line End Style Attributes" ) break;
        case 0x52 : ComOut( CGM_GDSF_ONLY, "Begin Data" ) break;
        case 0x53 : ComOut( CGM_GDSF_ONLY, "Data" ) break;
        case 0x54 : ComOut( CGM_GDSF_ONLY, "End Data" ) break;
        case 0x55 : ComOut( CGM_GDSF_ONLY, "Set Background Style" ) break;
        case 0x56 : ComOut( CGM_GDSF_ONLY, "Set EPS Mode" ) break;
        case 0x57 : ComOut( CGM_GDSF_ONLY, "Fill Mode" ) break;
        case 0x58 : ComOut( CGM_GDSF_ONLY, "Begin Symbol" ) break;
        case 0x59 : ComOut( CGM_GDSF_ONLY, "End Symbol" ) break;
        case 0x60 : ComOut( CGM_GDSF_ONLY, "Begin Layer" ) break;
        case 0x61 : ComOut( CGM_GDSF_ONLY, "End Layer" ) break;
        case 0x62 : ComOut( CGM_GDSF_ONLY, "Layer Visibility" ) break;
        case 0x64 : ComOut( CGM_GDSF_ONLY, "Set Text Offset" ) break;
        case 0xFF : ComOut( CGM_GDSF_ONLY, "Inquire Function Support" ) break;
        case 0xFE : ComOut( CGM_GDSF_ONLY, "Inquire Origin" ) break;
        case 0xFD : ComOut( CGM_GDSF_ONLY, "Inquire Foreign Data Mode" ) break;
        case 0xFC : ComOut( CGM_GDSF_ONLY, "Inquire Text Extent" ) break;
        case 0xFB : ComOut( CGM_GDSF_ONLY, "Inquire DPI" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass8()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL2, "Copy Segment" ) break;                     // NS
        case 0x02 : ComOut( CGM_LEVEL2, "Inheritance Filter" ) break;               // NS
        case 0x03 : ComOut( CGM_LEVEL2, "Clip Inheritance" ) break;                 // NS
        case 0x04 : ComOut( CGM_LEVEL2, "Segment Transformation" ) break;
        case 0x05 : ComOut( CGM_LEVEL2, "Segment HighLighting" ) break;             // NS
        case 0x06 : ComOut( CGM_LEVEL2, "Segment Display Priority" ) break;         // NS
        case 0x07 : ComOut( CGM_LEVEL2, "Segment Pick Priority" ) break;            // NS
        case 0xfe : ComOut( CGM_GDSF_ONLY, "INQ Current Position" ) break;
        case 0xff : ComOut( CGM_GDSF_ONLY, "INQ Inserted Object Extent" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;                           // NS
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass9()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_UNKNOWN_LEVEL, "Pixel Array" ) break;                   // NS
        case 0x02 : ComOut( CGM_UNKNOWN_LEVEL, "Create Bitmap" ) break;                 // NS
        case 0x03 : ComOut( CGM_UNKNOWN_LEVEL, "Delete Bitmap" ) break;                 // NS
        case 0x04 : ComOut( CGM_UNKNOWN_LEVEL, "Select Drawing Bitmap" ) break;         // NS
        case 0x05 : ComOut( CGM_UNKNOWN_LEVEL, "Display Bitmap" ) break;                // NS
        case 0x06 : ComOut( CGM_UNKNOWN_LEVEL, "Drawing Mode" ) break;
        case 0x07 : ComOut( CGM_UNKNOWN_LEVEL, "Mapped Bitmap ForeGrnd Color" ) break;  // NS
        case 0x08 : ComOut( CGM_UNKNOWN_LEVEL, "Fill Bitmap" ) break;                   // NS
        case 0x09 : ComOut( CGM_UNKNOWN_LEVEL, "Two Operand BitBlt" ) break;            // NS
        case 0x0a : ComOut( CGM_UNKNOWN_LEVEL, "Three Operand BitBlt" ) break;          // NS
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass15()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_UNKNOWN_LEVEL, "Inquire Error Stack" ) break;
        case 0x02 : ComOut( CGM_UNKNOWN_LEVEL, "Pop Error Stack" ) break;
        case 0x03 : ComOut( CGM_UNKNOWN_LEVEL, "Empty Error Stack" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass16()
{
    switch ( mnElementID )
    {
        case 0x82 : ComOut( CGM_GDSF_ONLY, "Inquire Device Supported" ) break;
        case 0x83 : ComOut( CGM_GDSF_ONLY, "Inquire Device Category" ) break;
        case 0x84 : ComOut( CGM_GDSF_ONLY, "Inquire Device Type" ) break;
        case 0x85 : ComOut( CGM_GDSF_ONLY, "Inquire Text Extent" ) break;
        case 0x86 : ComOut( CGM_GDSF_ONLY, "Inquire Append Text Extent" ) break;
        case 0x87 : ComOut( CGM_GDSF_ONLY, "Inquire Circle Extent" ) break;
        case 0x88 : ComOut( CGM_GDSF_ONLY, "Inquire Circle Arc 3 Pt Extent" ) break;
        case 0x89 : ComOut( CGM_GDSF_ONLY, "Inquire Circle Arc 3 Pt Close Extent" ) break;
        case 0x8a : ComOut( CGM_GDSF_ONLY, "Inquire Circle Arc Centre Extent" ) break;
        case 0x8b : ComOut( CGM_GDSF_ONLY, "Inquire Circle Arc Centre Close Extent" ) break;
        case 0x8c : ComOut( CGM_GDSF_ONLY, "Inquire Ellipse Extent" ) break;
        case 0x8d : ComOut( CGM_GDSF_ONLY, "Inquire Ellipse Arc Extent" ) break;
        case 0x8e : ComOut( CGM_GDSF_ONLY, "Inquire Ellipse Arc Close Extent" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


