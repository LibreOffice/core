/*************************************************************************
 *
 *  $RCSfile: class0.cxx,v $
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
                if ( mpChart == NULL )      // normal CGM Files determines "BeginPic"
                    mpOutAct->InsertPage();     // as the next slide
            }
        }
        break;
        case 0x04 : ComOut( CGM_LEVEL1, "Begin Picture Body" )
            mbPictureBody = sal_True;
        break;
        case 0x05 : ComOut( CGM_LEVEL1, "   End Picture" )
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
        case 0x13 : ComOut( CGM_LEVEL3, "Begin Tile Array" ) break;                 // NS
        case 0x14 : ComOut( CGM_LEVEL3, "End Tile Array" ) break;                   // NS
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


