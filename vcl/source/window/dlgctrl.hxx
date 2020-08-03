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

#pragma once

#include <vcl/window.hxx>

vcl::Window* ImplGetChildWindow( vcl::Window* pParent, sal_uInt16 n, sal_uInt16& nIndex, bool bTestEnable );

vcl::Window* ImplFindDlgCtrlWindow( vcl::Window* pParent, vcl::Window* pWindow, sal_uInt16& rIndex,
                               sal_uInt16& rFormStart, sal_uInt16& rFormEnd );

vcl::Window* ImplFindAccelWindow( vcl::Window* pParent, sal_uInt16& rIndex, sal_Unicode cCharCode,
                             sal_uInt16 nFormStart, sal_uInt16 nFormEnd, bool bCheckEnable = true );

sal_Unicode getAccel( const OUString& rStr );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
