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

#include <AnnotationWin.hxx>
#include <strings.hrc>

#include <unotools/useroptions.hxx>

#include <vcl/event.hxx>

#include <cmdid.h>

#include <swtypes.hxx>

namespace sw::annotation {

IMPL_LINK(SwAnnotationWin, SelectHdl, const OUString&, rIdent, void)
{
    if (rIdent.isEmpty())
        return;

    // tdf#136682 ensure this is the currently active sidebar win so the command
    // operates in an active sidebar context
    bool bSwitchedFocus = SetActiveSidebarWin();

    if (rIdent == "reply")
        ExecuteCommand(FN_REPLY);
    if (rIdent == "resolve" || rIdent == "unresolve")
        ExecuteCommand(FN_RESOLVE_NOTE);
    else if (rIdent == "resolvethread" || rIdent == "unresolvethread")
        ExecuteCommand(FN_RESOLVE_NOTE_THREAD);
    else if (rIdent == "delete")
        ExecuteCommand(FN_DELETE_COMMENT);
    else if (rIdent == "deletethread")
        ExecuteCommand(FN_DELETE_COMMENT_THREAD);
    else if (rIdent == "deleteby")
        ExecuteCommand(FN_DELETE_NOTE_AUTHOR);
    else if (rIdent == "deleteall")
        ExecuteCommand(FN_DELETE_ALL_NOTES);
    else if (rIdent == "formatall")
        ExecuteCommand(FN_FORMAT_ALL_NOTES);

    if (bSwitchedFocus)
        UnsetActiveSidebarWin();
    GrabFocusToDocument();
}

IMPL_LINK_NOARG(SwAnnotationWin, ToggleHdl, weld::Toggleable&, void)
{
    if (!mxMenuButton->get_active())
        return;

    bool bReadOnly = IsReadOnly();
    if (bReadOnly)
    {
        mxMenuButton->set_item_visible(u"reply"_ustr, false);
        mxMenuButton->set_item_visible(u"sep1"_ustr, false); // Separator after reply button.
        mxMenuButton->set_item_visible(u"resolve"_ustr, false);
        mxMenuButton->set_item_visible(u"unresolve"_ustr, false);
        mxMenuButton->set_item_visible(u"resolvethread"_ustr, false);
        mxMenuButton->set_item_visible(u"unresolvethread"_ustr, false);
        mxMenuButton->set_item_visible(u"delete"_ustr, false );
    }
    else
    {
        mxMenuButton->set_item_visible(u"reply"_ustr, !IsReadOnlyOrProtected());
        mxMenuButton->set_item_visible(u"sep1"_ustr, !IsReadOnlyOrProtected());
        mxMenuButton->set_item_visible(u"resolve"_ustr, !IsResolved());
        mxMenuButton->set_item_visible(u"unresolve"_ustr, IsResolved());
        mxMenuButton->set_item_visible(u"resolvethread"_ustr, !IsThreadResolved());
        mxMenuButton->set_item_visible(u"unresolvethread"_ustr, IsThreadResolved());
        mxMenuButton->set_item_visible(u"delete"_ustr, !IsReadOnlyOrProtected());
    }

    mxMenuButton->set_item_visible(u"deletethread"_ustr, !bReadOnly);
    mxMenuButton->set_item_visible(u"deleteby"_ustr, !bReadOnly);
    mxMenuButton->set_item_visible(u"deleteall"_ustr, !bReadOnly);
    mxMenuButton->set_item_visible(u"formatall"_ustr, !bReadOnly);
}

IMPL_LINK(SwAnnotationWin, KeyInputHdl, const KeyEvent&, rKeyEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    if (rKeyCode.GetCode() == KEY_TAB)
    {
        ActivatePostIt();
        GrabFocus();
        return true;
    }
    return false;
}

} // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
