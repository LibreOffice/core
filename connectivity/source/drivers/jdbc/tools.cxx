/*************************************************************************
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:27:12 $
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

#include <cstdarg>
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_STRING_HXX_
#include "java/lang/String.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
#include "java/lang/Class.hxx"
#endif
#ifndef CONNECTIVITY_java_util_Properties
#include "java/util/Property.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_DRIVERPROPERTYINFO_HPP_
#include <com/sun/star/sdbc/DriverPropertyInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

void java_util_Properties::setProperty(const ::rtl::OUString key, const ::rtl::OUString& value)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    jobject out(0);
    if( t.pEnv )
    {
        jvalue args[2];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,key);
        args[1].l = convertwchar_tToJavaString(t.pEnv,value);
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;";
        char * cMethodName = "setProperty";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
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
    char * cSignature = "()V";
    jobject tempObj;
    jmethodID mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->NewObject( getMyClass(), mID);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
}

// --------------------------------------------------------------------------------
jstring connectivity::convertwchar_tToJavaString(JNIEnv *pEnv,const ::rtl::OUString& _rTemp)
{
    jstring pStr = NULL;
    if (pEnv)
    {
        pStr = pEnv->NewString(_rTemp.getStr(), _rTemp.getLength());
        pEnv->ExceptionClear();
        OSL_ENSURE(pStr,"Could not create a jsstring object!");
    }
    return pStr;
}

// --------------------------------------------------------------------------------
java_util_Properties* connectivity::createStringPropertyArray(JNIEnv *pEnv,const Sequence< PropertyValue >& info )  throw(SQLException, RuntimeException)
{
    java_util_Properties* pProps = new java_util_Properties();
    const PropertyValue* pBegin = info.getConstArray();
    const PropertyValue* pEnd   = pBegin + info.getLength();

    for(;pBegin != pEnd;++pBegin)
    {
        // this is a special property to find the jdbc driver
        if( pBegin->Name.compareToAscii("JavaDriverClass") &&
            pBegin->Name.compareToAscii("CharSet") &&
            pBegin->Name.compareToAscii("AppendTableAlias") &&
            pBegin->Name.compareToAscii("ParameterNameSubstitution") &&
            pBegin->Name.compareToAscii("IsPasswordRequired") &&
            pBegin->Name.compareToAscii("IsAutoRetrievingEnabled") &&
            pBegin->Name.compareToAscii("AutoRetrievingStatement") &&
            pBegin->Name.compareToAscii("BooleanComparisonMode"))
        {
            ::rtl::OUString aStr;
            pBegin->Value >>= aStr;
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
jobject connectivity::XNameAccess2Map(JNIEnv *pEnv,const Reference< ::com::sun::star::container::XNameAccess > & _rMap)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq = _rMap->getElementNames();
    const ::rtl::OUString *pBegin   = aSeq.getConstArray();
    const ::rtl::OUString *pEnd     = pBegin + aSeq.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
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


