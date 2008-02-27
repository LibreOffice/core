/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdlinebits.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:11:49 $
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

#ifndef CMDLINEBITS_HXX
#define CMDLINEBITS_HXX

#include <sal/types.h>

typedef sal_uInt32 CmdLineBits;
const char* getForwardString();

#include <stdarg.h>

enum T_Print_Params {
    T_INFO = 1,
    T_VERBOSE,
    T_DEBUG,
    T_ERROR
};

// void t_print(const char*);
void t_print(const char*, ...);
void t_print(T_Print_Params, const char*, ...);

#endif
