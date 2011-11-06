/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#define THROWDEF_RTE_IAE_NCE    throw(CSS::uno::RuntimeException,CSS::lang::IllegalArgumentException,CSS::sheet::NoConvergenceException)
#define THROW_NCE           throw CSS::sheet::NoConvergenceException()

#define CHK_Freq            ( nFreq != 1 && nFreq != 2 && nFreq != 4 )
#define CHK_FINITE(d)       if( !::rtl::math::isFinite( d ) ) THROW_IAE
#define RETURN_FINITE(d)    if( ::rtl::math::isFinite( d ) ) return d; else THROW_IAE

#endif

