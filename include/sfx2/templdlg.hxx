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
#ifndef INCLUDED_SFX2_TEMPLDLG_HXX
#define INCLUDED_SFX2_TEMPLDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/ctrl.hxx>
#include <rsc/rscsfx.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>

namespace SfxTemplate
{
    // converts from SFX_STYLE_FAMILY Ids to 1-5
    sal_uInt16 SFX2_DLLPUBLIC SfxFamilyIdToNId(SfxStyleFamily nFamily);
    // converts from 1-5 to SFX_STYLE_FAMILY Ids
    SfxStyleFamily SFX2_DLLPUBLIC NIdToSfxFamilyId(sal_uInt16 nId);
}

class SfxTemplateDialog_Impl;

class SFX2_DLLPUBLIC SfxTemplatePanelControl : public DockingWindow
{
public:
    SfxTemplatePanelControl (SfxBindings* pBindings, vcl::Window* pParentWindow);
    virtual ~SfxTemplatePanelControl();
    virtual void dispose() override;

    virtual void DataChanged( const DataChangedEvent& _rDCEvt ) override;
    virtual void Resize() override;
    virtual void StateChanged( StateChangedType nStateChange ) override;
    void FreeResource();

private:
    std::unique_ptr<SfxTemplateDialog_Impl> pImpl;
    SfxBindings* mpBindings;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
