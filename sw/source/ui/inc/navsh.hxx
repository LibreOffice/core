/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UI_INC_NAVSH_HXX
#define INCLUDED_SW_SOURCE_UI_INC_NAVSH_HXX

#include "frmsh.hxx"

class SfxItemSet;

class SwNavigationShell: public SwBaseShell
{
public:
  SFX_DECL_INTERFACE(SW_NAVIGATIONSHELL)

  SwNavigationShell(SwView &rView);

  void  GetState(SfxItemSet &);
  void  Execute(SfxRequest &);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
