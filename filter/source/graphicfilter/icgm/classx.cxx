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
    pElement->nUnderlineColor = ImplGetBitmapColor();

// ---------------------------------------------------------------

void CGM::ImplDoClass6()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Escape*/
        {
            long    nIdentifier = ImplGetI( pElement->nIntegerPrecision );
            switch ( nIdentifier )
            {
                case 0 : /*inquire function support */break;
                case -1 : /*set underline mode*/
                {
                    ImplSetUnderlineMode()
                }
                break;
                case -2 : /*set script mode */break;
                case -3 : /*set shadow mode */break;
                case -6 : /*inquire origin offset */break;
                case -8 : /*set media size */break;
                case -10 : /*set character mode */break;
                case -14 : /*resolution mode */break;
                case -17 : /*line cap */break;
                case -18 : /*line join */break;
                case -19 : /*edge join */break;
                case -30 : /*media type */break;
                case -31 : /*number of copies */break;
                case -32 : /*orientation */break;
                case -33 : /*device color representation */break;
                case -34 : /*device font list */break;
                case -35 : /*color reversal mode */break;
                case -36 : /*line cap attributes */break;
                case -37 : /*begin effect */break;
                case -38 : /*end effect */break;
                case -39 : /*begin effect definition */break;
                case -40 : /*end effect definition */break;
                case -41 : /*end style definition */break;
                case -42 : /*begin eps data */break;
                case -43 : /*eps data */break;
                case -44 : /*end eps data */break;
                case -45 : /*set background style */break;
                case -46 : /*set eps mode */break;
                case -47 : /*fill mode */break;
                case -48 : /*begin symbol */break;
                case -49 : /*end symbol */break;
                case -50 : /*begin layer */break;
                case -51 : /*end layer */break;
                case -52 : /*layer visibility */break;
                case -53 : /*inquire foreign data */break;
                case -54 : /*set text offset */break;
                case -55 : /*begin group */break;
                case -56 : /*end group */break;
                case -100 : /*begin patch */break;
                case -101 : /*end patch */break;
                case -102 : /*begin block text region */break;
                case -103 : /*end block text region */break;
                case -120 : /*region margins */break;
                case -121 : /*region expansions */break;
                case -122 : /*region anchor */break;
                case -123 : /*paragraph horizontal align */break;
                case -124 : /*paragraph vertical align */break;
                case -125 : /*region line flow */break;
                case -130 : /*paragraph spacing */break;
                case -131 : /*paragraph identation */break;
                case -132 : /*paragraph tabs */break;
                case -133 : /*paragraph bullet */break;
                case -134 : /*paragraph bulet level */break;
                case -135 : /*line horuzontal align */break;
                case -136 : /*line vertical align */break;
                case -137 : /*line spacing */break;
                case -138 : /*word wrap */break;
                case -150 : /*forward advance distance */break;
                case -151 : /*word spacing */break;
                case -152 : /*external leading */break;
                case -160 : /*set gradient offset */break;
                case -161 : /*set gradient edge */break;
                case -162 : /*set gradient angle */break;
                case -163 : /*set gradient description */break;
                case -164 : /*set gradient style */break;
                case -165 : /*set back ground style */break;
                case -170 : /*geometric pattern draw style */break;
                case -190 : /*set character width */break;
                case -191 : /*hyperlink definitions */break;
                case -192 : /*set color name for pantone */break;
                case -32746 : /*set text font */break;
                case -32747 : /*font selection mode */break;
                case -32752 : /*connecting edge */break;
                case -32753 : /*set drawing mode */break;
                case -32754 : /*inquire clip rectangle */break;
                case -32755 : /*protection region indicator */break;
                case -32756 : /*end protection region */break;
                case -32757 : /*begin protection region */break;
                case -32758 : /*geometric pattern definition */break;
                case -32759 : /*hatch style definition */break;
                case -32760 : /*close figure */break;
                case -32761 : /*end figure*/
                {
                    mpOutAct->EndFigure();
                    mbFigure = sal_False;
                }
                break;
                case -32762 : /*begin figure*/
                {
                    mbFigure = sal_True;
                    mpOutAct->BeginFigure();
                }
                break;
                case -32763 : /*pop transformatin */break;
                case -32764 : /*push transformation */break;
                case -32765 : /*copy segment */break;
                case -32766 : /*endseg */break;
                case -32767 : /*begin segment */break;
                default : break;
            }
            mnParaSize = mnElementSize;
        }
        break;
        case 0x02 : /*Get Escape */break;
        case 0x11 : /*Set Underline Mode*/
        {
            ImplSetUnderlineMode();
        }
        break;
        case 0x12 : /*Set Script Mode */break;
        case 0x13 : /*Set Shadow Mode */break;
        case 0x18 : /*Set Media Size */break;
        case 0x20 : /*Set Character Mode */break;
        case 0x24 : /*Resolution Mode */break;
        case 0x27 : /*Line Cap */break;
        case 0x28 : /*Line Join */break;
        case 0x29 : /*Edge Join */break;
        case 0x40 : /*Media Type */break;
        case 0x41 : /*Number of Copies */break;
        case 0x42 : /*Origin */break;
        case 0x45 : /*Color Reversal Mode */break;
        case 0x46 : /*Line Cap Attributes */break;
        case 0x49 : /*Begin Effect Definition */break;
        case 0x50 : /*End Effect Definition */break;
        case 0x51 : /*Line End Style Attributes */break;
        case 0x52 : /*Begin Data */break;
        case 0x53 : /*Data */break;
        case 0x54 : /*End Data */break;
        case 0x55 : /*Set Background Style */break;
        case 0x56 : /*Set EPS Mode */break;
        case 0x57 : /*Fill Mode */break;
        case 0x58 : /*Begin Symbol */break;
        case 0x59 : /*End Symbol */break;
        case 0x60 : /*Begin Layer */break;
        case 0x61 : /*End Layer */break;
        case 0x62 : /*Layer Visibility */break;
        case 0x64 : /*Set Text Offset */break;
        case 0xFF : /*Inquire Function Support */break;
        case 0xFE : /*Inquire Origin */break;
        case 0xFD : /*Inquire Foreign Data Mode */break;
        case 0xFC : /*Inquire Text Extent */break;
        case 0xFB : /*Inquire DPI */break;
        default: break;
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass8()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Copy Segment */break;                     // NS
        case 0x02 : /*Inheritance Filter */break;               // NS
        case 0x03 : /*Clip Inheritance */break;                 // NS
        case 0x04 : /*Segment Transformation */break;
        case 0x05 : /*Segment HighLighting */break;             // NS
        case 0x06 : /*Segment Display Priority */break;         // NS
        case 0x07 : /*Segment Pick Priority */break;            // NS
        case 0xfe : /*INQ Current Position */break;
        case 0xff : /*INQ Inserted Object Extent */break;
        default: break;                         // NS
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass9()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Pixel Array */break;                  // NS
        case 0x02 : /*Create Bitmap */break;                    // NS
        case 0x03 : /*Delete Bitmap */break;                    // NS
        case 0x04 : /*Select Drawing Bitmap */break;            // NS
        case 0x05 : /*Display Bitmap */break;               // NS
        case 0x06 : /*Drawing Mode */break;
        case 0x07 : /*Mapped Bitmap ForeGrnd Color */break; // NS
        case 0x08 : /*Fill Bitmap */break;                  // NS
        case 0x09 : /*Two Operand BitBlt */break;           // NS
        case 0x0a : /*Three Operand BitBlt */break;         // NS
        default: break;
    }
};

// ---------------------------------------------------------------

void CGM::ImplDoClass15()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Inquire Error Stack */break;
        case 0x02 : /*Pop Error Stack */break;
        case 0x03 : /*Empty Error Stack */break;
        default: break;
    }
};

// ---------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
