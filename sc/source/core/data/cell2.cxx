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

#include "cell.hxx"

#include "document.hxx"
#include "editutil.hxx"

#include "editeng/editobj.hxx"
#include "editeng/editstat.hxx"
#include "editeng/fieldupdater.hxx"

// STATIC DATA -----------------------------------------------------------

#ifdef USE_MEMPOOL
IMPL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

// ============================================================================

ScEditCell::ScEditCell(EditTextObject* pObject, ScDocument* pDoc) :
    ScBaseCell(CELLTYPE_EDIT),
    mpData(pObject), mpString(NULL), mpDoc(pDoc) {}

ScEditCell::ScEditCell(
    const EditTextObject& rObject, ScDocument* pDoc, const SfxItemPool* pFromPool) :
    ScBaseCell(CELLTYPE_EDIT),
    mpString(NULL),
    mpDoc(pDoc)
{
    SetTextObject(&rObject, pFromPool);
}

ScEditCell::ScEditCell(const ScEditCell& rCell, ScDocument& rDoc, const ScAddress& rDestPos) :
    ScBaseCell(rCell), mpString(NULL), mpDoc(&rDoc)
{
    SetTextObject( rCell.mpData, rCell.mpDoc->GetEditPool() );
    UpdateFields(rDestPos.Tab());
}

ScEditCell::ScEditCell(const OUString& rString, ScDocument* pDoc)  :
    ScBaseCell(CELLTYPE_EDIT),
    mpData(NULL),
    mpString(NULL),
    mpDoc(pDoc)
{
    OSL_ENSURE( rString.indexOf('\n') != -1 ||
                rString.indexOf(CHAR_CR) != -1,
                "EditCell mit einfachem Text !?!?" );

    EditEngine& rEngine = mpDoc->GetEditEngine();
    rEngine.SetText( rString );
    mpData = rEngine.CreateTextObject();
}

ScEditCell::~ScEditCell()
{
    delete mpData;
    delete mpString;

#if OSL_DEBUG_LEVEL > 0
    eCellType = CELLTYPE_DESTROYED;
#endif
}

void ScEditCell::ClearData()
{
    delete mpString;
    mpString = NULL;
    delete mpData;
    mpData = NULL;
}

void ScEditCell::SetData(const EditTextObject& rObject, const SfxItemPool* pFromPool)
{
    ClearData();
    SetTextObject(&rObject, pFromPool);
}

void ScEditCell::SetData(EditTextObject* pObject)
{
    ClearData();
    mpData = pObject;
}

OUString ScEditCell::GetString() const
{
    if (mpString)
        return *mpString;

    if (mpData)
    {
        // Also Text from URL fields, Doc-Engine is a ScFieldEditEngine
        EditEngine& rEngine = mpDoc->GetEditEngine();
        rEngine.SetText(*mpData);
        OUString sRet = ScEditUtil::GetMultilineString(rEngine); // string with line separators between paragraphs
        // cache short strings for formulas
        if ( sRet.getLength() < 256 )
            mpString = new OUString(sRet);   //! non-const
        return sRet;
    }

    return OUString();
}

const EditTextObject* ScEditCell::GetData() const
{
    return mpData;
}

void ScEditCell::RemoveCharAttribs( const ScPatternAttr& rAttr )
{
    ScEditUtil::RemoveCharAttribs(*mpData, rAttr);
}

void ScEditCell::UpdateFields(SCTAB nTab)
{
    editeng::FieldUpdater aUpdater = mpData->GetFieldUpdater();
    aUpdater.updateTableFields(nTab);
}

void ScEditCell::SetTextObject(
    const EditTextObject* pObject, const SfxItemPool* pFromPool )
{
    if (!pObject)
    {
        mpData = NULL;
        return;
    }

    if ( pFromPool && mpDoc->GetEditPool() == pFromPool )
    {
        mpData = pObject->Clone();
        return;
    }

    mpData = ScEditUtil::Clone(*pObject, *mpDoc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
