/*************************************************************************
 *
 *  $RCSfile: JDriver.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 14:39:33 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_DRIVER_HXX_
#include "java/sql/Driver.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
#include "java/lang/Class.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_
#include "java/sql/DriverManager.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERPOPERTYINFO_HXX_
#include "java/sql/DriverPropertyInfo.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#include "java/sql/Connection.hxx"
#endif
#include "java/util/Property.hxx"
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
java_sql_Driver::java_sql_Driver(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : java_lang_Object(_rxFactory)
{
}
// --------------------------------------------------------------------------------
jclass java_sql_Driver::theClass = 0;
// --------------------------------------------------------------------------------
java_sql_Driver::~java_sql_Driver()
{}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString java_sql_Driver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.JDBCDriver");
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > java_sql_Driver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL connectivity::java_sql_Driver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new java_sql_Driver(_rxFactory));
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_Driver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
jclass java_sql_Driver::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( t.pEnv )
        {
            jclass tempClass = t.pEnv->FindClass("java/sql/Driver");
            OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
            jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
            t.pEnv->DeleteLocalRef( tempClass );
            saveClassRef( globClass );
        }
    }
    return theClass;
}
// --------------------------------------------------------------------------------
void java_sql_Driver::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -----------------------------------------------------------------------------
void java_sql_Driver::loadDriverFromProperties(const Sequence< PropertyValue >& info,::rtl::OUString& _rsGeneratedValueStatement,sal_Bool& _rbAutoRetrievingEnabled,sal_Bool& _bParameterSubstitution,sal_Bool& _bIgnoreDriverPrivileges)
{
    // first try if the jdbc driver is alraedy registered at the driver manager
    SDBThreadAttach t(getORB()); OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    try
    {
        const PropertyValue* pBegin = info.getConstArray();
        const PropertyValue* pEnd   = pBegin + info.getLength();
        for(jsize i=0;pBegin != pEnd;++pBegin)
        {
            if ( !object && !pBegin->Name.compareToAscii("JavaDriverClass") )
            {
                // here I try to find the class for jdbc driver
                java_sql_SQLException_BASE::getMyClass();
                java_lang_Throwable::getMyClass();

                ::rtl::OUString aStr;
                pBegin->Value >>= aStr;
                // the driver manager holds the class of the driver for later use
                // if forName didn't find the class it will throw an exception
                java_lang_Class *pDrvClass = java_lang_Class::forName(aStr);
                if(pDrvClass)
                {
                    saveRef(t.pEnv, pDrvClass->newInstanceObject());
                    jclass tempClass = t.pEnv->GetObjectClass(object);
                    if(object)
                    {
                        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
                        t.pEnv->DeleteLocalRef( tempClass );
                        saveClassRef( globClass );
                    }
                    delete pDrvClass;
                }
            }
            else if(!pBegin->Name.compareToAscii("IsAutoRetrievingEnabled"))
            {
                pBegin->Value >>= _rbAutoRetrievingEnabled;
            }
            else if(!pBegin->Name.compareToAscii("AutoRetrievingStatement"))
            {
                pBegin->Value >>= _rsGeneratedValueStatement;
            }
            else if(!pBegin->Name.compareToAscii("ParameterNameSubstitution"))
            {
                pBegin->Value >>= _bParameterSubstitution;
            }
            else if(!pBegin->Name.compareToAscii("IgnoreDriverPrivileges"))
            {
                pBegin->Value >>= _bIgnoreDriverPrivileges;
            }
        }
    }
    catch(SQLException& e)
    {
        if( object )
        {
            t.pEnv->DeleteGlobalRef( object );
            object = NULL;
        }
        throw SQLException(::rtl::OUString::createFromAscii("The specified driver could not be loaded!"),*this,::rtl::OUString(),1000,makeAny(e));
    }
    catch(Exception&)
    {
        if( object )
        {
            t.pEnv->DeleteGlobalRef( object );
            object = NULL;
        }
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The specified driver could not be loaded!")) ,*this);
    }
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Driver::connect( const ::rtl::OUString& url, const
                                                         Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t(getORB()); OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if(!t.pEnv)
        throw SQLException(::rtl::OUString::createFromAscii("No Java installed!"),*this,::rtl::OUString::createFromAscii("S1000"),1000 ,Any());
    Reference< XConnection > xRet;


    ::rtl::OUString     sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
    sal_Bool            bAutoRetrievingEnabled = sal_False; // set to <TRUE/> when we should allow to query for generated values
    sal_Bool            bParameterSubstitution = sal_False; // set to <TRUE/> when we should subsitute named paramteres
    sal_Bool            bIgnoreDriverPrivileges= sal_False;
    loadDriverFromProperties(info,sGeneratedValueStatement,bAutoRetrievingEnabled,bParameterSubstitution,bIgnoreDriverPrivileges);
    jobject out(0);

    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/util/Properties;)Ljava/sql/Connection;";
        char * cMethodName = "connect";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        ThrowSQLException(t.pEnv,*this);
        if( mID )
        {
            jvalue args[2];
            // Parameter konvertieren
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            java_util_Properties* pProps = createStringPropertyArray(t.pEnv,info);
            args[1].l = pProps->getJavaObject();

            out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l );
            try
            {
                ThrowSQLException(t.pEnv,*this);
            }
            catch(const SQLException& )
            {
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
                delete pProps;
                if( object )
                {
                    t.pEnv->DeleteGlobalRef( object );
                    object = NULL;
                }
                throw;
            }
            // und aufraeumen
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
            delete pProps;
            if( object )
            {
                t.pEnv->DeleteGlobalRef( object );
                object = NULL;
            }
            ThrowSQLException(t.pEnv,*this);

        } //mID
        if( object )
        {
            t.pEnv->DeleteGlobalRef( theClass );
            theClass = NULL;
            t.pEnv->DeleteGlobalRef( object );
            object = NULL;
        }
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    Reference< XConnection > xOut;
    return out==0 ? 0 : new java_sql_Connection( t.pEnv, out,this,sGeneratedValueStatement,bAutoRetrievingEnabled,bParameterSubstitution ,bIgnoreDriverPrivileges);
    //  return xOut;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::acceptsURL( const ::rtl::OUString& url ) throw(SQLException, RuntimeException)
{
    // don't ask the real driver for the url
    // I feel responsible for all jdbc url's
    static const ::rtl::OUString s_sJdbcPrefix = ::rtl::OUString::createFromAscii("jdbc:");
    return 0 == url.compareTo(s_sJdbcPrefix, 5);
}
// -------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL java_sql_Driver::getPropertyInfo( const ::rtl::OUString& url,
                                                                         const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);

    SDBThreadAttach t(getORB()); OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");

    if ( !object )
    {
        // driver was not loaded so far, load it by name
        ::rtl::OUString     sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
        sal_Bool            bAutoRetrievingEnabled = sal_False; // set to when we should allow to query for generated values
        sal_Bool            bParameterSubstitution = sal_False; // set to <TRUE/> when we should subsitute named paramteres
        sal_Bool            bIgnoreDriverPrivileges= sal_False;
        loadDriverFromProperties(info,sGeneratedValueStatement,bAutoRetrievingEnabled,bParameterSubstitution,bIgnoreDriverPrivileges);
    }

    if(!object)
    {
        // one of these must throw an exception
        ThrowSQLException(t.pEnv,*this);
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The specified driver could not be loaded!")) ,*this);
    }

    jobjectArray out(0);

    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/lang/String;Ljava/util/Properties;)[Ljava/sql/DriverPropertyInfo;";
        char * cMethodName = "getPropertyInfo";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args[2];
            // Parameter konvertieren
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            java_util_Properties* pProps = createStringPropertyArray(t.pEnv,info);
            args[1].l = pProps->getJavaObject();

            out = (jobjectArray)t.pEnv->CallObjectMethodA( object, mID, args );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
            delete pProps;
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return copyArrayAndDelete( t.pEnv, out, DriverPropertyInfo(),java_sql_DriverPropertyInfo(NULL,NULL));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  ) throw(RuntimeException)
{
    if(!object)
        return 1;
    jint out(0);
    SDBThreadAttach t(getORB()); OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMajorVersion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  ) throw(RuntimeException)
{
    if(!object)
        return 0;
    jint out(0);
    SDBThreadAttach t(getORB()); OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getMinorVersion";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------


