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

#pragma once

#include <java/lang/Object.hxx>
#include <com/sun/star/sdbc/XClob.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{


    //************ Class: java.sql.SQLWarning

    class java_sql_Clob :   public java_lang_Object,
                            public ::cppu::WeakImplHelper< css::sdbc::XClob>
    {
    protected:
    // Static data for the class
        static jclass theClass;
        virtual ~java_sql_Clob() override;
    public:
        virtual jclass getMyClass() const override;

        // A ctor that is needed for returning the object
        java_sql_Clob( JNIEnv * pEnv, jobject myObj );

        // XClob
        virtual sal_Int64 SAL_CALL length(  ) override;
        virtual OUString SAL_CALL getSubString( sal_Int64 pos, sal_Int32 length ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream(  ) override;
        virtual sal_Int64 SAL_CALL position( const OUString& searchstr, sal_Int32 start ) override;
        virtual sal_Int64 SAL_CALL positionOfClob( const css::uno::Reference< css::sdbc::XClob >& pattern, sal_Int64 start ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
