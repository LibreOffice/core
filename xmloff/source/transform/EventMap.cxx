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

#include "EventMap.hxx"
#include <xmloff/xmlnamespace.hxx>


XMLTransformerEventMapEntry const aTransformerEventMap[] =
{
    { XML_NAMESPACE_DOM,    u"select"_ustr,   u"on-select"_ustr },
    { XML_NAMESPACE_OFFICE, u"insert-start"_ustr, u"on-insert-start"_ustr },
    { XML_NAMESPACE_OFFICE, u"insert-done"_ustr,  u"on-insert-done"_ustr },
    { XML_NAMESPACE_OFFICE, u"mail-merge"_ustr,   u"on-mail-merge"_ustr },
    { XML_NAMESPACE_OFFICE, u"alpha-char-input"_ustr, u"on-alpha-char-input"_ustr },
    { XML_NAMESPACE_OFFICE, u"non-alpha-char-input"_ustr, u"on-non-alpha-char-input"_ustr },
    { XML_NAMESPACE_DOM,    u"resize"_ustr,   u"on-resize"_ustr },
    { XML_NAMESPACE_OFFICE, u"move"_ustr, u"on-move"_ustr },
    { XML_NAMESPACE_OFFICE, u"page-count-change"_ustr, u"on-page-count-change"_ustr },
    { XML_NAMESPACE_DOM,    u"mouseover"_ustr,    u"on-mouse-over"_ustr },
    { XML_NAMESPACE_DOM,    u"click"_ustr,    u"on-click"_ustr },
    { XML_NAMESPACE_DOM,    u"mouseout"_ustr, u"on-mouse-out"_ustr },
    { XML_NAMESPACE_OFFICE, u"load-error"_ustr,   u"on-load-error"_ustr },
    { XML_NAMESPACE_OFFICE, u"load-cancel"_ustr,  u"on-load-cancel"_ustr },
    { XML_NAMESPACE_OFFICE, u"load-done"_ustr,    u"on-load-done"_ustr },
    { XML_NAMESPACE_DOM,    u"load"_ustr, u"on-load"_ustr },
    { XML_NAMESPACE_DOM,    u"unload"_ustr,   u"on-unload"_ustr },
    { XML_NAMESPACE_OFFICE, u"start-app"_ustr,    u"on-start-app"_ustr },
    { XML_NAMESPACE_OFFICE, u"close-app"_ustr,    u"on-close-app"_ustr },
    { XML_NAMESPACE_OFFICE, u"new"_ustr,  u"on-new"_ustr },
    { XML_NAMESPACE_OFFICE, u"save"_ustr, u"on-save"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-as"_ustr,  u"on-save-as"_ustr },
    { XML_NAMESPACE_DOM,    u"DOMFocusIn"_ustr,   u"on-focus"_ustr },
    { XML_NAMESPACE_DOM,    u"DOMFocusOut"_ustr,  u"on-unfocus"_ustr },
    { XML_NAMESPACE_OFFICE, u"print"_ustr,    u"on-print"_ustr },
    { XML_NAMESPACE_DOM,    u"error"_ustr,    u"on-error"_ustr },
    { XML_NAMESPACE_OFFICE, u"load-finished"_ustr,    u"on-load-finished"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-finished"_ustr,    u"on-save-finished"_ustr },
    { XML_NAMESPACE_OFFICE, u"modify-changed"_ustr,   u"on-modify-changed"_ustr },
    { XML_NAMESPACE_OFFICE, u"prepare-unload"_ustr,   u"on-prepare-unload"_ustr },
    { XML_NAMESPACE_OFFICE, u"new-mail"_ustr, u"on-new-mail"_ustr },
    { XML_NAMESPACE_OFFICE, u"toggle-fullscreen"_ustr,    u"on-toggle-fullscreen"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-done"_ustr,    u"on-save-done"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-as-done"_ustr, u"on-save-as-done"_ustr },
    { XML_NAMESPACE_OFFICE, u"create"_ustr,    u"on-create"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-as-failed"_ustr,    u"on-save-as-failed"_ustr },
    { XML_NAMESPACE_OFFICE, u"save-failed"_ustr,    u"on-save-failed"_ustr },
    { XML_NAMESPACE_OFFICE, u"copy-to-failed"_ustr,    u"on-copy-to-failed"_ustr },
    { XML_NAMESPACE_OFFICE, u"title-changed"_ustr,    u"on-title-changed"_ustr },

    { XML_NAMESPACE_FORM,   u"approveaction"_ustr,    u"on-approveaction"_ustr },
    { XML_NAMESPACE_FORM,   u"performaction"_ustr,    u"on-performaction"_ustr },
    { XML_NAMESPACE_DOM,    u"change"_ustr,   u"on-change"_ustr },
    { XML_NAMESPACE_FORM,   u"textchange"_ustr,   u"on-textchange"_ustr },
    { XML_NAMESPACE_FORM,   u"itemstatechange"_ustr,  u"on-itemstatechange"_ustr },
    { XML_NAMESPACE_DOM,    u"keydown"_ustr,  u"on-keydown"_ustr },
    { XML_NAMESPACE_DOM,    u"keyup"_ustr,    u"on-keyup"_ustr },
    { XML_NAMESPACE_FORM,   u"mousedrag"_ustr,    u"on-mousedrag"_ustr },
    { XML_NAMESPACE_DOM,    u"mousemove"_ustr,    u"on-mousemove"_ustr },
    { XML_NAMESPACE_DOM,    u"mousedown"_ustr,    u"on-mousedown"_ustr },
    { XML_NAMESPACE_DOM,    u"mouseup"_ustr,  u"on-mouseup"_ustr },
    { XML_NAMESPACE_FORM,   u"approvereset"_ustr, u"on-approvereset"_ustr },
    { XML_NAMESPACE_DOM,    u"reset"_ustr,    u"on-reset"_ustr },
    { XML_NAMESPACE_DOM,    u"submit"_ustr,   u"on-submit"_ustr },
    { XML_NAMESPACE_FORM,   u"approveupdate"_ustr,    u"on-approveupdate"_ustr },
    { XML_NAMESPACE_FORM,   u"update"_ustr,   u"on-update"_ustr },
    { XML_NAMESPACE_FORM,   u"startreload"_ustr,  u"on-startreload"_ustr },
    { XML_NAMESPACE_FORM,   u"reload"_ustr,   u"on-reload"_ustr },
    { XML_NAMESPACE_FORM,   u"startunload"_ustr,  u"on-startunload"_ustr },
    { XML_NAMESPACE_FORM,   u"confirmdelete"_ustr,    u"on-confirmdelete"_ustr },
    { XML_NAMESPACE_FORM,   u"approverowchange"_ustr, u"on-approverowchange"_ustr },
    { XML_NAMESPACE_FORM,   u"rowchange"_ustr,    u"on-rowchange"_ustr },
    { XML_NAMESPACE_FORM,   u"approvecursormove"_ustr,    u"on-approvecursormove"_ustr },
    { XML_NAMESPACE_FORM,   u"cursormove"_ustr,   u"on-cursormove"_ustr },
    { XML_NAMESPACE_FORM,   u"supplyparameter"_ustr,  u"on-supplyparameter"_ustr },
    { XML_NAMESPACE_FORM,   u"adjust"_ustr,   u"on-adjust"_ustr },
    { 0, u""_ustr, u""_ustr }
};

XMLTransformerEventMapEntry const aFormTransformerEventMap[] =
{
    { XML_NAMESPACE_DOM,    u"mouseover"_ustr,    u"on-mouseover"_ustr },
    { XML_NAMESPACE_DOM,    u"mouseout"_ustr, u"on-mouseout"_ustr },
    { XML_NAMESPACE_DOM,    u"DOMFocusOut"_ustr,  u"on-blur"_ustr },
    { 0, u""_ustr, u""_ustr }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
