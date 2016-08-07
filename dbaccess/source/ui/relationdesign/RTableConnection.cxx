/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
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
    ::std::vector<OConnectionLine*>::const_iterator aEnd = pConnLineList->end();
    for(;aIter != aEnd;++aIter)
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
