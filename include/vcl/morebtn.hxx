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

#ifndef INCLUDED_VCL_MOREBTN_HXX
#define INCLUDED_VCL_MOREBTN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/button.hxx>

struct ImplMoreButtonData;


class VCL_DLLPUBLIC MoreButton : public PushButton
{
    ImplMoreButtonData* mpMBData;
    bool                mbState;

                        MoreButton( const MoreButton & ) = delete;
                        MoreButton& operator=( const MoreButton & ) = delete;
    SAL_DLLPRIVATE void ShowState();

    SAL_DLLPRIVATE void ImplInitMoreButton( vcl::Window* pParent, WinBits nStyle );

public:
    explicit            MoreButton( vcl::Window* pParent, WinBits nStyle );
    virtual             ~MoreButton() override;
    virtual void        dispose() override;

    void                Click() override;

    using PushButton::SetState;
};

#endif // INCLUDED_VCL_MOREBTN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
