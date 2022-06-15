/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <xcb/xcb.h>

#include <QtCore/QAbstractNativeEventFilter>

class QtXcbEventFilter : public QAbstractNativeEventFilter
{
    static constexpr const char* m_sFrameAtomName = "_NET_FRAME_EXTENTS\0";
    static xcb_atom_t m_nFrameAtom;
    static bool m_bDidAtomLookups;

    static void fetchAtoms();

public:
    static xcb_atom_t lookupAtom(xcb_connection_t*, const char* const sAtomName);

    bool nativeEventFilter(const QByteArray& eventType, void* message, long*) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
