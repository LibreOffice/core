/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dumputils.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:24:33 $
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

#ifndef INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DUMPUTILS_HXX
#define INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DUMPUTILS_HXX \
    INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DUMPUTILS_HXX

namespace rtl { class OString; }
class FileStream;

namespace codemaker { namespace cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType);

bool dumpNamespaceClose(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType);

void dumpTypeIdentifier(FileStream & out, rtl::OString const & registryType);

} }

#endif // INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DUMPUTILS_HXX
