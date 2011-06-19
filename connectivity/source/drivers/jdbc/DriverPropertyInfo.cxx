/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "java/sql/DriverPropertyInfo.hxx"
#include "java/lang/Boolean.hxx"
#include "java/tools.hxx"
#include "java/lang/String.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
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
jclass java_sql_DriverPropertyInfo::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/DriverPropertyInfo");
    return theClass;
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
::rtl::OUString java_sql_DriverPropertyInfo::name()
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"name","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
::rtl::OUString java_sql_DriverPropertyInfo::description()
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"description","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
::rtl::OUString java_sql_DriverPropertyInfo::value()
{
    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"value","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv,(jstring)t.pEnv->GetObjectField( object, id));
    } //t.pEnv
    return aStr;
}
// --------------------------------------------------------------------------------
sal_Bool java_sql_DriverPropertyInfo::required()
{
    jboolean out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"required","Z");
        if(id)
            out = t.pEnv->GetBooleanField( object, id);
    } //t.pEnv
    return out;
}
// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString> java_sql_DriverPropertyInfo::choices()
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"choices","[Ljava/lang/String;");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
