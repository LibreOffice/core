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

#include <thread.hxx>
#include <sysdep.hxx>
#if defined(WNT)
#include <svwin.h>
#endif
#include <tlintl.hxx>
#include <tlfsys.hxx>
#include <tlbigint.hxx>

#include <sv.hxx>
#include <svtool.hxx>
#define _ANIMATION
#include <svgraph.hxx>
#include <svsystem.hxx>
#include <svcontnr.hxx>
#include <sfx.hxx>
#include <sfxitems.hxx>
#include <sfxipool.hxx>
#include <sfxiiter.hxx>
#include <sfxdoc.hxx>
#include <sfxview.hxx>
#include <sfxdlg.hxx>
#include <sfxstyle.hxx>
#include <editeng/svxenum.hxx>
#include <sbx.hxx>
#include <hmwrap.hxx>
#include <mail.hxx>
#include <urlobj.hxx>
#include <inet.hxx>
#include <inetui.hxx>
#include <svtruler.hxx>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sfx.hrc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
