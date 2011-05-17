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

#ifndef UUI_NAMECLASHDLG_HXX
#define UUI_NAMECLASHDLG_HXX

#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/edit.hxx"

//============================================================================

enum NameClashResolveDialogResult { ABORT, RENAME, OVERWRITE };

class NameClashDialog : public ModalDialog
{
    FixedText     maFTMessage;
    Edit          maEDNewName;
    PushButton    maBtnOverwrite;
    PushButton    maBtnRename;
    CancelButton  maBtnCancel;
    HelpButton    maBtnHelp;
    rtl::OUString maSameName;
    rtl::OUString maNewName;

    DECL_LINK( ButtonHdl_Impl, PushButton * );

public:
    NameClashDialog( Window* pParent, ResMgr* pResMgr,
                     rtl::OUString const & rTargetFolderURL,
                     rtl::OUString const & rClashingName,
                     rtl::OUString const & rProposedNewName,
                     bool bAllowOverwrite );
    rtl::OUString getNewName() const { return maNewName; }
};

#endif // UUI_COOKIEDG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
