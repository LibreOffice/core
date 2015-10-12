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

#ifndef INCLUDED_CUI_SOURCE_CUSTOMIZE_MACROPG_IMPL_HXX
#define INCLUDED_CUI_SOURCE_CUSTOMIZE_MACROPG_IMPL_HXX

class _SvxMacroTabPage_Impl
{
public:
    explicit _SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet );

    VclPtr<PushButton>                     pAssignPB;
    VclPtr<PushButton>                     pAssignComponentPB;
    VclPtr<PushButton>                     pDeletePB;
    Image                           aMacroImg;
    Image                           aComponentImg;
    OUString                        sStrEvent;
    OUString                        sAssignedMacro;
    VclPtr<MacroEventListBox>              pEventLB;
    bool                        bReadOnly;
    bool                        bIDEDialogMode;
};

class AssignComponentDialog : public ModalDialog
{
private:
    VclPtr<Edit>           mpMethodEdit;
    VclPtr<OKButton>       mpOKButton;

    OUString maURL;

    DECL_LINK_TYPED(ButtonHandler, Button*, void);

public:
    AssignComponentDialog( vcl::Window * pParent, const OUString& rURL );
    virtual ~AssignComponentDialog();
    virtual void dispose() override;

    OUString getURL() const
        { return maURL; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
