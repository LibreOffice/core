/*************************************************************************
 *
 *  $RCSfile: PreparedStatement.cxx,v $
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
#ifndef _CONNECTIVITY_JAVA_SQL_PREPAREDSTATEMENT_HXX_
#include "java/sql/PreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSET_HXX_
#include "java/sql/ResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#include "java/sql/Connection.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#include "java/sql/Timestamp.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//**************************************************************
//************ Class: java.sql.PreparedStatement
//**************************************************************
IMPLEMENT_SERVICE_INFO(java_sql_PreparedStatement,"com.sun.star.sdbcx.JPreparedStatement","com.sun.star.sdbc.PreparedStatement");

jclass java_sql_PreparedStatement::theClass = 0;

java_sql_PreparedStatement::~java_sql_PreparedStatement()
{}

jclass java_sql_PreparedStatement::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/PreparedStatement"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}
// -------------------------------------------------------------------------

void java_sql_PreparedStatement::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL java_sql_PreparedStatement::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(  rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XPreparedBatchExecution*>(this));
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_PreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedBatchExecution > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_PreparedStatement::execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "execute";
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

sal_Int32 SAL_CALL java_sql_PreparedStatement::executeUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "executeUpdate";
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

void SAL_CALL java_sql_PreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[2];
        // Parameter konvertieren
        args[0].i = parameterIndex;
        args[1].l = convertwchar_tToJavaString(t.pEnv,x);
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/String;)V";
        char * cMethodName = "setString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef((jstring)args[1].l);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL java_sql_PreparedStatement::getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
//  jobject out(0);
//  SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
//  if( t.pEnv ){
//
//      // temporaere Variable initialisieren
//      char * cSignature = "()Ljava/sql/Connection;";
//      char * cMethodName = "getConnection";
//      // Java-Call absetzen
//      jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
//      if( mID ){
//          out = t.pEnv->CallObjectMethod( object, mID);
//          ThrowSQLException(t.pEnv,*this);
//      } //mID
//  } //t.pEnv
//  // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
//  return out==0 ? 0 : new java_sql_Connection( t.pEnv, out );
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_PreparedStatement::executeQuery(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/ResultSet;";
        char * cMethodName = "executeQuery";
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

void SAL_CALL java_sql_PreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].z = x;
        // temporaere Variable initialisieren
        char * cSignature = "(IZ)V";
        char * cMethodName = "setBoolean";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].z);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].b =  x;
        // temporaere Variable initialisieren
        char * cSignature = "(IB)V";
        char * cMethodName = "setByte";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].b);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = parameterIndex;
        java_sql_Date aT(x);
        args[1].l = aT.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Date;)V";
        char * cMethodName = "setDate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------


void SAL_CALL java_sql_PreparedStatement::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        java_sql_Time aT(x);
        args[1].l = aT.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Time;)V";
        char * cMethodName = "setTime";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        java_sql_Timestamp aT(x);
        args[1].l = aT.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Timestamp;)V";
        char * cMethodName = "setTimestamp";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------



void SAL_CALL java_sql_PreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].d = x;
        // temporaere Variable initialisieren
        char * cSignature = "(ID)V";
        char * cMethodName = "setDouble";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].d);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].f = x;
        // temporaere Variable initialisieren
        char * cSignature = "(IF)V";
        char * cMethodName = "setFloat";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].f);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].i = (sal_Int32)x;
        // temporaere Variable initialisieren
        char * cSignature = "(II)V";
        char * cMethodName = "setInt";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].j = x;
        // temporaere Variable initialisieren
        char * cSignature = "(IJ)V";
        char * cMethodName = "setLong";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].j);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].i = (sal_Int32)sqlType;
        // temporaere Variable initialisieren
        char * cSignature = "(II)V";
        char * cMethodName = "setNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setClob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Clob;)V";
        char * cMethodName = "setClob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBlob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Blob;)V";
        char * cMethodName = "setBlob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setArray( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Array;)V";
        char * cMethodName = "setArray";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setRef( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Ref;)V";
        char * cMethodName = "setRef";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/Object;II)V";
        char * cMethodName = "setObject";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l,targetSqlType,scale);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/Object;)V";
        char * cMethodName = "setObject";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/Object;)V";
        char * cMethodName = "setObject";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].s = (sal_Int16)x;
        // temporaere Variable initialisieren
        char * cSignature = "(IS)V";
        char * cMethodName = "setShort";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].s);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        jbyteArray pByteArray = t.pEnv->NewByteArray(x.getLength());
        t.pEnv->SetByteArrayRegion(pByteArray,0,x.getLength(),(jbyte*)x.getConstArray());
        args[1].l = pByteArray;
        // temporaere Variable initialisieren
        char * cSignature = "(I[B)V";
        char * cMethodName = "setBytes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].l);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
        t.pEnv->DeleteLocalRef(pByteArray);
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        jvalue args2[3];
        jbyteArray pByteArray = t.pEnv->NewByteArray(length);
        Sequence< sal_Int8> aSeq;
        x->readBytes(aSeq,length);
        t.pEnv->SetByteArrayRegion(pByteArray,0,length,(jbyte*)aSeq.getConstArray());
        args2[0].l =  pByteArray;
        args2[1].i =  0;
        args2[2].i =  (sal_Int32)length;

        // temporaere Variable initialisieren
        char * cSignatureStream = "([BII)V";
        // Java-Call absetzen
        jclass aClass = t.pEnv->FindClass("java/io/CharArrayInputStream");
        jmethodID mID2 = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
        jobject tempObj = NULL;
        if(mID2)
            tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );

        //
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/io/InputStream;I)V";
        char * cMethodName = "setCharacterStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            t.pEnv->CallVoidMethod( object, mID, args[0].i,tempObj,length);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        t.pEnv->DeleteLocalRef(pByteArray);
        t.pEnv->DeleteLocalRef(tempObj);
        t.pEnv->DeleteLocalRef(aClass);
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        jvalue args2[3];
        jbyteArray pByteArray = t.pEnv->NewByteArray(length);
        Sequence< sal_Int8> aSeq;
        x->readBytes(aSeq,length);
        t.pEnv->SetByteArrayRegion(pByteArray,0,length,(jbyte*)aSeq.getConstArray());
        args2[0].l =  pByteArray;
        args2[1].i =  0;
        args2[2].i =  (sal_Int32)length;

        // temporaere Variable initialisieren
        char * cSignatureStream = "([BII)V";
        // Java-Call absetzen
        jclass aClass = t.pEnv->FindClass("java/io/ByteArrayInputStream");
        jmethodID mID2 = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
        jobject tempObj = NULL;
        if(mID2)
            tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );

        //
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/io/InputStream;I)V";
        char * cMethodName = "setBinaryStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            t.pEnv->CallVoidMethod( object, mID, args[0].i,tempObj,(sal_Int32)length);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        t.pEnv->DeleteLocalRef(pByteArray);
        t.pEnv->DeleteLocalRef(tempObj);
        t.pEnv->DeleteLocalRef(aClass);
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "clearParameters";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "clearBatch";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::addBatch( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "addBatch";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL java_sql_PreparedStatement::executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()[I";
        char * cMethodName = "executeBatch";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jintArray out = (jintArray)t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            if(out)
            {
                jboolean p = sal_False;
                aSeq.realloc(t.pEnv->GetArrayLength(out));
                memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
                t.pEnv->DeleteLocalRef(out);
            }
        } //mID
    } //t.pEnv
    return aSeq;
}
// -------------------------------------------------------------------------


