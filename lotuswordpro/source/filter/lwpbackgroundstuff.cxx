/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 * For LWP filter architecture prototype
 *  Implemention of class LwpBackgroundStuff.
 */

#include "lwpbackgroundstuff.hxx"
#include "lwppttntbl.hxx"
#include "xfilter/xfbgimage.hxx"

#include "tools/stream.hxx"
#include "vcl/dibtools.hxx"
#include "vcl/bmpacc.hxx"
#include "svx/xbitmap.hxx"


void LwpBackgroundStuff::Read(LwpObjectStream* pStrm)
{
    m_nID = pStrm->QuickReaduInt16();
    m_aFillColor.Read(pStrm);
    m_aPatternColor.Read(pStrm);
    pStrm->SkipExtra();
}

void LwpBackgroundStuff::GetPattern(sal_uInt16 btPttnIndex, sal_uInt8* pPttnArray)
{
    if (btPttnIndex > 71)
    {
        assert(false);
        return;
    }
    if (pPttnArray)
    {
        assert((2 < btPttnIndex) && (btPttnIndex < 72));
        const sal_uInt8* pTempArray = s_pLwpPatternTab[btPttnIndex];
        for(sal_uInt8 i = 0; i < 32; i++)
        {
            pPttnArray[i] = (i%4 == 0) ? pTempArray[7-i/4] : 0;
        }
    }
}

LwpColor* LwpBackgroundStuff::GetFillColor()
{
    if (m_nID == BACK_PATTERN)
    {
        return &m_aPatternColor;
    }

    return &m_aFillColor;
}

XFBGImage* LwpBackgroundStuff::GetFillPattern()
{
    // not pattern fill?
    if (!this->IsPatternFill())
    {
        return NULL;
    }

    // get pattern array from pattern table
    sal_uInt8* pPttnArray = new sal_uInt8 [32];
    this->GetPattern(m_nID, pPttnArray);

    // create bitmap object from the pattern array
    Bitmap aBmp( Size(8, 8), 1 );
    BitmapWriteAccess* pWA = aBmp.AcquireWriteAccess();
    sal_uInt8* pBuf = pWA->GetBuffer();
    memcpy(pBuf, pPttnArray, 32);
    aBmp.ReleaseAccess(pWA);

    if (pPttnArray)
    {
        delete [] pPttnArray;
        pPttnArray = NULL;
    }

    // create XOBitmap object from bitmap object
    XOBitmap aXOBitmap( aBmp );
    aXOBitmap.Bitmap2Array();
    aXOBitmap.SetBitmapType( XBITMAP_8X8 );

    // set back/fore-ground colors
    if (m_aFillColor.IsValidColor() && m_aPatternColor.IsValidColor())
    {
        Color aBackColor(static_cast<sal_uInt8>(m_aFillColor.GetRed()),
            static_cast<sal_uInt8>(m_aFillColor.GetGreen()),
            static_cast<sal_uInt8>(m_aFillColor.GetBlue()));
        Color aForeColor(static_cast<sal_uInt8>(m_aPatternColor.GetRed()),
            static_cast<sal_uInt8>(m_aPatternColor.GetGreen()),
            static_cast<sal_uInt8>(m_aPatternColor.GetBlue()));

        if( aXOBitmap.GetBackgroundColor() == COL_BLACK )
        {
            aXOBitmap.SetPixelColor( aBackColor );
            aXOBitmap.SetBackgroundColor( aForeColor );
        }
        else
        {
            aXOBitmap.SetPixelColor( aForeColor );
            aXOBitmap.SetBackgroundColor( aBackColor );
        }
    }

    // transfer image data from XOBitmap->SvStream->BYTE-Array
    SvMemoryStream aPicMemStream;
    aXOBitmap.Array2Bitmap();
    WriteDIB(aXOBitmap.GetBitmap(), aPicMemStream, true, true);
    sal_uInt32 nSize = aPicMemStream.GetEndOfData();
    sal_uInt8* pImageBuff = new sal_uInt8 [nSize];
    memcpy(pImageBuff, aPicMemStream.GetData(), nSize);

    // create XFBGImage object.
    XFBGImage* pXFBGImage = new XFBGImage();
    pXFBGImage->SetImageData(pImageBuff, nSize);

    delete [] pImageBuff;
    pImageBuff = NULL;


    pXFBGImage->SetRepeate();
    // end of add

    return pXFBGImage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
