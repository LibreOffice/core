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

#include <uielement/statusbarmanager.hxx>

#include <vcl/status.hxx>

namespace framework
{

class FrameworkStatusBar : public StatusBar
{
    public:

        FrameworkStatusBar( vcl::Window*           pParent,
                            WinBits           nWinBits );

        void         SetStatusBarManager( StatusBarManager* pStatusBarManager );

        virtual void StateChanged( StateChangedType nType ) override;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
        virtual void UserDraw(const UserDrawEvent& rUDEvt) override;
        virtual void Command( const CommandEvent &rEvt ) override;
        virtual void MouseMove( const MouseEvent& rMEvt ) override;
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

    private:
        StatusBarManager*       m_pMgr;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
