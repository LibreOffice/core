/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ANALYSISDEFS_HXX
#define ANALYSISDEFS_HXX

#define XPROPSET            ::com::sun::star::beans::XPropertySet
#define REF(c)              ::com::sun::star::uno::Reference< c >
#define constREFXPS         const REF(XPROPSET)
#define SEQ(c)              ::com::sun::star::uno::Sequence< c >
#define SEQSEQ(c)           ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< c > >
#define SEQofLocName        SEQ( ::com::sun::star::sheet::LocalizedName )
#define ANY                 ::com::sun::star::uno::Any
#define SEQ_ANY             SEQ(ANY)
#define STRFROMASCII(s)     OUString::createFromAscii( s )
#define STRFROMANSI(s)      OUString( s, strlen( s ), RTL_TEXTENCODING_MS_1252 )
#define THROWDEF_RTE        throw(::com::sun::star::uno::RuntimeException)
#define THROW_RTE           throw ::com::sun::star::uno::RuntimeException()
#define THROWDEF_RTE_IAE    throw(::com::sun::star::uno::RuntimeException,::com::sun::star::lang::IllegalArgumentException)
#define THROW_IAE           throw ::com::sun::star::lang::IllegalArgumentException()
#define THROWDEF_RTE_IAE_NCE    throw(::com::sun::star::uno::RuntimeException,::com::sun::star::lang::IllegalArgumentException,::com::sun::star::sheet::NoConvergenceException)

#define CHK_Freq            ( nFreq != 1 && nFreq != 2 && nFreq != 4 )
#define CHK_FINITE(d)       if( !::rtl::math::isFinite( d ) ) THROW_IAE
#define RETURN_FINITE(d)    if( ::rtl::math::isFinite( d ) ) return d; else THROW_IAE

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
