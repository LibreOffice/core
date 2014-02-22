/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "formevents.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlevent.hxx>

namespace xmloff
{

    
    static const XMLEventNameTranslation aEventTranslations[] =
    {
        { "XApproveActionListener::approveAction",      XML_NAMESPACE_FORM, "approveaction" }, 
        { "XActionListener::actionPerformed",           XML_NAMESPACE_FORM, "performaction" }, 
        { "XChangeListener::changed",                   XML_NAMESPACE_DOM, "change" }, 
        { "XTextListener::textChanged",                 XML_NAMESPACE_FORM, "textchange" }, 
        { "XItemListener::itemStateChanged",            XML_NAMESPACE_FORM, "itemstatechange" }, 
        { "XFocusListener::focusGained",                XML_NAMESPACE_DOM, "DOMFocusIn" }, 
        { "XFocusListener::focusLost",                  XML_NAMESPACE_DOM, "DOMFocusOut" }, 
        { "XKeyListener::keyPressed",                   XML_NAMESPACE_DOM, "keydown" }, 
        { "XKeyListener::keyReleased",                  XML_NAMESPACE_DOM, "keyup" }, 
        { "XMouseListener::mouseEntered",               XML_NAMESPACE_DOM, "mouseover" }, 
        { "XMouseMotionListener::mouseDragged",         XML_NAMESPACE_FORM, "mousedrag" }, 
        { "XMouseMotionListener::mouseMoved",           XML_NAMESPACE_DOM, "mousemove" }, 
        { "XMouseListener::mousePressed",               XML_NAMESPACE_DOM, "mousedown" }, 
        { "XMouseListener::mouseReleased",              XML_NAMESPACE_DOM, "mouseup" }, 
        { "XMouseListener::mouseExited",                XML_NAMESPACE_DOM, "mouseout" }, 
        { "XResetListener::approveReset",               XML_NAMESPACE_FORM, "approvereset" }, 
        { "XResetListener::resetted",                   XML_NAMESPACE_DOM, "reset" }, 
        { "XSubmitListener::approveSubmit",             XML_NAMESPACE_DOM, "submit" }, 
        { "XUpdateListener::approveUpdate",             XML_NAMESPACE_FORM, "approveupdate" }, 
        { "XUpdateListener::updated",                   XML_NAMESPACE_FORM, "update" }, 
        { "XLoadListener::loaded",                      XML_NAMESPACE_DOM, "load" }, 
        { "XLoadListener::reloading",                   XML_NAMESPACE_FORM, "startreload" }, 
        { "XLoadListener::reloaded",                    XML_NAMESPACE_FORM, "reload" }, 
        { "XLoadListener::unloading",                   XML_NAMESPACE_FORM, "startunload" }, 
        { "XLoadListener::unloaded",                    XML_NAMESPACE_DOM, "unload" }, 
        { "XConfirmDeleteListener::confirmDelete",      XML_NAMESPACE_FORM, "confirmdelete" }, 
        { "XRowSetApproveListener::approveRowChange",   XML_NAMESPACE_FORM, "approverowchange" }, 
        { "XRowSetListener::rowChanged",                XML_NAMESPACE_FORM, "rowchange" }, 
        { "XRowSetApproveListener::approveCursorMove",  XML_NAMESPACE_FORM, "approvecursormove" }, 
        { "XRowSetListener::cursorMoved",               XML_NAMESPACE_FORM, "cursormove" }, 
        { "XDatabaseParameterListener::approveParameter",XML_NAMESPACE_FORM, "supplyparameter" }, 
        { "XSQLErrorListener::errorOccured",            XML_NAMESPACE_DOM, "error" }, 
        { "XAdjustmentListener::adjustmentValueChanged",XML_NAMESPACE_FORM, "adjust" }, 
        { 0, 0, 0 }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
