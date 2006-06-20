/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Throwable.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:37:05 $
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
#ifndef _CONNECTIVITY_JAVA_LANG_THROWABLE_HXX_
#include "java/lang/Throwable.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Throwable
//**************************************************************

jclass java_lang_Throwable::theClass = 0;

java_lang_Throwable::~java_lang_Throwable()
{}

jclass java_lang_Throwable::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/lang/Throwable"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}
// -----------------------------------------------------------------------------

void java_lang_Throwable::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getMessage() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getMessage";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,NULL);

            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getLocalizedMessage() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getLocalizedMessage";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,NULL);

            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -----------------------------------------------------------------------------
::rtl::OUString java_lang_Throwable::toString() const
{
    ::rtl::OUString aStr;
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "toString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,NULL);

            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -----------------------------------------------------------------------------


