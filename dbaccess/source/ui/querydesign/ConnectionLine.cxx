/*************************************************************************
 *
 *  $RCSfile: ConnectionLine.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:18:26 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _INC_MATH
#include <math.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace dbaui;
const long DESCRIPT_LINE_WIDTH = 15;
const long HIT_SENSITIVE_RADIUS = 5;
//========================================================================
// class OConnectionLine
//========================================================================
DBG_NAME(OConnectionLine);
//------------------------------------------------------------------------
OConnectionLine::OConnectionLine( OTableConnection* _pConn, OConnectionLineData* _pLineData )
    : m_pTabConn( _pConn )
     ,m_pData( _pLineData )
     ,m_pSourceEntry( NULL )
     ,m_pDestEntry( NULL )
{
    DBG_CTOR(OConnectionLine,NULL);
}

//------------------------------------------------------------------------
OConnectionLine::OConnectionLine( OTableConnection* _pConn, const String& _rSourceFieldName,
                                  const String& _rDestFieldName )
    : m_pTabConn( _pConn )
     ,m_pSourceEntry( NULL )
     ,m_pDestEntry( NULL )
{
    DBG_CTOR(OConnectionLine,NULL);
    m_pData->SetSourceFieldName( _rSourceFieldName );
    m_pData->SetDestFieldName( _rDestFieldName );
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
void OConnectionLine::SetSourceFieldName( const String& rSourceFieldName )
{
    m_pData->SetSourceFieldName( rSourceFieldName );
//  m_pSourceEntry = m_pTabConn->GetSourceWin()->GetListBox()->GetEntryFromText( rSourceFieldName );
}

//------------------------------------------------------------------------
void OConnectionLine::SetDestFieldName( const String& rDestFieldName )
{
    m_pData->SetDestFieldName( rDestFieldName );
//  m_pDestEntry = m_pTabConn->GetDestWin()->GetListBox()->GetEntryFromText( rDestFieldName );
}

//------------------------------------------------------------------------
OConnectionLine& OConnectionLine::operator=( const OConnectionLine& rLine )
{
    if( &rLine == this )
        return *this;

    // da mir die Daten nicht gehoeren, loesche ich die alten nicht
    m_pData->CopyFrom(*rLine.GetData());
        // CopyFrom ist virtuell, damit ist es kein Problem, wenn m_pData von einem von OTableConnectionData abgeleiteten Typ ist

    m_pTabConn = rLine.m_pTabConn;
    m_pSourceEntry = rLine.m_pSourceEntry;
    m_pDestEntry = rLine.m_pDestEntry;
    m_aSourceConnPos = rLine.m_aSourceConnPos;
    m_aDestConnPos = rLine.m_aDestConnPos;
    m_aSourceDescrLinePos = rLine.m_aSourceDescrLinePos;
    m_aDestDescrLinePos = rLine.m_aDestDescrLinePos;

    return *this;
}

//------------------------------------------------------------------------
BOOL OConnectionLine::Connect( const String& rSourceFieldName, const String& rDestFieldName )
{
    //////////////////////////////////////////////////////////////////////
    // Parameter duerfen nicht leer sein
    if( !rSourceFieldName.Len() || !rDestFieldName.Len() )
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // Feldnamen setzen
    m_pData->SetSourceFieldName( rSourceFieldName );
    m_pData->SetDestFieldName( rDestFieldName );

    //////////////////////////////////////////////////////////////////////
    // Entries setzen
/*  m_pSourceEntry = m_pTabConn->GetSourceWin()->GetListBox()->GetEntryFromText( rSourceFieldName );
    if( !m_pSourceEntry )
        return FALSE;
    m_pDestEntry = m_pTabConn->GetDestWin()->GetListBox()->GetEntryFromText( rDestFieldName );
    if( !m_pDestEntry )
        return FALSE;
*/

    return TRUE;
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

    //////////////////////////////////////////////////////////////////////
    // Linie verlaeuft in z-Form
    if( Abs(m_aSourceConnPos.X() - m_aDestConnPos.X()) > Abs(m_aSourceDescrLinePos.X() - m_aDestDescrLinePos.X()) )
    {
        aTopLeft.X() -= DESCRIPT_LINE_WIDTH;
        aBottomRight.X() += DESCRIPT_LINE_WIDTH;
    }

    aBoundingRect = Rectangle( aTopLeft-Point(0,17), aBottomRight+Point(0,2) );

    return aBoundingRect;
}

//------------------------------------------------------------------------
BOOL OConnectionLine::RecalcLine()
{
    //////////////////////////////////////////////////////////////////////
    // Fenster und Entries muessen gesetzt sein
    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();

    if( !pSourceWin || !pDestWin )
        return FALSE;

    m_pSourceEntry = pSourceWin->GetListBox()->GetEntryFromText( GetData()->GetSourceFieldName() );
    m_pDestEntry = pDestWin->GetListBox()->GetEntryFromText( GetData()->GetDestFieldName() );

    if( !m_pSourceEntry || !m_pDestEntry )
        return FALSE;

    //////////////////////////////////////////////////////////////////////
    // X-Koordinaten bestimmen
    Point aSourceCenter( 0, 0 );
    Point aDestCenter( 0, 0 );

    aSourceCenter.X() = pSourceWin->GetPosPixel().X() + (long)( 0.5*pSourceWin->GetSizePixel().Width() );
    aDestCenter.X() = pDestWin->GetPosPixel().X() + (long)( 0.5*pDestWin->GetSizePixel().Width() );
    if( aDestCenter.X()>aSourceCenter.X() )
    {
        //////////////////////////////////////////////////////////////////////
        // DestWin liegt rechts vom SourceWin
        m_aSourceConnPos.X() = pSourceWin->GetPosPixel().X()+pSourceWin->GetSizePixel().Width();
        m_aSourceDescrLinePos.X() = m_aSourceConnPos.X();
        m_aSourceConnPos.X() += DESCRIPT_LINE_WIDTH;

        m_aDestConnPos.X() = pDestWin->GetPosPixel().X();
        m_aDestDescrLinePos.X() = m_aDestConnPos.X();
        m_aDestConnPos.X() -= DESCRIPT_LINE_WIDTH;
    }
    else
    {
        //////////////////////////////////////////////////////////////////////
        // DestWin liegt links vom SourceWin
        m_aSourceConnPos.X() = pSourceWin->GetPosPixel().X();
        m_aSourceDescrLinePos.X() = m_aSourceConnPos.X();
        m_aSourceConnPos.X() -= DESCRIPT_LINE_WIDTH;

        m_aDestConnPos.X() = pDestWin->GetPosPixel().X()+pDestWin->GetSizePixel().Width();
        m_aDestDescrLinePos.X() = m_aDestConnPos.X();
        m_aDestConnPos.X() += DESCRIPT_LINE_WIDTH;
    }

    //////////////////////////////////////////////////////////////////////
    // aSourceConnPosY bestimmen
    OTableWindowListBox* pListBox = pSourceWin->GetListBox();
    long nRowHeight = pListBox->GetEntryHeight();

    m_aSourceConnPos.Y() = pSourceWin->GetPosPixel().Y();
    m_aSourceConnPos.Y() += pListBox->GetPosPixel().Y();
    long nEntryPos = pListBox->GetEntryPos( m_pSourceEntry ).Y();

    if( nEntryPos >= 0 )
    {
        m_aSourceConnPos.Y() += nEntryPos;
        m_aSourceConnPos.Y() += (long)( 0.5 * nRowHeight );
    }
    else
        m_aSourceConnPos.Y() -= (long)( 0.5 * nRowHeight );

    long nListBoxBottom = pSourceWin->GetPosPixel().Y()
                            +pListBox->GetPosPixel().Y()
                            +pListBox->GetSizePixel().Height();
    if( m_aSourceConnPos.Y() > nListBoxBottom )
        m_aSourceConnPos.Y() = nListBoxBottom + 2;

    m_aSourceDescrLinePos.Y() = m_aSourceConnPos.Y();

    //////////////////////////////////////////////////////////////////////
    // aDestConnPosY bestimmen
    pListBox = pDestWin->GetListBox();

    m_aDestConnPos.Y() = pDestWin->GetPosPixel().Y();
    m_aDestConnPos.Y() += pListBox->GetPosPixel().Y();
    nEntryPos = pListBox->GetEntryPos( m_pDestEntry ).Y();
    if( nEntryPos >= 0 )
    {
        m_aDestConnPos.Y() += nEntryPos;
        m_aDestConnPos.Y() += (long)( 0.5 * nRowHeight );
    }
    else
        m_aDestConnPos.Y() -= (long)( 0.5 * nRowHeight );

    nListBoxBottom = pDestWin->GetPosPixel().Y()
                        +pListBox->GetPosPixel().Y()
                        +pListBox->GetSizePixel().Height();
    if( m_aDestConnPos.Y() > nListBoxBottom )
        m_aDestConnPos.Y() = nListBoxBottom + 2;

    m_aDestDescrLinePos.Y() = m_aDestConnPos.Y();

    return TRUE;
}

//------------------------------------------------------------------------
Rectangle OConnectionLine::GetSourceTextPos() const
{
    const OTableWindow* pDestWin = m_pTabConn->GetDestWin();
    OTableWindowListBox* pListBox = pDestWin ? pDestWin->GetListBox() : NULL;
    DBG_ASSERT(pDestWin && pListBox, "OConnectionLine::GetSourceTextPos : invalid call !");

    long nRowHeight = pListBox->GetEntryHeight();

    Rectangle aReturn;
    aReturn.Top() = m_aSourceConnPos.Y() - nRowHeight;
    aReturn.Bottom() = aReturn.Top() + nRowHeight;
    if (m_aSourceDescrLinePos.X() < m_aSourceConnPos.X())
    {
        aReturn.Left() = m_aSourceDescrLinePos.X();
        aReturn.Right() = aReturn.Left() + m_aSourceConnPos.X() - m_aSourceDescrLinePos.X();
    }
    else
    {
        aReturn.Left()  = m_aSourceConnPos.X();
        aReturn.Right() = aReturn.Left() + m_aSourceDescrLinePos.X() - m_aSourceConnPos.X();
    }

    return aReturn;
}

//------------------------------------------------------------------------
Rectangle OConnectionLine::GetDestTextPos() const
{
    const OTableWindow* pSourceWin = m_pTabConn->GetSourceWin();
    OTableWindowListBox* pListBox = pSourceWin ? pSourceWin->GetListBox() : NULL;
    DBG_ASSERT(pSourceWin && pListBox, "OConnectionLine::GetSourceTextPos : invalid call !");

    long nRowHeight = pListBox->GetEntryHeight();

    Rectangle aReturn;
    aReturn.Top() = m_aDestConnPos.Y() - nRowHeight;
    aReturn.Bottom() = aReturn.Top() + nRowHeight;
    if (m_aDestDescrLinePos.X() < m_aDestConnPos.X())
    {
        aReturn.Left() = m_aDestDescrLinePos.X();
        aReturn.Right() = aReturn.Left() + m_aDestConnPos.X() - m_aDestDescrLinePos.X();
    }
    else
    {
        aReturn.Left() = m_aDestConnPos.X();
        aReturn.Right() = aReturn.Left() + m_aDestDescrLinePos.X() - m_aDestConnPos.X();
    }

    return aReturn;
}

//------------------------------------------------------------------------
void OConnectionLine::Draw( OutputDevice* pOutDev )
{
    const UINT16 nRectSize = 3;

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

    pOutDev->DrawLine( m_aSourceDescrLinePos, m_aSourceConnPos );
    pOutDev->DrawLine( m_aDestDescrLinePos, m_aDestConnPos );
    pOutDev->DrawLine( m_aSourceConnPos, m_aDestConnPos );
    // wenn die  Linie selektiert ist, sollte sie dicker erscheinen
    // da OutputDevice nach meinem Wissen (das ich nur aus dem Headerfile habe) kein
    // SetLineWidth (o.ä.) hat, ein Fake
    if (m_pTabConn->IsSelected())
    {
        UINT16 xOffset, yOffset;
        if (abs(m_aSourceConnPos.Y() - m_aDestConnPos.Y()) > abs(m_aSourceConnPos.X()-m_aDestConnPos.X()))
        {
            xOffset = 1;
            yOffset = 0;
        } else
        {
            xOffset = 0;
            yOffset = 1;
        }
        pOutDev->DrawLine(m_aSourceConnPos + Point(-xOffset, -yOffset), m_aDestConnPos + Point(-xOffset, -yOffset));
        pOutDev->DrawLine(m_aSourceConnPos + Point(xOffset, yOffset), m_aDestConnPos + Point(xOffset, yOffset));
    }


    //////////////////////////////////////////////////////////////////////
    // Zeichnen der Verbindungs-Rechtecke
//  pOutDev->SetLineColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());
    Rectangle aSourceRect( m_aSourceDescrLinePos-Point(nRectSize,nRectSize),
                           m_aSourceDescrLinePos+Point(nRectSize,nRectSize) );

    Rectangle aDestRect( m_aDestDescrLinePos-Point(nRectSize,nRectSize),
                         m_aDestDescrLinePos+Point(nRectSize,nRectSize) );
    pOutDev->SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());
    pOutDev->DrawRect( aSourceRect );
    pOutDev->DrawRect( aDestRect );
}

//------------------------------------------------------------------------
BOOL OConnectionLine::IsValid()
{
    return m_pData->IsValid();
}
//------------------------------------------------------------------------
double dist_Euklid(const Point &p1, const Point& p2,const Point& pM, Point& q)
{
    int vx = p2.X() - p1.X();
    int vy = p2.Y() - p1.Y();
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
BOOL OConnectionLine::CheckHit( const Point& rMousePos )
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
            return TRUE;
    }

    return FALSE;
}


