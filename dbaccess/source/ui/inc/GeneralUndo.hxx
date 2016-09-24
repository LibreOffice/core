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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_GENERALUNDO_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_GENERALUNDO_HXX

#include <svl/undo.hxx>
#include "moduledbu.hxx"

namespace dbaui
{
    // SbaCommentUndoAction - Undo-Basisklasse fuer Aktionen, deren GetComment einen aus einer Sba-Resource
    // geladenen String liefert

    class OCommentUndoAction : public SfxUndoAction
    {
        OModuleClient    m_aModuleClient;

    protected:
        OUString         m_strComment; // undo, redo comment

    public:
        OCommentUndoAction(sal_uInt16 nCommentID) { m_strComment = OUString(ModuleRes(nCommentID)); }

        virtual OUString GetComment() const override { return m_strComment; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_GENERALUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
