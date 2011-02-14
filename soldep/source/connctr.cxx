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
#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif
#include <soldep/connctr.hxx>
#include <soldep/objwin.hxx>
#include <soldep/depwin.hxx>
#include "math.h"

sal_Bool Connector::msbHideMode = sal_False;

Connector::Connector( DepWin* pParent, WinBits nWinStyle ) :
mpStartWin( 0L ),
mpEndWin( 0L ),
mnStartId( 0 ),
mnEndId( 0 ),
bVisible( sal_False ),
len( 70 )
{
    mpParent = pParent;
    if ( mpParent )
        mpParent->AddConnector( this );
}

Connector::~Connector()
{
    if ( mpStartWin )
        mpStartWin->RemoveConnector( this );
    if ( mpEndWin )
        mpEndWin->RemoveConnector( this );
    if ( mpParent )
        mpParent->RemoveConnector( this );
    mpParent->Invalidate( Rectangle( mStart, mEnd ));
    mpParent->Invalidate( Rectangle( mEnd - Point( 3, 3), mEnd + Point( 3, 3)));
}

void Connector::Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin, sal_Bool bVis )
{
    mpStartWin = pStartWin;
    mpEndWin = pEndWin;
    mpStartWin->AddConnector( this );
    mpEndWin->AddConnector( this );
    mCenter = GetMiddle();
    mStart = pStartWin->GetFixPoint( mCenter );
    mEnd = pEndWin->GetFixPoint( mCenter );
    mnStartId = pStartWin->GetId();
    mnEndId = pEndWin->GetId();
    bVisible = bVis;

//  if ( mpParent->IsPaintEnabled())
    if ( IsVisible() )
    {
        mpParent->DrawLine( mEnd, mStart );
        mpParent->DrawEllipse( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2)));
    }
    UpdateVisibility(); //null_Project
}

void Connector::UpdateVisibility()
{
    bVisible = mpStartWin->IsVisible() && mpEndWin->IsVisible();
}


Point Connector::GetMiddle()
{
    Point aStartPoint = mpStartWin->GetPosPixel();
    Size aStartSize = mpStartWin->GetSizePixel();
    int nMoveHorz, nMoveVert;
    aStartPoint.Move( aStartSize.Width() / 2, aStartSize.Height() / 2 );

    Point aEndPoint = mpEndWin->GetPosPixel();
    Size aEndSize = mpEndWin->GetSizePixel();

    aEndPoint.Move( aEndSize.Width() / 2, aEndSize.Height() / 2 );

    Point aRetPoint = aEndPoint;

    nMoveHorz = aStartPoint.X() - aEndPoint.X();
    if ( nMoveHorz )
        nMoveHorz /= 2;
    nMoveVert = aStartPoint.Y() - aEndPoint.Y();
    if ( nMoveVert )
        nMoveVert /= 2;
    aRetPoint.Move( nMoveHorz, nMoveVert );
    return aRetPoint;

}

void Connector::Paint( const Rectangle& rRect )
{
    //MyApp *pApp = (MyApp*)GetpApp();
    //SolDep *pSoldep = pApp->GetSolDep();
    if (msbHideMode)
    {
    if (!(mpStartWin->IsNullObject())) //null_project
    {
        if ( mpStartWin->GetMarkMode() == 0 || mpStartWin->GetMarkMode() == MARKMODE_DEPENDING )
        {
            mpStartWin->SetViewMask(0); //objwin invisible
        } else
        {
            mpStartWin->SetViewMask(1); //objwin visible
        }
    }
    if (!(mpEndWin->IsNullObject()))
    {
        if ( mpEndWin->GetMarkMode() == 0 || mpEndWin->GetMarkMode() == MARKMODE_DEPENDING )
        {
            mpEndWin->SetViewMask(0); //objwin invisible
        } else
        {
            mpEndWin->SetViewMask(1); //objwin visible
        }
    }
    UpdateVisibility();
    } else //IsHideMode
    {
        //bVisible = sal_True;
        if (!(mpStartWin->IsNullObject())) //null_project
        {
             mpStartWin->SetViewMask(1);
        }
        if (!(mpEndWin->IsNullObject())) //null_project
        {
             mpEndWin->SetViewMask(1);
        }
        UpdateVisibility();
    }
    if ( (mpStartWin->GetBodyText() != ByteString("null")) &&              //null_project
         (mpEndWin->GetBodyText() != ByteString("null")) && IsVisible())  //null_project
    {
        mpParent->DrawLine( mEnd, mStart );
        mpParent->DrawEllipse( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2)));
    }
}

void Connector::DrawOutput( OutputDevice* pDevice, const Point& rOffset )
{
    if ( (mpStartWin->GetBodyText() != ByteString("null")) &&              //null_project
         (mpEndWin->GetBodyText() != ByteString("null")) && IsVisible())  //null_project
    {
        pDevice->DrawLine( pDevice->PixelToLogic( mEnd ) - rOffset, pDevice->PixelToLogic( mStart ) - rOffset );
        Rectangle aRect( pDevice->PixelToLogic( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2) ) ) );
        aRect.Move( -rOffset.X(), -rOffset.Y() );
        pDevice->DrawEllipse( aRect );
    }
}

void Connector::UpdatePosition( ObjectWin* pWin, sal_Bool bPaint )
{
//  more than one call ?
//
    Point OldStart, OldEnd;
    static sal_uIntPtr nCallCount = 0;

    //MyApp *pApp = (MyApp*)GetpApp();
    //SolDep *pSoldep = pApp->GetSolDep();
    if (msbHideMode)
        bVisible = 1;

    if ( nCallCount )           // only one call
        nCallCount++;
    else
    {
        nCallCount++;
        while ( nCallCount )
        {
            if ( bPaint )
            {
                OldStart = mStart;
                OldEnd = mEnd;
            }
            mCenter = GetMiddle();
            mStart=mpStartWin->GetFixPoint( mCenter, bPaint );
            mEnd=mpEndWin->GetFixPoint( mCenter, bPaint );
            if ( bPaint )
            {
                mpParent->Invalidate( Rectangle( OldStart, OldEnd ));
                mpParent->Invalidate( Rectangle( OldEnd - Point( 2, 2), OldEnd + Point( 2, 2)));
//Don't paint "null_project" connectors
                if ( (mpStartWin->GetBodyText() != ByteString("null")) &&               //null_project
                     (mpEndWin->GetBodyText() != ByteString("null")))                   //null_project
                {
                    Paint ( Rectangle( mEnd - Point( 3, 3), mEnd + Point( 3, 3)));
                    Paint ( Rectangle( mEnd, mStart ));
                }
            }
            nCallCount--;
        }
    }
}

sal_uInt16 Connector::Save( SvFileStream& rOutFile )
{
    rOutFile << mpStartWin->GetId();
    rOutFile << mpEndWin->GetId();

    return 0;
}

sal_uInt16 Connector::Load( SvFileStream& rInFile )
{
    rInFile >> mnStartId;
    rInFile >> mnEndId;

    return 0;
}

ObjectWin* Connector::GetOtherWin( ObjectWin* pWin )
{
// get correspondent object ptr
    if ( mpStartWin == pWin )
        return mpEndWin;
    else
        if ( mpEndWin == pWin )
            return mpStartWin;

    return NULL;
}

sal_uIntPtr Connector::GetOtherId( sal_uIntPtr nId )
{
// get correspondent object id
    if ( mnStartId == nId )
        return mnEndId;
    else
        if ( mnEndId == nId )
            return mnStartId;

    return NULL;
}

sal_uIntPtr Connector::GetLen()
{
    double dx, dy;

    dx = mStart.X() - mEnd.X();
    dy = mStart.Y() - mEnd.Y();

    return (sal_uIntPtr) sqrt( dx * dx + dy * dy );
}

sal_Bool Connector::IsStart( ObjectWin* pWin )
{
    return pWin == mpStartWin;
}
