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

#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>

#include <UndoCore.hxx>

SwUndoOutlineLeftRight::SwUndoOutlineLeftRight( const SwPaM& rPam,
                                                short nOff )
    : SwUndo( SwUndoId::OUTLINE_LR, rPam.GetDoc() ), SwUndRng( rPam ), m_nOffset( nOff )
{
}

void SwUndoOutlineLeftRight::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPaM( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().OutlineUpDown(rPaM, -m_nOffset);
}

void SwUndoOutlineLeftRight::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPaM( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().OutlineUpDown(rPaM,  m_nOffset);
}

void SwUndoOutlineLeftRight::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().OutlineUpDown(rContext.GetRepeatPaM(), m_nOffset);
}


SwUndoOutlineEdit::SwUndoOutlineEdit(const SwNumRule& rOldRule, const SwNumRule& rNewRule,
                                     const SwDoc& rDoc)
    : SwUndo(SwUndoId::OUTLINE_EDIT, rDoc)
    , m_aNewNumRule(rNewRule)
    , m_aOldNumRule(rOldRule)
{
}

void SwUndoOutlineEdit::UndoImpl(::sw::UndoRedoContext& rContext)
{
    rContext.GetDoc().SetOutlineNumRule(m_aOldNumRule);
}

void SwUndoOutlineEdit::RedoImpl(::sw::UndoRedoContext& rContext)
{
    rContext.GetDoc().SetOutlineNumRule(m_aNewNumRule);
}

void SwUndoOutlineEdit::RepeatImpl(::sw::RepeatContext& rContext)
{
    rContext.GetDoc().SetOutlineNumRule(m_aNewNumRule);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
