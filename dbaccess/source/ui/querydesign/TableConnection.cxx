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
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef DBACCESS_CONNECTIONLINEACCESS_HXX
#include "ConnectionLineAccess.hxx"
#endif
#include <algorithm>


using namespace dbaui;
using namespace comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

//========================================================================
// class OTableConnection
//========================================================================
namespace dbaui
{
    DBG_NAME(OTableConnection)
    //------------------------------------------------------------------------
    OTableConnection::OTableConnection( OJoinTableView* _pContainer,const TTableConnectionData::value_type& _pTabConnData )
        :Window(_pContainer)
        ,m_pData( _pTabConnData )
        ,m_pParent( _pContainer )
        ,m_bSelected( sal_False )
    {
        DBG_CTOR(OTableConnection,NULL);
        Init();
        Show();
    }

    //------------------------------------------------------------------------
    OTableConnection::OTableConnection( const OTableConnection& _rConn ) : Window(_rConn.m_pParent)
        ,m_pData(_rConn.GetData()->NewInstance())
    {
        DBG_CTOR(OTableConnection,NULL);
        *this = _rConn;
    }

    //------------------------------------------------------------------------
    void OTableConnection::Init()
    {
        //////////////////////////////////////////////////////////////////////
        // Linienliste mit Defaults initialisieren
        OConnectionLineDataVec* pLineData = GetData()->GetConnLineDataList();
        OConnectionLineDataVec::const_iterator aIter = pLineData->begin();
        OConnectionLineDataVec::const_iterator aEnd = pLineData->end();
        m_vConnLine.reserve(pLineData->size());
        for(;aIter != aEnd;++aIter)
            m_vConnLine.push_back( new OConnectionLine(this, *aIter) );
    }

    //------------------------------------------------------------------------
    OConnectionLine* OTableConnection::CreateConnLine( const OConnectionLine& rConnLine )
    {
        return new OConnectionLine( rConnLine );
    }
    // -----------------------------------------------------------------------------
    void OTableConnection::clearLineData()
    {
        ::std::vector<OConnectionLine*>::iterator aLineEnd = m_vConnLine.end();
        for(::std::vector<OConnectionLine*>::iterator aLineIter = m_vConnLine.begin();aLineIter != aLineEnd;++aLineIter)
            delete *aLineIter;
        m_vConnLine.clear();
    }
    //------------------------------------------------------------------------
    void OTableConnection::UpdateLineList()
    {
        //////////////////////////////////////////////////////////////////////
        // Linienliste loeschen
        clearLineData();

        Init();
    }

    //------------------------------------------------------------------------
    OTableConnection& OTableConnection::operator=( const OTableConnection& rConn )
    {
        if( &rConn == this )
            return *this;

        // Linienliste loeschen
        clearLineData();

        // Linienliste kopieren
        if(! rConn.GetConnLineList()->empty() )
        {
            const ::std::vector<OConnectionLine*>* pLine = rConn.GetConnLineList();
            ::std::vector<OConnectionLine*>::const_iterator aIter = pLine->begin();
            ::std::vector<OConnectionLine*>::const_iterator aEnd = pLine->end();
            m_vConnLine.reserve(pLine->size());
            for(;aIter != aEnd;++aIter)
                m_vConnLine.push_back( CreateConnLine( **aIter ));
        }

        // da mir die Daten nicht gehoeren, loesche ich die alten nicht
        m_pData->CopyFrom(*rConn.GetData());
            // CopyFrom ist virtuell, damit ist es kein Problem, wenn m_pData von einem von OTableConnectionData abgeleiteten Typ ist

        m_bSelected = rConn.m_bSelected;
        m_pParent = rConn.m_pParent;

        return *this;
    }


    //------------------------------------------------------------------------
    bool OTableConnection::RecalcLines()
    {
        // call RecalcLines on each line
        ::std::for_each(m_vConnLine.begin(),m_vConnLine.end(),::std::mem_fun(&OConnectionLine::RecalcLine));
        return true;
    }
    //------------------------------------------------------------------------
    OTableWindow* OTableConnection::GetSourceWin() const
    {
        TTableWindowData::value_type pRef = GetData()->getReferencingTable();
        OTableWindow* pRet = m_pParent->GetTabWindow( pRef->GetWinName() );
        if ( !pRet )
        {
            pRet = m_pParent->GetTabWindow( pRef->GetComposedName() );
        }
        return pRet;
    }
    //------------------------------------------------------------------------
    OTableWindow* OTableConnection::GetDestWin() const
    {
        TTableWindowData::value_type pRef = GetData()->getReferencedTable();
        OTableWindow* pRet = m_pParent->GetTabWindow( pRef->GetWinName() );
        if ( !pRet )
        {
            pRet = m_pParent->GetTabWindow( pRef->GetComposedName() );
        }
        return pRet;
    }

    //------------------------------------------------------------------------
    void OTableConnection::Select()
    {
        m_bSelected = sal_True;
        m_pParent->Invalidate( GetBoundingRect(), INVALIDATE_NOCHILDREN);
    }

    //------------------------------------------------------------------------
    void OTableConnection::Deselect()
    {
        m_bSelected = sal_False;
        InvalidateConnection();
    }

    //------------------------------------------------------------------------
    sal_Bool OTableConnection::CheckHit( const Point& rMousePos ) const
    {
        //////////////////////////////////////////////////////////////////////
        // check if the point hit our line
        ::std::vector<OConnectionLine*>::const_iterator aIter = ::std::find_if(m_vConnLine.begin(),
                                                                         m_vConnLine.end(),
                                                                         ::std::bind2nd(TConnectionLineCheckHitFunctor(),rMousePos));
        return aIter != m_vConnLine.end();
    }

    //------------------------------------------------------------------------
    bool OTableConnection::InvalidateConnection()
    {
        Rectangle rcBounding = GetBoundingRect();
        rcBounding.Bottom() += 1;
        rcBounding.Right() += 1;
            // ich glaube, dass sich Invalidate und Draw(Rectangle) nicht konsistent verhalten : jedenfalls waere dadurch zu
            // erklaeren, warum ohne diesen Fake hier beim Loeschen einer Connection ein Strich an ihrem unteren Ende stehen bleibt :
            // Invalidate erfasst dabei offensichtlich eine Pixelzeile weniger als Draw.
            // Oder alles haengt ganz anders zusammen ... jedenfalls klappt es so ...
        m_pParent->Invalidate( rcBounding, INVALIDATE_NOCHILDREN );

        return true;
    }

    //------------------------------------------------------------------------
    Rectangle OTableConnection::GetBoundingRect() const
    {
        //////////////////////////////////////////////////////////////////////
        // Aus allen Linien das umgebende Rechteck bestimmen
        Rectangle aBoundingRect( Point(0,0), Point(0,0) );
        Rectangle aTempRect;
        ::std::vector<OConnectionLine*>::const_iterator aEnd = m_vConnLine.end();
        for(::std::vector<OConnectionLine*>::const_iterator aIter = m_vConnLine.begin();aIter != aEnd;++aIter)
        {
            aTempRect = (*aIter)->GetBoundingRect();

            //////////////////////////////////////////////////////////////////////
            // Ist das BoundingRect dieser Linie gueltig?
            if( (aTempRect.GetWidth()!=1) && (aTempRect.GetHeight()!=1) )
            {
                if( (aBoundingRect.GetWidth()==1) && (aBoundingRect.GetHeight()==1) )
                    aBoundingRect = aTempRect;
                else
                    aBoundingRect.Union( aTempRect );
            }
        }

        return aBoundingRect;
    }

    //------------------------------------------------------------------------
    void OTableConnection::Draw( const Rectangle& /*rRect*/ )
    {
        //////////////////////////////////////////////////////////////////////
        // Linien zeichnen
        ::std::for_each(m_vConnLine.begin(),m_vConnLine.end(),TConnectionLineDrawFunctor(m_pParent));
    }
    // -----------------------------------------------------------------------------
}



