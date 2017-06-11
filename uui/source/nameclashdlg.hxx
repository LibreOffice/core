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

#ifndef INCLUDED_UUI_SOURCE_NAMECLASHDLG_HXX
#define INCLUDED_UUI_SOURCE_NAMECLASHDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>


enum NameClashResolveDialogResult { ABORT, RENAME, OVERWRITE };

class NameClashDialog : public ModalDialog
{
    VclPtr<FixedText>     m_pFTMessage;
    VclPtr<Edit>          m_pEDNewName;
    VclPtr<PushButton>    m_pBtnOverwrite;
    VclPtr<PushButton>    m_pBtnRename;
    VclPtr<CancelButton>  m_pBtnCancel;

    OUString maSameName;
    OUString maNewName;

    DECL_LINK( ButtonHdl_Impl, Button *, void );

public:
    NameClashDialog( vcl::Window* pParent, const std::locale& rLocale,
                     OUString const & rTargetFolderURL,
                     OUString const & rClashingName,
                     OUString const & rProposedNewName,
                     bool bAllowOverwrite );
    virtual ~NameClashDialog() override;
    virtual void dispose() override;
    const OUString& getNewName() const { return maNewName; }
};

#endif // UUI_COOKIEDG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
