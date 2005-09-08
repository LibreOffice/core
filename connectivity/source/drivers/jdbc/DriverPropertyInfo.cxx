/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverPropertyInfo.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:10:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERPOPERTYINFO_HXX_
#include "java/sql/DriverPropertyInfo.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_BOOLEAN_HXX_
#include "java/lang/Boolean.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_STRING_HXX_
#include "java/lang/String.hxx"
#endif
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

