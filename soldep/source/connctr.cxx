/*************************************************************************
 *
 *  $RCSfile: connctr.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-26 14:48:13 $
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

#include "connctr.hxx"
#include "objwin.hxx"
#include "depwin.hxx"

#include "math.h"

Connector::Connector( DepWin* pParent, WinBits nWinStyle ) :
mpStartWin( 0L ),
mpEndWin( 0L ),
mnStartId( 0 ),
mnEndId( 0 ),
len( 70 ),
bVisible( FALSE )
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

void Connector::Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin )
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

    if ( mpParent->IsPaintEnabled())
    {
        mpParent->DrawLine( mEnd, mStart );
        mpParent->DrawEllipse( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2)));
    }
    UpdateVisibility();
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
    if ( IsVisible()) {
        mpParent->DrawLine( mEnd, mStart );
        mpParent->DrawEllipse( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2)));
    }
}

void Connector::UpdatePosition( ObjectWin* pWin, BOOL bPaint )
{
//  more than one call ?
//
    Point OldStart, OldEnd;
    static nCallCount;
//  BOOL bPaint = TRUE;

    if ( nCallCount )
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
//who uses this rectangle???
                Paint ( Rectangle( mEnd - Point( 3, 3), mEnd + Point( 3, 3)));
                Paint ( Rectangle( mEnd, mStart ));
            }
            nCallCount--;
        }
    }
//  mpParent->DrawLine( mEnd, mStart );
//  mpParent->DrawEllipse( Rectangle( mEnd - Point( 2, 2), mEnd + Point( 2, 2)));
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
    if ( mpStartWin == pWin )
        return mpEndWin;
    else
        if ( mpEndWin == pWin )
            return mpStartWin;

    return NULL;
}

ULONG Connector::GetOtherId( ULONG nId )
{
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

    return sqrt( dx * dx + dy * dy );
}

BOOL Connector::IsStart( ObjectWin* pWin )
{
    return pWin == mpStartWin;
}

