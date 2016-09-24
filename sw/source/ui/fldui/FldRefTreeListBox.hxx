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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDREFTREELISTBOX_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDREFTREELISTBOX_HXX

#include <svtools/treelistbox.hxx>

class SwFieldRefTreeListBox : public SvTreeListBox
{
protected:
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

public:
    SwFieldRefTreeListBox(vcl::Window* pParent, WinBits nStyle);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
