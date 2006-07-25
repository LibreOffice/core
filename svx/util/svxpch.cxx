/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxpch.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:30:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
#include <thread.hxx>
#include <sysdep.hxx>
#if defined(WNT) || defined (WIN)
#include <svwin.h>
#endif
#include <tlintl.hxx>
#include <tlfsys.hxx>
#include <tlbigint.hxx>

#include <sysdep.hxx>
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
#include <svxenum.hxx>
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

