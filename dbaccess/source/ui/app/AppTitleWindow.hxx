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

#include <ChildWindow.hxx>

namespace dbaui
{
    class OTitleWindow final
    {
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;
        std::unique_ptr<weld::Container> m_xTitleFrame;
        std::unique_ptr<weld::Label> m_xTitle;
        std::unique_ptr<weld::Container> m_xChildContainer;
        std::shared_ptr<OChildWindow> m_xChild;

    public:
        OTitleWindow(weld::Container* pParent, const char* pTitleId);
        ~OTitleWindow();

        void GrabFocus();

        bool HasChildPathFocus() const;

        /** gets the window which should be used as a child's parent */
        weld::Container* getChildContainer();

        /** sets the child window which should be displayed below the title. It will be destroyed at the end.
            @param  _pChild
                The child window.
        */
        void setChildWindow(const std::shared_ptr<OChildWindow>& rChild);

        /** gets the child window.

            @return
                The child window.
        */
        OChildWindow* getChildWindow() const { return m_xChild.get(); }

        /** sets the title text out of the resource
            @param  pTitleId
                The resource id of the title text.
        */
        void setTitle(const char* pTitleId);
    };
} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
