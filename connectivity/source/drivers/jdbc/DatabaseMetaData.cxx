/*************************************************************************
 *
 *  $RCSfile: DatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 10:46:14 $
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
#ifndef _CONNECTIVITY_JAVA_SQL_DATABASEMETADATA_HXX_
#include "java/sql/DatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#include "java/sql/Connection.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSET_HXX_
#include "java/sql/ResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_STRING_HXX_
#include "java/lang/String.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//**************************************************************
//************ Class: java.sql.DatabaseMetaData
//**************************************************************

jclass java_sql_DatabaseMetaData::theClass              = 0;

java_sql_DatabaseMetaData::~java_sql_DatabaseMetaData()
{}

jclass java_sql_DatabaseMetaData::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/DatabaseMetaData"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}
// -------------------------------------------------------------------------


void java_sql_DatabaseMetaData::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/ResultSet;";
        char * cMethodName = "getTypeInfo";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/ResultSet;";
        char * cMethodName = "getCatalogs";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getCatalogSeparator(  ) throw(SQLException, RuntimeException)
{
    jstring out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getCatalogSeparator";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        char * cSignature = "()Ljava/sql/ResultSet;";
        char * cMethodName = "getSchemas";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumnPrivileges(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[4];
        // temporaere Variable initialisieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        args[3].l = convertwchar_tToJavaString(t.pEnv,columnNamePattern);

        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTables";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);
            ThrowSQLException(t.pEnv,*this);
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(columnNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumns(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[3];
        // temporaere Variable initialisieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);

        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTables";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(tableNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTables(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[4];
        sal_Int32 len = types.getLength();
        if(len)
        {
            jobjectArray pObjArray = t.pEnv->NewObjectArray((jsize) len, java_lang_String::getMyClass(), 0);
            const ::rtl::OUString* pBegin = types.getConstArray();
            for(sal_Int32 i=0;i<len;i++,++pBegin)
            {
                jstring aT = convertwchar_tToJavaString(t.pEnv,*pBegin);
                //jstring aT = t.pEnv->NewStringUTF(_par3.GetToken(i));
                t.pEnv->SetObjectArrayElement(pObjArray,(jsize)i,aT);
            }
            args[3].l = pObjArray;
        }else
            args[3].l = 0;
        // temporaere Variable initialisieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);

        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTables";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);
            ThrowSQLException(t.pEnv,*this);
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(tableNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            //for(INT16 i=0;i<len;i++)
            t.pEnv->DeleteLocalRef((jobjectArray)args[3].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getProcedureColumns(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[4];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,procedureNamePattern);
        args[3].l = convertwchar_tToJavaString(t.pEnv,columnNamePattern);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getProcedures";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(procedureNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(columnNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getProcedures( const Any&
                catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,procedureNamePattern);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getProcedures";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(procedureNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getVersionColumns(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getVersionColumns";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxBinaryLiteralLength";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxRowSize";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxCatalogNameLength";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxCharLiteralLength";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnNameLength";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnsInIndex";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxCursorNameLength";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxConnections";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnsInTable";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxStatementLength";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxTableNameLength";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxTablesInSelect";
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
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getExportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getExportedKeys";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getImportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getImportedKeys";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getPrimaryKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getPrimaryKeys";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getIndexInfo(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
        sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[5];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        args[3].z = unique;
        args[4].z = approximate;
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)Ljava/sql/ResultSet;";
        char * cMethodName = "getIndexInfo";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,args[3].z,args[4].z );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getBestRowIdentifier(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
        sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);

        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTablePrivileges";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,scope,nullable);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTablePrivileges(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[3];
        // Parameter konvertieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);

        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTablePrivileges";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(tableNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCrossReference(
        const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
        const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
        const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[6];
        // Parameter konvertieren
        args[0].l = primaryCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(primaryCatalog)) : 0;
        args[1].l = primarySchema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,primarySchema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,primaryTable);
        args[3].l = foreignCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(foreignCatalog)) : 0;
        args[4].l = foreignSchema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,foreignSchema);
        args[5].l = convertwchar_tToJavaString(t.pEnv,foreignTable);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getCrossReference";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[2].l,args[2].l,args[3].l,args[4].l,args[5].l );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            if(primaryCatalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(primaryTable.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(foreignCatalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
            if(args[4].l)
                t.pEnv->DeleteLocalRef((jstring)args[4].l);
            if(foreignTable.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[5].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "doesMaxRowSizeIncludeBlobs";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesLowerCaseQuotedIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesLowerCaseIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesMixedCaseQuotedIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesMixedCaseIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesUpperCaseQuotedIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "storesUpperCaseIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsAlterTableWithAddColumn";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsAlterTableWithDropColumn";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxIndexLength";
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
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsNonNullableColumns";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)0;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getCatalogTerm";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)0;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getIdentifierQuoteString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)0;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getExtraNameCharacters";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsDifferentTableCorrelationNames";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::isCatalogAtStart(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isCatalogAtStart";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "dataDefinitionIgnoredInTransactions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "dataDefinitionCausesTransactionCommit";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsDataManipulationTransactionsOnly";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsDataDefinitionAndDataManipulationTransactions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsPositionedDelete";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsPositionedUpdate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOpenStatementsAcrossRollback";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOpenStatementsAcrossCommit";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOpenCursorsAcrossCommit";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOpenCursorsAcrossRollback";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "supportsTransactionIsolationLevel";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, level);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSchemasInDataManipulation";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsANSI92FullSQL";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsANSI92EntryLevelSQL";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsIntegrityEnhancementFacility";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSchemasInIndexDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSchemasInTableDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCatalogsInTableDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCatalogsInIndexDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCatalogsInDataManipulation";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOuterJoins";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/ResultSet;";
        char * cMethodName = "getTableTypes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxStatements";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxProcedureNameLength";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxSchemaNameLength";
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
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsTransactions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "allProceduresAreCallable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsStoredProcedures";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSelectForUpdate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "allTablesAreSelectable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isReadOnly";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "usesLocalFiles";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "usesLocalFilePerTable";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsTypeConversion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "nullPlusNonNullIsNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsColumnAliasing";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsTableCorrelationNames";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)fromType;
        args[1].i = (sal_Int32)toType;
        // temporaere Variable initialisieren
        char * cSignature = "(II)Z";
        char * cMethodName = "supportsConvert";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID, args[0].i,args[0].i );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsExpressionsInOrderBy";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsGroupBy";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsGroupByBeyondSelect";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsGroupByUnrelated";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsMultipleTransactions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsMultipleResultSets";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsLikeEscapeClause";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsOrderByUnrelated";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsUnion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsUnionAll";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsMixedCaseIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsMixedCaseQuotedIdentifiers";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "nullsAreSortedAtEnd";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "nullsAreSortedAtStart";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "nullsAreSortedHigh";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "nullsAreSortedLow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSchemasInProcedureCalls";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSchemasInPrivilegeDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCatalogsInProcedureCalls";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCatalogsInPrivilegeDefinitions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCorrelatedSubqueries";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSubqueriesInComparisons";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSubqueriesInExists";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSubqueriesInIns";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsSubqueriesInQuantifieds";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsANSI92IntermediateSQL";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{

    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getURL";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getUserName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getDriverName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getDriverVersion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getDatabaseProductVersion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getDatabaseProductName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getProcedureTerm";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getSchemaTerm";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getDriverMajorVersion";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getDefaultTransactionIsolation";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getDriverMinorVersion";
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
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getSQLKeywords";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getSearchStringEscape";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getStringFunctions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getTimeDateFunctions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getSystemFunctions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    jstring out = (jstring)NULL;
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getNumericFunctions";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsExtendedSQLGrammar";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsCoreSQLGrammar";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsMinimumSQLGrammar";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsFullOuterJoins";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    jboolean out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsLimitedOuterJoins";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnsInGroupBy";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =           t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnsInOrderBy";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxColumnsInSelect";
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
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    jint out;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMaxUserNameLength";
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
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "supportsResultSetType";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(II)Z";
        char * cMethodName = "supportsResultSetConcurrency";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType,concurrency);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "ownUpdatesAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "ownDeletesAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "ownInsertsAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "othersUpdatesAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "othersDeletesAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "othersInsertsAreVisible";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "updatesAreDetected";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "deletesAreDetected";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "insertsAreDetected";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID,setType);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "supportsBatchUpdates";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out =   t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getUDTs(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern,
        const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[4];
        // temporaere Variable initialisieren
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,connectivity::getString(catalog)) : 0;
        args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
        args[2].l = convertwchar_tToJavaString(t.pEnv,typeNamePattern);
        jintArray pArray = t.pEnv->NewIntArray(types.getLength());
        t.pEnv->SetIntArrayRegion(pArray,0,types.getLength(),(jint*)types.getConstArray());
        args[3].l = pArray;

        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        char * cMethodName = "getTables";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);
            ThrowSQLException(t.pEnv,*this);
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(schemaPattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(typeNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(args[3].l)
                t.pEnv->DeleteLocalRef((jintArray)args[3].l);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_DatabaseMetaData::getConnection(  ) throw(SQLException, RuntimeException)
{
//  jobject out(0);
//  SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
//  if( t.pEnv ){
//      char * cSignature = "()Ljava/sql/Connection;";
//      char * cMethodName = "getConnection";
//      // Java-Call absetzen
//      jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
//      if( mID )
//      {
//          out = t.pEnv->CallObjectMethod( object, mID);
//          ThrowSQLException(t.pEnv,*this);
//      } //mID
//  } //t.pEnv
//  // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
//  return out ? new java_sql_Connection( t.pEnv, out,this ) : 0;
        return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

