/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: analysisdefs.hxx,v $
 * $Revision: 1.8 $
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

#ifndef ANALYSISDEFS_HXX
#define ANALYSISDEFS_HXX

#define CSS                 ::com::sun::star
#define XPROPSET            CSS::beans::XPropertySet
#define REF(c)              CSS::uno::Reference< c >
#define constREFXPS         const REF(XPROPSET)
#define SEQ(c)              CSS::uno::Sequence< c >
#define SEQSEQ(c)           CSS::uno::Sequence< CSS::uno::Sequence< c > >
#define SEQofLocName        SEQ( CSS::sheet::LocalizedName )
#define ANY                 CSS::uno::Any
#define SEQ_ANY             SEQ(ANY)
#define STRING              ::rtl::OUString
#define STRFROMASCII(s)     STRING::createFromAscii( s )
#define STRFROMANSI(s)      STRING( s, strlen( s ), RTL_TEXTENCODING_MS_1252 )
#define THROWDEF_RTE        throw(CSS::uno::RuntimeException)
#define THROW_RTE           throw CSS::uno::RuntimeException()
#define THROWDEF_RTE_IAE    throw(CSS::uno::RuntimeException,CSS::lang::IllegalArgumentException)
#define THROW_IAE           throw CSS::lang::IllegalArgumentException()

#define CHK_Freq            ( nFreq != 1 && nFreq != 2 && nFreq != 4 )
#define CHK_FINITE(d)       if( !::rtl::math::isFinite( d ) ) THROW_IAE
#define RETURN_FINITE(d)    if( ::rtl::math::isFinite( d ) ) return d; else THROW_IAE

#endif

