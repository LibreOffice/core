/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connctr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 14:43:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif
#include <soldep/connctr.hxx>
#include <soldep/objwin.hxx>
#include <soldep/depwin.hxx>
#include "math.h"

BOOL Connector::msbHideMode = FALSE;

Connector::Connector( DepWin* pParent, WinBits nWinStyle ) :
mpStartWin( 0L ),
mpEndWin( 0L ),
mnStartId( 0 ),
mnEndId( 0 ),
bVisible( FALSE ),
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

void Connector::Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin, BOOL bVis )
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
    /*
        if ((mpStartWin->GetMarkMode() == 0) || (mpEndWin->GetMarkMode() == 0))
        {
            //bVisible = FALSE;
            UpdateVisibility();
            fprintf( ((MyApp*)GetpApp())->pDebugFile, "FALSE connctr: Start: %s %i - End: %s %i\n",
                mpStartWin->GetBodyText().GetBuffer(),mpStartWin->GetMarkMode(),
                mpEndWin->GetBodyText().GetBuffer(),mpEndWin->GetMarkMode());
        } else
        {
            bVisible = TRUE;
            fprintf( ((MyApp*)GetpApp())->pDebugFile, "TRUE connctr: Start: %s %i - End: %s %i\n",
                mpStartWin->GetBodyText().GetBuffer(),mpStartWin->GetMarkMode(),
                mpEndWin->GetBodyText().GetBuffer(),mpEndWin->GetMarkMode());
        }
     */
    if (!(mpStartWin->IsNullObject())) //null_project
    {
        if (mpStartWin->GetMarkMode() == 0)
        {
            mpStartWin->SetViewMask(0); //objwin invisible
        } else
        {
            mpStartWin->SetViewMask(1); //objwin visible
        }
    }
    if (!(mpEndWin->IsNullObject()))
    {
        if (mpEndWin->GetMarkMode() == 0)
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
        //bVisible = TRUE;
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

void Connector::UpdatePosition( ObjectWin* pWin, BOOL bPaint )
{
//  more than one call ?
//
    Point OldStart, OldEnd;
    static ULONG nCallCount = 0;

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

USHORT Connector::Save( SvFileStream& rOutFile )
{
    rOutFile << mpStartWin->GetId();
    rOutFile << mpEndWin->GetId();

    return 0;
}

USHORT Connector::Load( SvFileStream& rInFile )
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

ULONG Connector::GetOtherId( ULONG nId )
{
// get correspondent object id
    if ( mnStartId == nId )
        return mnEndId;
    else
        if ( mnEndId == nId )
            return mnStartId;

    return NULL;
}

ULONG Connector::GetLen()
{
    double dx, dy;

    dx = mStart.X() - mEnd.X();
    dy = mStart.Y() - mEnd.Y();

    return (ULONG) sqrt( dx * dx + dy * dy );
}

BOOL Connector::IsStart( ObjectWin* pWin )
{
    return pWin == mpStartWin;
}
