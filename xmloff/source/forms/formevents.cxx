/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formevents.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:09:46 $
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

#ifndef _XMLOFF_FORMS_FORMEVENTS_HXX_
#include "formevents.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= event translation table
    //=====================================================================
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
        { 0, 0 }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3.298.1  2005/09/05 14:38:59  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.3  2004/07/13 08:12:38  rt
 *  INTEGRATION: CWS oasis (1.2.62); FILE MERGED
 *  2004/06/02 10:53:57 mib 1.2.62.2: - #i20153#: continued events
 *  2004/05/28 16:38:00 mib 1.2.62.1: - #i20153#: Events
 *
 *  Revision 1.2.62.2  2004/06/02 10:53:57  mib
 *  - #i20153#: continued events
 *
 *  Revision 1.2.62.1  2004/05/28 16:38:00  mib
 *  - #i20153#: Events
 *
 *  Revision 1.2  2003/12/11 12:09:40  kz
 *  INTEGRATION: CWS frmcontrols01 (1.1.212); FILE MERGED
 *  2003/10/22 13:17:43 fs 1.1.212.1: #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.1.212.1  2003/10/22 13:17:43  fs
 *  #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.1  2001/01/02 15:56:27  fs
 *  initial checkin - code common to event ex- and import
 *
 *
 *  Revision 1.0 21.12.00 15:19:52  fs
 ************************************************************************/

