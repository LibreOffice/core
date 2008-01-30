/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BigDecimal.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:02:34 $
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
#ifndef _CONNECTIVITY_JAVA_MATH_BIGDECIMAL_HXX_
#define _CONNECTIVITY_JAVA_MATH_BIGDECIMAL_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif
//**************************************************************
//************ Class: java.lang.Boolean
//**************************************************************
namespace connectivity
{
    class java_math_BigDecimal : public java_lang_Object
    {
        // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_math_BigDecimal();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_math_BigDecimal( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}

        java_math_BigDecimal( const ::rtl::OUString& _par0 );
        java_math_BigDecimal( const double& _par0 );
    };
}

#endif // _CONNECTIVITY_JAVA_MATH_BIGDECIMAL_HXX_


