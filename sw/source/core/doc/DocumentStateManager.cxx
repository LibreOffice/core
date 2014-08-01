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
#include <docstat.hxx>
#include <acorrect.hxx>

namespace sw
{

DocumentStateManager::DocumentStateManager( SwDoc& i_rSwdoc ) :
    m_rSwdoc( i_rSwdoc ),
    mbModified(false),
    mbLoaded(false),
    mbUpdateExpFld(false),
    mbNewDoc(false),
    mbPageNums(false),
    mbInCallModified(false)
{
}

void DocumentStateManager::SetModified()
{
    m_rSwdoc.ClearSwLayouterEntries();
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    sal_IntPtr nCall = mbModified ? 3 : 2;
    mbModified = true;
    m_rSwdoc.GetDocumentStatisticsManager().GetDocStat().bModified = true;
    if( m_rSwdoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rSwdoc.GetOle2Link().Call( (void*)nCall );
        mbInCallModified = false;
    }

    if( m_rSwdoc.GetAutoCorrExceptWord() && !m_rSwdoc.GetAutoCorrExceptWord()->IsDeleted() )
        m_rSwdoc.DeleteAutoCorrExceptWord();
}

void DocumentStateManager::ResetModified()
{
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    sal_IntPtr nCall = mbModified ? 1 : 0;
    mbModified = false;
    m_rSwdoc.GetIDocumentUndoRedo().SetUndoNoModifiedPosition();
    if( nCall && m_rSwdoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rSwdoc.GetOle2Link().Call( (void*)nCall );
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

bool DocumentStateManager::IsLoaded() const
{
    return mbLoaded;
}

bool DocumentStateManager::IsUpdateExpFld() const
{
    return mbUpdateExpFld;
}

bool DocumentStateManager::IsNewDoc() const
{
    return mbNewDoc;
}

bool DocumentStateManager::IsPageNums() const
{
  return mbPageNums;
}

void DocumentStateManager::SetPageNums(bool b)
{
    mbPageNums = b;
}

void DocumentStateManager::SetNewDoc(bool b)
{
    mbNewDoc = b;
}

void DocumentStateManager::SetUpdateExpFldStat(bool b)
{
    mbUpdateExpFld = b;
}

void DocumentStateManager::SetLoaded(bool b)
{
    mbLoaded = b;
}

}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
