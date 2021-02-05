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

#include <vcl/fixed.hxx>

namespace dbaui
{
    class OTableWindow;
    class OTableWindowTitle : public FixedText
    {
        VclPtr<OTableWindow> m_pTabWin;

    protected:
        virtual void Command(const CommandEvent& rEvt) override;
        virtual void MouseButtonDown( const MouseEvent& rEvt ) override;
        virtual void KeyInput( const KeyEvent& rEvt ) override;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

    public:
        OTableWindowTitle( OTableWindow* pParent );
        virtual ~OTableWindowTitle() override;
        virtual void dispose() override;
        virtual void LoseFocus() override;
        virtual void GetFocus() override;
        virtual void RequestHelp( const HelpEvent& rHEvt ) override;
        // window override
        virtual void StateChanged( StateChangedType nStateChange ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
