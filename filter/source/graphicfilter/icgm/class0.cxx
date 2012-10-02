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

void CGM::ImplDoClass0()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Begin Metafile*/
        {
            ImplSetMapMode();
            mbMetaFile = sal_True;
        }
        break;
        case 0x02 : /*End MetaFile*/
        {
            if ( mpBitmapInUse )                            // vorhandene grafik verarbeiten,
            {
                CGMBitmapDescriptor* pBmpDesc = mpBitmapInUse->GetBitmap();
                // irgendetwas mit der Bitmap anfangen
                mpOutAct->DrawBitmap( pBmpDesc );
                delete mpBitmapInUse;
                mpBitmapInUse = NULL;
            }
            mbIsFinished = sal_True;
            mbPictureBody = sal_False;
            mbMetaFile = sal_False;
        }
        break;
        case 0x03 : /*Begin Picture*/
        {
            ImplDefaultReplacement();
            ImplSetMapMode();
            if ( mbPicture )
                mbStatus = sal_False;
            else
            {
                *pCopyOfE = *pElement;
                mbPicture = mbFirstOutPut = sal_True;
                mbFigure = sal_False;
                mnAct4PostReset = 0;
                if ( mpChart == NULL )      // normal CGM Files determines "BeginPic"
                    mpOutAct->InsertPage();     // as the next slide
            }
        }
        break;
        case 0x04 : /*Begin Picture Body*/
            mbPictureBody = sal_True;
        break;
        case 0x05 : /*  End Picture*/
        {
            if ( mbPicture )
            {
                if ( mpBitmapInUse )                            // vorhandene grafik verarbeiten,
                {
                    CGMBitmapDescriptor* pBmpDesc = mpBitmapInUse->GetBitmap();
                    // irgendetwas mit der Bitmap anfangen
                    mpOutAct->DrawBitmap( pBmpDesc );
                    delete mpBitmapInUse;
                    mpBitmapInUse = NULL;
                }
                mpOutAct->EndFigure();                          // eventuelle figuren schliessen
                mpOutAct->EndGrouping();                        // eventuelle gruppierungen noch abschliessen
                *pElement = *pCopyOfE;
                mbFigure = mbFirstOutPut = mbPicture = mbPictureBody = sal_False;
            }
            else
                mbStatus = sal_False;
        }
        break;
        case 0x06 : /*Begin Segment*/
            pElement->bSegmentCount = sal_True;
        break;
        case 0x07 : /*End Segment*/
            pElement->bSegmentCount = sal_True;
        break;
        case 0x08 : /*Begin Figure*/
            mbFigure = sal_True;
            mpOutAct->BeginFigure();
        break;
        case 0x09 : /*End Figure*/
            mpOutAct->EndFigure();
            mbFigure = sal_False;
        break;
        case 0x0d : /*Begin Protection Region */break;
        case 0x0e : /*End Protection Region */break;
        case 0x0f : /*Begin Compound Line */break;
        case 0x10 : /*End Compound Line */break;
        case 0x11 : /*Begin Compound Text Path */break;
        case 0x12 : /*End Compound Text Path */break;
        case 0x13 : /*Begin Tile Array */break;                 // NS
        case 0x14 : /*End Tile Array */break;                   // NS
        case 0xff : /*Filter Setup */break;
        case 0xfe : /*Begin Block Text Region */break;
        case 0xfd : /*End Block Text Region */break;
        case 0xfc : /*Begin Group*/
            mpOutAct->BeginGroup();
        break;
        case 0xfb : /*End Group*/
            mpOutAct->EndGroup();
        break;
        case 0xfa : /*Begin Patch */break;
        case 0xf9 : /*Begin Patch */break;
        default: break;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
