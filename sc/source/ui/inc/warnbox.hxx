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

#ifndef INCLUDED_SC_SOURCE_UI_INC_WARNBOX_HXX
#define INCLUDED_SC_SOURCE_UI_INC_WARNBOX_HXX

#include <vcl/msgbox.hxx>

/** Message box with warning image and "Do not show again" checkbox. */
class ScCbWarningBox : public WarningBox
{
public:
    /** @param rMsgStr  Resource ID for the message text.
        @param bDefYes  true = "Yes" focused, false = "No" focused. */
                                ScCbWarningBox( vcl::Window* pParent, const OUString& rMsgStr, bool bDefYes = true );

    /** Opens dialog if IsDialogEnabled() returns true.
        @descr  If after executing the dialog the checkbox "Do not show again" is set,
                the method DisableDialog() will be called. */
    virtual sal_Int16           Execute() override;

    /** Called before executing the dialog. If this method returns false, the dialog will not be opened. */
    virtual bool                IsDialogEnabled();
    /** Called, when dialog is exited and the option "Do not show again" is set. */
    virtual void                DisableDialog();
};

/** Warning box for "Replace cell contents?". */
class ScReplaceWarnBox : public ScCbWarningBox
{
public:
                                ScReplaceWarnBox( vcl::Window* pParent );

    /** Reads the configuration key "ReplaceCellsWarning". */
    virtual bool                IsDialogEnabled() override;
    /** Sets the configuration key "ReplaceCellsWarning" to false. */
    virtual void                DisableDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
