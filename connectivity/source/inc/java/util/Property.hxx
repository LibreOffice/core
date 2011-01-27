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
#ifndef CONNECTIVITY_java_util_Properties
#define CONNECTIVITY_java_util_Properties

#include "java/lang/Object.hxx"

namespace connectivity
{
    class java_util_Properties : public java_lang_Object
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_util_Properties();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_util_Properties( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}
        java_util_Properties( );
        void setProperty(const ::rtl::OUString key, const ::rtl::OUString& value);
    };
}

#endif // CONNECTIVITY_java_util_Properties

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
