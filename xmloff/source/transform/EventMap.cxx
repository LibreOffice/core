/*************************************************************************
 *
 *  $RCSfile: EventMap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:48:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
