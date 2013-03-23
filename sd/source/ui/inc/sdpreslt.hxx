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

#include <boost/ptr_container/ptr_vector.hpp>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>

class SfxItemSet;

namespace sd {
class DrawDocShell;
}



class SdPresLayoutDlg
    : public ModalDialog
{
public:
    SdPresLayoutDlg(
        ::sd::DrawDocShell* pDocShell,
        ::Window* pWindow,
        const SfxItemSet& rInAttrs);

    virtual ~SdPresLayoutDlg (void);

    void                GetAttr(SfxItemSet& rOutAttrs);

    DECL_LINK(ClickLayoutHdl, void *);
    DECL_LINK(ClickLoadHdl, void *);

private:
    ::sd::DrawDocShell* mpDocSh;
    FixedText           maFtLayout;
    ValueSet            maVS;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    CheckBox            maCbxMasterPage;
    CheckBox            maCbxCheckMasters;
    PushButton          maBtnLoad;

    const SfxItemSet&   mrOutAttrs;

    boost::ptr_vector<String> maLayoutNames;

    String              maName;          ///< layout name or file name
    long                mnLayoutCount;   ///< number of master pages in the document
    const String        maStrNone;

    void                FillValueSet();
    void                Reset();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
