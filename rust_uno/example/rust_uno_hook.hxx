/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

extern "C" {
/// Function from the Rust UNO binding library that we'll call from LibreOffice
void run_rust_uno_test();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
