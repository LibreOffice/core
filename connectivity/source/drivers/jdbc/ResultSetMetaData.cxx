/*************************************************************************
 *
 *  $RCSfile: ResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 17:03:18 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSETMETADATA_HXX_
#include "java/sql/ResultSetMetaData.hxx"
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
//************ Class: java.sql.ResultSetMetaData
//**************************************************************

jclass java_sql_ResultSetMetaData::theClass = 0;

java_sql_ResultSetMetaData::~java_sql_ResultSetMetaData()
{}

jclass java_sql_ResultSetMetaData::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/ResultSetMetaData"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}
// -------------------------------------------------------------------------

void java_sql_ResultSetMetaData::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getColumnDisplaySize";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID,column);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getColumnType";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID,column);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getColumnCount";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isCaseSensitive";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            out = (sal_Bool)t.pEnv->CallIntMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getSchemaName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getColumnName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getTableName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getCatalogName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getColumnTypeName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getColumnLabel";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getColumnClassName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isCurrency";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            out = (sal_Bool)t.pEnv->CallIntMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isAutoIncrement";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            out = (sal_Bool)t.pEnv->CallIntMethod( object, mID, column);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isSigned";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            out = (sal_Bool)t.pEnv->CallIntMethod( object, mID, column);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getPrecision";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getScale";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)column;
        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "isNullable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isSearchable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isReadOnly";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isDefinitelyWritable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "isWritable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

