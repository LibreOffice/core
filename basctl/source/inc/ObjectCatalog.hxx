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

#include "bastype2.hxx"
#include "bastypes.hxx"

#include <vcl/weld.hxx>

namespace basctl
{
/*!
 *  @brief A docking window that contains a tree of the currently loaded macros
 *
 *  The class creates Object Catalog window with the currently loaded macros
 *  in a tree structure which allows user to quickly select the necessary
 *  macro in BasicIDE.
 */
class ObjectCatalog : public DockingWindow
{
public:
    explicit ObjectCatalog(vcl::Window* pParent);
    virtual ~ObjectCatalog() override;
    virtual void dispose() override;

    ///  Update the entries of Object Catalog Treelist
    void UpdateEntries() { m_xTree->UpdateEntries(); }
    void SetCurrentEntry(BaseWindow* pCurWin);

private:
    std::unique_ptr<weld::Label> m_xTitle; ///< Title of the Object Catalog window
    std::unique_ptr<SbTreeListBox> m_xTree; ///< The Treelist of the objects in window

    /*!
     *  Function for resize by DockingWindow.
     *  It is called by DockingWindow when IsFloatingMode() changes.
     */
    virtual void ToggleFloatingMode() override;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
