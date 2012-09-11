/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_PRES_LAYOUT_DLG_HXX
#define SD_PRES_LAYOUT_DLG_HXX

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

    String              maName;          // Layoutname oder Dateiname
    long                mnLayoutCount;  // Anzahl, der im Dokument vorhandenen MasterPages
    const String        maStrNone;

    void                FillValueSet();
    void                Reset();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
