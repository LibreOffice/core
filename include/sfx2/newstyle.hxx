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
#ifndef INCLUDED_SFX2_NEWSTYLE_HXX
#define INCLUDED_SFX2_NEWSTYLE_HXX

#include <comphelper/string.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/combobox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class SfxStyleSheetBasePool;

class SFX2_DLLPUBLIC SfxNewStyleDlg : public ModalDialog
{
private:
    VclPtr<ComboBox> m_pColBox;
    VclPtr<OKButton> m_pOKBtn;

    VclPtr<MessageDialog> aQueryOverwriteBox;
    SfxStyleSheetBasePool&  rPool;

    DECL_DLLPRIVATE_LINK_TYPED( OKHdl, ComboBox&, void );
    DECL_DLLPRIVATE_LINK_TYPED( OKClickHdl, Button *, void );
    DECL_DLLPRIVATE_LINK( ModifyHdl, ComboBox * );

public:
    SfxNewStyleDlg( vcl::Window* pParent, SfxStyleSheetBasePool& );
    virtual ~SfxNewStyleDlg();
    virtual void dispose() override;

    OUString                GetName() const { return comphelper::string::stripStart(m_pColBox->GetText(), ' '); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
