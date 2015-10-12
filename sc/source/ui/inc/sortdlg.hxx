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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SORTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SORTDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include "scui_def.hxx"

class ScSortDlg : public SfxTabDialog
{
public:
    ScSortDlg(vcl::Window* pParent, const SfxItemSet* pArgSet);

    void SetHeaders( bool bHeaders )  { bIsHeaders = bHeaders; }
    void SetByRows ( bool bByRows  )  { bIsByRows = bByRows; }
    bool GetHeaders() const           { return bIsHeaders; }
    bool GetByRows () const           { return bIsByRows; }

private:
    bool    bIsHeaders;
    bool    bIsByRows;
};

class ScSortWarningDlg : public ModalDialog
{
public:
    ScSortWarningDlg( vcl::Window* pParent, const OUString& rExtendText,const OUString& rCurrentText );
    virtual ~ScSortWarningDlg();
    virtual void dispose() override;
    DECL_LINK_TYPED( BtnHdl, Button*, void );
private:
    VclPtr<FixedText>       aFtText;
    VclPtr<PushButton>      aBtnExtSort;
    VclPtr<PushButton>      aBtnCurSort;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SORTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
