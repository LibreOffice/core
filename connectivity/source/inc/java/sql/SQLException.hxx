/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SQLException.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:23:28 $
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
#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_EXCEPTION_HXX_
#include "java/lang/Exception.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif

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
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_sql_SQLException_BASE();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj );

        ::rtl::OUString getSQLState() const;
        sal_Int32       getErrorCode() const;
        starsdbc::SQLException getNextException() const;
    };

}
#endif // _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

