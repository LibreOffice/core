/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnings_guard_unicode_brkiter.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:12:37 $
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

#ifndef INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H
#define INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H

// Because the GCC system_header mechanism doesn't work in .c/.cxx compilation
// units and more important affects the rest of the current include file, the
// warnings guard is separated into this header file on its own.

// External unicode includes (from icu) cause warning C4668 on Windows.
// We want to minimize the patches to external headers, so the warnings are
// disabled here instead of in the header file itself.
#ifdef _MSC_VER
#pragma warning(push, 1)
#elif defined __GNUC__
#pragma GCC system_header
#endif
#include <unicode/brkiter.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H
