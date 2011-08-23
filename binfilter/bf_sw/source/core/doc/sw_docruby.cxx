/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <string.h>			// fuer strchr()


#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HDL
#include <com/sun/star/i18n/UnicodeType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _RUBYLIST_HXX
#include <rubylist.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
namespace binfilter {

/*N*/ SV_IMPL_PTRARR( SwRubyList, SwRubyListEntryPtr )

using namespace ::com::sun::star::i18n;

/*N*/SwRubyListEntry::~SwRubyListEntry()
/*N*/{
/*N*/}
}
