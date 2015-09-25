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

#ifndef INCLUDED_SC_SOURCE_UI_INC_REDCOM_HXX
#define INCLUDED_SC_SOURCE_UI_INC_REDCOM_HXX

#include "chgtrack.hxx"

class ScDocShell;
class AbstractSvxPostItDialog;

class ScRedComDialog
{
private:

    ScChangeAction  *pChangeAction;
    ScDocShell      *pDocShell;
    OUString        aComment;
    AbstractSvxPostItDialog* pDlg;

    DECL_LINK_TYPED( PrevHdl, AbstractSvxPostItDialog&, void );
    DECL_LINK_TYPED( NextHdl, AbstractSvxPostItDialog&, void );

protected:

    void    ReInit(ScChangeAction *);
    void    SelectCell();

    ScChangeAction *FindPrev(ScChangeAction *pAction);
    ScChangeAction *FindNext(ScChangeAction *pAction);

public:

    ScRedComDialog( vcl::Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *pShell, ScChangeAction *pAction, bool bPrevNext = false);
    ~ScRedComDialog();

    short Execute();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
