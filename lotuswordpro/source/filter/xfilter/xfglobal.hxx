/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Global functions or include files that are needed.
 * It's all for the full filter.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 ************************************************************************/
#ifndef		_GLOBAL_HXX
#define		_GLOBAL_HXX

#include	<cassert>

#ifndef _SAL_TYPES_H
#include	"sal/types.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include	<rtl/ustring.hxx>
#endif

#ifndef	_IXFOBJECT_HXX
#include	"ixfobject.hxx"
#endif

#ifndef	_IXFSTREAM_HXX
#include	"ixfstream.hxx"
#endif

#ifndef _IXFATTRLIST_HXX
#include	"ixfattrlist.hxx"
#endif

#ifndef _XFUTIL_HXX
#include	"xfutil.hxx"
#endif

#ifndef	_XFDEFS_HXX
#include	"xfdefs.hxx"
#endif

#define PI 3.1415926
#define	FLOAT_MIN	0.001
#define MAX2(a,b)    (((a) > (b)) ? (a) : (b))
#define MAX3(a,b,c)	MAX2(a,MAX2(b,c))
#define MIN2(a,b)    (((a) < (b)) ? (a) : (b))
#define MIN3(a,b,c)	MIN2(a,MIN2(b,c))
#define	FABS(f)		(f>0?f:-f)

void	XFGlobalReset();

/**
 * @brief
 * Global object, it manages all global variables for the full filter.
 * Before loading a file, we'll need to reset all global variables.
 * XFGlobal::Reset can do this for you.
 */
class	XFGlobal
{
public:
    /**
     * @descr	Generate a name for the section.
     */
    static rtl::OUString	GenSectionName();

    /**
     * @descr	Gen a name for the frame.
     */
    static rtl::OUString	GenFrameName();

    /**
     * @descr	Generate a name for graphics.
     */
    static rtl::OUString	GenGraphName();

    /**
     * @descr	Generate a name for a table.
     */
    static rtl::OUString	GenTableName();

    /**
     * @descr	Generate a name for a note.
     */
    static rtl::OUString	GenNoteName();

    /**
     * @descr	Generate a name for the stroke style.
     */
    static rtl::OUString	GenStrokeDashName();

    /**
     * @descr	Generate a name for the area fill style.
     */
    static rtl::OUString	GenAreaName();

    /**
     * @descr	Generate a name for a ole obejct.
     */
    static rtl::OUString	GenObjName();

    /**
     * @descr	Generate a name for an image object
     */
    static rtl::OUString	GenImageName();

    /**
     * @descr	Reset all global variables.
     */
    static void	Reset();

public:
    static int	s_nSectionID;
    static int	s_nFrameID;
    static int	s_nGraphID;
    static int	s_nTableID;
    static int	s_nNoteID;
    static int	s_nStrokeDashID;
    static int	s_nAreaID;
    static int s_nObjID;
    static int s_nImageID;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
