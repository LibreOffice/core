/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kde_headers.h,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#define INCLUDED_VCL_KDE_HEADERS_H

/* ********* Suppress warnings if needed */
#include "sal/config.h"

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


#endif
