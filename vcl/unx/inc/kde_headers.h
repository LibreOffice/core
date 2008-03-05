/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kde_headers.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:50:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#define INCLUDED_VCL_KDE_HEADERS_H

/* ********* Suppress warnings if needed */
#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

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
