/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QSize>

class QWidget;

// Shared top-level window geometry.
QSize defaultWindowMinimumSize();
QSize defaultWindowSize();
void centerOnWorkArea(QWidget* window);

// Un-minimizes and raises; activation is best effort.
void surfaceWindow(QWidget* window);

// Ctrl+Q closes every window (each with its save round-trip), quitting the app.
void installQuitShortcut(QWidget* window);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
