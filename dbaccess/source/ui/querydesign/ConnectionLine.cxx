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

#include "ConnectionLine.hxx"
#include "ConnectionLineData.hxx"
#include "TableWindow.hxx"
#include "TableWindowListBox.hxx"
#include "TableConnection.hxx"
#include <vcl/svapp.hxx>
#ifndef _INC_MATH
#include <math.h>
#endif
#include <osl/diagnose.h>
#include <vcl/lineinfo.hxx>


using namespace dbaui;
const long DESCRIPT_LINE_WIDTH = 15;
const long HIT_SENSITIVE_RADIUS = 5;

namespace
{
    /** calcRect creates a new rectangle with the given points
            @param  _rBase      the base point
            @param  _aVector    the vector which will be added
    */
    inline Rectangle calcRect(const Point& _rBase,const Point& _aVector)
    {
        return Rectangle( _rBase - _aVector, _rBase + _aVector );
    }
    // -----------------------------------------------------------------------------
    /** GetTextPos calculate the rectangle for the connection to be drawn
            @param  _pWin           the table window where to draw it
            @param  _aConnPos       the connection point
            @param  _aDescrLinePos  the description line pos
    */
    Rectangle GetTextPos(const OTableWindow* _pWin, const Point& _aConnPos,const Point& _aDescrLinePos)
    {
        OTableWindowListBox* pListBox = _pWin ? _pWin->GetListBox() : NULL;
        OSL_ENSURE(_pWin && pListBox, "OConnectionLine::GetSourceTextPos : invalid call !");

        Rectangle aReturn;
        if ( pListBox )
        {
            const long nRowHeight = pListBox->GetEntryHeight();
            aReturn.Top() = _aConnPos.Y() - nRowHeight;
            aReturn.Bottom() = aReturn.Top() + nRowHeight;
            if (_aDescrLinePos.X() < _aConnPos.X())
            {
                aReturn.Left() = _aDescrLinePos.X();
                aReturn.Right() = aReturn.Left() + _aConnPos.X() - _aDescrLinePos.X();
            }
            else
            {
                aReturn.Left() = _aConnPos.X();
                aReturn.Right() = aReturn.Left() + _aDescrLinePos.X() - _aConnPos.X();
            }
        }

        return aReturn;
    }
    // -----------------------------------------------------------------------------
    /** calcPointsYValue calculate the points Y value in relation to the listbox entry
            @param  _pWin           the corresponding window
            @param  _pEntry         the source or dest entry
            @param  _rNewConPos     (in/out) the connection pos
            @param  _rNewDescrPos   (in/out) the description pos
    */
    void calcPointsYValue(const OTableWindow* _pWin,SvTreeListEntry* _pEntry,Point& _rNewConPos,Point& _rNewDescrPos)
    {
        const OTableWindowListBox* pListBox = _pWin->GetListBox();
        _rNewConPos.Y() = _pWin->GetPosPixel().Y();
        if ( _pEntry )
        {
            const long nRowHeight = pListBox->GetEntryHeight();
            _rNewConPos.Y() += pListBox->GetPosPixel().Y();
            long nEntryPos = pListBox->GetEntryPosition( _pEntry ).Y();

            if( nEntryPos >= 0 )
            {
                _rNewConPos.Y() += nEntryPos;
                _rNewConPos.Y() += (long)( 0.5 * nRowHeight );
            }
            else
                _rNewConPos.Y() -= (long)( 0.5 * nRowHeight );

            long nListBoxBottom = _pWin->GetPosPixel().Y()
                                    + pListBox->GetPosPixel().Y()
                                    + pListBox->GetSizePixel().Height();
            if( _rNewConPos.Y() > nListBoxBottom )
                _rNewConPos.Y() = nListBoxBottom + 2;
        }
        else
            _rNewConPos.Y() += static_cast<sal_Int32>(pListBox->GetPosPixel().Y()*0.5);

        _rNewDescrPos.Y() = _rNewConPos.Y();
    }
    // -----------------------------------------------------------------------------
}

//========================================================================
// class OConnectionLine
//========================================================================
DBG_NAME(OConnectionLine)
//------------------------------------------------------------------------
OConnectionLine::OConnectionLine( OTableConnection* _pConn, OConnectionLineDataRef _pLineData )
    : m_pTabConn( _pConn )
     ,m_pData( _pLineData )
{
    DBG_CTOR(OConnectionLine,NULL);
}

//------------------------------------------------------------------------
OConnectionLine::OConnectionLine( const OConnectionLine& _rLine )
{
    DBG_CTOR(OConnectionLine,NULL);
    m_pData = new OConnectionLineData( *_rLine.GetData() );
    *this = _rLine;
}

//------------------------------------------------------------------------
OConnectionLine::~OConnectionLine()
{
    DBG_DTOR(OConnectionLine,NULL);
}

//------------------------------------------------------------------------
OConnectionLine& OConnectionLine::operator=( const OConnectionLine& rLine )
{
    if( &rLine != this )
    {
        // da mir die Daten nicht gehoeren, loesche ich die alten nicht
        m_pData->CopyFrom(*rLine.GetData());
            // CopyFrom ist virtuell, damit ist es kein Problem, wenn m_pData von einem von OTableConnectionData abgeleiteten Typ ist

        m_pTabConn = rLine.m_pTabConn;
        m_aSourceConnPos = rLine.m_aSourceConnPos;
        m_aDestConnPos = rLine.m_aDestConnPos;
        m_aSourceDescrLinePos = rLine.m_aSourceDescrLinePos;
        m_aDestDescrLinePos = rLine.m_aDestDescrLinePos;
    }

    return *this;
}

//------------------------------------------------------------------------
Rectangle OConnectionLine::GetBoundingRect()
{
    //////////////////////////////////////////////////////////////////////
    // Umgebendes Rechteck bestimmen
    Rectangle aBoundingRect( Point(0,0), Point(0,0) );
    if( !IsValid() )
        return aBoundingRect;

    Point aTopLeft;
    Point aBottomRight;

    if( m_aSourceDescrLinePos.Y() <= m_aDestDescrLinePos.Y() )
    {
        aTopLeft.Y() = m_aSourceDescrLinePos.Y();
        aBottomRight.Y() = m_aDestDescrLinePos.Y();
    }
    else
    {
        aTopLeft.Y() = m_aDestDescrLinePos.Y();
        aBottomRight.Y() = m_aSourceDescrLinePos.Y();
    }

    if( m_aSourceDescrLinePos.X() <= m_aDestDescrLinePos.X() )
    {
        aTopLeft.X() = m_aSourceDescrLinePos.X();
        aBottomRight.X() = m_aDestDescrLinePos.X();
    }
    else
    {
        aTopLeft.X() = m_aDestDescrLinePos.X();
        aBottomRight.X() = m_aSourceDescrLinePos.X();
    }

    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();
    //////////////////////////////////////////////////////////////////////
    // Linie verlaeuft in z-Form
    if( pSourceWin == pDestWin || Abs(m_aSourceConnPos.X() - m_aDestConnPos.X()) > Abs(m_aSourceDescrLinePos.X() - m_aDestDescrLinePos.X()) )
    {
        aTopLeft.X() -= DESCRIPT_LINE_WIDTH;
        aBottomRight.X() += DESCRIPT_LINE_WIDTH;
    }

    aBoundingRect = Rectangle( aTopLeft-Point(2,17), aBottomRight+Point(2,2) );

    return aBoundingRect;
}
// -----------------------------------------------------------------------------
void calcPointX1(const OTableWindow* _pWin,Point& _rNewConPos,Point& _rNewDescrPos)
{
    _rNewConPos.X() = _pWin->GetPosPixel().X() + _pWin->GetSizePixel().Width();
    _rNewDescrPos.X() = _rNewConPos.X();
    _rNewConPos.X() += DESCRIPT_LINE_WIDTH;
}
// -----------------------------------------------------------------------------
void calcPointX2(const OTableWindow* _pWin,Point& _rNewConPos,Point& _rNewDescrPos)
{
    _rNewConPos.X() = _pWin->GetPosPixel().X();
    _rNewDescrPos.X() = _rNewConPos.X();
    _rNewConPos.X() -= DESCRIPT_LINE_WIDTH;
}
//------------------------------------------------------------------------
sal_Bool OConnectionLine::RecalcLine()
{
    //////////////////////////////////////////////////////////////////////
    // Fenster und Entries muessen gesetzt sein
    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();

    if( !pSourceWin || !pDestWin )
        return sal_False;

    SvTreeListEntry* pSourceEntry = pSourceWin->GetListBox()->GetEntryFromText( GetData()->GetSourceFieldName() );
    SvTreeListEntry* pDestEntry = pDestWin->GetListBox()->GetEntryFromText( GetData()->GetDestFieldName() );

    //////////////////////////////////////////////////////////////////////
    // X-Koordinaten bestimmen
    Point aSourceCenter( 0, 0 );
    Point aDestCenter( 0, 0 );

    aSourceCenter.X() = pSourceWin->GetPosPixel().X() + (long)( 0.5*pSourceWin->GetSizePixel().Width() );
    aDestCenter.X() = pDestWin->GetPosPixel().X() + (long)( 0.5*pDestWin->GetSizePixel().Width() );

    const OTableWindow* pFirstWin   = pDestWin;
    const OTableWindow* pSecondWin  = pSourceWin;
    Point* pFirstConPos             = &m_aDestConnPos;
    Point* pFirstDescrPos           = &m_aDestDescrLinePos;
    Point* pSecondConPos            = &m_aSourceConnPos;
    Point* pSecondDescrPos          = &m_aSourceDescrLinePos;
    if( aDestCenter.X() > aSourceCenter.X() )
    {
        pFirstWin       = pSourceWin;
        pSecondWin      = pDestWin;
        pFirstConPos    = &m_aSourceConnPos;
        pFirstDescrPos  = &m_aSourceDescrLinePos;
        pSecondConPos   = &m_aDestConnPos;
        pSecondDescrPos = &m_aDestDescrLinePos;
    }

    if ( pFirstWin == pSecondWin && pSourceEntry != pDestEntry )
        calcPointX2(pFirstWin,*pFirstConPos,*pFirstDescrPos);
    else
        calcPointX1(pFirstWin,*pFirstConPos,*pFirstDescrPos);
    calcPointX2(pSecondWin,*pSecondConPos,*pSecondDescrPos);

    //////////////////////////////////////////////////////////////////////
    // aSourceConnPosY bestimmen
    calcPointsYValue(pSourceWin,pSourceEntry,m_aSourceConnPos,m_aSourceDescrLinePos);

    //////////////////////////////////////////////////////////////////////
    // aDestConnPosY bestimmen
    calcPointsYValue(pDestWin,pDestEntry,m_aDestConnPos,m_aDestDescrLinePos);

    return sal_True;
}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------
void OConnectionLine::Draw( OutputDevice* pOutDev )
{
    const sal_uInt16 nRectSize = 3;

    //////////////////////////////////////////////////////////////////////
    // Neue Dimensionen berechnen
    if( !RecalcLine() )
        return;

    //////////////////////////////////////////////////////////////////////
    // Zeichnen der Linien
    if (m_pTabConn->IsSelected())
        pOutDev->SetLineColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    else
        pOutDev->SetLineColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());

    LineInfo aLineInfo;
    if ( m_pTabConn->IsSelected() )
        aLineInfo.SetWidth(3);
    Polygon aPoly;
    aPoly.Insert(0,m_aSourceDescrLinePos);
    aPoly.Insert(1,m_aSourceConnPos);
    aPoly.Insert(2,m_aDestConnPos);
    aPoly.Insert(3,m_aDestDescrLinePos);
    pOutDev->DrawPolyLine(aPoly,aLineInfo);

    //////////////////////////////////////////////////////////////////////
    // draw the connection rectangles
    pOutDev->SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());

    Point aVector(nRectSize,nRectSize);
    pOutDev->DrawRect( calcRect(m_aSourceDescrLinePos,aVector) );
    pOutDev->DrawRect( calcRect( m_aDestDescrLinePos,aVector) );
}
// -----------------------------------------------------------------------------
sal_Bool OConnectionLine::IsValid() const
{
    return m_pData.is();
}
//------------------------------------------------------------------------
double dist_Euklid(const Point &p1, const Point& p2,const Point& pM, Point& q)
{
    Point v(p2 - p1);
    Point w(pM - p1);
    double a = sqrt((double)(v.X()*v.X() + v.Y()*v.Y()));
    double l = (v.X() * w.Y() - v.Y() * w.X()) / a;
    double a2 = w.X()*v.X()+w.Y()*v.Y();
    a = a2 / (a * a);
    q.X() = long(p1.X() + a * v.X());
    q.Y() = long(p1.Y() + a * v.Y());
    return l;
}
//------------------------------------------------------------------------
bool OConnectionLine::CheckHit( const Point& rMousePos ) const
{
    //////////////////////////////////////////////////////////////////////
    /*
        Vorgehensweise beim HitTest:
        Es wird der Abstand nach Euklid berechnet.
    */
    Point q;
    double l = fabs(dist_Euklid(m_aSourceConnPos,m_aDestConnPos,rMousePos,q));
    if( l < HIT_SENSITIVE_RADIUS)
    {
        if(::std::min(m_aSourceConnPos.X(),m_aDestConnPos.X()) <= q.X() && ::std::min(m_aSourceConnPos.Y(),m_aDestConnPos.Y()) <= q.Y()
            && q.X() <= ::std::max(m_aDestConnPos.X(),m_aSourceConnPos.X())   && q.Y() <= ::std::max(m_aDestConnPos.Y(),m_aSourceConnPos.Y()))
            return true;
    }

    return false;
}
// -----------------------------------------------------------------------------
Rectangle OConnectionLine::GetSourceTextPos() const
{
    return GetTextPos(m_pTabConn->GetSourceWin(),m_aSourceConnPos,m_aSourceDescrLinePos);
}
// -----------------------------------------------------------------------------
Rectangle OConnectionLine::GetDestTextPos() const
{
    return GetTextPos(m_pTabConn->GetDestWin(),m_aDestConnPos,m_aDestDescrLinePos);
}
// -----------------------------------------------------------------------------
Point OConnectionLine::getMidPoint() const
{
    Point aDest = m_aDestConnPos - m_aSourceConnPos;
    aDest.X() /= 2;
    aDest.Y() /= 2;

    return m_aSourceConnPos + aDest;
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
