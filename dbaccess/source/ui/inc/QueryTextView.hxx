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

#include <vcl/InterimItemWindow.hxx>
#include "querycontainerwindow.hxx"
#include "sqledit.hxx"

namespace dbaui
{
    class OQueryTextView final : public InterimItemWindow
    {
        friend class OQueryViewSwitch;

        OQueryController& m_rController;
        std::unique_ptr<SQLEditView> m_xSQL;
        std::unique_ptr<weld::CustomWeld> m_xSQLEd;

        Timer m_timerUndoActionCreation;
        OUString m_strOrigText;      // is restored on undo
        Timer m_timerInvalidate;
        bool m_bStopTimer;

        DECL_LINK(OnUndoActionTimer, Timer*, void);
        DECL_LINK(OnInvalidateTimer, Timer*, void);
        DECL_LINK(ModifyHdl, LinkParamNone*, void);

    public:
        OQueryTextView(OQueryContainerWindow* pParent, OQueryController& rController);
        virtual ~OQueryTextView() override;
        virtual void dispose() override;

        void SetSQLText(const OUString& rNewText);
        OUString GetSQLText() const;

        virtual void GetFocus() override;

        bool isCutAllowed() const;
        void copy();
        void cut();
        void paste();
        // clears the whole query
        void clear();
        // set the statement for representation
        void setStatement(const OUString& _rsStatement);
        OUString getStatement() const;

        void stopTimer();
        void startTimer();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
