/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Throwable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:03:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_LANG_THROWABLE_HXX_
#define _CONNECTIVITY_JAVA_LANG_THROWABLE_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif

namespace connectivity
{
    //**************************************************************
    //************ Class: java.lang.Throwable
    //**************************************************************
    class java_lang_Throwable : public java_lang_Object
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_lang_Throwable();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_lang_Throwable( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}
        ::rtl::OUString getMessage() const;
        ::rtl::OUString getLocalizedMessage() const;
        virtual ::rtl::OUString toString() const;
    };
}
#endif // _CONNECTIVITY_JAVA_LANG_THROWABLE_HXX_

