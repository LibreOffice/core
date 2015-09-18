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
#include <DocumentStateManager.hxx>
#include <doc.hxx>
#include <DocumentStatisticsManager.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentLayoutManager.hxx>
#include <docstat.hxx>
#include <acorrect.hxx>

namespace sw
{

DocumentStateManager::DocumentStateManager( SwDoc& i_rSwdoc ) :
    m_rDoc( i_rSwdoc ),
    mbModified(false),
    mbLoaded(false),
    mbUpdateExpField(false),
    mbNewDoc(false),
    mbInCallModified(false)
{
}

void DocumentStateManager::SetModified()
{
    m_rDoc.GetDocumentLayoutManager().ClearSwLayouterEntries();
    mbModified = true;
    m_rDoc.GetDocumentStatisticsManager().GetDocStat().bModified = true;
    if( m_rDoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rDoc.GetOle2Link().Call( true );
        mbInCallModified = false;
    }

    if( m_rDoc.GetAutoCorrExceptWord() && !m_rDoc.GetAutoCorrExceptWord()->IsDeleted() )
        m_rDoc.DeleteAutoCorrExceptWord();
}

void DocumentStateManager::ResetModified()
{
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    bool bOldModified = mbModified;
    mbModified = false;
    m_rDoc.GetDocumentStatisticsManager().GetDocStat().bModified = false;
    m_rDoc.GetIDocumentUndoRedo().SetUndoNoModifiedPosition();
    if( bOldModified && m_rDoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rDoc.GetOle2Link().Call( false );
        mbInCallModified = false;
    }
}

bool DocumentStateManager::IsModified() const
{
    return mbModified;
}

bool DocumentStateManager::IsInCallModified() const
{
    return mbInCallModified;
}

bool DocumentStateManager::IsUpdateExpField() const
{
    return mbUpdateExpField;
}

bool DocumentStateManager::IsNewDoc() const
{
    return mbNewDoc;
}

void DocumentStateManager::SetNewDoc(bool b)
{
    mbNewDoc = b;
}

void DocumentStateManager::SetUpdateExpFieldStat(bool b)
{
    mbUpdateExpField = b;
}

void DocumentStateManager::SetLoaded(bool b)
{
    mbLoaded = b;
}

}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
