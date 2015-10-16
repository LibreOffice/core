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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>

class QueryFolderNameDialog : public ModalDialog
{
private:
    VclPtr<Edit>     m_pNameEdit;
    VclPtr<VclFrame> m_pNameLine;
    VclPtr<OKButton> m_pOKBtn;

    DECL_LINK_TYPED( OKHdl, Button*, void );
    DECL_LINK_TYPED( NameHdl, Edit&, void );

public:
    QueryFolderNameDialog(vcl::Window* _pParent, const OUString& rTitle,
        const OUString& rDefaultText, OUString* pGroupName = NULL);
    virtual ~QueryFolderNameDialog();
    virtual void dispose() override;
    OUString GetName() const { return m_pNameEdit->GetText(); }
};

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
