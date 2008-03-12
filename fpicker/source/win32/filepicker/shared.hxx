/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shared.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:35:35 $
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

#ifndef FPICKER_WIN32_FILEPICKER_SHARED_HXX
#define FPICKER_WIN32_FILEPICKER_SHARED_HXX

#include <rtl/ustring.hxx>

const ::rtl::OUString BACKSLASH          = ::rtl::OUString::createFromAscii( "\\" );
const ::rtl::OUString FILTER_SEPARATOR   = ::rtl::OUString::createFromAscii( "------------------------------------------" );
const ::rtl::OUString ALL_FILES_WILDCARD = ::rtl::OUString::createFromAscii( "*.*" );
const ::sal_Bool      ALLOW_DUPLICATES   = sal_True;

#endif