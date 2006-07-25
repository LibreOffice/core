/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formevents.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:24:08 $
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
        { 0, 0, 0 }
    };

    const XMLEventNameTranslation* g_pFormsEventTranslation = aEventTranslations;

//.........................................................................
}   // namespace xmloff
//.........................................................................

