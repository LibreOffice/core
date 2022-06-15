/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtXcbEventFilter.hxx>

#include <QtX11Extras/QX11Info>

#include <QtInstance.hxx>

xcb_atom_t QtXcbEventFilter::m_nFrameAtom = 0;
bool QtXcbEventFilter::m_bDidAtomLookups = false;

xcb_atom_t QtXcbEventFilter::lookupAtom(xcb_connection_t* pConn, const char* const sAtomName)
{
    xcb_atom_t nAtom = 0;
    xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(pConn, 1, strlen(sAtomName), sAtomName);
    xcb_intern_atom_reply_t* atom_reply = xcb_intern_atom_reply(pConn, atom_cookie, nullptr);
    if (atom_reply)
    {
        nAtom = atom_reply->atom;
        free(atom_reply);
    }
    return nAtom;
}

void QtXcbEventFilter::fetchAtoms()
{
    if (m_bDidAtomLookups)
        return;
    m_bDidAtomLookups = true;

    xcb_connection_t* pXcbConn = QX11Info::connection();
    m_nFrameAtom = lookupAtom(pXcbConn, m_sFrameAtomName);
}

bool QtXcbEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long*)
{
    if (eventType != "xcb_generic_event_t")
        return false;
    xcb_generic_event_t* ev = static_cast<xcb_generic_event_t*>(message);
    if (ev->response_type != XCB_PROPERTY_NOTIFY)
        return false;
    fetchAtoms();
    if (!m_nFrameAtom)
        return false;

    xcb_property_notify_event_t* property = static_cast<xcb_property_notify_event_t*>(message);
    if (property->atom == m_nFrameAtom)
    {
        QtInstance* pInst = GetQtInstance();
        pInst->notifyDecorationChange(property->window);
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
