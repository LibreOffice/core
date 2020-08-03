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

#include <vcl/weld.hxx>

#include <memory>

namespace sd {

#define SMALL_ICON_SIZE     16
#define TOP_OFFSET           5
#define ICON_HEIGHT         42
#define ICON_WIDTH          47
#define ICON_OFFSET         72
#define RIGHT_ICON_OFFSET    5
#define SPACE_BETWEEN        3

class ClientBox;
struct ClientBoxEntry;
struct ClientInfo;

typedef std::shared_ptr<ClientBoxEntry> TClientBoxEntry;

struct ClientBoxEntry
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xDeviceName;
    std::unique_ptr<weld::Label> m_xPinLabel;
    std::unique_ptr<weld::Entry> m_xPinBox;
    std::unique_ptr<weld::Button> m_xDeauthoriseButton;

    std::shared_ptr<ClientInfo> m_xClientInfo;
    ClientBox* m_pClientBox;

    DECL_LINK(DeauthoriseHdl, weld::Button&, void);
    DECL_LINK(FocusHdl, weld::Widget&, void);

    ClientBoxEntry(ClientBox* pClientBox, const std::shared_ptr<ClientInfo>& pClientInfo);
    ~ClientBoxEntry();
};

class ClientBox
{
    std::unique_ptr<weld::ScrolledWindow> m_xScroll;
    std::unique_ptr<weld::Container> m_xContents;

    std::vector< TClientBoxEntry > m_vEntries;
    ClientBoxEntry* m_pActive;

public:
    ClientBox(std::unique_ptr<weld::ScrolledWindow> xScroll, std::unique_ptr<weld::Container> xContents);
    weld::Container* GetContainer() { return m_xContents.get(); }
    ~ClientBox();

    ClientBoxEntry* GetActiveEntry();

    void addEntry(const std::shared_ptr<ClientInfo>& pClientInfo);
    void setActive(ClientBoxEntry* pClientData);
    void clearEntries();

    void populateEntries();
};

} // end namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
