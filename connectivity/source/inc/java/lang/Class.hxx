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
#ifndef _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
#define _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
//**************************************************************
//************ Class: java.lang.Class
//**************************************************************
#include "java/lang/Object.hxx"

namespace connectivity
{
    class java_lang_Class : public java_lang_Object
    {
    protected:
    // static Data for the Class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_lang_Class();
        // a Constructor, that is needed for when Returning the Object is needed:
        java_lang_Class( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}

        static java_lang_Class * forName( const ::rtl::OUString &_par0 );
        // return the jre object
        jobject newInstanceObject();

    };
}

#endif // _CONNECTIVITY_JAVA_LANG_CLASS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
