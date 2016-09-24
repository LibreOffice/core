/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_TABLINE_HXX
#define INCLUDED_SVX_TABLINE_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/svdpage.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlineit.hxx>
#include <svx/xsetit.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/menubtn.hxx>

class SvxBrushItem;
class SdrModel;
class SdrObject;

//defines for the symbol selection on the page
//Positive value (0,...n) declares the index in pSymbollist, else:
#define SVX_SYMBOLTYPE_NONE       (-3)  //symbol hidden
#define SVX_SYMBOLTYPE_AUTO       (-2)  //symbol automatically generated
#define SVX_SYMBOLTYPE_BRUSHITEM  (-1)  //symbol is contained as graphic in item:
#define SVX_SYMBOLTYPE_UNKNOWN  (-100) //unknown/not initialized

#endif // INCLUDED_SVX_TABLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
