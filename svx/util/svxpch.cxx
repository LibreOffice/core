/*************************************************************************
 *
 *  $RCSfile: svxpch.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-30 11:46:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#include "segmentc.hxx"

