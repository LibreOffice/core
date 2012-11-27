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


#ifndef _SD_LAYER_DLG_HXX_
#define _SD_LAYER_DLG_HXX_

#include <sddllapi.h>

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include "sdresid.hxx"
#include "strings.hrc"

class SfxItemSet;

class SD_DLLPUBLIC SdInsertLayerDlg : public ModalDialog
{
private:
    FixedText           maFtName;
    Edit                maEdtName;
    FixedText           maFtTitle;
    Edit                maEdtTitle;
    FixedText           maFtDesc;
    MultiLineEdit       maEdtDesc;
    CheckBox            maCbxVisible;
    CheckBox            maCbxPrintable;
    CheckBox            maCbxLocked;
    FixedLine           maFixedLine;
    HelpButton          maBtnHelp;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;

    const SfxItemSet&   mrOutAttrs;

public:

    SdInsertLayerDlg( Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr );
    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // _SD_LAYER_DLG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
