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
#include <RTableConnectionData.hxx>
#include <RelationTableView.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <ConnectionLine.hxx>

using namespace dbaui;
ORelationTableConnection::ORelationTableConnection( ORelationTableView* pContainer,
                                                   const TTableConnectionData::value_type& pTabConnData )
    :OTableConnection( pContainer, pTabConnData )
{
}

ORelationTableConnection::ORelationTableConnection( const ORelationTableConnection& rConn )
    : VclReferenceBase(), OTableConnection( rConn )
{
    // no own members, thus the base class functionality is enough
}

ORelationTableConnection& ORelationTableConnection::operator=( const ORelationTableConnection& rConn )
{
    // this doesn't change anything, since the base class tests this, too and I don't have my own members to copy
    if (&rConn == this)
        return *this;

    OTableConnection::operator=( rConn );
    return *this;
}

void ORelationTableConnection::Draw(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    OTableConnection::Draw(rRenderContext, rRect);
    ORelationTableConnectionData* pData = static_cast< ORelationTableConnectionData* >(GetData().get());
    if (!pData || pData->GetCardinality() == Cardinality::Undefined)
        return;

    // search lines for top line
    tools::Rectangle aBoundingRect;
    tools::Long nTop = GetBoundingRect().Bottom();
    tools::Long nTemp;

    const OConnectionLine* pTopLine = nullptr;
    const std::vector<std::unique_ptr<OConnectionLine>>& rConnLineList = GetConnLineList();

    for (auto const& elem : rConnLineList)
    {
        if( elem->IsValid() )
        {
            aBoundingRect = elem->GetBoundingRect();
            nTemp = aBoundingRect.Top();
            if(nTemp < nTop)
            {
                nTop = nTemp;
                pTopLine = elem.get();
            }
        }
    }

    // cardinality
    if (!pTopLine)
        return;

    tools::Rectangle aSourcePos = pTopLine->GetSourceTextPos();
    tools::Rectangle aDestPos = pTopLine->GetDestTextPos();

    OUString aSourceText;
    OUString aDestText;

    switch (pData->GetCardinality())
    {
    case Cardinality::OneMany:
        aSourceText = "1";
        aDestText   = "n";
        break;

    case Cardinality::ManyOne:
        aSourceText = "n";
        aDestText   = "1";
        break;

    case Cardinality::OneOne:
        aSourceText = "1";
        aDestText   = "1";
        break;
    default: break;
    }

    if (IsSelected())
        rRenderContext.SetTextColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    else
        rRenderContext.SetTextColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());

    rRenderContext.DrawText(aSourcePos, aSourceText, DrawTextFlags::Clip | DrawTextFlags::Center | DrawTextFlags::Bottom);
    rRenderContext.DrawText(aDestPos, aDestText, DrawTextFlags::Clip | DrawTextFlags::Center | DrawTextFlags::Bottom);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
