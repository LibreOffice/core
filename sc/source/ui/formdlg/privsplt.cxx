/*************************************************************************
 *
 *  $RCSfile: privsplt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "privsplt.hxx"

/*************************************************************************
#*  Member:     ScPrivatSplit                               Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     MD_Test
#*
#*  Funktion:   Konstruktor der Klasse ScPrivatSplit
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ScPrivatSplit::ScPrivatSplit( Window* pParent, const ResId& rResId,
                             SC_SPLIT_DIRECTION eSplit):
                        Control( pParent, rResId )
{
    Point aPos=GetPosPixel();
    nOldX=(short)aPos.X();
    nOldY=(short)aPos.Y();
    nNewX=(short)aPos.X();
    nNewY=(short)aPos.Y();
    eScSplit=eSplit;
    aXMovingRange.Min()=nNewX;
    aXMovingRange.Max()=nNewX;
    aYMovingRange.Min()=nNewY;
    aYMovingRange.Max()=nNewY;

    aWinPointer=GetPointer();

    aMovingFlag=FALSE;
    if(eScSplit==SC_SPLIT_HORZ)
    {
        aWinPointer=Pointer(POINTER_HSPLIT);
    }
    else
    {
        aWinPointer=Pointer(POINTER_VSPLIT);
    }
    SetPointer(aWinPointer);
}


/*************************************************************************
#*  Member:     MouseButtonDown                         Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Reagiert auf einen einzelnen Mouse-Event. Nach Aufruf
#*              werden alle Mauseingaben an dieses Control weitergeleitet.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScPrivatSplit::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos=LogicToPixel(rMEvt.GetPosPixel());

    nOldX=(short)aPos.X();
    nOldY=(short)aPos.Y();

    CaptureMouse();
}

/*************************************************************************
#*  Member:     MouseButtonUp                           Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Ende einer Benutzeraktion mit der Maus. Es werden
#*              die aktuelle Maus- Koordinaten ermittelt und fuer
#*              die Verschiebung des Fensters verwendet.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScPrivatSplit::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();

    Point aPos=LogicToPixel(rMEvt.GetPosPixel());
    Point a2Pos=GetPosPixel();
    Point a3Pos=a2Pos;

    if(eScSplit==SC_SPLIT_HORZ)
    {
        nNewX=(short)aPos.X();
        nDeltaX=nNewX-nOldX;
        a2Pos.X()+=nDeltaX;
        if(a2Pos.X()<aXMovingRange.Min())
        {
            nDeltaX=(short)(aXMovingRange.Min()-a3Pos.X());
            a2Pos.X()=aXMovingRange.Min();
        }
        else if(a2Pos.X()>aXMovingRange.Max())
        {
            nDeltaX=(short)(aXMovingRange.Max()-a3Pos.X());
            a2Pos.X()=aXMovingRange.Max();
        }
    }
    else
    {
        nNewY=(short)aPos.Y();
        nDeltaY=nNewY-nOldY;
        a2Pos.Y()+=nDeltaY;
        if(a2Pos.Y()<aYMovingRange.Min())
        {
            nDeltaY=(short)(aYMovingRange.Min()-a3Pos.Y());
            a2Pos.Y()=aYMovingRange.Min();
        }
        else if(a2Pos.Y()>aYMovingRange.Max())
        {
            nDeltaY=(short)(aYMovingRange.Max()-a3Pos.Y());
            a2Pos.Y()=aYMovingRange.Max();
        }
    }
    SetPosPixel(a2Pos);
    Invalidate();
    Update();
    CtrModified();
}

/*************************************************************************
#*  Member:     MouseMove                                   Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Reagiert kontinuierlich auf Mausbewegungen. Es werden
#*              die aktuelle Maus- Koordinaten ermittelt und fuer
#*              die Verschiebung des Fensters verwendet.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScPrivatSplit::MouseMove( const MouseEvent& rMEvt )
{
    Point aPos=LogicToPixel(rMEvt.GetPosPixel());
    Point a2Pos=GetPosPixel();
    Point a3Pos=a2Pos;
    if(rMEvt.IsLeft())
    {
        if(eScSplit==SC_SPLIT_HORZ)
        {
            nNewX=(short)aPos.X();
            nDeltaX=nNewX-nOldX;
            a2Pos.X()+=nDeltaX;

            if(a2Pos.X()<aXMovingRange.Min())
            {
                nDeltaX=(short)(aXMovingRange.Min()-a3Pos.X());
                a2Pos.X()=aXMovingRange.Min();
            }
            else if(a2Pos.X()>aXMovingRange.Max())
            {
                nDeltaX=(short)(aXMovingRange.Max()-a3Pos.X());
                a2Pos.X()=aXMovingRange.Max();
            }
        }
        else
        {
            nNewY=(short)aPos.Y();
            nDeltaY=nNewY-nOldY;
            a2Pos.Y()+=nDeltaY;
            if(a2Pos.Y()<aYMovingRange.Min())
            {
                nDeltaY=(short)(aYMovingRange.Min()-a3Pos.Y());
                a2Pos.Y()=aYMovingRange.Min();
            }
            else if(a2Pos.Y()>aYMovingRange.Max())
            {
                nDeltaY=(short)(aYMovingRange.Max()-a3Pos.Y());
                a2Pos.Y()=aYMovingRange.Max();
            }
        }

        SetPosPixel(a2Pos);

        CtrModified();
        Invalidate();
        Update();
    }
}

/*************************************************************************
#*  Member:     SetXRange                                   Datum:14.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Setzt den Range fuer die X- Verschiebung
#*
#*  Input:      neuer Bereich
#*
#*  Output:     ---
#*
#************************************************************************/
void ScPrivatSplit::SetXRange(Range cRgeX)
{
    aXMovingRange=cRgeX;
}

/*************************************************************************
#*  Member:     SetYRange                                   Datum:14.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Setzt den Range fuer die Y- Verschiebung
#*
#*  Input:      neuer Bereich
#*
#*  Output:     ---
#*
#************************************************************************/
void ScPrivatSplit::SetYRange(Range cRgeY)
{
    aYMovingRange=cRgeY;
}



/*************************************************************************
#*  Member:     GetDeltaY                                   Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Liefert die relative x-Verschiebung zurueck
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
short ScPrivatSplit::GetDeltaX()
{
    return nDeltaX;
}

/*************************************************************************
#*  Member:     GetDeltaY                                   Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Liefert die relative y-Verschiebung zurueck
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
short ScPrivatSplit::GetDeltaY()
{
    return nDeltaY;
}

/*************************************************************************
#*  Member:     CtrModified                                 Datum:13.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScPrivatSplit
#*
#*  Funktion:   Teilt einem installierten Handler mit, dass
#*              eine Veraenderung eingetreten ist.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ScPrivatSplit::CtrModified()
{
    aCtrModifiedLink.Call( this );
}

void ScPrivatSplit::MoveSplitTo(Point aPos)
{
    Point a2Pos=GetPosPixel();
    nOldX=(short)a2Pos.X();
    nOldY=(short)a2Pos.Y();
    Point a3Pos=a2Pos;

    if(eScSplit==SC_SPLIT_HORZ)
    {
        nNewX=(short)aPos.X();
        nDeltaX=nNewX-nOldX;
        a2Pos.X()+=nDeltaX;
        if(a2Pos.X()<aXMovingRange.Min())
        {
            nDeltaX=(short)(aXMovingRange.Min()-a3Pos.X());
            a2Pos.X()=aXMovingRange.Min();
        }
        else if(a2Pos.X()>aXMovingRange.Max())
        {
            nDeltaX=(short)(aXMovingRange.Max()-a3Pos.X());
            a2Pos.X()=aXMovingRange.Max();
        }
    }
    else
    {
        nNewY=(short)aPos.Y();
        nDeltaY=nNewY-nOldY;
        a2Pos.Y()+=nDeltaY;
        if(a2Pos.Y()<aYMovingRange.Min())
        {
            nDeltaY=(short)(aYMovingRange.Min()-a3Pos.Y());
            a2Pos.Y()=aYMovingRange.Min();
        }
        else if(a2Pos.Y()>aYMovingRange.Max())
        {
            nDeltaY=(short)(aYMovingRange.Max()-a3Pos.Y());
            a2Pos.Y()=aYMovingRange.Max();
        }
    }
    SetPosPixel(a2Pos);
    Invalidate();
    Update();
    CtrModified();
}


void ScPrivatSplit::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetButtonTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        SetBackground( rStyleSettings.GetFaceColor());
    }
    if ( IsBackground() )
    {
        SetFillColor( GetBackground().GetColor() );
        SetBackground();
    }
    Invalidate();
}

// -----------------------------------------------------------------------

void ScPrivatSplit::StateChanged( StateChangedType nType )
{
    if ( (nType == STATE_CHANGE_ZOOM) ||
         (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ScPrivatSplit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}


