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
#ifndef INCLUDED_SVTOOLS_TEMPLDLG_HXX
#define INCLUDED_SVTOOLS_TEMPLDLG_HXX

#include <svtools/svtdllapi.h>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>

struct SvtTmplDlg_Impl;

// class SvtDocumentTemplateDialog ---------------------------------------

class SvtTemplateWindow;

class SVT_DLLPUBLIC SvtDocumentTemplateDialog : public ModalDialog
{
private:
    FixedHyperlink      aMoreTemplatesLink;
    FixedLine           aLine;
    PushButton          aManageBtn;
    PushButton          aPackageBtn;
    PushButton          aEditBtn;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    SvtTmplDlg_Impl*    pImpl;

    DECL_DLLPRIVATE_LINK(           SelectHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           DoubleClickHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           NewFolderHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           SendFocusHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           OKHdl_Impl, PushButton* );
    DECL_DLLPRIVATE_LINK(           OrganizerHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           PackageHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK(           UpdateHdl_Impl, Timer* );
    DECL_DLLPRIVATE_LINK(           OpenLinkHdl_Impl, void* );

public:
    SvtDocumentTemplateDialog( Window* pParent );

    /** ctor for calling the dialog for <em>selection</em> only, not for <em>opening</em> a document
        <p>If you use this ctor, the dialog will behave differently in the following areas:
        <ul><li>The <em>Edit</em> button will be hidden.</li>
            <li>Upon pressing em>Open</em>, the selected file will not be opened. Instead, it's
                URL is available (see <method>GetSelectedFileURL</method>).</li>
        </ul>

    */
    virtual ~SvtDocumentTemplateDialog();

    void        SelectTemplateFolder();

private:
    SVT_DLLPRIVATE void InitImpl( );
    SVT_DLLPRIVATE bool CanEnableEditBtn() const;
};

#endif // INCLUDED_SVTOOLS_TEMPLDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
