/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 * Timothy Pearson <kb9vqf@pearsoncomputing.net> (C) 2012, All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 *
 ************************************************************************/

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
