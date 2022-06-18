/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtX11Support.hxx>

#include <config_vclplug.h>

#include <QtCore/QVersionNumber>

#include <QtInstance.hxx>
#include <QtTools.hxx>

#if CHECK_QT5_USING_X11
#include <QtX11Extras/QX11Info>
#endif

#if QT5_HAVE_XCB_ICCCM
#include <xcb/xcb_icccm.h>
#endif

#include <unx/gensys.h>

xcb_atom_t QtX11Support::m_nWindowGroupAtom = 0;
bool QtX11Support::m_bDidAtomLookups = false;

xcb_atom_t QtX11Support::lookupAtom(xcb_connection_t* pConn, const char* const sAtomName)
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

void QtX11Support::fetchAtoms()
{
#if CHECK_QT5_USING_X11
    if (m_bDidAtomLookups)
        return;
    m_bDidAtomLookups = true;

    xcb_connection_t* pXcbConn = QX11Info::connection();
    m_nWindowGroupAtom = lookupAtom(pXcbConn, m_sWindowGroupName);
#endif
}

void QtX11Support::setApplicationID(const xcb_window_t nWinId, std::u16string_view rWMClass)
{
#if CHECK_QT5_USING_X11
    OString aResClass = OUStringToOString(rWMClass, RTL_TEXTENCODING_ASCII_US);
    const char* pResClass
        = !aResClass.isEmpty() ? aResClass.getStr() : SalGenericSystem::getFrameClassName();
    OString aResName = SalGenericSystem::getFrameResName();

    // the WM_CLASS data consists of two concatenated cstrings, including the terminating '\0' chars
    const uint32_t data_len = aResName.getLength() + 1 + strlen(pResClass) + 1;
    char* data = new char[data_len];
    memcpy(data, aResName.getStr(), aResName.getLength() + 1);
    memcpy(data + aResName.getLength() + 1, pResClass, strlen(pResClass) + 1);

    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, nWinId, XCB_ATOM_WM_CLASS,
                        XCB_ATOM_STRING, 8, data_len, data);
    delete[] data;
#else
    Q_UNUSED(nWinId);
    Q_UNUSED(rWMClass);
#endif
}

bool QtX11Support::fixICCCMwindowGroup(const xcb_window_t nWinId)
{
#if CHECK_QT5_USING_X11 && QT5_HAVE_XCB_ICCCM
    // older Qt5 just sets WM_CLIENT_LEADER, but not the XCB_ICCCM_WM_HINT_WINDOW_GROUP
    // see Qt commit 0de4b326d8 ("xcb: fix issue with dialogs hidden by other windows")
    // or QTBUG-46626. So LO has to set this itself to help some WMs.
    if (QVersionNumber::fromString(qVersion()) >= QVersionNumber(5, 12))
        return false;

    xcb_connection_t* pXcbConn = QX11Info::connection();
    xcb_icccm_wm_hints_t hints;

    xcb_get_property_cookie_t prop_cookie = xcb_icccm_get_wm_hints_unchecked(pXcbConn, nWinId);
    if (!xcb_icccm_get_wm_hints_reply(pXcbConn, prop_cookie, &hints, nullptr))
        return false;

    if (hints.flags & XCB_ICCCM_WM_HINT_WINDOW_GROUP)
        return false;

    fetchAtoms();
    if (!m_nWindowGroupAtom)
        return false;

    prop_cookie = xcb_get_property(pXcbConn, 0, nWinId, m_nWindowGroupAtom, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_reply_t* prop_reply = xcb_get_property_reply(pXcbConn, prop_cookie, nullptr);
    if (!prop_reply)
        return true;

    if (xcb_get_property_value_length(prop_reply) != 4)
    {
        free(prop_reply);
        return true;
    }

    xcb_window_t leader = *static_cast<xcb_window_t*>(xcb_get_property_value(prop_reply));
    free(prop_reply);

    hints.flags |= XCB_ICCCM_WM_HINT_WINDOW_GROUP;
    hints.window_group = leader;
    xcb_icccm_set_wm_hints(pXcbConn, nWinId, &hints);
    return true;
#else
    Q_UNUSED(nWinId);
    return false;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
