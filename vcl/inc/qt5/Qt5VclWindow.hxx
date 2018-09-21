/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>

#include <QtCore/QObject>

#include <vcl/window.hxx>

class Qt5Frame;
class Qt5Widget;

// Wrapper class to hold a vcl::Window while being able to pass it as a QObject
class VCLPLUG_QT5_PUBLIC Qt5VclWindow : public QObject
{
    Q_OBJECT

public:
    Qt5VclWindow(vcl::Window* pWindow);
    VclPtr<vcl::Window> m_pWindow;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
