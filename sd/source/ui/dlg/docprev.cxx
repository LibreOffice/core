/*************************************************************************
 *
 *  $RCSfile: docprev.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:32 $
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

#ifndef _SFX_OBJSH_HXX // SfxObjectShell
#include <sfx2/objsh.hxx>
#endif

#ifndef _SV_GDIMTF_HXX // GDIMetaFile
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_VIRDEV_HXX // class VirtualDevice
#include <vcl/virdev.hxx>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SD_FADEDEF_H // enum FadeSpeed
#include <fadedef.h>
#endif

#ifndef _SV_CTRL_HXX // class Control
#include <vcl/ctrl.hxx>
#endif

#ifndef _SD_FADER_HXX
#include "fader.hxx"
#endif

#include "docprev.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "viewshel.hxx"
#include "showview.hxx"

using namespace ::com::sun::star;

const int SdDocPreviewWin::FRAME = 4;

void SdDocPreviewWin::SetObjectShell( SfxObjectShell* pObj, USHORT nShowPage )
{

    SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pObj);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;
    if(pDoc)
    {
        const USHORT nPageCount = pDoc->GetSdPageCount(PK_STANDARD);
        USHORT nPgNum = 0;
        while( nPgNum < nPageCount )
        {
            pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PK_STANDARD ), nPgNum == nShowPage );
            nPgNum++;
        }
    }

    GDIMetaFile* pFile = pObj ? pObj->GetPreviewMetaFile( ) : 0;
    delete pMetaFile;
    pMetaFile = pFile;
    m_pObj = pObj;
    Invalidate();
}

SdDocPreviewWin::SdDocPreviewWin( Window* pParent, const ResId& rResId )
: Control(pParent, rResId), pMetaFile( 0 ), bInEffect(FALSE), m_pObj(NULL)
{
}

SdDocPreviewWin::SdDocPreviewWin( Window* pParent )
: Control(pParent, 0 ), pMetaFile( 0 ), bInEffect(FALSE), m_pObj(NULL)
{
    Resize();
    Show();
}

void SdDocPreviewWin::Resize()
{
    Invalidate();
}

void SdDocPreviewWin::SetGDIFile( GDIMetaFile* pFile )
{
    delete pMetaFile;
    pMetaFile = pFile;
    Invalidate();
}

void SdDocPreviewWin::CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint )
{
    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
    long nWidth = rSize.Width() - 2*FRAME;
    long nHeight = rSize.Height() - 2*FRAME;
    if( nWidth < 0 ) nWidth = 0;
    if( nHeight < 0 ) nHeight = 0;

    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
    double dRatioPreV=((double) nWidth ) / nHeight;

    if (dRatio>dRatioPreV)
    {
        rSize=Size(nWidth, (USHORT)(nWidth/dRatio));
        rPoint=Point( 0, (USHORT)((nHeight-rSize.Height())/2));
    }
    else
    {
        rSize=Size((USHORT)(nHeight*dRatio), nHeight);
        rPoint=Point((USHORT)((nWidth-rSize.Width())/2),0);
    }
}

void SdDocPreviewWin::ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev )
{
    Point aPoint;
    Size aSize = pVDev->GetOutputSize();
    Point bPoint(aSize.Width()-2*FRAME, aSize.Height()-2*FRAME );
    CalcSizeAndPos( pFile, aSize, aPoint );
    bPoint -= aPoint;
    aPoint += Point( FRAME, FRAME );

    pVDev->SetLineColor();
    pVDev->SetFillColor( Color( COL_LIGHTGRAY ) );
    pVDev->DrawRect(Rectangle( Point(0,0 ), pVDev->GetOutputSize()));
    if( pFile )
    {
        pVDev->SetFillColor( Color( COL_WHITE ) );
        pVDev->DrawRect(Rectangle(aPoint, aSize));
        pFile->WindStart();
        pFile->Play( pVDev, aPoint, aSize  );
    }
}

void SdDocPreviewWin::Paint( const Rectangle& rRect )
{
    ImpPaint( pMetaFile, (VirtualDevice*)this );
}

void SdDocPreviewWin::ShowEffect( presentation::FadeEffect eEffect, FadeSpeed eSpeed )
{
    if(bInEffect || !pMetaFile)
        return;

    bInEffect = TRUE;

    Point aPoint;
    Size aSize = GetOutputSize();
    Point bPoint( aSize.Width() - 2*FRAME, aSize.Height() - 2*FRAME );
    CalcSizeAndPos( pMetaFile, aSize, aPoint );
    bPoint -= aPoint;

    aPoint += Point( FRAME, FRAME );
    bPoint += Point( FRAME, FRAME );

    // Hintergrund Schwarz
    SetLineColor();
    SetFillColor( Color( COL_LIGHTGRAY ) );
    DrawRect(Rectangle( Point(0,0 ), GetOutputSize()));

    // korrigierte Seitengroesse, sonst kommt die letzte Pixelreihe(spalte)
    // nicht mit
    Size aPixelSize = PixelToLogic(Size(1,1));
    aSize.Width()   += aPixelSize.Width();
    aSize.Height() += aPixelSize.Height();

    // virtuelle Devices anlegen
    MapMode aMapMode = GetMapMode();
    aMapMode.SetOrigin(Point(0,0));

    VirtualDevice* pVDev = new VirtualDevice(*this);
    pVDev->SetMapMode(aMapMode);
    pVDev->SetOutputSize(aSize); // aCPageSize);

    if( pMetaFile )
    {
        pMetaFile->WindStart();
        pMetaFile->Play( pVDev, Point( 0,0 ), aSize  );
    }

    // ein Fader zum Ueberblenden
    Fader* pFader = new Fader(this);
    pFader->SetEffect( eEffect );
    pFader->SetSpeed( eSpeed );
    pFader->SetSource(Rectangle(Point(), aSize));
    pFader->SetTarget(Rectangle(aPoint, aSize));

    // virtuelle Devices an Fader uebergeben
    pFader->SetNewVirtualDevice(pVDev);

    // ueberblenden
    pFader->Fade();

    delete pFader;
    delete pVDev;

    bInEffect = FALSE;
}

long SdDocPreviewWin::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
        if ( pMEvt->IsLeft() )
        {
            if( rNEvt.GetWindow() == this )
            {
                if(aClickHdl.IsSet())
                    aClickHdl.Call(this);
            }
        }
    }

    return Control::Notify( rNEvt );
}


