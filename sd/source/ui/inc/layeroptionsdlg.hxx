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

#ifndef INCLUDED_SD_SOURCE_UI_INC_LAYEROPTIONSDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_LAYEROPTIONSDLG_HXX

#include <sddllapi.h>

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/vclmedit.hxx>
#include "sdresid.hxx"
#include "strings.hrc"

class SfxItemSet;

class SD_DLLPUBLIC SdInsertLayerDlg : public ModalDialog
{
private:
    VclPtr<Edit>               m_pEdtName;
    VclPtr<Edit>               m_pEdtTitle;
    VclPtr<VclMultiLineEdit>   m_pEdtDesc;
    VclPtr<CheckBox>           m_pCbxVisible;
    VclPtr<CheckBox>           m_pCbxPrintable;
    VclPtr<CheckBox>           m_pCbxLocked;

    const SfxItemSet&   mrOutAttrs;

public:

    SdInsertLayerDlg( vcl::Window* pWindow, const SfxItemSet& rInAttrs,
        bool bDeletable, const OUString& rStr );
    virtual ~SdInsertLayerDlg();
    virtual void dispose() override;

    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // INCLUDED_SD_SOURCE_UI_INC_LAYEROPTIONSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
