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

#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/msforms/XReturnInteger.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbahelper.hxx>

namespace ooo
{
    namespace vba
    {
        template< typename T1,  typename T2 >
        class DefaultReturnHelper : public ::cppu::WeakImplHelper< T2, css::script::XDefaultProperty >
        {
            T1 mnValue;
            public:
            DefaultReturnHelper( const T1& nValue ) : mnValue( nValue ) {}
            virtual void  SAL_CALL setValue( T1 nValue ) SAL_OVERRIDE { mnValue = nValue; }
            virtual T1  SAL_CALL getValue() SAL_OVERRIDE { return mnValue; }
            OUString SAL_CALL getDefaultPropertyName(  ) SAL_OVERRIDE { return OUString("Value"); }
        };

        typedef DefaultReturnHelper< sal_Int32, ov::msforms::XReturnInteger > ReturnInteger_BASE;
        class ReturnInteger : public ReturnInteger_BASE
        {
            public:
            ReturnInteger( sal_Int32 nValue ) : ReturnInteger_BASE( nValue ){}
        };

    } // vba
} // ooo

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
