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

#include <ConnectionLine.hxx>
#include <ConnectionLineData.hxx>
#include <TableWindow.hxx>
#include <TableWindowListBox.hxx>
#include <TableConnection.hxx>
#include <vcl/svapp.hxx>
#include <math.h>
#include <osl/diagnose.h>
#include <vcl/lineinfo.hxx>
#include <vcl/settings.hxx>

using namespace dbaui;
const long DESCRIPT_LINE_WIDTH = 15;
const long HIT_SENSITIVE_RADIUS = 5;

namespace
{
    /** calcRect creates a new rectangle with the given points
            @param  _rBase      the base point
            @param  _aVector    the vector which will be added
    */
    tools::Rectangle calcRect(const Point& _rBase,const Point& _aVector)
    {
        return tools::Rectangle( _rBase - _aVector, _rBase + _aVector );
    }
    /** GetTextPos calculate the rectangle for the connection to be drawn
            @param  _pWin           the table window where to draw it
            @param  _aConnPos       the connection point
            @param  _aDescrLinePos  the description line pos
    */
    tools::Rectangle GetTextPos(const OTableWindow* _pWin, const Point& _aConnPos,const Point& _aDescrLinePos)
    {
        VclPtr<OTableWindowListBox> pListBox = _pWin ? _pWin->GetListBox() : nullptr;
        OSL_ENSURE(_pWin && pListBox, "OConnectionLine::GetSourceTextPos : invalid call !");

        tools::Rectangle aReturn;
        if ( pListBox )
        {
            const long nRowHeight = pListBox->get_widget().get_height_rows(1);
            aReturn.SetTop( _aConnPos.Y() - nRowHeight );
            aReturn.SetBottom( aReturn.Top() + nRowHeight );
            if (_aDescrLinePos.X() < _aConnPos.X())
            {
                aReturn.SetLeft( _aDescrLinePos.X() );
                aReturn.SetRight( aReturn.Left() + _aConnPos.X() - _aDescrLinePos.X() );
            }
            else
            {
                aReturn.SetLeft( _aConnPos.X() );
                aReturn.SetRight( aReturn.Left() + _aDescrLinePos.X() - _aConnPos.X() );
            }
        }

        return aReturn;
    }
    /** calcPointsYValue calculate the points Y value in relation to the listbox entry
            @param  _pWin           the corresponding window
            @param  _nEntry         the source or dest entry
            @param  _rNewConPos     (in/out) the connection pos
            @param  _rNewDescrPos   (in/out) the description pos
    */
    void calcPointsYValue(const OTableWindow* _pWin, int _nEntry, Point& _rNewConPos, Point& _rNewDescrPos)
    {
        const OTableWindowListBox* pListBox = _pWin->GetListBox();
        _rNewConPos.setY( _pWin->GetPosPixel().Y() );

        std::unique_ptr<weld::TreeIter> xEntry;
        const weld::TreeView& rTreeView = pListBox->get_widget();

        if (_nEntry != -1)
        {
            _rNewConPos.AdjustY(pListBox->GetPosPixel().Y() );
            xEntry = rTreeView.make_iterator();
            if (!rTreeView.get_iter_first(*xEntry) || !rTreeView.iter_nth_sibling(*xEntry, _nEntry))
                xEntry.reset();
        }

        if (xEntry)
        {
            auto nEntryPos = rTreeView.get_row_area(*xEntry).Center().Y();

            if( nEntryPos >= 0 )
            {
                _rNewConPos.AdjustY(nEntryPos);
            }
            else
            {
                const auto nRowHeight = rTreeView.get_height_rows(1);
                _rNewConPos.AdjustY( -static_cast<long>( 0.5 * nRowHeight ) );
            }

            long nListBoxBottom = _pWin->GetPosPixel().Y()
                                    + pListBox->GetPosPixel().Y()
                                    + pListBox->GetSizePixel().Height();
            if( _rNewConPos.Y() > nListBoxBottom )
                _rNewConPos.setY( nListBoxBottom + 2 );
        }
        else
            _rNewConPos.AdjustY(static_cast<sal_Int32>(pListBox->GetPosPixel().Y()*0.5) );

        _rNewDescrPos.setY( _rNewConPos.Y() );
    }
}

OConnectionLine::OConnectionLine( OTableConnection* _pConn, OConnectionLineDataRef const & _pLineData )
    : m_pTabConn( _pConn )
    , m_pData( _pLineData )
{
}

OConnectionLine::OConnectionLine( const OConnectionLine& _rLine )
    : m_pTabConn(nullptr)
{
    m_pData = new OConnectionLineData( *_rLine.GetData() );
    *this = _rLine;
}

OConnectionLine::~OConnectionLine()
{
}

OConnectionLine& OConnectionLine::operator=( const OConnectionLine& rLine )
{
    if( &rLine != this )
    {
        // as the data does not belong to me, I don't delete the old one
        m_pData->CopyFrom(*rLine.GetData());
            // CopyFrom is virtual, therefore it is not a problem, if m_pData is of a type derived from OTableConnectionData

        m_pTabConn = rLine.m_pTabConn;
        m_aSourceConnPos = rLine.m_aSourceConnPos;
        m_aDestConnPos = rLine.m_aDestConnPos;
        m_aSourceDescrLinePos = rLine.m_aSourceDescrLinePos;
        m_aDestDescrLinePos = rLine.m_aDestDescrLinePos;
    }

    return *this;
}

tools::Rectangle OConnectionLine::GetBoundingRect() const
{
    // determine surrounding rectangle
    tools::Rectangle aBoundingRect( Point(0,0), Point(0,0) );
    if( !IsValid() )
        return aBoundingRect;

    Point aTopLeft;
    Point aBottomRight;

    if( m_aSourceDescrLinePos.Y() <= m_aDestDescrLinePos.Y() )
    {
        aTopLeft.setY( m_aSourceDescrLinePos.Y() );
        aBottomRight.setY( m_aDestDescrLinePos.Y() );
    }
    else
    {
        aTopLeft.setY( m_aDestDescrLinePos.Y() );
        aBottomRight.setY( m_aSourceDescrLinePos.Y() );
    }

    if( m_aSourceDescrLinePos.X() <= m_aDestDescrLinePos.X() )
    {
        aTopLeft.setX( m_aSourceDescrLinePos.X() );
        aBottomRight.setX( m_aDestDescrLinePos.X() );
    }
    else
    {
        aTopLeft.setX( m_aDestDescrLinePos.X() );
        aBottomRight.setX( m_aSourceDescrLinePos.X() );
    }

    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();
    // line proceeds in z-Form
    if( pSourceWin == pDestWin || std::abs(m_aSourceConnPos.X() - m_aDestConnPos.X()) > std::abs(m_aSourceDescrLinePos.X() - m_aDestDescrLinePos.X()) )
    {
        aTopLeft.AdjustX( -DESCRIPT_LINE_WIDTH );
        aBottomRight.AdjustX(DESCRIPT_LINE_WIDTH );
    }

    aBoundingRect = tools::Rectangle( aTopLeft-Point(2,17), aBottomRight+Point(2,2) );

    return aBoundingRect;
}

static void calcPointX1(const OTableWindow* _pWin,Point& _rNewConPos,Point& _rNewDescrPos)
{
    _rNewConPos.setX( _pWin->GetPosPixel().X() + _pWin->GetSizePixel().Width() );
    _rNewDescrPos.setX( _rNewConPos.X() );
    _rNewConPos.AdjustX(DESCRIPT_LINE_WIDTH );
}

static void calcPointX2(const OTableWindow* _pWin,Point& _rNewConPos,Point& _rNewDescrPos)
{
    _rNewConPos.setX( _pWin->GetPosPixel().X() );
    _rNewDescrPos.setX( _rNewConPos.X() );
    _rNewConPos.AdjustX( -DESCRIPT_LINE_WIDTH );
}

bool OConnectionLine::RecalcLine()
{
    // Windows and entries must be set
    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();

    if( !pSourceWin || !pDestWin )
        return false;

    int nSourceEntry = pSourceWin->GetListBox()->GetEntryFromText( GetData()->GetSourceFieldName() );
    int nDestEntry = pDestWin->GetListBox()->GetEntryFromText( GetData()->GetDestFieldName() );

    // determine X-coordinates
    Point aSourceCenter( 0, 0 );
    Point aDestCenter( 0, 0 );

    aSourceCenter.setX( pSourceWin->GetPosPixel().X() + static_cast<long>( 0.5*pSourceWin->GetSizePixel().Width() ) );
    aDestCenter.setX( pDestWin->GetPosPixel().X() + static_cast<long>( 0.5*pDestWin->GetSizePixel().Width() ) );

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

    if (pFirstWin == pSecondWin && nSourceEntry != nDestEntry)
        calcPointX2(pFirstWin,*pFirstConPos,*pFirstDescrPos);
    else
        calcPointX1(pFirstWin,*pFirstConPos,*pFirstDescrPos);
    calcPointX2(pSecondWin,*pSecondConPos,*pSecondDescrPos);

    // determine aSourceConnPosY
    calcPointsYValue(pSourceWin, nSourceEntry, m_aSourceConnPos,m_aSourceDescrLinePos);

    // determine aDestConnPosY
    calcPointsYValue(pDestWin, nDestEntry, m_aDestConnPos,m_aDestDescrLinePos);

    return true;
}

void OConnectionLine::Draw( OutputDevice* pOutDev )
{
    const sal_uInt16 nRectSize = 3;

    // calculate new dimension
    if( !RecalcLine() )
        return;

    // draw lines
    if (m_pTabConn->IsSelected())
        pOutDev->SetLineColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    else
        pOutDev->SetLineColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());

    LineInfo aLineInfo;
    if ( m_pTabConn->IsSelected() )
        aLineInfo.SetWidth(3);
    tools::Polygon aPoly;
    aPoly.Insert(0,m_aSourceDescrLinePos);
    aPoly.Insert(1,m_aSourceConnPos);
    aPoly.Insert(2,m_aDestConnPos);
    aPoly.Insert(3,m_aDestDescrLinePos);
    pOutDev->DrawPolyLine(aPoly,aLineInfo);

    // draw the connection rectangles
    pOutDev->SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());

    Point aVector(nRectSize,nRectSize);
    pOutDev->DrawRect( calcRect(m_aSourceDescrLinePos,aVector) );
    pOutDev->DrawRect( calcRect( m_aDestDescrLinePos,aVector) );
}

bool OConnectionLine::IsValid() const
{
    return m_pData.is();
}

static double dist_Euklid(const Point &p1, const Point& p2,const Point& pM, Point& q)
{
    Point v(p2 - p1);
    Point w(pM - p1);
    double a = sqrt(static_cast<double>(v.X()*v.X() + v.Y()*v.Y()));
    double l = (v.X() * w.Y() - v.Y() * w.X()) / a;
    double a2 = w.X()*v.X()+w.Y()*v.Y();
    a = a2 / (a * a);
    q.setX( long(p1.X() + a * v.X()) );
    q.setY( long(p1.Y() + a * v.Y()) );
    return l;
}

bool OConnectionLine::CheckHit( const Point& rMousePos ) const
{
    /*
        course of action with HitTest:
        the distance is calculated according to Euklid.
    */
    Point q;
    double l = fabs(dist_Euklid(m_aSourceConnPos,m_aDestConnPos,rMousePos,q));
    if( l < HIT_SENSITIVE_RADIUS)
    {
        if(std::min(m_aSourceConnPos.X(),m_aDestConnPos.X()) <= q.X() && std::min(m_aSourceConnPos.Y(),m_aDestConnPos.Y()) <= q.Y()
            && q.X() <= std::max(m_aDestConnPos.X(),m_aSourceConnPos.X())   && q.Y() <= std::max(m_aDestConnPos.Y(),m_aSourceConnPos.Y()))
            return true;
    }

    return false;
}

tools::Rectangle OConnectionLine::GetSourceTextPos() const
{
    return GetTextPos(m_pTabConn->GetSourceWin(),m_aSourceConnPos,m_aSourceDescrLinePos);
}

tools::Rectangle OConnectionLine::GetDestTextPos() const
{
    return GetTextPos(m_pTabConn->GetDestWin(),m_aDestConnPos,m_aDestDescrLinePos);
}

Point OConnectionLine::getMidPoint() const
{
    Point aDest = m_aDestConnPos - m_aSourceConnPos;
    aDest.setX( aDest.X() / 2 );
    aDest.setY( aDest.Y() / 2 );

    return m_aSourceConnPos + aDest;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
