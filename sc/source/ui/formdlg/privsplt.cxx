/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"



#include "privsplt.hxx"

/*************************************************************************
#*  Member:     ScPrivatSplit
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

    aMovingFlag=false;
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
#*  Member:     MouseButtonDown
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
#*  Member:     MouseButtonUp
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
#*  Member:     MouseMove
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
#*  Member:     SetYRange
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
#*  Member:     GetDeltaY
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
#*  Member:     GetDeltaY
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
#*  Member:     CtrModified
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


void ScPrivatSplit::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
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
        ImplInitSettings( sal_True, false, false );
        Invalidate();
    }
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( false, sal_True, false );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( false, false, sal_True );
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
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
