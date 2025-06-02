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

#include <undoflystrattr.hxx>
#include <frmfmt.hxx>
#include <utility>


SwUndoFlyStrAttr::SwUndoFlyStrAttr( SwFlyFrameFormat& rFlyFrameFormat,
                                    const SwUndoId eUndoId,
                                    OUString sOldStr,
                                    OUString sNewStr )
    : SwUndo( eUndoId, *rFlyFrameFormat.GetDoc() ),
      mrFlyFrameFormat( rFlyFrameFormat ),
      msOldStr(std::move( sOldStr )),
      msNewStr(std::move( sNewStr ))
{
    assert(eUndoId == SwUndoId::FLYFRMFMT_TITLE
        || eUndoId == SwUndoId::FLYFRMFMT_DESCRIPTION);
}

SwUndoFlyStrAttr::~SwUndoFlyStrAttr()
{
}

void SwUndoFlyStrAttr::UndoImpl(::sw::UndoRedoContext &)
{
    switch ( GetId() )
    {
        case SwUndoId::FLYFRMFMT_TITLE:
        {
            mrFlyFrameFormat.SetObjTitle( msOldStr, true );
        }
        break;
        case SwUndoId::FLYFRMFMT_DESCRIPTION:
        {
            mrFlyFrameFormat.SetObjDescription( msOldStr, true );
        }
        break;
        default:
        {
        }
    }
}

void SwUndoFlyStrAttr::RedoImpl(::sw::UndoRedoContext &)
{
    switch ( GetId() )
    {
        case SwUndoId::FLYFRMFMT_TITLE:
        {
            mrFlyFrameFormat.SetObjTitle( msNewStr, true );
        }
        break;
        case SwUndoId::FLYFRMFMT_DESCRIPTION:
        {
            mrFlyFrameFormat.SetObjDescription( msNewStr, true );
        }
        break;
        default:
        {
        }
    }
}

SwRewriter SwUndoFlyStrAttr::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule( UndoArg1, mrFlyFrameFormat.GetName() );

    return aResult;
}

SwUndoFlyDecorative::SwUndoFlyDecorative(SwFlyFrameFormat& rFlyFrameFormat,
        bool const isDecorative)
    : SwUndo(SwUndoId::FLYFRMFMT_DECORATIVE, *rFlyFrameFormat.GetDoc())
    , m_rFlyFrameFormat(rFlyFrameFormat)
    , m_IsDecorative(isDecorative)
{
}

SwUndoFlyDecorative::~SwUndoFlyDecorative()
{
}

void SwUndoFlyDecorative::UndoImpl(::sw::UndoRedoContext &)
{
    m_rFlyFrameFormat.SetObjDecorative(!m_IsDecorative);
}

void SwUndoFlyDecorative::RedoImpl(::sw::UndoRedoContext &)
{
    m_rFlyFrameFormat.SetObjDecorative(m_IsDecorative);
}

SwRewriter SwUndoFlyDecorative::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, m_rFlyFrameFormat.GetName());

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
