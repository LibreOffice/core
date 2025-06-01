/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtX11Support.hxx>

#include <QtCore/QVersionNumber>

#include <QtTools.hxx>

#if CHECK_QT5_USING_X11
#include <QtX11Extras/QX11Info>
#endif

#include <unx/gensys.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
