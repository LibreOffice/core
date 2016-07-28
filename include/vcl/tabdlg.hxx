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

#ifndef INCLUDED_VCL_TABDLG_HXX
#define INCLUDED_VCL_TABDLG_HXX

#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

class FixedLine;
class TabControl;

class VCL_DLLPUBLIC TabDialog : public Dialog
{
private:
    VclPtr<FixedLine>   mpFixedLine;
    VclPtr<vcl::Window> mpViewWindow;
    WindowAlign         meViewAlign;
    bool                mbPosControls;

    SAL_DLLPRIVATE void ImplInitTabDialogData();
    SAL_DLLPRIVATE void ImplPosControls();

public:
                        TabDialog( vcl::Window* pParent,
                                   WinBits nStyle = WB_STDTABDIALOG );
                        TabDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );
    virtual             ~TabDialog();
    virtual void        dispose() override;

    virtual void        StateChanged( StateChangedType nStateChange ) override;

    void                SetViewWindow( vcl::Window* pWindow ) { mpViewWindow = pWindow; }
    vcl::Window*        GetViewWindow() const { return mpViewWindow; }
    void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }

    // Screenshot interface
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const override;
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription) override;
};

#endif // INCLUDED_VCL_TABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
