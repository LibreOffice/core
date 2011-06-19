/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/sql/SQLException.hxx"
#include "java/tools.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//**************************************************************
//************ Class: java.sql.SQLException
//**************************************************************
java_sql_SQLException::java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const Reference< XInterface> & _rContext)
    : starsdbc::SQLException(   _rException.getMessage(),
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

jclass java_sql_SQLException_BASE::theClass = 0;

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

starsdbc::SQLException java_sql_SQLException_BASE::getNextException()  const
{
    SDBThreadAttach t;
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getNextException","()Ljava/sql/SQLException;", mID);
    // WARNING: the caller will become the owner of the returned pointers !!!
    if( out )
    {
        java_sql_SQLException_BASE  warn_base(t.pEnv,out);
        return (starsdbc::SQLException)java_sql_SQLException(warn_base,0);
    }

    return starsdbc::SQLException();
}

::rtl::OUString java_sql_SQLException_BASE::getSQLState() const
{
    static jmethodID mID(NULL);
    return callStringMethod("getSQLState",mID);
}
sal_Int32 java_sql_SQLException_BASE::getErrorCode() const
{
    static jmethodID mID(NULL);
    return callIntMethod("getErrorCode",mID);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
