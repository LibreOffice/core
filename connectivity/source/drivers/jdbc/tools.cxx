/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tools.cxx,v $
 * $Revision: 1.29 $
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
    if( t.pEnv )
    {
        jvalue args[2];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,key);
        args[1].l = convertwchar_tToJavaString(t.pEnv,value);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;";
        static const char * cMethodName = "setProperty";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallObjectMethod(object, mID, args[0].l,args[1].l);
            ThrowSQLException(t.pEnv,NULL);
        }
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

jclass java_util_Properties::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/util/Properties" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_util_Properties::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
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
    static jmethodID mID = NULL;
    if ( !mID  )
        mID  = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
        if( pBegin->Name.compareToAscii("JavaDriverClass") &&
            pBegin->Name.compareToAscii("JavaDriverClassPath") &&
            pBegin->Name.compareToAscii("SystemProperties") &&
            pBegin->Name.compareToAscii("CharSet") &&
            pBegin->Name.compareToAscii("AppendTableAlias") &&
            pBegin->Name.compareToAscii("GenerateASBeforeCorrelationName") &&
            pBegin->Name.compareToAscii("EscapeDateTime") &&
            pBegin->Name.compareToAscii("ParameterNameSubstitution") &&
            pBegin->Name.compareToAscii("IsPasswordRequired") &&
            pBegin->Name.compareToAscii("IsAutoRetrievingEnabled") &&
            pBegin->Name.compareToAscii("AutoRetrievingStatement") &&
            pBegin->Name.compareToAscii("UseCatalogInSelect") &&
            pBegin->Name.compareToAscii("UseSchemaInSelect") &&
            pBegin->Name.compareToAscii("AutoIncrementCreation") &&
            pBegin->Name.compareToAscii("Extension") &&
            pBegin->Name.compareToAscii("NoNameLengthLimit") &&
            pBegin->Name.compareToAscii("EnableSQL92Check") &&
            pBegin->Name.compareToAscii("EnableOuterJoinEscape") &&
            pBegin->Name.compareToAscii("BooleanComparisonMode") &&
            pBegin->Name.compareToAscii("IgnoreCurrency") &&
            pBegin->Name.compareToAscii("TypeInfoSettings") &&
            pBegin->Name.compareToAscii("IgnoreDriverPrivileges"))
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
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames = _rMap->getElementNames();
    if ( aNames.getLength() > 0 )
        ::dbtools::throwFeatureNotImplementedException( "Type maps", NULL );
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
        if(pEnv->IsInstanceOf(pThrowable,java_sql_SQLException_BASE::getMyClass()))
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


