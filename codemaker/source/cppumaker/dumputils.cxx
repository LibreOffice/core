/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dumputils.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:24:21 $
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

#include "dumputils.hxx"

#include "codemaker/global.hxx"
#include "codemaker/commoncpp.hxx"

#include "rtl/ustring.hxx"
#include "sal/types.h"


namespace codemaker { namespace cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            rtl::OString id(registryType.getToken(0, '/', i));
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "namespace " << id << " {";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

bool dumpNamespaceClose(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            i = registryType.indexOf('/', i);
            if (i >= 0) {
                ++i;
            }
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "}";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

void dumpTypeIdentifier(FileStream & out, rtl::OString const & registryType) {
    out << registryType.copy(registryType.lastIndexOf('/') + 1);
}

} }
