/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
jclass java_sql_DriverPropertyInfo::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
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

