/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResultSetMetaData.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/sql/ResultSetMetaData.hxx"
#include "java/sql/Connection.hxx"
#include "java/tools.hxx"
#include <rtl/logfile.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

#define NULLABLE_UNDEFINED  99
//**************************************************************
//************ Class: java.sql.ResultSetMetaData
//**************************************************************

jclass java_sql_ResultSetMetaData::theClass = 0;
java_sql_ResultSetMetaData::java_sql_ResultSetMetaData( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rResultSetLogger, java_sql_Connection& _rCon  )
    :java_lang_Object( pEnv, myObj )
    ,m_aLogger( _rResultSetLogger )
    ,m_pConnection( &_rCon )
    ,m_nColumnCount(-1)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::java_sql_ResultSetMetaData" );
    SDBThreadAttach::addRef();
}
java_sql_ResultSetMetaData::~java_sql_ResultSetMetaData()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_ResultSetMetaData::getMyClass()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getMyClass" );
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::saveClassRef" );
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnDisplaySize" );
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {

        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)I";
            static const char * cMethodName = "getColumnDisplaySize";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallIntMethod( object, mID,column);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnType" );
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {

        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)I";
            static const char * cMethodName = "getColumnType";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallIntMethod( object, mID,column);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnCount" );
    if ( m_nColumnCount == -1 )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        {

            // temporaere Variable initialisieren
            // Java-Call absetzen
            static jmethodID mID = NULL;
            if ( !mID  )
            {
                static const char * cSignature = "()I";
                static const char * cMethodName = "getColumnCount";

                mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
                if ( !mID )
                    throw SQLException();
            }
            {
                m_nColumnCount = t.pEnv->CallIntMethod( object, mID);
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            } //mID
        } //t.pEnv
    } // if ( m_nColumnCount == -1 )
    return m_nColumnCount;

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isCaseSensitive" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isCaseSensitive";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getSchemaName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getSchemaName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getColumnName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getTableName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getTableName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getCatalogName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getCatalogName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnTypeName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getColumnTypeName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnLabel" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getColumnLabel";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnServiceName" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/String;";
            static const char * cMethodName = "getColumnClassName";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        }

            // und aufraeumen
    } //t.pEnv
    return aStr;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isCurrency" );
    if ( m_pConnection->isIgnoreCurrencyEnabled() )
        return sal_False;
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isCurrency";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isAutoIncrement" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isAutoIncrement";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

            out = t.pEnv->CallBooleanMethod( object, mID, column);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isSigned" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isSigned";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }

            out = t.pEnv->CallBooleanMethod( object, mID, column);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getPrecision" );
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)I";
            static const char * cMethodName = "getPrecision";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getScale" );
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)I";
            static const char * cMethodName = "getScale";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isNullable" );
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)I";
            static const char * cMethodName = "isNullable";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallIntMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isSearchable" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isSearchable";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isReadOnly" );
    jboolean out(sal_False);
    SDBThreadAttach t;
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isReadOnly";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isDefinitelyWritable" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isDefinitelyWritable";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isWritable" );
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // temporaere Variable initialisieren
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
        {
            static const char * cSignature = "(I)Z";
            static const char * cMethodName = "isWritable";

            mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if ( !mID )
                throw SQLException();
        }
        {
            out = t.pEnv->CallBooleanMethod( object, mID, column );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

