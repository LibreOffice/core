/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:34:02 $
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
#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#include "TPrivilegesResultSet.hxx"
#ifndef CONNECTIVITY_DIAGNOSE_EX_H
#include "diagnose_ex.h"
#endif

using namespace ::comphelper;

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
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_DatabaseMetaData::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/DatabaseMetaData"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}
// -----------------------------------------------------------------------------
java_sql_DatabaseMetaData::java_sql_DatabaseMetaData( JNIEnv * pEnv, jobject myObj,java_sql_Connection* _pConnection )
    :ODatabaseMetaDataBase(_pConnection)
    ,java_lang_Object( pEnv, myObj )
    ,m_pConnection(_pConnection)
{
    SDBThreadAttach::addRef();
}

// -------------------------------------------------------------------------
void java_sql_DatabaseMetaData::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getTypeInfo";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getCatalogs";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getCatalogSeparator";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getSchemas";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getColumnPrivileges";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args[4];
            // temporaere Variable initialisieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            args[3].l = convertwchar_tToJavaString(t.pEnv,columnNamePattern);

            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);

            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(columnNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getColumns";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args[4];
            // temporaere Variable initialisieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);
            args[3].l = convertwchar_tToJavaString(t.pEnv,columnNamePattern);

            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);

            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(args[2].l)
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(args[3].l)
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){


        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getTables";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        OSL_VERIFY_RES( !isExceptionOccured(t.pEnv,sal_True),"Exception occured!");
        if( mID )
        {
            jvalue args[4];
            sal_Int32 len = types.getLength();
            if(len)
            {
                jobjectArray pObjArray = static_cast<jobjectArray>(t.pEnv->NewObjectArray((jsize) len, java_lang_String::getMyClass(), 0));
                OSL_VERIFY_RES( !isExceptionOccured(t.pEnv,sal_True),"Exception occured!");

                const ::rtl::OUString* pBegin = types.getConstArray();
                for(sal_Int32 i=0;i<len;i++,++pBegin)
                {
                    jstring aT = convertwchar_tToJavaString(t.pEnv,*pBegin);
                    //jstring aT = t.pEnv->NewStringUTF(_par3.GetToken(i));
                    t.pEnv->SetObjectArrayElement(pObjArray,(jsize)i,aT);
                    OSL_VERIFY_RES( !isExceptionOccured(t.pEnv,sal_True),"Exception occured!");

                }
                args[3].l = pObjArray;
            }else
                args[3].l = 0;
            // temporaere Variable initialisieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);
            jthrowable jThrow = t.pEnv->ExceptionOccurred();
            if ( jThrow )
                t.pEnv->ExceptionClear();// we have to clear the exception here because we want to handle it itself

            sal_Bool bExcepOccured;

            if(catalog.hasValue())
            {
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
                bExcepOccured = isExceptionOccured(t.pEnv,sal_True); OSL_ENSURE(!bExcepOccured,"Exception occured!");
            }
            if(args[1].l)
            {
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
                bExcepOccured = isExceptionOccured(t.pEnv,sal_True); OSL_ENSURE(!bExcepOccured,"Exception occured!");
            }
            if(tableNamePattern.getLength())
            {
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
                bExcepOccured = isExceptionOccured(t.pEnv,sal_True); OSL_ENSURE(!bExcepOccured,"Exception occured!");
            }
            //for(INT16 i=0;i<len;i++)
            t.pEnv->DeleteLocalRef((jobjectArray)args[3].l);
            bExcepOccured = isExceptionOccured(t.pEnv,sal_True); OSL_ENSURE(!bExcepOccured,"Exception occured!");

            if ( jThrow )
            {
                if(t.pEnv->IsInstanceOf(jThrow,java_sql_SQLException_BASE::getMyClass()))
                {
                    java_sql_SQLException_BASE* pException = new java_sql_SQLException_BASE(t.pEnv,jThrow);
                    SQLException e( pException->getMessage(),
                                        *this,
                                        pException->getSQLState(),
                                        pException->getErrorCode(),
                                        Any()
                                    );
                    delete pException;
                    throw  e;
                }
            }

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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getProcedureColumns";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[4];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,procedureNamePattern);
            args[3].l = convertwchar_tToJavaString(t.pEnv,columnNamePattern);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(procedureNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(columnNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[3].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getProcedures";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,procedureNamePattern);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(procedureNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);

            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getVersionColumns";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxBinaryLiteralLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxRowSize";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxCatalogNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxCharLiteralLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnsInIndex";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxCursorNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxConnections";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnsInTable";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxStatementLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxTableNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxTablesInSelect";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getExportedKeys";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getImportedKeys";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getPrimaryKeys";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getIndexInfo";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[5];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            args[3].z = unique;
            args[4].z = approximate;
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,args[3].z,args[4].z );

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getBestRowIdentifier";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,table);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,scope,nullable);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(table.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTablePrivileges(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    if ( m_pConnection->isIgnoreDriverPrivilegesEnabled() )
    {
        return new OResultSetPrivileges(this,catalog,schemaPattern,tableNamePattern);
    }
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){


        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getTablePrivileges";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[3];
            // Parameter konvertieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l);

            // und aufraeumen
            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(args[1].l)
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(tableNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    Reference< XResultSet > xReturn = out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );

    if ( xReturn.is() )
    {
        // we have to check the result columns for the tables privleges
        Reference< XResultSetMetaDataSupplier > xMetaSup(xReturn,UNO_QUERY);
        if ( xMetaSup.is() )
        {
            Reference< XResultSetMetaData> xMeta = xMetaSup->getMetaData();
            if ( xMeta.is() && xMeta->getColumnCount() != 7 )
            {
                // here we know that the count of column doesn't match
                ::std::map<sal_Int32,sal_Int32> aColumnMatching;
                static const ::rtl::OUString sPrivs[] = {
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE_CAT")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE_SCHEM")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE_NAME")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GRANTOR")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GRANTEE")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PRIVILEGE")),
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IS_GRANTABLE"))
                                        };

                ::rtl::OUString sColumnName;
                sal_Int32 nCount = xMeta->getColumnCount();
                for (sal_Int32 i = 1 ; i <= nCount ; ++i)
                {
                    sColumnName = xMeta->getColumnName(i);
                    for (sal_uInt32 j = 0 ; j < sizeof(sPrivs)/sizeof(sPrivs[0]); ++j)
                    {
                        if ( sPrivs[j] == sColumnName )
                        {
                            aColumnMatching.insert( ::std::map<sal_Int32,sal_Int32>::value_type(i,j+1) );
                            break;
                        }
                    }

                }
                // fill our own resultset
                ODatabaseMetaDataResultSet* pNewPrivRes = new ODatabaseMetaDataResultSet();
                Reference< XResultSet > xTemp = xReturn;
                xReturn = pNewPrivRes;
                pNewPrivRes->setTablePrivilegesMap();
                ODatabaseMetaDataResultSet::ORows aRows;
                Reference< XRow > xRow(xTemp,UNO_QUERY);
                ::rtl::OUString sValue;

                ODatabaseMetaDataResultSet::ORow aRow(8);
                while ( xRow.is() && xTemp->next() )
                {
                    ::std::map<sal_Int32,sal_Int32>::iterator aIter = aColumnMatching.begin();
                    ::std::map<sal_Int32,sal_Int32>::iterator aEnd  = aColumnMatching.end();
                    for (;aIter != aEnd ; ++aIter)
                    {
                        sValue = xRow->getString(aIter->first);
                        if ( xRow->wasNull() )
                            aRow[aIter->second] = ODatabaseMetaDataResultSet::getEmptyValue();
                        else
                            aRow[aIter->second] = new ORowSetValueDecorator(sValue);
                    }

                    aRows.push_back(aRow);
                }
                pNewPrivRes->setRows(aRows);
            }
        }
    }
    return xReturn;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCrossReference(
        const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
        const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
        const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getCrossReference";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[6];
            // Parameter konvertieren
            args[0].l = primaryCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(primaryCatalog)) : 0;
            args[1].l = primarySchema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,primarySchema);
            args[2].l = convertwchar_tToJavaString(t.pEnv,primaryTable);
            args[3].l = foreignCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(foreignCatalog)) : 0;
            args[4].l = foreignSchema.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,foreignSchema);
            args[5].l = convertwchar_tToJavaString(t.pEnv,foreignTable);
            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[2].l,args[2].l,args[3].l,args[4].l,args[5].l );

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
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "doesMaxRowSizeIncludeBlobs";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesLowerCaseQuotedIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesLowerCaseIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesMixedCaseQuotedIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesMixedCaseIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesUpperCaseQuotedIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "storesUpperCaseIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsAlterTableWithAddColumn";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsAlterTableWithDropColumn";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxIndexLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsNonNullableColumns";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getCatalogTerm";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getIdentifierQuoteString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getExtraNameCharacters";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsDifferentTableCorrelationNames";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "isCatalogAtStart";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "dataDefinitionIgnoredInTransactions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "dataDefinitionCausesTransactionCommit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsDataManipulationTransactionsOnly";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsDataDefinitionAndDataManipulationTransactions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsPositionedDelete";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsPositionedUpdate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOpenStatementsAcrossRollback";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOpenStatementsAcrossCommit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOpenCursorsAcrossCommit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOpenCursorsAcrossRollback";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "supportsTransactionIsolationLevel";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSchemasInDataManipulation";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsANSI92FullSQL";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsANSI92EntryLevelSQL";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsIntegrityEnhancementFacility";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSchemasInIndexDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSchemasInTableDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCatalogsInTableDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCatalogsInIndexDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCatalogsInDataManipulation";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOuterJoins";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getTableTypes";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxStatements";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxProcedureNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxSchemaNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsTransactions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "allProceduresAreCallable";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsStoredProcedures";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSelectForUpdate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "allTablesAreSelectable";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_True);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "isReadOnly";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "usesLocalFiles";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "usesLocalFilePerTable";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsTypeConversion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "nullPlusNonNullIsNull";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsColumnAliasing";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsTableCorrelationNames";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(II)Z";
        static const char * cMethodName = "supportsConvert";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out =           t.pEnv->CallBooleanMethod( object, mID, fromType,toType );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsExpressionsInOrderBy";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsGroupBy";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsGroupByBeyondSelect";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsGroupByUnrelated";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsMultipleTransactions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsMultipleResultSets";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsLikeEscapeClause";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsOrderByUnrelated";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsUnion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsUnionAll";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsMixedCaseIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsMixedCaseQuotedIdentifiers";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "nullsAreSortedAtEnd";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "nullsAreSortedAtStart";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "nullsAreSortedHigh";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "nullsAreSortedLow";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSchemasInProcedureCalls";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSchemasInPrivilegeDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCatalogsInProcedureCalls";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCatalogsInPrivilegeDefinitions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCorrelatedSubqueries";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSubqueriesInComparisons";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSubqueriesInExists";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSubqueriesInIns";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsSubqueriesInQuantifieds";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsANSI92IntermediateSQL";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    ::rtl::OUString aValue = m_pConnection->getURL();
    if ( !aValue.getLength() )
    {
        SDBThreadAttach t;
        if( t.pEnv ){

            // temporaere Variable initialisieren
            static const char * cSignature = "()Ljava/lang/String;";
            static const char * cMethodName = "getURL";
            // Java-Call absetzen
            static jmethodID mID = NULL;
            if ( !mID  )
                mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if( mID ){
                jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
                ThrowSQLException(t.pEnv,*this);
                aValue = JavaString2String(t.pEnv,out);
            } //mID
        } //t.pEnv
        // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    }
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getUserName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getDriverName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getDriverVersion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
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
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getDatabaseProductVersion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
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
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getDatabaseProductName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getProcedureTerm";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getSchemaTerm";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getDriverMajorVersion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getDefaultTransactionIsolation";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getDriverMinorVersion";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getSQLKeywords";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getSearchStringEscape";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getStringFunctions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getTimeDateFunctions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getSystemFunctions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_DatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getNumericFunctions";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsExtendedSQLGrammar";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsCoreSQLGrammar";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsMinimumSQLGrammar";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsFullOuterJoins";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jboolean out(sal_False);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsLimitedOuterJoins";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnsInGroupBy";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnsInOrderBy";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxColumnsInSelect";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxUserNameLength";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "supportsResultSetType";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(II)Z";
        static const char * cMethodName = "supportsResultSetConcurrency";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "ownUpdatesAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "ownDeletesAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "ownInsertsAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "othersUpdatesAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "othersDeletesAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "othersInsertsAreVisible";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "updatesAreDetected";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "deletesAreDetected";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "insertsAreDetected";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "supportsBatchUpdates";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){


        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[I;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getUDTs";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args[4];
            // temporaere Variable initialisieren
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : 0;
            args[1].l = schemaPattern.toChar() == '%' ? NULL : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,typeNamePattern);
            jintArray pArray = t.pEnv->NewIntArray(types.getLength());
            t.pEnv->SetIntArrayRegion(pArray,0,types.getLength(),(jint*)types.getConstArray());
            args[3].l = pArray;

            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);

            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
            if(schemaPattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[1].l);
            if(typeNamePattern.getLength())
                t.pEnv->DeleteLocalRef((jstring)args[2].l);
            if(args[3].l)
                t.pEnv->DeleteLocalRef((jintArray)args[3].l);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? new java_sql_ResultSet( t.pEnv, out ) : 0;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_DatabaseMetaData::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

