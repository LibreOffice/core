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

#include "RTableConnection.hxx"
#include <tools/debug.hxx>
#include "RelationTableView.hxx"
#include <vcl/svapp.hxx>
#include "ConnectionLine.hxx"

using namespace dbaui;
// class ORelationTableConnection
DBG_NAME(ORelationTableConnection)
ORelationTableConnection::ORelationTableConnection( ORelationTableView* pContainer,
                                                   const TTableConnectionData::value_type& pTabConnData )
    :OTableConnection( pContainer, pTabConnData )
{
    DBG_CTOR(ORelationTableConnection,NULL);
}

ORelationTableConnection::ORelationTableConnection( const ORelationTableConnection& rConn )
    : OTableConnection( rConn )
{
    DBG_CTOR(ORelationTableConnection,NULL);
    // keine eigenen Members, also reicht die Basisklassenfunktionalitaet
}

ORelationTableConnection::~ORelationTableConnection()
{
    DBG_DTOR(ORelationTableConnection,NULL);
}

ORelationTableConnection& ORelationTableConnection::operator=( const ORelationTableConnection& rConn )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    // nicht dass es was aendern wuerde, da die Basisklasse das auch testet und ich keine eigenen Members zu kopieren habe
    if (&rConn == this)
        return *this;

    OTableConnection::operator=( rConn );
    return *this;
}

void ORelationTableConnection::Draw( const Rectangle& rRect )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    OTableConnection::Draw( rRect );
    ORelationTableConnectionData* pData = static_cast< ORelationTableConnectionData* >(GetData().get());
    if ( pData && (pData->GetCardinality() == CARDINAL_UNDEFINED) )
        return;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
