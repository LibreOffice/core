/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Copyright (C) 2012, Timothy Pearson <kb9vqf@pearsoncomputing.net>
 */

#ifndef INCLUDED_VCL_TDE_HEADERS_H
#define INCLUDED_VCL_TDE_HEADERS_H

/* ********* Suppress warnings if needed */
#include "sal/config.h"

#include <cstddef>

#if defined __GNUC__
#pragma GCC system_header
#endif


/* ********* Hack, but needed because of conflicting types... */
#define Region TQtXRegion


/* ********* TQt headers */
#include <tqaccessible.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqfont.h>
#include <tqframe.h>
#include <tqlineedit.h>
#include <tqlistview.h>
#include <tqmainwindow.h>
#include <tqmenudata.h>
#include <tqpaintdevice.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqrangecontrol.h>
#include <tqstring.h>
#include <tqtabbar.h>
#include <tqtabwidget.h>
#include <tqtoolbar.h>
#include <tqtoolbutton.h>
#include <tqwidget.h>
#include <tqprogressbar.h>

/* ********* See hack on top of this file */
#undef Region


/* ********* TDE base headers */
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeconfig.h>
#include <tdeversion.h>
#include <tdeemailsettings.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <tdemainwindow.h>
#include <tdemenubar.h>
#include <tdeprotocolmanager.h>
#include <tdestartupinfo.h>
#include <tdestyle.h>


/* ********* TDE address book connectivity headers */
#include <tdeabc/addressbook.h>
#include <tdeabc/addressee.h>
#include <tdeabc/field.h>
#include <tdeabc/stdaddressbook.h>

#include "tde_defines.h"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
