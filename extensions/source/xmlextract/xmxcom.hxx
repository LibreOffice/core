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



#ifndef _XMXCOM_HXX
#define _XMXCOM_HXX

#include <cppuhelper/weak.hxx>
#ifndef __RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XXMLExtractor.hpp>

// -----------------------------------------------------------------------------

#define NMSP_CPPU       ::cppu
#define NMSP_RTL        ::rtl
#define NMSP_UNO        ::com::sun::star::uno
#define NMSP_LANG       ::com::sun::star::lang
#define NMSP_IO         ::com::sun::star::io
#define NMSP_REGISTRY   ::com::sun::star::registry


#define REF( _def_Obj )                     NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )                     NMSP_UNO::Sequence< _def_Obj >
#define ANY                                 NMSP_UNO::Any
#define B2UCONST( _def_pChar )              (NMSP_RTL::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))

#endif // _XMXCOM_HXX
