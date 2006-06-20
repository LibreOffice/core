/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commoncpp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:22:06 $
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

#ifndef INCLUDED_CODEMAKER_COMMONCPP_HXX
#define INCLUDED_CODEMAKER_COMMONCPP_HXX

#ifndef INCLUDED_CODEMAKER_CODEMAKER_HXX
#include "codemaker/codemaker.hxx"
#endif

namespace codemaker { namespace cpp {

rtl::OString typeToPrefix(TypeManager const & manager, rtl::OString const & type);

rtl::OString scopedCppName(rtl::OString const & type, bool bNoNameSpace=false,
                           bool shortname=false);

rtl::OString translateUnoToCppType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & nucleus, bool shortname);

enum IdentifierTranslationMode {
    ITM_GLOBAL,
    ITM_NONGLOBAL,
    ITM_KEYWORDSONLY
};

rtl::OString translateUnoToCppIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix,
    IdentifierTranslationMode transmode = ITM_GLOBAL,
    rtl::OString const * forbidden = 0);

} }

#endif // INCLUDED_CODEMAKER_COMMONCPP_HXX
