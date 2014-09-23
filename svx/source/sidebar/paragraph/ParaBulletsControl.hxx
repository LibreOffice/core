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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARABULLETSCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARABULLETSCONTROL_HXX

#include "svx/sidebar/PopupControl.hxx"
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "ParaPropertyPanel.hxx"
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <vcl/button.hxx>


namespace svx { namespace sidebar {

class ParaBulletsControl:public svx::sidebar::PopupControl
{
private:
    SvxNumValueSet3 maBulletsVS;
    PushButton      maMoreButton;
    ParaPropertyPanel&     mrParaPropertyPanel;
    SfxBindings*    mpBindings;

    DECL_LINK(BulletSelectHdl_Impl, ValueSet*);
    DECL_LINK(MoreButtonClickHdl_Impl, void*);

public:
    ParaBulletsControl(vcl::Window* pParent, svx::sidebar::ParaPropertyPanel& rPanel);
    virtual ~ParaBulletsControl();
    void UpdateValueSet();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
