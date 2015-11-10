/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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



//************ Class: java.sql.Driver


jclass java_sql_DriverPropertyInfo::theClass = nullptr;

java_sql_DriverPropertyInfo::~java_sql_DriverPropertyInfo()
{}

jclass java_sql_DriverPropertyInfo::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/DriverPropertyInfo");
    return theClass;
}


java_sql_DriverPropertyInfo::operator css::sdbc::DriverPropertyInfo()
{
    css::sdbc::DriverPropertyInfo aInfo;
    aInfo.Name = name();
    aInfo.Description = description();
    aInfo.IsRequired = required();
    aInfo.Value = value();
    aInfo.Choices = choices();

    return aInfo;
}

OUString java_sql_DriverPropertyInfo::name()
{
    OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"name","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv, static_cast<jstring>(t.pEnv->GetObjectField( object, id)));
    } //t.pEnv
    return aStr;
}

OUString java_sql_DriverPropertyInfo::description()
{
    OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"description","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv, static_cast<jstring>(t.pEnv->GetObjectField( object, id)));
    } //t.pEnv
    return aStr;
}

OUString java_sql_DriverPropertyInfo::value()
{
    OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"value","Ljava/lang/String;");
        if(id)
            aStr = JavaString2String(t.pEnv, static_cast<jstring>(t.pEnv->GetObjectField( object, id)));
    } //t.pEnv
    return aStr;
}

bool java_sql_DriverPropertyInfo::required()
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

Sequence< OUString> java_sql_DriverPropertyInfo::choices()
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jfieldID id = t.pEnv->GetFieldID(getMyClass(),"choices","[Ljava/lang/String;");
        if(id)
        {
            const java_lang_String * pEmpty = nullptr;
            const OUString * pEmpty2 = nullptr;
            return copyArrayAndDelete(t.pEnv, static_cast<jobjectArray>(t.pEnv->GetObjectField( object, id)), pEmpty2, pEmpty);
        }
    } //t.pEnv
    return Sequence< OUString>();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
