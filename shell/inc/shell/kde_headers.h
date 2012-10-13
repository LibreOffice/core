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

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#define INCLUDED_VCL_KDE_HEADERS_H

#ifdef ENABLE_TDE

#include "shell/tde_headers.h"

#else // ENABLE_TDE

/* ********* Suppress warnings if needed */
#include "sal/config.h"

#include <cstddef>

#if defined __GNUC__
#pragma GCC system_header
#endif


/* ********* Hack, but needed because of conflicting types... */
#define Region QtXRegion


/* ********* Qt headers */
#include <qaccessible.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qmenudata.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qrangecontrol.h>
#include <qstring.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qprogressbar.h>

/* ********* See hack on top of this file */
#undef Region


/* ********* KDE base headers */
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kemailsettings.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kprotocolmanager.h>
#include <kstartupinfo.h>
#include <kstyle.h>


/* ********* KDE address book connectivity headers */
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/field.h>
#include <kabc/stdaddressbook.h>

#endif // ENABLE_TDE

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
