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

#include "TableConnection.hxx"
#include "ConnectionLine.hxx"
#include "TableConnectionData.hxx"
#include "JoinTableView.hxx"
#include <comphelper/stl_types.hxx>
#include "ConnectionLineAccess.hxx"

using namespace dbaui;
using namespace comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

// class OTableConnection
namespace dbaui
{
    OTableConnection::OTableConnection( OJoinTableView* _pContainer,const TTableConnectionData::value_type& _pTabConnData )
        :Window(_pContainer)
        ,m_pData( _pTabConnData )
        ,m_pParent( _pContainer )
        ,m_bSelected( false )
    {
        Init();
        Show();
    }

    OTableConnection::OTableConnection( const OTableConnection& _rConn ) : Window(_rConn.m_pParent.get())
        ,m_pData(_rConn.GetData()->NewInstance())
        ,m_pParent(nullptr)
    {
        *this = _rConn;
    }

    void OTableConnection::Init()
    {
        // initialise linelist with defaults
        OConnectionLineDataVec& rLineData = GetData()->GetConnLineDataList();
        OConnectionLineDataVec::const_iterator aIter = rLineData.begin();
        OConnectionLineDataVec::const_iterator aEnd = rLineData.end();
        m_vConnLine.reserve(rLineData.size());
        for(;aIter != aEnd;++aIter)
            m_vConnLine.push_back( new OConnectionLine(this, *aIter) );
    }

    OConnectionLine* OTableConnection::CreateConnLine( const OConnectionLine& rConnLine )
    {
        return new OConnectionLine( rConnLine );
    }
    void OTableConnection::clearLineData()
    {
        ::std::vector<OConnectionLine*>::const_iterator aLineEnd = m_vConnLine.end();
        for(::std::vector<OConnectionLine*>::const_iterator aLineIter = m_vConnLine.begin();aLineIter != aLineEnd;++aLineIter)
            delete *aLineIter;
        m_vConnLine.clear();
    }
    void OTableConnection::UpdateLineList()
    {
        // delete linelist
        clearLineData();

        Init();
    }

    OTableConnection& OTableConnection::operator=( const OTableConnection& rConn )
    {
        if( &rConn == this )
            return *this;

        // delete linelist
        clearLineData();

        // copy linelist
        if(! rConn.GetConnLineList().empty() )
        {
            const ::std::vector<OConnectionLine*>& rLine = rConn.GetConnLineList();
            ::std::vector<OConnectionLine*>::const_iterator aIter = rLine.begin();
            ::std::vector<OConnectionLine*>::const_iterator aEnd = rLine.end();
            m_vConnLine.reserve(rLine.size());
            for(;aIter != aEnd;++aIter)
                m_vConnLine.push_back( CreateConnLine( **aIter ));
        }

        // as the data are not mine, I also do not delete the old
        m_pData->CopyFrom(*rConn.GetData());
        // CopyFrom is virtual, therefore it is not a problem if m_pData is a derived type of OTableConnectionData

        m_bSelected = rConn.m_bSelected;
        m_pParent = rConn.m_pParent;

        return *this;
    }

    void OTableConnection::RecalcLines()
    {
        // call RecalcLines on each line
        for( const auto& pLine : m_vConnLine )
            pLine->RecalcLine();
    }
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

    void OTableConnection::Select()
    {
        m_bSelected = true;
        m_pParent->Invalidate( GetBoundingRect(), InvalidateFlags::NoChildren);
    }

    void OTableConnection::Deselect()
    {
        m_bSelected = false;
        InvalidateConnection();
    }

    bool OTableConnection::CheckHit( const Point& rMousePos ) const
    {
        // check if the point hit our line
        return ::std::any_of(m_vConnLine.begin(),
                             m_vConnLine.end(),
                             [&rMousePos]
                             ( const OConnectionLine* pLine )
                             { return pLine->CheckHit( rMousePos ); } );
    }

    void OTableConnection::InvalidateConnection()
    {
        Rectangle rcBounding = GetBoundingRect();
        rcBounding.Bottom() += 1;
        rcBounding.Right() += 1;
        // I believe Invalidate and Draw(Rectangle) do not behave consistent: in any case it
        // could explain, why without the fake here when deleting a connection a dash remains at the lower end:
        // Invalidate records obviously one pixel line less as Draw.
        // Or everything works differently .....  in any case it works ....
        m_pParent->Invalidate( rcBounding, InvalidateFlags::NoChildren );
    }

    Rectangle OTableConnection::GetBoundingRect() const
    {
        // determine all lines of the surrounding rectangle
        Rectangle aBoundingRect( Point(0,0), Point(0,0) );
        Rectangle aTempRect;
        ::std::vector<OConnectionLine*>::const_iterator aEnd = m_vConnLine.end();
        for(::std::vector<OConnectionLine*>::const_iterator aIter = m_vConnLine.begin();aIter != aEnd;++aIter)
        {
            aTempRect = (*aIter)->GetBoundingRect();

            // is the BoundingRect of this line valid?
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

    void OTableConnection::Draw(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
    {
        // Draw line
        for( const auto& pLine : m_vConnLine )
             pLine->Draw( &rRenderContext );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
