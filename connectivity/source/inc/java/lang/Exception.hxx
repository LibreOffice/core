/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Exception.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _CONNECTIVITY_JAVA_LANG_EXCEPTION_HXX_
#define _CONNECTIVITY_JAVA_LANG_EXCEPTION_HXX_

#include "java/lang/Throwable.hxx"

namespace connectivity
{

    //**************************************************************
    //************ Class: java.lang.Exception
    //**************************************************************
    class java_lang_Exception : public java_lang_Throwable{
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_lang_Exception();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_lang_Exception( JNIEnv * pEnv, jobject myObj ) : java_lang_Throwable( pEnv, myObj ){}

    };
}
#endif // _CONNECTIVITY_JAVA_LANG_EXCEPTION_HXX_

