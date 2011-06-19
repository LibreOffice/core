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

#include <vcl.hxx>

#include <sal/types.h>
#include <vcl/button.hxx>

static PushButton* get_button (Dialog const* dialog, sal_uInt32 type)
{
    Window* child = dialog->GetWindow (WINDOW_FIRSTCHILD);
    while (child)
    {
        if (child->GetType () == type)
            return static_cast <PushButton*> (child);
        child = child->GetWindow (WINDOW_NEXT);
    }

    return 0;
}

#define IMPLEMENT_CLOSING_DIALOG(cls)\
    Closing##cls::Closing##cls (Window* parent, WinBits bits)\
    : cls (parent, bits)\
        , mClosing (false)\
    {\
    }\
    sal_Bool Closing##cls::Close ()\
    {\
        if (mClosing)\
            EndDialog (false);\
        else if (PushButton *cancel = get_button (this, WINDOW_CANCELBUTTON))\
            cancel->Click ();\
        else if (PushButton *ok = get_button (this, WINDOW_OKBUTTON))\
            ok->Click ();\
        mClosing = true;\
        return false;\
    }

IMPLEMENT_CLOSING_DIALOG (Dialog);
IMPLEMENT_CLOSING_DIALOG (ModelessDialog);
IMPLEMENT_CLOSING_DIALOG (ModalDialog);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
