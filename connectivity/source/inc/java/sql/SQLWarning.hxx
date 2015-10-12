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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_SQLWARNING_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_SQLWARNING_HXX

#include "java/sql/SQLException.hxx"

namespace connectivity
{
    //************ Class: java.sql.SQLWarning

    class java_sql_SQLWarning_BASE : public java_sql_SQLException_BASE
    {
    protected:
    // Static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const override;
        virtual ~java_sql_SQLWarning_BASE();
        // A ctor that is needed for returning the object
        java_sql_SQLWarning_BASE( JNIEnv * pEnv, jobject myObj ) : java_sql_SQLException_BASE( pEnv, myObj ){}

    };

    class java_sql_SQLWarning : public java_sql_SQLException
    {
    public:
        java_sql_SQLWarning(const java_sql_SQLWarning_BASE& _rW,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext)
            : java_sql_SQLException(_rW,_rContext) {}
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_SQLWARNING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
