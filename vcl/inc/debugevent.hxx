/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/timer.hxx>
#include <sal/types.h>

namespace vcl { class Window; }

class DebugEventInjector final : private Timer {
  sal_uInt32 mnEventsLeft;
  DebugEventInjector( sal_uInt32 nMaxEvents );

  static vcl::Window *ChooseWindow();
  static void InjectTextEvent();
  static void InjectMenuEvent();
  static void InjectEvent();
  static void InjectKeyNavEdit();
  virtual void Invoke() override;

  public:
     static DebugEventInjector *getCreate();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
