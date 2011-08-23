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

#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= event translation table
    //=====================================================================
    static const XMLEventNameTranslation aEventTranslations[] =
    {
        { "XApproveActionListener::approveAction",		"on-approveaction" },
        { "XActionListener::actionPerformed",			"on-performaction" },
        { "XChangeListener::changed",					"on-change" },
        { "XTextListener::textChanged",					"on-textchange" },
        { "XItemListener::itemStateChanged",			"on-itemstatechange" },
        { "XFocusListener::focusGained",				"on-focus" },
        { "XFocusListener::focusLost",					"on-blur" },
        { "XKeyListener::keyPressed",					"on-keydown" },
        { "XKeyListener::keyReleased",					"on-keyup" },
        { "XMouseListener::mouseEntered",				"on-mouseover" },
        { "XMouseMotionListener::mouseDragged",			"on-mousedrag" },
        { "XMouseMotionListener::mouseMoved",			"on-mousemove" },
        { "XMouseListener::mousePressed",				"on-mousedown" },
        { "XMouseListener::mouseReleased",				"on-mouseup" },
        { "XMouseListener::mouseExited",				"on-mouseout" },
        { "XResetListener::approveReset",				"on-approvereset" },
        { "XResetListener::resetted",					"on-reset" },
        { "XSubmitListener::approveSubmit",				"on-submit" },
        { "XUpdateListener::approveUpdate",				"on-approveupdate" },
        { "XUpdateListener::updated",					"on-update" },
        { "XLoadListener::loaded",						"on-load" },
        { "XLoadListener::reloading",					"on-startreload" },
        { "XLoadListener::reloaded",					"on-reload" },
        { "XLoadListener::unloading",					"on-startunload" },
        { "XLoadListener::unloaded",					"on-unload" },
        { "XConfirmDeleteListener::confirmDelete",		"on-confirmdelete" },
        { "XRowSetApproveListener::approveRowChange",	"on-approverowchange" },
        { "XRowSetListener::rowChanged",				"on-rowchange" },
        { "XRowSetApproveListener::approveCursorMove",	"on-approvecursormove" },
        { "XRowSetListener::cursorMoved",				"on-cursormove" },
        { "XDatabaseParameterListener::approveParameter","on-supplyparameter" },
        { "XSQLErrorListener::errorOccured",			"on-error" },
        { 0, 0 }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter
