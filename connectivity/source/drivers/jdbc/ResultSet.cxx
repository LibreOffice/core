/*************************************************************************
 *
 *  $RCSfile: ResultSet.cxx,v $
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
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSET_HXX_
#include "java/sql/ResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_
#include "java/sql/Statement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_
#include "java/sql/SQLWarning.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_ARRAY_HXX_
#include "java/sql/Array.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_REF_HXX_
#include "java/sql/Ref.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CLOB_HXX_
#include "java/sql/Clob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#include "java/sql/Timestamp.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_BLOB_HXX_
#include "java/sql/Blob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSETMETADATA_HXX_
#include "java/sql/ResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_IO_INPUTSTREAM_HXX_
#include "java/io/InputStream.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif


using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(java_sql_ResultSet,"com.sun.star.sdbcx.JResultSet","com.sun.star.sdbc.ResultSet");
//**************************************************************
//************ Class: java.sql.ResultSet
//**************************************************************

jclass java_sql_ResultSet::theClass = 0;

java_sql_ResultSet::~java_sql_ResultSet()
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "()V";
        char * cMethodName = "close";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
            t.pEnv->CallVoidMethod( object, mID);
    }
}

jclass java_sql_ResultSet::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/ResultSet"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_ResultSet::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------
void java_sql_ResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL java_sql_ResultSet::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = java_sql_ResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_ResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),java_sql_ResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,columnName);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;)I";
        char * cMethodName = "findColumn";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, args[0].l );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------


Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_ResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/io/InputStream;";
        char * cMethodName = "getBinaryStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_ResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/io/InputStream;";
        char * cMethodName = "getCharacterStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "getBoolean";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL java_sql_ResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jbyte out;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)B";
        char * cMethodName = "getByte";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallByteMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL java_sql_ResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
        Sequence< sal_Int8 > aSeq;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "(I)[B";
        char * cMethodName = "getBytes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jbyteArray out = (jbyteArray)t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            if (out)
            {
                jboolean p = sal_False;
                aSeq.realloc(t.pEnv->GetArrayLength(out));
                memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
                t.pEnv->DeleteLocalRef(out);
            }
            // und aufraeumen
        } //mID
    } //t.pEnv
    return aSeq;
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL java_sql_ResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Date;";
        char * cMethodName = "getDate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Date( t.pEnv, out );
}
// -------------------------------------------------------------------------

double SAL_CALL java_sql_ResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jdouble out;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)D";
        char * cMethodName = "getDouble";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallDoubleMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

float SAL_CALL java_sql_ResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jfloat out;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)F";
        char * cMethodName = "getFloat";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallFloatMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getInt";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL java_sql_ResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jlong out;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)J";
        char * cMethodName = "getLong";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL java_sql_ResultSet::getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/ResultSetMetaData;";
        char * cMethodName = "getMetaData";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSetMetaData( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL java_sql_ResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getArray";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Array( t.pEnv, out );
}
// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL java_sql_ResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getClob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Clob( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL java_sql_ResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getBlob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Blob( t.pEnv, out );
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL java_sql_ResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getRef";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Ref( t.pEnv, out );
}
// -------------------------------------------------------------------------

Any SAL_CALL java_sql_ResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[2];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        args[1].l = XNameAccess2Map(t.pEnv,typeMap);
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/Object;";
        char * cMethodName = "getObject";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethodA( object, mID, args);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jstring)args[1].l);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
        return out==0 ? Any() : Any();//new java_lang_Object( t.pEnv, out );
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL java_sql_ResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jshort out;
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)S";
        char * cMethodName = "getShort";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallShortMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int16)out;
}
// -------------------------------------------------------------------------


::rtl::OUString SAL_CALL java_sql_ResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jstring out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aStr;
}
// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL java_sql_ResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Time;";
        char * cMethodName = "getTime";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Time( t.pEnv, out );
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL java_sql_ResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Timestamp;";
        char * cMethodName = "getTimestamp";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Timestamp( t.pEnv, out );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isAfterLast";
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
sal_Bool SAL_CALL java_sql_ResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isFirst";
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
sal_Bool SAL_CALL java_sql_ResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isLast";
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
void SAL_CALL java_sql_ResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "isBeforeFirst";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "afterLast";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "close";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::first(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "first";
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

sal_Bool SAL_CALL java_sql_ResultSet::last(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "last";
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
sal_Bool SAL_CALL java_sql_ResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "absolute";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID,row);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "relative";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID,row);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "previous";
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
Reference< XInterface > SAL_CALL java_sql_ResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
//  jobject out(0);
//  SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
//  if( t.pEnv )
//  {
//      // temporaere Variable initialisieren
//      char * cSignature = "()Ljava/sql/Statement;";
//      char * cMethodName = "getStatement";
//      // Java-Call absetzen
//      jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
//      if( mID ){
//          out = t.pEnv->CallObjectMethod( object, mID );
//          ThrowSQLException(t.pEnv,*this);
//      } //mID
//  } //t.pEnv
//  // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
//  return out==0 ? 0 : (::cppu::OWeakObject*)new java_sql_Statement( t.pEnv, out );
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "rowDeleted";
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
sal_Bool SAL_CALL java_sql_ResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "rowInserted";
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
sal_Bool SAL_CALL java_sql_ResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "rowUpdated";
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

sal_Bool SAL_CALL java_sql_ResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "isBeforeFirst";
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

sal_Bool SAL_CALL java_sql_ResultSet::next(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "next";
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

sal_Bool SAL_CALL java_sql_ResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "wasNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::cancel(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "cancel";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "clearWarnings";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL java_sql_ResultSet::getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/sql/SQLWarning;";
        char * cMethodName = "getWarnings";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
        return out ? makeAny(java_sql_SQLWarning(java_sql_SQLWarning_BASE( t.pEnv, out ),*this)) : ::com::sun::star::uno::Any();
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "insertRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "updateRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "deleteRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "cancelRowUpdates";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "moveToInsertRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "moveToCurrentRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)V";
        char * cMethodName = "updateNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(IZ)V";
        char * cMethodName = "updateNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(IB)V";
        char * cMethodName = "updateByte";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(IS)V";
        char * cMethodName = "updateShort";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(II)V";
        char * cMethodName = "updateInt";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(IJ)V";
        char * cMethodName = "updateLong";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

// -----------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(IF)V";
        char * cMethodName = "updateFloat";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(ID)V";
        char * cMethodName = "updateDouble";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,x);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,x);
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/String)V";
        char * cMethodName = "updateString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        jbyteArray aArray = t.pEnv->NewByteArray(x.getLength());
        t.pEnv->SetByteArrayRegion(aArray,0,x.getLength(),(jbyte*)x.getConstArray());

        // Parameter konvertieren
        args[0].l = aArray;
        // temporaere Variable initialisieren
        char * cSignature = "(I[B)V";
        char * cMethodName = "updateBytes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jbyteArray)args[0].l);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        java_sql_Date aD(x);
        args[0].l = aD.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Date;)V";
        char * cMethodName = "updateDate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        java_sql_Time aD(x);
        args[0].l = aD.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/sql/Time;)V";
        char * cMethodName = "updateTime";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        java_sql_Timestamp aD(x);
        // Parameter konvertieren
        args[0].l = aD.getJavaObject();
        // temporaere Variable initialisieren
        char * cSignature = "(I;Ljava/sql/Timestamp;)V";
        char * cMethodName = "updateTimestamp";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        args[0].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/io/InputStream;I)V";
        char * cMethodName = "updateBinaryStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l,length);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jobject)args[0].l);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        args[0].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/io/InputStream;I)V";
        char * cMethodName = "updateCharacterStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l,length);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jobject)args[0].l);
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t;
    if( t.pEnv )
    {
        jvalue args[1];
        switch(x.getValueTypeClass())
        {
                        case TypeClass_VOID:
                args[0].l = NULL;
                break;

                        case TypeClass_BOOLEAN:
                {
                    sal_Bool f;
                    x >>= f;
                    updateBoolean(columnIndex,f);
                }
                break;
                        case TypeClass_BYTE:
                {
                    sal_Int8 f;
                    x >>= f;
                    updateByte(columnIndex,f);
                }
                break;
                        case TypeClass_SHORT:
                        case TypeClass_UNSIGNED_SHORT:
                {
                    sal_Int16 f;
                    x >>= f;
                    updateShort(columnIndex,f);
                }
                break;
                        case TypeClass_LONG:
                        case TypeClass_UNSIGNED_LONG:
                {
                    sal_Int32 f;
                    x >>= f;
                    updateInt(columnIndex,f);
                }
                break;
                        case TypeClass_HYPER:
                        case TypeClass_UNSIGNED_HYPER:
                {
                    sal_Int64 f;
                    x >>= f;
                    updateLong(columnIndex,f);
                }
                break;
                        case TypeClass_FLOAT:
                {
                    float f;
                    x >>= f;
                    updateFloat(columnIndex,f);
                }
                break;
                        case TypeClass_DOUBLE:
                updateDouble(columnIndex,connectivity::getDouble(x));
                break;
                        case TypeClass_CHAR:
                        case TypeClass_STRING:
                updateString(columnIndex,connectivity::getString(x));
                break;
                        case TypeClass_ENUM:
            default:
                OSL_ENSHURE(0,"UNKOWN TYPE for java_sql_ResultSet::updateObject");
        }
        return;
        // Parameter konvertieren
        // temporaere Variable initialisieren
        char * cSignature = "(ILjava/lang/Object;)V";
        char * cMethodName = "updateObject";
        // Java-Call absetzen
//      jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
//      if( mID ){
//          t.pEnv->CallVoidMethodA( object, mID,columnIndex,args);
            ThrowSQLException(t.pEnv,*this);
//          switch(x.getValueTypeClass())
//          {
//                              case TypeClass_CHAR:
//                              case TypeClass_STRING:
//                  t.pEnv->DeleteLocalRef((jobject)args[0].l);
//                  break;
//          }
//      }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSHURE(0,"java_sql_ResultSet::updateNumericObject: NYI");
//  SDBThreadAttach t;
//  if( t.pEnv )
//  {
//      jvalue args[1];
//      // Parameter konvertieren
//      args[0].l =
//      // temporaere Variable initialisieren
//      char * cSignature = "(I;Ljava/lang/Object;I)V";
//      char * cMethodName = "updateObject";
//      // Java-Call absetzen
//      jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
//      if( mID ){
//          t.pEnv->CallVoidMethod( object, mID,columnIndex,args[0].l,scale);
            //  ThrowSQLException(t.pEnv,*this);
//          t.pEnv->DeleteLocalRef((jobject)args[0].l);
//      }
//  }
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getResultSetConcurrency() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getResultSetConcurrency";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getResultSetType() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getResultSetType";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getFetchDirection() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getFetchDirection";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getFetchSize() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getFetchSize";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
::rtl::OUString java_sql_ResultSet::getCursorName() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "getCursorName";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}

//------------------------------------------------------------------------------
void java_sql_ResultSet::setFetchDirection(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)V";
        char * cMethodName = "setFetchDirection";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv

}
//------------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()V";
        char * cMethodName = "refreshRow";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv
}
//------------------------------------------------------------------------------
void java_sql_ResultSet::setFetchSize(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)V";
        char * cMethodName = "setFetchSize";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            ThrowSQLException(t.pEnv,*(::cppu::OWeakObject*)this);
        } //mID
    } //t.pEnv

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* java_sql_ResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & java_sql_ResultSet::getInfoHelper()
{
    return *const_cast<java_sql_ResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool java_sql_ResultSet::convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void java_sql_ResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::uno::Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(connectivity::getINT32(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void java_sql_ResultSet::getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}



