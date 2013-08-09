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
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlevent.hxx>

namespace xmloff
{

    //= event translation table
    static const XMLEventNameTranslation aEventTranslations[] =
    {
        { "XApproveActionListener::approveAction",      XML_NAMESPACE_FORM, "approveaction" }, // "on-approveaction"
        { "XActionListener::actionPerformed",           XML_NAMESPACE_FORM, "performaction" }, // "on-performaction"
        { "XChangeListener::changed",                   XML_NAMESPACE_DOM, "change" }, // "on-change"
        { "XTextListener::textChanged",                 XML_NAMESPACE_FORM, "textchange" }, // "on-textchange"
        { "XItemListener::itemStateChanged",            XML_NAMESPACE_FORM, "itemstatechange" }, // "on-itemstatechange"
        { "XFocusListener::focusGained",                XML_NAMESPACE_DOM, "DOMFocusIn" }, // "on-focus"
        { "XFocusListener::focusLost",                  XML_NAMESPACE_DOM, "DOMFocusOut" }, // "on-blur"
        { "XKeyListener::keyPressed",                   XML_NAMESPACE_DOM, "keydown" }, // "on-keydown"
        { "XKeyListener::keyReleased",                  XML_NAMESPACE_DOM, "keyup" }, // "on-keyup"
        { "XMouseListener::mouseEntered",               XML_NAMESPACE_DOM, "mouseover" }, // "on-mouseover"
        { "XMouseMotionListener::mouseDragged",         XML_NAMESPACE_FORM, "mousedrag" }, // "on-mousedrag"
        { "XMouseMotionListener::mouseMoved",           XML_NAMESPACE_DOM, "mousemove" }, // "on-mousemove"
        { "XMouseListener::mousePressed",               XML_NAMESPACE_DOM, "mousedown" }, // "on-mousedown"
        { "XMouseListener::mouseReleased",              XML_NAMESPACE_DOM, "mouseup" }, // "on-mouseup"
        { "XMouseListener::mouseExited",                XML_NAMESPACE_DOM, "mouseout" }, // "on-mouseout"
        { "XResetListener::approveReset",               XML_NAMESPACE_FORM, "approvereset" }, // "on-approvereset"
        { "XResetListener::resetted",                   XML_NAMESPACE_DOM, "reset" }, // "on-reset"
        { "XSubmitListener::approveSubmit",             XML_NAMESPACE_DOM, "submit" }, // "on-submit"
        { "XUpdateListener::approveUpdate",             XML_NAMESPACE_FORM, "approveupdate" }, // "on-approveupdate"
        { "XUpdateListener::updated",                   XML_NAMESPACE_FORM, "update" }, // "on-update"
        { "XLoadListener::loaded",                      XML_NAMESPACE_DOM, "load" }, // "on-load"
        { "XLoadListener::reloading",                   XML_NAMESPACE_FORM, "startreload" }, // "on-startreload"
        { "XLoadListener::reloaded",                    XML_NAMESPACE_FORM, "reload" }, // "on-reload"
        { "XLoadListener::unloading",                   XML_NAMESPACE_FORM, "startunload" }, // "on-startunload"
        { "XLoadListener::unloaded",                    XML_NAMESPACE_DOM, "unload" }, // "on-unload"
        { "XConfirmDeleteListener::confirmDelete",      XML_NAMESPACE_FORM, "confirmdelete" }, // "on-confirmdelete"
        { "XRowSetApproveListener::approveRowChange",   XML_NAMESPACE_FORM, "approverowchange" }, // "on-approverowchange"
        { "XRowSetListener::rowChanged",                XML_NAMESPACE_FORM, "rowchange" }, // "on-rowchange"
        { "XRowSetApproveListener::approveCursorMove",  XML_NAMESPACE_FORM, "approvecursormove" }, // "on-approvecursormove"
        { "XRowSetListener::cursorMoved",               XML_NAMESPACE_FORM, "cursormove" }, // "on-cursormove"
        { "XDatabaseParameterListener::approveParameter",XML_NAMESPACE_FORM, "supplyparameter" }, // "on-supplyparameter"
        { "XSQLErrorListener::errorOccured",            XML_NAMESPACE_DOM, "error" }, // "on-error"
        { "XAdjustmentListener::adjustmentValueChanged",XML_NAMESPACE_FORM, "adjust" }, // "on-adjust"
        { 0, 0, 0 }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
