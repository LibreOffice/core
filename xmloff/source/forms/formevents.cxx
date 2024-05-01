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

#include "formevents.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlevent.hxx>

namespace xmloff
{

    //= event translation table
    constexpr XMLEventNameTranslation aEventTranslations[] =
    {
        { u"XApproveActionListener::approveAction"_ustr,      XML_NAMESPACE_FORM, u"approveaction"_ustr }, // "on-approveaction"
        { u"XActionListener::actionPerformed"_ustr,           XML_NAMESPACE_FORM, u"performaction"_ustr }, // "on-performaction"
        { u"XChangeListener::changed"_ustr,                   XML_NAMESPACE_DOM, u"change"_ustr }, // "on-change"
        { u"XTextListener::textChanged"_ustr,                 XML_NAMESPACE_FORM, u"textchange"_ustr }, // "on-textchange"
        { u"XItemListener::itemStateChanged"_ustr,            XML_NAMESPACE_FORM, u"itemstatechange"_ustr }, // "on-itemstatechange"
        { u"XFocusListener::focusGained"_ustr,                XML_NAMESPACE_DOM, u"DOMFocusIn"_ustr }, // "on-focus"
        { u"XFocusListener::focusLost"_ustr,                  XML_NAMESPACE_DOM, u"DOMFocusOut"_ustr }, // "on-blur"
        { u"XKeyListener::keyPressed"_ustr,                   XML_NAMESPACE_DOM, u"keydown"_ustr }, // "on-keydown"
        { u"XKeyListener::keyReleased"_ustr,                  XML_NAMESPACE_DOM, u"keyup"_ustr }, // "on-keyup"
        { u"XMouseListener::mouseEntered"_ustr,               XML_NAMESPACE_DOM, u"mouseover"_ustr }, // "on-mouseover"
        { u"XMouseMotionListener::mouseDragged"_ustr,         XML_NAMESPACE_FORM, u"mousedrag"_ustr }, // "on-mousedrag"
        { u"XMouseMotionListener::mouseMoved"_ustr,           XML_NAMESPACE_DOM, u"mousemove"_ustr }, // "on-mousemove"
        { u"XMouseListener::mousePressed"_ustr,               XML_NAMESPACE_DOM, u"mousedown"_ustr }, // "on-mousedown"
        { u"XMouseListener::mouseReleased"_ustr,              XML_NAMESPACE_DOM, u"mouseup"_ustr }, // "on-mouseup"
        { u"XMouseListener::mouseExited"_ustr,                XML_NAMESPACE_DOM, u"mouseout"_ustr }, // "on-mouseout"
        { u"XResetListener::approveReset"_ustr,               XML_NAMESPACE_FORM, u"approvereset"_ustr }, // "on-approvereset"
        { u"XResetListener::resetted"_ustr,                   XML_NAMESPACE_DOM, u"reset"_ustr }, // "on-reset"
        { u"XSubmitListener::approveSubmit"_ustr,             XML_NAMESPACE_DOM, u"submit"_ustr }, // "on-submit"
        { u"XUpdateListener::approveUpdate"_ustr,             XML_NAMESPACE_FORM, u"approveupdate"_ustr }, // "on-approveupdate"
        { u"XUpdateListener::updated"_ustr,                   XML_NAMESPACE_FORM, u"update"_ustr }, // "on-update"
        { u"XLoadListener::loaded"_ustr,                      XML_NAMESPACE_DOM, u"load"_ustr }, // "on-load"
        { u"XLoadListener::reloading"_ustr,                   XML_NAMESPACE_FORM, u"startreload"_ustr }, // "on-startreload"
        { u"XLoadListener::reloaded"_ustr,                    XML_NAMESPACE_FORM, u"reload"_ustr }, // "on-reload"
        { u"XLoadListener::unloading"_ustr,                   XML_NAMESPACE_FORM, u"startunload"_ustr }, // "on-startunload"
        { u"XLoadListener::unloaded"_ustr,                    XML_NAMESPACE_DOM, u"unload"_ustr }, // "on-unload"
        { u"XConfirmDeleteListener::confirmDelete"_ustr,      XML_NAMESPACE_FORM, u"confirmdelete"_ustr }, // "on-confirmdelete"
        { u"XRowSetApproveListener::approveRowChange"_ustr,   XML_NAMESPACE_FORM, u"approverowchange"_ustr }, // "on-approverowchange"
        { u"XRowSetListener::rowChanged"_ustr,                XML_NAMESPACE_FORM, u"rowchange"_ustr }, // "on-rowchange"
        { u"XRowSetApproveListener::approveCursorMove"_ustr,  XML_NAMESPACE_FORM, u"approvecursormove"_ustr }, // "on-approvecursormove"
        { u"XRowSetListener::cursorMoved"_ustr,               XML_NAMESPACE_FORM, u"cursormove"_ustr }, // "on-cursormove"
        { u"XDatabaseParameterListener::approveParameter"_ustr,XML_NAMESPACE_FORM, u"supplyparameter"_ustr }, // "on-supplyparameter"
        { u"XSQLErrorListener::errorOccured"_ustr,            XML_NAMESPACE_DOM, u"error"_ustr }, // "on-error"
        { u"XAdjustmentListener::adjustmentValueChanged"_ustr,XML_NAMESPACE_FORM, u"adjust"_ustr }, // "on-adjust"
        { u""_ustr, 0, u""_ustr }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
