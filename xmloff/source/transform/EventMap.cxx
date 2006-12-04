/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventMap.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-04 08:12:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_EVENTMAP_HXX
#include "EventMap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif


XMLTransformerEventMapEntry aTransformerEventMap[] =
{
    { XML_NAMESPACE_DOM,    "select",   "on-select" },
    { XML_NAMESPACE_OFFICE, "insert-start", "on-insert-start" },
    { XML_NAMESPACE_OFFICE, "insert-done",  "on-insert-done" },
    { XML_NAMESPACE_OFFICE, "mail-merge",   "on-mail-merge" },
    { XML_NAMESPACE_OFFICE, "alpha-char-input", "on-alpha-char-input" },
    { XML_NAMESPACE_OFFICE, "non-alpha-char-input", "on-non-alpha-char-input" },
    { XML_NAMESPACE_DOM,    "resize",   "on-resize" },
    { XML_NAMESPACE_OFFICE, "move", "on-move" },
    { XML_NAMESPACE_OFFICE, "page-count-change", "on-page-count-change" },
    { XML_NAMESPACE_DOM,    "mouseover",    "on-mouse-over" },
    { XML_NAMESPACE_DOM,    "click",    "on-click" },
    { XML_NAMESPACE_DOM,    "mouseout", "on-mouse-out" },
    { XML_NAMESPACE_OFFICE, "load-error",   "on-load-error" },
    { XML_NAMESPACE_OFFICE, "load-cancel",  "on-load-cancel" },
    { XML_NAMESPACE_OFFICE, "load-done",    "on-load-done" },
    { XML_NAMESPACE_DOM,    "load", "on-load" },
    { XML_NAMESPACE_DOM,    "unload",   "on-unload" },
    { XML_NAMESPACE_OFFICE, "start-app",    "on-start-app" },
    { XML_NAMESPACE_OFFICE, "close-app",    "on-close-app" },
    { XML_NAMESPACE_OFFICE, "new",  "on-new" },
    { XML_NAMESPACE_OFFICE, "save", "on-save" },
    { XML_NAMESPACE_OFFICE, "save-as",  "on-save-as" },
    { XML_NAMESPACE_DOM,    "DOMFocusIn",   "on-focus" },
    { XML_NAMESPACE_DOM,    "DOMFocusOut",  "on-unfocus" },
    { XML_NAMESPACE_OFFICE, "print",    "on-print" },
    { XML_NAMESPACE_DOM,    "error",    "on-error" },
    { XML_NAMESPACE_OFFICE, "load-finished",    "on-load-finished" },
    { XML_NAMESPACE_OFFICE, "save-finished",    "on-save-finished" },
    { XML_NAMESPACE_OFFICE, "modify-changed",   "on-modify-changed" },
    { XML_NAMESPACE_OFFICE, "prepare-unload",   "on-prepare-unload" },
    { XML_NAMESPACE_OFFICE, "new-mail", "on-new-mail" },
    { XML_NAMESPACE_OFFICE, "toggle-fullscreen",    "on-toggle-fullscreen" },
    { XML_NAMESPACE_OFFICE, "save-done",    "on-save-done" },
    { XML_NAMESPACE_OFFICE, "save-as-done", "on-save-as-done" },
    { XML_NAMESPACE_OFFICE, "create",    "on-create" },
    { XML_NAMESPACE_OFFICE, "save-as-failed",    "on-save-as-failed" },
    { XML_NAMESPACE_OFFICE, "save-failed",    "on-save-failed" },
    { XML_NAMESPACE_OFFICE, "copy-to-failed",    "on-copy-to-failed" },
    { XML_NAMESPACE_OFFICE, "title-changed",    "on-title-changed" },

    { XML_NAMESPACE_FORM,   "approveaction",    "on-approveaction" },
    { XML_NAMESPACE_FORM,   "performaction",    "on-performaction" },
    { XML_NAMESPACE_DOM,    "change",   "on-change" },
    { XML_NAMESPACE_FORM,   "textchange",   "on-textchange" },
    { XML_NAMESPACE_FORM,   "itemstatechange",  "on-itemstatechange" },
    { XML_NAMESPACE_DOM,    "keydown",  "on-keydown" },
    { XML_NAMESPACE_DOM,    "keyup",    "on-keyup" },
    { XML_NAMESPACE_FORM,   "mousedrag",    "on-mousedrag" },
    { XML_NAMESPACE_DOM,    "mousemove",    "on-mousemove" },
    { XML_NAMESPACE_DOM,    "mousedown",    "on-mousedown" },
    { XML_NAMESPACE_DOM,    "mouseup",  "on-mouseup" },
    { XML_NAMESPACE_FORM,   "approvereset", "on-approvereset" },
    { XML_NAMESPACE_DOM,    "reset",    "on-reset" },
    { XML_NAMESPACE_DOM,    "submit",   "on-submit" },
    { XML_NAMESPACE_FORM,   "approveupdate",    "on-approveupdate" },
    { XML_NAMESPACE_FORM,   "update",   "on-update" },
    { XML_NAMESPACE_FORM,   "startreload",  "on-startreload" },
    { XML_NAMESPACE_FORM,   "reload",   "on-reload" },
    { XML_NAMESPACE_FORM,   "startunload",  "on-startunload" },
    { XML_NAMESPACE_FORM,   "confirmdelete",    "on-confirmdelete" },
    { XML_NAMESPACE_FORM,   "approverowchange", "on-approverowchange" },
    { XML_NAMESPACE_FORM,   "rowchange",    "on-rowchange" },
    { XML_NAMESPACE_FORM,   "approvecursormove",    "on-approvecursormove" },
    { XML_NAMESPACE_FORM,   "cursormove",   "on-cursormove" },
    { XML_NAMESPACE_FORM,   "supplyparameter",  "on-supplyparameter" },
    { XML_NAMESPACE_FORM,   "adjust",   "on-adjust" },
    { 0, 0, 0 }
};

XMLTransformerEventMapEntry aFormTransformerEventMap[] =
{
    { XML_NAMESPACE_DOM,    "mouseover",    "on-mouseover" },
    { XML_NAMESPACE_DOM,    "mouseout", "on-mouseout" },
    { XML_NAMESPACE_DOM,    "DOMFocusOut",  "on-blur" },
    { 0, 0, 0 }
};
