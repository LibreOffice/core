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
#if 1

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "sdpage.hxx"

class SdDrawDocument;

namespace sd
{

class MasterLayoutDialog : public ModalDialog
{
private:
    SdDrawDocument* mpDoc;
    SdPage*         mpCurrentPage;

    FixedLine       maFLPlaceholders;
    CheckBox        maCBDate;
    CheckBox        maCBPageNumber;
    CheckBox        maCBHeader;
    CheckBox        maCBFooter;

    OKButton        maPBOK;
    CancelButton    maPBCancel;

    sal_Bool            mbOldHeader;
    sal_Bool            mbOldFooter;
    sal_Bool            mbOldDate;
    sal_Bool            mbOldPageNumber;

    void applyChanges();
    void remove( PresObjKind eKind );
    void create( PresObjKind eKind );

public:
    MasterLayoutDialog( Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~MasterLayoutDialog();

    virtual short Execute();
};

}

#endif // _SD_MASTERLAYOUT_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
