/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <cstdarg>
#include "java/tools.hxx"
#include "java/lang/String.hxx"
#include "java/lang/Class.hxx"
#include "java/util/Property.hxx"
#include <com/sun/star/sdbc/DriverPropertyInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

void java_util_Properties::setProperty(const ::rtl::OUString key, const ::rtl::OUString& value)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    jobject out(0);

    {
        jvalue args[2];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,key);
        args[1].l = convertwchar_tToJavaString(t.pEnv,value);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;";
        static const char * cMethodName = "setProperty";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallObjectMethod(object, mID, args[0].l,args[1].l);
        ThrowSQLException(t.pEnv,NULL);
        t.pEnv->DeleteLocalRef((jstring)args[1].l);
        t.pEnv->DeleteLocalRef((jstring)args[0].l);
        ThrowSQLException(t.pEnv,0);
        if(out)
            t.pEnv->DeleteLocalRef(out);
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
}
jclass java_util_Properties::theClass = 0;

java_util_Properties::~java_util_Properties()
{}

jclass java_util_Properties::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/util/Properties");
    return theClass;
}

//--------------------------------------------------------------------------
java_util_Properties::java_util_Properties( ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "()V";
    jobject tempObj;
    static jmethodID mID(NULL);
    obtainMethodId(t.pEnv, "<init>",cSignature, mID);
    tempObj = t.pEnv->NewObject( getMyClass(), mID);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
}

// --------------------------------------------------------------------------------
jstring connectivity::convertwchar_tToJavaString(JNIEnv *pEnv,const ::rtl::OUString& _rTemp)
{
    OSL_ENSURE(pEnv,"Environment is NULL!");
    jstring pStr = pEnv->NewString(_rTemp.getStr(), _rTemp.getLength());
    pEnv->ExceptionClear();
    OSL_ENSURE(pStr,"Could not create a jsstring object!");
    return pStr;
}

// --------------------------------------------------------------------------------
java_util_Properties* connectivity::createStringPropertyArray(const Sequence< PropertyValue >& info )  throw(SQLException, RuntimeException)
{
    java_util_Properties* pProps = new java_util_Properties();
    const PropertyValue* pBegin = info.getConstArray();
    const PropertyValue* pEnd   = pBegin + info.getLength();

    for(;pBegin != pEnd;++pBegin)
    {
        // this is a special property to find the jdbc driver
        if  (   pBegin->Name.compareToAscii( "JavaDriverClass" )
            &&  pBegin->Name.compareToAscii( "JavaDriverClassPath" )
            &&  pBegin->Name.compareToAscii( "SystemProperties" )
            &&  pBegin->Name.compareToAscii( "CharSet" )
            &&  pBegin->Name.compareToAscii( "AppendTableAliasName" )
            &&  pBegin->Name.compareToAscii( "AddIndexAppendix" )
            &&  pBegin->Name.compareToAscii( "FormsCheckRequiredFields" )
            &&  pBegin->Name.compareToAscii( "GenerateASBeforeCorrelationName" )
            &&  pBegin->Name.compareToAscii( "EscapeDateTime" )
            &&  pBegin->Name.compareToAscii( "ParameterNameSubstitution" )
            &&  pBegin->Name.compareToAscii( "IsPasswordRequired" )
            &&  pBegin->Name.compareToAscii( "IsAutoRetrievingEnabled" )
            &&  pBegin->Name.compareToAscii( "AutoRetrievingStatement" )
            &&  pBegin->Name.compareToAscii( "UseCatalogInSelect" )
            &&  pBegin->Name.compareToAscii( "UseSchemaInSelect" )
            &&  pBegin->Name.compareToAscii( "AutoIncrementCreation" )
            &&  pBegin->Name.compareToAscii( "Extension" )
            &&  pBegin->Name.compareToAscii( "NoNameLengthLimit" )
            &&  pBegin->Name.compareToAscii( "EnableSQL92Check" )
            &&  pBegin->Name.compareToAscii( "EnableOuterJoinEscape" )
            &&  pBegin->Name.compareToAscii( "BooleanComparisonMode" )
            &&  pBegin->Name.compareToAscii( "IgnoreCurrency" )
            &&  pBegin->Name.compareToAscii( "TypeInfoSettings" )
            &&  pBegin->Name.compareToAscii( "IgnoreDriverPrivileges" )
            &&  pBegin->Name.compareToAscii( "ImplicitCatalogRestriction" )
            &&  pBegin->Name.compareToAscii( "ImplicitSchemaRestriction" )
            &&  pBegin->Name.compareToAscii( "SupportsTableCreation" )
            &&  pBegin->Name.compareToAscii( "UseJava" )
            &&  pBegin->Name.compareToAscii( "Authentication" )
            &&  pBegin->Name.compareToAscii( "PreferDosLikeLineEnds" )
            &&  pBegin->Name.compareToAscii( "PrimaryKeySupport" )
            &&  pBegin->Name.compareToAscii( "RespectDriverResultSetType" )
            )
        {
            ::rtl::OUString aStr;
            OSL_VERIFY( pBegin->Value >>= aStr );
            pProps->setProperty(pBegin->Name,aStr);
        }
    }
    return pProps;
}
// --------------------------------------------------------------------------------
::rtl::OUString connectivity::JavaString2String(JNIEnv *pEnv,jstring _Str)
{
    ::rtl::OUString aStr;
    if(_Str)
    {
        jboolean bCopy(sal_True);
        const jchar* pChar = pEnv->GetStringChars(_Str,&bCopy);
        jsize len = pEnv->GetStringLength(_Str);
        aStr = ::rtl::OUString(pChar,len);

        if(bCopy)
            pEnv->ReleaseStringChars(_Str,pChar);
        pEnv->DeleteLocalRef(_Str);
    }
    return aStr;
}
// --------------------------------------------------------------------------------
jobject connectivity::convertTypeMapToJavaMap(JNIEnv* /*pEnv*/,const Reference< ::com::sun::star::container::XNameAccess > & _rMap)
{
    if ( _rMap.is() )
    {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames = _rMap->getElementNames();
        if ( aNames.getLength() > 0 )
            ::dbtools::throwFeatureNotImplementedException( "Type maps", NULL );
    }
    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool connectivity::isExceptionOccured(JNIEnv *pEnv,sal_Bool _bClear)
{
    if ( !pEnv )
        return sal_False;

    jthrowable pThrowable = pEnv->ExceptionOccurred();
    sal_Bool bRet = pThrowable != NULL;
    if ( pThrowable )
    {
        if ( _bClear )
            pEnv->ExceptionClear();
#if OSL_DEBUG_LEVEL > 1
        if(pEnv->IsInstanceOf(pThrowable,java_sql_SQLException_BASE::st_getMyClass()))
        {

            java_sql_SQLException_BASE* pException = new java_sql_SQLException_BASE(pEnv,pThrowable);
            ::rtl::OUString sError = pException->getMessage();
            delete pException;
        }
#else
        pEnv->DeleteLocalRef(pThrowable);
#endif

    }

    return bRet;
}
// -----------------------------------------------------------------------------
jobject connectivity::createByteInputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length)
{
    SDBThreadAttach t;
    if( !t.pEnv || !x.is() )
        return NULL;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    jclass clazz = java_lang_Object::findMyClass("java/io/ByteArrayInputStream");
    static jmethodID mID(NULL);
    if  ( !mID )
    {
        static const char * cSignature = "([B)V";
        mID  = t.pEnv->GetMethodID( clazz, "<init>", cSignature );
        OSL_ENSURE( mID, cSignature );
        if  ( !mID )
            throw SQLException();
    } // if  ( !_inout_MethodID )
    jbyteArray pByteArray = t.pEnv->NewByteArray(length);
    Sequence< sal_Int8 > aData;
    x->readBytes(aData,length);
    jboolean p = sal_False;
    rtl_copyMemory(t.pEnv->GetByteArrayElements(pByteArray,&p),aData.getArray(),aData.getLength());
    jobject out = t.pEnv->NewObject( clazz, mID,pByteArray);
    t.pEnv->DeleteLocalRef((jbyteArray)pByteArray);
    return out;
}
// -----------------------------------------------------------------------------
jobject connectivity::createCharArrayReader(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length)
{
    SDBThreadAttach t;
    if( !t.pEnv || !x.is() )
        return NULL;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    jclass clazz = java_lang_Object::findMyClass("java/io/CharArrayReader");
    static jmethodID mID(NULL);
    if  ( !mID )
    {
        static const char * cSignature = "([C)V";
        mID  = t.pEnv->GetMethodID( clazz, "<init>", cSignature );
        OSL_ENSURE( mID, cSignature );
        if  ( !mID )
            throw SQLException();
    } // if  ( !_inout_MethodID )
    jcharArray pCharArray = t.pEnv->NewCharArray(length);
    Sequence< sal_Int8 > aData;
    x->readBytes(aData,length);
    jboolean p = sal_False;
    rtl_copyMemory(t.pEnv->GetCharArrayElements(pCharArray,&p),aData.getArray(),aData.getLength());
    jobject out = t.pEnv->NewObject( clazz, mID,pCharArray);
    t.pEnv->DeleteLocalRef((jcharArray)pCharArray);
    return out;
}
// -----------------------------------------------------------------------------
