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
#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

#include "java/lang/Exception.hxx"
#include <com/sun/star/sdbc/SQLException.hpp>

namespace connectivity
{
    namespace starsdbc  = ::com::sun::star::sdbc;
    //**************************************************************
    //************ Class: java.sql.SQLException
    //**************************************************************
    class java_sql_SQLException_BASE;
    class java_sql_SQLException :   public starsdbc::SQLException
    {
    public:
        // A ctor that is needed for returning the object
        java_sql_SQLException(){}
        java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext);
    };

    class java_sql_SQLException_BASE :  public java_lang_Exception
    {
    protected:
    // Static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_SQLException_BASE();
        // A ctor that is needed for returning the object
        java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj );

        OUString getSQLState() const;
        sal_Int32       getErrorCode() const;
        starsdbc::SQLException getNextException() const;

        static jclass st_getMyClass();
    };

}
#endif // _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
