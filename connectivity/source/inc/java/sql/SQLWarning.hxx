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

#ifndef _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_

#include "java/sql/SQLException.hxx"

namespace connectivity
{
    //  namespace starsdbc  = ::com::sun::star::sdbc;
    //  namespace ::com::sun::star::uno = ::com::sun::star::uno;
    //**************************************************************
    //************ Class: java.sql.SQLWarning
    //**************************************************************
    class java_sql_SQLWarning_BASE : public java_sql_SQLException_BASE
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_SQLWarning_BASE();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLWarning_BASE( JNIEnv * pEnv, jobject myObj ) : java_sql_SQLException_BASE( pEnv, myObj ){}

    };

    class java_sql_SQLWarning : public java_sql_SQLException
    {
    public:
        java_sql_SQLWarning(const java_sql_SQLWarning_BASE& _rW,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext)
            : java_sql_SQLException(_rW,_rContext) {}
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
