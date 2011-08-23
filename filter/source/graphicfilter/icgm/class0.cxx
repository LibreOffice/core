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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"
#include <main.hxx>
#include <outact.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass0()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1, "Begin Metafile" )
        {
            ImplSetMapMode();
            mbMetaFile = sal_True;
        }
        break;
        case 0x02 : ComOut( CGM_LEVEL1, "End MetaFile" )
        {
            if ( mpBitmapInUse )							// vorhandene grafik verarbeiten,
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
        case 0x03 : ComOut( CGM_LEVEL1, "Begin Picture" )
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
                if ( mpChart == NULL )		// normal CGM Files determines "BeginPic"
                    mpOutAct->InsertPage();		// as the next slide
            }
        }
        break;
        case 0x04 : ComOut( CGM_LEVEL1, "Begin Picture Body" )
            mbPictureBody = sal_True;
        break;
        case 0x05 : ComOut( CGM_LEVEL1, "	End Picture" )
        {
            if ( mbPicture )
            {
                if ( mpBitmapInUse )							// vorhandene grafik verarbeiten,
                {
                    CGMBitmapDescriptor* pBmpDesc = mpBitmapInUse->GetBitmap();
                    // irgendetwas mit der Bitmap anfangen
                    mpOutAct->DrawBitmap( pBmpDesc );
                    delete mpBitmapInUse;
                    mpBitmapInUse = NULL;
                }
                mpOutAct->EndFigure();							// eventuelle figuren schliessen
                mpOutAct->EndGrouping();						// eventuelle gruppierungen noch abschliessen
                *pElement = *pCopyOfE;
                mbFigure = mbFirstOutPut = mbPicture = mbPictureBody = sal_False;
            }
            else
                mbStatus = sal_False;
        }
        break;
        case 0x06 : ComOut( CGM_LEVEL2, "Begin Segment" )
            pElement->bSegmentCount = sal_True;
        break;
        case 0x07 : ComOut( CGM_LEVEL2, "End Segment" )
            pElement->bSegmentCount = sal_True;
        break;
        case 0x08 : ComOut( CGM_LEVEL2, "Begin Figure" )
            mbFigure = sal_True;
            mpOutAct->BeginFigure();
        break;
        case 0x09 : ComOut( CGM_LEVEL2, "End Figure" )
            mpOutAct->EndFigure();
            mbFigure = sal_False;
        break;
        case 0x0d : ComOut( CGM_LEVEL3, "Begin Protection Region" ) break;
        case 0x0e : ComOut( CGM_LEVEL3, "End Protection Region" ) break;
        case 0x0f : ComOut( CGM_LEVEL3, "Begin Compound Line" ) break;
        case 0x10 : ComOut( CGM_LEVEL3, "End Compound Line" ) break;
        case 0x11 : ComOut( CGM_LEVEL3, "Begin Compound Text Path" ) break;
        case 0x12 : ComOut( CGM_LEVEL3, "End Compound Text Path" ) break;
        case 0x13 : ComOut( CGM_LEVEL3, "Begin Tile Array" ) break;					// NS
        case 0x14 : ComOut( CGM_LEVEL3, "End Tile Array" ) break;					// NS
        case 0xff : ComOut( CGM_GDSF_ONLY, "Filter Setup" ) break;
        case 0xfe : ComOut( CGM_GDSF_ONLY, "Begin Block Text Region" ) break;
        case 0xfd : ComOut( CGM_GDSF_ONLY, "End Block Text Region" ) break;
        case 0xfc : ComOut( CGM_GDSF_ONLY, "Begin Group" )
            mpOutAct->BeginGroup();
        break;
        case 0xfb : ComOut( CGM_GDSF_ONLY, "End Group" )
            mpOutAct->EndGroup();
        break;
        case 0xfa : ComOut( CGM_GDSF_ONLY, "Begin Patch" ) break;
        case 0xf9 : ComOut( CGM_GDSF_ONLY, "Begin Patch" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


