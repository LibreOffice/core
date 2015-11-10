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

#include "java/sql/SQLException.hxx"
#include "java/tools.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//************ Class: java.sql.SQLException

java_sql_SQLException::java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const Reference< XInterface> & _rContext)
    : css::sdbc::SQLException(   _rException.getMessage(),
                                _rContext,
                                _rException.getSQLState(),
                                _rException.getErrorCode(),
                                makeAny(_rException.getNextException())
                            )
{
}

java_sql_SQLException_BASE::java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj ) : java_lang_Exception( pEnv, myObj )
{
}

jclass java_sql_SQLException_BASE::theClass = nullptr;

java_sql_SQLException_BASE::~java_sql_SQLException_BASE()
{}


jclass java_sql_SQLException_BASE::getMyClass() const
{
    return st_getMyClass();
}
jclass java_sql_SQLException_BASE::st_getMyClass()
{
    // The class needs to be fetched just once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/SQLException");
    return theClass;
}

css::sdbc::SQLException java_sql_SQLException_BASE::getNextException()  const
{
    SDBThreadAttach t;
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getNextException","()Ljava/sql/SQLException;", mID);
    // WARNING: the caller will become the owner of the returned pointers !!!
    if( out )
    {
        java_sql_SQLException_BASE  warn_base(t.pEnv,out);
        return css::sdbc::SQLException(java_sql_SQLException(warn_base,nullptr));
    }

    return css::sdbc::SQLException();
}

OUString java_sql_SQLException_BASE::getSQLState() const
{
    static jmethodID mID(nullptr);
    return callStringMethod("getSQLState",mID);
}
sal_Int32 java_sql_SQLException_BASE::getErrorCode() const
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowSQL("getErrorCode", mID);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
