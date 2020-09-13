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
#pragma once

#include "GeneralUndo.hxx"
#include <strings.hrc>

namespace dbaui
{
    class OQueryTextView;

    // OSqlEditUndoAct - Undo-class for changing sql text
    class OSqlEditUndoAct final : public OCommentUndoAction
    {
        OQueryTextView& m_rOwner;
        OUString    m_strNextText;

        virtual void    Undo() override { ToggleText(); }
        virtual void    Redo() override { ToggleText(); }

        void ToggleText();
    public:
        OSqlEditUndoAct(OQueryTextView& rEdit) : OCommentUndoAction(STR_QUERY_UNDO_MODIFYSQLEDIT), m_rOwner(rEdit) { }

        void SetOriginalText(const OUString& strText) { m_strNextText = strText; }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
