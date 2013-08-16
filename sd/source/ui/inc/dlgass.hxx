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

#ifndef INC_DLGASS
#define INC_DLGASS

#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <svtools/svmedit.hxx>
#include "assclass.hxx"
#include <vcl/lstbox.hxx>
#include <sfx2/app.hxx>
#include <com/sun/star/uno/Sequence.h>

#include "sdenumdef.hxx"

class AssistentDlgImpl;

class AssistentDlg:public ModalDialog
{
private:
    AssistentDlgImpl* mpImpl;

public:
    AssistentDlg(Window* pParent, sal_Bool bAutoPilot);
    ~AssistentDlg();

    DECL_LINK( FinishHdl, void * );

    SfxObjectShellLock GetDocument();
    OutputType GetOutputMedium() const;
    sal_Bool IsSummary() const;
    StartType GetStartType() const;
    OUString GetDocPath() const;
    sal_Bool GetStartWithFlag() const;

    sal_Bool IsDocEmpty() const;
    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > GetPassword();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
