/*************************************************************************
 *
 *  $RCSfile: TableConnection.cxx,v $
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
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


using namespace dbaui;
using namespace comphelper;

TYPEINIT0(OTableConnection);
//========================================================================
// class OTableConnection
//========================================================================
DBG_NAME(OTableConnection);
//------------------------------------------------------------------------
OTableConnection::OTableConnection( OJoinTableView* _pContainer, OTableConnectionData* _pTabConnData )
    :m_bSelected( FALSE )
    ,m_pParent( _pContainer )
    ,m_pData( _pTabConnData )
{
    DBG_CTOR(OTableConnection,NULL);
    Init();
}

//------------------------------------------------------------------------
OTableConnection::OTableConnection( const OTableConnection& _rConn )
{
    DBG_CTOR(OTableConnection,NULL);
    m_pData = _rConn.GetData()->NewInstance();
    *this = _rConn;
}
//------------------------------------------------------------------------
OTableConnection::~OTableConnection()
{
    //////////////////////////////////////////////////////////////////////
    // clear vector
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
        delete *aIter;
    m_vConnLine.clear();
    DBG_DTOR(OTableConnection,NULL);
}

//------------------------------------------------------------------------
void OTableConnection::Init()
{
    //////////////////////////////////////////////////////////////////////
    // Linienliste mit Defaults initialisieren
    ::std::vector<OConnectionLineData*>* pLineData = GetData()->GetConnLineDataList();
    ::std::vector<OConnectionLineData*>::const_iterator aIter = pLineData->begin();
    for(;aIter != pLineData->end();++aIter)
    {
        OConnectionLine* pConnLine = new OConnectionLine(this, *aIter);
        m_vConnLine.push_back( pConnLine);
    }
}

//------------------------------------------------------------------------
OConnectionLine* OTableConnection::CreateConnLine( const OConnectionLine& rConnLine )
{
    return new OConnectionLine( rConnLine );
}
//------------------------------------------------------------------------
void OTableConnection::UpdateLineList()
{
    //////////////////////////////////////////////////////////////////////
    // Linienliste loeschen
    for(::std::vector<OConnectionLine*>::iterator aLineIter = m_vConnLine.begin();aLineIter != m_vConnLine.end();++aLineIter)
        delete *aLineIter;
    m_vConnLine.clear();

    //////////////////////////////////////////////////////////////////////
    // Linienliste aus der Datenliste neu erstellen
    ::std::vector<OConnectionLineData*>* pLineData = GetData()->GetConnLineDataList();
    ::std::vector<OConnectionLineData*>::const_iterator aIter = pLineData->begin();
    for(;aIter != pLineData->end();++aIter)
    {
        OConnectionLine* pConnLine = new OConnectionLine( this, *aIter );
        m_vConnLine.push_back(pConnLine);
    }

}

//------------------------------------------------------------------------
OTableConnection& OTableConnection::operator=( const OTableConnection& rConn )
{
    if( &rConn == this )
        return *this;

    // Linienliste loeschen
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
        delete *aIter;
    m_vConnLine.clear();

    // Linienliste kopieren
    if( rConn.GetConnLineList()->size() )
    {
        const ::std::vector<OConnectionLine*>* pLine = rConn.GetConnLineList();
        ::std::vector<OConnectionLine*>::const_iterator aIter = pLine->begin();
        for(;aIter != pLine->end();++aIter)
        {
            OConnectionLine* pDestLine = CreateConnLine( **aIter );
            m_vConnLine.push_back( pDestLine);
        }
    }

    // da mir die Daten nicht gehoeren, loesche ich die alten nicht
    m_pData->CopyFrom(*rConn.GetData());
        // CopyFrom ist virtuell, damit ist es kein Problem, wenn m_pData von einem von OTableConnectionData abgeleiteten Typ ist

    m_bSelected = rConn.m_bSelected;
    m_pParent = rConn.m_pParent;

    return *this;
}


//------------------------------------------------------------------------
void OTableConnection::RecalcLines()
{
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
        (*aIter)->RecalcLine();
}
//------------------------------------------------------------------------
OTableWindow* OTableConnection::GetSourceWin() const
{
    return m_pParent->GetWindow( GetData()->GetSourceWinName() );
}
//------------------------------------------------------------------------
OTableWindow* OTableConnection::GetDestWin() const
{
    return m_pParent->GetWindow( GetData()->GetDestWinName() );
}

//------------------------------------------------------------------------
void OTableConnection::Select()
{
    m_bSelected = TRUE;
    m_pParent->Invalidate( GetBoundingRect(), INVALIDATE_NOCHILDREN);
}

//------------------------------------------------------------------------
void OTableConnection::Deselect()
{
    m_bSelected = FALSE;
    Rectangle rcBounding = GetBoundingRect();
    rcBounding.Bottom() += 1;
    rcBounding.Right() += 1;
    m_pParent->Invalidate( rcBounding, INVALIDATE_NOCHILDREN );
        // Kommentar siehe ::Invalidate ...
}

//------------------------------------------------------------------------
BOOL OTableConnection::CheckHit( const Point& rMousePos )
{
    //////////////////////////////////////////////////////////////////////
    // Pruefen, ob auf eine der Linien geclickt worden ist
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
    {
        if( (*aIter)->IsValid() && (*aIter)->CheckHit(rMousePos) )
            return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------
void OTableConnection::Invalidate()
{
    Rectangle rcBounding = GetBoundingRect();
    rcBounding.Bottom() += 1;
    rcBounding.Right() += 1;
        // ich glaube, dass sich Invalidate und Draw(Rectangle) nicht konsistent verhalten : jedenfalls waere dadurch zu
        // erklaeren, warum ohne diesen Fake hier beim Loeschen einer Connection ein Strich an ihrem unteren Ende stehen bleibt :
        // Invalidate erfasst dabei offensichtlich eine Pixelzeile weniger als Draw.
        // Oder alles haengt ganz anders zusammen ... jedenfalls klappt es so ...
    m_pParent->Invalidate( rcBounding, INVALIDATE_NOCHILDREN );
}

//------------------------------------------------------------------------
Rectangle OTableConnection::GetBoundingRect()
{
    //////////////////////////////////////////////////////////////////////
    // Aus allen Linien das umgebende Rechteck bestimmen
    Rectangle aBoundingRect( Point(0,0), Point(0,0) );
    Rectangle aTempRect;
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
    {
        aTempRect = (*aIter)->GetBoundingRect();

        //////////////////////////////////////////////////////////////////////
        // Ist das BoundingRect dieser Linie gueltig?
        if( (aTempRect.GetWidth()!=1) && (aTempRect.GetHeight()!=1) )
        {
            if( (aBoundingRect.GetWidth()==1) && (aBoundingRect.GetHeight()==1) )
                aBoundingRect = (*aIter)->GetBoundingRect();
            else
                aBoundingRect.Union( (*aIter)->GetBoundingRect() );
        }
    }

    return aBoundingRect;
}

//------------------------------------------------------------------------
void OTableConnection::Draw( const Rectangle& rRect )
{
    //////////////////////////////////////////////////////////////////////
    // Nur neu zeichnen, wenn Connection innerhalb des ungueltigen Bereichs liegt
/*  Rectangle aIntersectRect = GetBoundingRect().GetIntersection( rRect );
    Rectangle aBoundingRect = GetBoundingRect();
    if( !aIntersectRect.GetWidth() && !aIntersectRect.GetHeight() )
        return;
*/

    //////////////////////////////////////////////////////////////////////
    // Linien zeichnen
    for(::std::vector<OConnectionLine*>::iterator aIter = m_vConnLine.begin();aIter != m_vConnLine.end();++aIter)
        (*aIter)->Draw( m_pParent );
}



