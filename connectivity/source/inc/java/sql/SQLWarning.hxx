/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SQLWarning.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:23:44 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_

#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#include "java/sql/SQLException.hxx"
#endif
//#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
//#include <com/sun/star/sdbc/SQLWarning.hpp>
//#endif

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
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
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

