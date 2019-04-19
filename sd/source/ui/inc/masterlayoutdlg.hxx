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

#include <vcl/weld.hxx>

#include <pres.hxx>

class SdDrawDocument;
class SdPage;

namespace sd
{

class MasterLayoutDialog : public weld::GenericDialogController
{
private:
    SdDrawDocument* mpDoc;
    SdPage*         mpCurrentPage;

    std::unique_ptr<weld::CheckButton> mxCBDate;
    std::unique_ptr<weld::CheckButton> mxCBPageNumber;
    std::unique_ptr<weld::CheckButton> mxCBSlideNumber;
    std::unique_ptr<weld::CheckButton> mxCBHeader;
    std::unique_ptr<weld::CheckButton> mxCBFooter;

    bool            mbOldHeader;
    bool            mbOldFooter;
    bool            mbOldDate;
    bool            mbOldPageNumber;

    void applyChanges();
    void remove( PresObjKind eKind );
    void create( PresObjKind eKind );

public:
    MasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage);
    virtual ~MasterLayoutDialog() override;

    virtual short run() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
