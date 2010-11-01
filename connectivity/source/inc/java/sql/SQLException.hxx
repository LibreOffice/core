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
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLException(){}
        java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext);
    };

    class java_sql_SQLException_BASE :  public java_lang_Exception
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_SQLException_BASE();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj );

        ::rtl::OUString getSQLState() const;
        sal_Int32       getErrorCode() const;
        starsdbc::SQLException getNextException() const;

        static jclass st_getMyClass();
    };

}
#endif // _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
