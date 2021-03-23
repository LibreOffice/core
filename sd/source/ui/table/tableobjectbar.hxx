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

#include <sfx2/shell.hxx>
#include <glob.hxx>

namespace sd {

class View;
class ViewShell;

}

namespace sd::ui::table {

class TableObjectBar final : public SfxShell
{
public:
    SFX_DECL_INTERFACE( SD_IF_SDDRAWTABLEOBJECTBAR )

    TableObjectBar( ::sd::ViewShell* pSdViewShell, ::sd::View* pSdView);
    virtual ~TableObjectBar() override;

    void            GetState( SfxItemSet& rSet );
    void            GetAttrState( SfxItemSet& rSet );
    void            Execute( SfxRequest& rReq );

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

    ::sd::View* mpView;
    ::sd::ViewShell* mpViewSh;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
