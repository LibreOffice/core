/*************************************************************************
 *
 *  $RCSfile: SQLException.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#include "java/sql/SQLException.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
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


jclass java_sql_SQLException_BASE::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/sql/SQLException");
        OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        if(!tempClass)
        {
            t.pEnv->ExceptionDescribe();
            t.pEnv->ExceptionClear();
        }
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_SQLException_BASE::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

starsdbc::SQLException java_sql_SQLException_BASE::getNextException()  const
{
    jobject out = NULL;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/Exception;";
        char * cMethodName = "getNextException";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? (starsdbc::SQLException)java_sql_SQLException(java_sql_SQLException_BASE(t.pEnv,out),0) : starsdbc::SQLException();
}

::rtl::OUString java_sql_SQLException_BASE::getSQLState() const
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getSQLState";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring) t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
sal_Int32 java_sql_SQLException_BASE::getErrorCode() const
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getErrorCode";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

