/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RTableConnection.cxx,v $
 * $Revision: 1.10.68.1 $
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
#ifndef DBAUI_RTABLECONNECTION_HXX
#include "RTableConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif

using namespace dbaui;
//========================================================================
// class ORelationTableConnection
//========================================================================
DBG_NAME(ORelationTableConnection)
//------------------------------------------------------------------------
ORelationTableConnection::ORelationTableConnection( ORelationTableView* pContainer,
                                                   const TTableConnectionData::value_type& pTabConnData )
    :OTableConnection( pContainer, pTabConnData )
{
    DBG_CTOR(ORelationTableConnection,NULL);
}

//------------------------------------------------------------------------
ORelationTableConnection::ORelationTableConnection( const ORelationTableConnection& rConn )
    : OTableConnection( rConn )
{
    DBG_CTOR(ORelationTableConnection,NULL);
    // keine eigenen Members, also reicht die Basisklassenfunktionalitaet
}

//------------------------------------------------------------------------
ORelationTableConnection::~ORelationTableConnection()
{
    DBG_DTOR(ORelationTableConnection,NULL);
}

//------------------------------------------------------------------------
ORelationTableConnection& ORelationTableConnection::operator=( const ORelationTableConnection& rConn )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    // nicht dass es was aendern wuerde, da die Basisklasse das auch testet und ich keine eigenen Members zu kopieren habe
    if (&rConn == this)
        return *this;

    OTableConnection::operator=( rConn );
    return *this;
}


//------------------------------------------------------------------------
void ORelationTableConnection::Draw( const Rectangle& rRect )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    OTableConnection::Draw( rRect );
    ORelationTableConnectionData* pData = static_cast< ORelationTableConnectionData* >(GetData().get());
    if ( pData && (pData->GetCardinality() == CARDINAL_UNDEFINED) )
        return;

    //////////////////////////////////////////////////////////////////////
    // Linien nach oberster Linie durchsuchen
    Rectangle aBoundingRect;
    long nTop = GetBoundingRect().Bottom();
    long nTemp;

    const OConnectionLine* pTopLine = NULL;
    const ::std::vector<OConnectionLine*>* pConnLineList = GetConnLineList();
    ::std::vector<OConnectionLine*>::const_iterator aIter = pConnLineList->begin();
    for(;aIter != pConnLineList->end();++aIter)
    {
        if( (*aIter)->IsValid() )
        {
            aBoundingRect = (*aIter)->GetBoundingRect();
            nTemp = aBoundingRect.Top();
            if( nTemp<nTop )
            {
                nTop = nTemp;
                pTopLine = (*aIter);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Kardinalitaet antragen
    if( !pTopLine )
        return;

    Rectangle aSourcePos = pTopLine->GetSourceTextPos();
    Rectangle aDestPos = pTopLine->GetDestTextPos();

    String aSourceText;
    String aDestText;

    switch( pData->GetCardinality() )
    {
    case CARDINAL_ONE_MANY:
        aSourceText  ='1';
        aDestText  ='n';
        break;

    case CARDINAL_MANY_ONE:
        aSourceText  ='n';
        aDestText  ='1';
        break;

    case CARDINAL_ONE_ONE:
        aSourceText  ='1';
        aDestText  ='1';
        break;
    }

    if (IsSelected())
        GetParent()->SetTextColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    else
        GetParent()->SetTextColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());


    GetParent()->DrawText( aSourcePos, aSourceText, TEXT_DRAW_CLIP | TEXT_DRAW_CENTER | TEXT_DRAW_BOTTOM);
    GetParent()->DrawText( aDestPos, aDestText, TEXT_DRAW_CLIP | TEXT_DRAW_CENTER | TEXT_DRAW_BOTTOM);
}
// -----------------------------------------------------------------------------
