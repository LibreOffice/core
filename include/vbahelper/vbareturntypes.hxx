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
#ifndef INCLUDED_VBAHELPER_VBARETURNTYPES_HXX
#define INCLUDED_VBAHELPER_VBARETURNTYPES_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <ooo/vba/msforms/XReturnInteger.hpp>
#include <ooo/vba/msforms/XReturnBoolean.hpp>
#include <ooo/vba/msforms/XReturnSingle.hpp>
#include <ooo/vba/msforms/XReturnEffect.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace ooo
{
    namespace vba
    {
        template< typename T1,  typename T2 >
        class DefaultReturnHelper : public ::cppu::WeakImplHelper2< T2, css::script::XDefaultProperty >
        {
            T1 mnValue;
            public:
            DefaultReturnHelper( const T1& nValue ) : mnValue( nValue ) {}
            virtual void  SAL_CALL setValue( T1 nValue ) throw (::com::sun::star::uno::RuntimeException) { mnValue = nValue; }
            virtual T1  SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException) { return mnValue; }
            OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return OUString("Value"); }
        };

        typedef DefaultReturnHelper< sal_Int32, ov::msforms::XReturnInteger > ReturnInteger_BASE;
        class ReturnInteger : public ReturnInteger_BASE
        {
            public:
            ReturnInteger( sal_Int32 nValue ) : ReturnInteger_BASE( nValue ){}
        };

        typedef DefaultReturnHelper< sal_Bool, ov::msforms::XReturnBoolean > ReturnBoolean_BASE;
        class ReturnBoolean : public ReturnBoolean_BASE
        {
            public:
            ReturnBoolean( bool bValue ) : ReturnBoolean_BASE( bValue ){}
        };
        typedef DefaultReturnHelper< float, ov::msforms::XReturnSingle > ReturnSingle_BASE;
        class ReturnSingle : public ReturnSingle_BASE
        {
            public:
            ReturnSingle( float nValue ) : ReturnSingle_BASE( nValue ){}
        };
        typedef DefaultReturnHelper< short, ov::msforms::XReturnEffect > ReturnEffect_BASE;
        class ReturnEffect : public ReturnEffect_BASE
        {
            public:
            ReturnEffect( short nValue ) : ReturnEffect_BASE( nValue ){}
        };
    } // vba
} // ooo

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
