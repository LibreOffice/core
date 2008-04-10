/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DriverPropertyInfo.cxx,v $
 * $Revision: 1.11 $
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
#include "java/sql/DriverPropertyInfo.hxx"
#include "java/lang/Boolean.hxx"
#include "java/tools.hxx"
#include "java/lang/String.hxx"
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


//**************************************************************
//************ Class: java.sql.Driver
//**************************************************************

jclass java_sql_DriverPropertyInfo::theClass = 0;
// --------------------------------------------------------------------------------
java_sql_DriverPropertyInfo::~java_sql_DriverPropertyInfo()
{}
// --------------------------------------------------------------------------------
jclass java_sql_DriverPropertyInfo::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/DriverPropertyInfo");
        OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

// --------------------------------------------------------------------------------
void java_sql_DriverPropertyInfo::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// --------------------------------------------------------------------------------
java_sql_DriverPropertyInfo::operator starsdbc::DriverPropertyInfo()
{
    starsdbc::DriverPropertyInfo aInfo;
    aInfo.Name = name();
    aInfo.Description = description();
    aInfo.IsRequired = required();
    aInfo.Value = value();
    aInfo.Choices = choices();

    return aInfo;
}
// --------------------------------------------------------------------------------
::rtl::OUString java_sql_DriverPropertyInfo::name() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jfieldID id = t.pEnv->GetFieldID(java_sql_DriverPropertyInfo::getMyClass(),"name","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
::rtl::OUString java_sql_DriverPropertyInfo::description() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jfieldID id = t.pEnv->GetFieldID(java_sql_DriverPropertyInfo::getMyClass(),"description","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
::rtl::OUString java_sql_DriverPropertyInfo::value() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jfieldID id = t.pEnv->GetFieldID(java_sql_DriverPropertyInfo::getMyClass(),"value","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
sal_Bool java_sql_DriverPropertyInfo::required() const
{
    jboolean out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jfieldID id = t.pEnv->GetFieldID(java_sql_DriverPropertyInfo::getMyClass(),"required","Z");
        if(id)
            out = t.pEnv->GetBooleanField( object, id);
    } //t.pEnv
    return out;
}
// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString> java_sql_DriverPropertyInfo::choices() const
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jfieldID id = t.pEnv->GetFieldID(java_sql_DriverPropertyInfo::getMyClass(),"choices","[Ljava/lang/String;");
        if(id)
        {
            const java_lang_String * pEmpty = NULL;
            const ::rtl::OUString * pEmpty2 = NULL;
            return copyArrayAndDelete(t.pEnv,(jobjectArray)t.pEnv->GetObjectField( object, id), pEmpty2, pEmpty);
        }
    } //t.pEnv
    return Sequence< ::rtl::OUString>();
}
// --------------------------------------------------------------------------------

