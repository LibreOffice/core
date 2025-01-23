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

#include <vcl/window.hxx>


namespace svt::table
{
    class TableControl_Impl;
    class TableFunctionSet;

    /** the window containing the content area (including headers) of
        a table control
    */
    class TableDataWindow : public vcl::Window
    {
    private:
        TableControl_Impl&  m_rTableControl;
        Link<LinkParamNone*,void> m_aSelectHdl;

    public:
        explicit TableDataWindow( TableControl_Impl& _rTableControl );
        virtual ~TableDataWindow() override;
        virtual void dispose() override;

        void SetSelectHdl(const Link<LinkParamNone*,void>& rLink)
        {
            m_aSelectHdl = rLink;
        }

        // Window overridables
        virtual void        Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        virtual void        MouseMove( const MouseEvent& rMEvt) override;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt) override;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt) override;
        virtual bool        EventNotify(NotifyEvent& rNEvt) override;
        virtual void        RequestHelp( const HelpEvent& rHEvt ) override;

    private:
        static void impl_hideTipWindow();
    };

} // namespace svt::table



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
