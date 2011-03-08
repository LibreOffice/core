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

#ifndef SC_WARNBOX_HXX
#define SC_WARNBOX_HXX

#include <vcl/msgbox.hxx>


// ============================================================================

/** Message box with warning image and "Do not show again" checkbox. */
class ScCbWarningBox : public WarningBox
{
public:
    /** @param rMsgStr  Resource ID for the message text.
        @param bDefYes  true = "Yes" focused, false = "No" focused. */
                                ScCbWarningBox( Window* pParent, const String& rMsgStr, bool bDefYes = true );

    /** Opens dialog if IsDialogEnabled() returns true.
        @descr  If after executing the dialog the checkbox "Do not show again" is set,
                the method DisableDialog() will be called. */
    virtual sal_Int16           Execute();

    /** Called before executing the dialog. If this method returns false, the dialog will not be opened. */
    virtual bool                IsDialogEnabled();
    /** Called, when dialog is exited and the option "Do not show again" is set. */
    virtual void                DisableDialog();
};


// ----------------------------------------------------------------------------

/** Warning box for "Replace cell contents?". */
class ScReplaceWarnBox : public ScCbWarningBox
{
public:
                                ScReplaceWarnBox( Window* pParent );

    /** Reads the configuration key "ReplaceCellsWarning". */
    virtual bool                IsDialogEnabled();
    /** Sets the configuration key "ReplaceCellsWarning" to false. */
    virtual void                DisableDialog();
};


// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
