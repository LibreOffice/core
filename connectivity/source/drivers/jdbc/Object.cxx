/*************************************************************************
 *
 *  $RCSfile: Object.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:05:14 $
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


#ifndef _CONNECTIVITY_JAVA_LANG_OBJJECT_HXX_
#include "java/lang/Class.hxx"
#endif
#include "connectivity/CommonTools.hxx"

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#include "java/sql/SQLException.hxx"
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


// -----------------------------------------------------------------------------
::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM2(const ::rtl::Reference< jvmaccess::VirtualMachine >& _rVM = ::rtl::Reference< jvmaccess::VirtualMachine >(),
                                                        sal_Bool _bSet = sal_False)
{
    static ::rtl::Reference< jvmaccess::VirtualMachine > s_VM;
    if ( _rVM.is() || _bSet )
        s_VM = _rVM;
    return s_VM;
}
// -----------------------------------------------------------------------------
::rtl::Reference< jvmaccess::VirtualMachine > java_lang_Object::getVM(const Reference<XMultiServiceFactory >& _rxFactory)
{
    ::rtl::Reference< jvmaccess::VirtualMachine > xVM = getJavaVM2();
    if ( !xVM.is() && _rxFactory.is() )
        xVM = getJavaVM2(::connectivity::getJavaVM(_rxFactory));

    return xVM;
}
// -----------------------------------------------------------------------------
SDBThreadAttach::SDBThreadAttach()
 : pEnv(NULL)
{
    m_aGuard = ::std::auto_ptr< jvmaccess::VirtualMachine::AttachGuard>(new jvmaccess::VirtualMachine::AttachGuard(java_lang_Object::getVM()) );
    if ( m_aGuard.get() )
        pEnv = m_aGuard->getEnvironment();
}
// -----------------------------------------------------------------------------
SDBThreadAttach::~SDBThreadAttach()
{
}
// -----------------------------------------------------------------------------
sal_Int32& getJavaVMRefCount()
{
    static sal_Int32 s_nRefCount = 0;
    return s_nRefCount;
}
// -----------------------------------------------------------------------------
void SDBThreadAttach::addRef()
{
    getJavaVMRefCount()++;
}
// -----------------------------------------------------------------------------
void SDBThreadAttach::releaseRef()
{
    getJavaVMRefCount()--;
    if ( getJavaVMRefCount() == 0 )
        getJavaVM2(::rtl::Reference< jvmaccess::VirtualMachine >(),sal_True);
}
// -----------------------------------------------------------------------------
// statische Variablen der Klasse:
jclass java_lang_Object::theClass = 0;
sal_uInt32 java_lang_Object::nObjCount = 0;

jclass java_lang_Object::getMyClass()
{
    if( !theClass )
    {
        SDBThreadAttach t;

        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/lang/Object" );
        theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
    }
    return theClass;
}
// der eigentliche Konstruktor
java_lang_Object::java_lang_Object(const Reference<XMultiServiceFactory >& _rxFactory)
            : object( 0 ),m_xFactory(_rxFactory)
{
}

// der protected-Konstruktor fuer abgeleitete Klassen
java_lang_Object::java_lang_Object( JNIEnv * pXEnv, jobject myObj )
    : object( NULL )
{
    SDBThreadAttach t;
    if( t.pEnv && myObj )
        object = t.pEnv->NewGlobalRef( myObj );
}

java_lang_Object::~java_lang_Object()
{
    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
            t.pEnv->DeleteGlobalRef( object );
    }
}

// der protected-Konstruktor fuer abgeleitete Klassen
void java_lang_Object::saveRef( JNIEnv * pXEnv, jobject myObj )
{
    OSL_ENSURE( myObj, "object in c++ -> Java Wrapper" );
    SDBThreadAttach t;
    if( t.pEnv && myObj )
        object = t.pEnv->NewGlobalRef( myObj );
}


java_lang_Class * java_lang_Object::getClass()
{
    jobject out;
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/Class;";
        char * cMethodName = "getClass";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallObjectMethodA( object, mID, NULL );
            ThrowSQLException(t.pEnv,NULL);
            return new java_lang_Class( t.pEnv, out );
        } //mID
    } //pEnv
    return NULL;
}

::rtl::OUString java_lang_Object::toString()
{

    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "toString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jstring out(0);
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,NULL);
            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //pEnv
    return  aStr;
}
// --------------------------------------------------------------------------------
void java_lang_Object::ThrowSQLException(JNIEnv * pEnv,const Reference< XInterface> & _rContext) throw(SQLException, RuntimeException)
{
    jthrowable jThrow = NULL;
    if(pEnv && (jThrow = pEnv->ExceptionOccurred()))
    {
        pEnv->ExceptionClear();// we have to clear the exception here because we want to handle it itself
        if(pEnv->IsInstanceOf(jThrow,java_sql_SQLException_BASE::getMyClass()))
        {
            java_sql_SQLException_BASE* pException = new java_sql_SQLException_BASE(pEnv,jThrow);
            SQLException e( pException->getMessage(),
                                _rContext,
                                pException->getSQLState(),
                                pException->getErrorCode(),
                                Any()
                            );
            delete pException;
            throw  e;
        }
        else if(pEnv->IsInstanceOf(jThrow,java_lang_Throwable::getMyClass()))
        {
            java_lang_Throwable *pThrow = new java_lang_Throwable(pEnv,jThrow);
            ::rtl::OUString aMsg = pThrow->getMessage();
            if(!aMsg.getLength())
                aMsg = pThrow->getLocalizedMessage();
            if(!aMsg.getLength())
                aMsg = pThrow->toString();
            delete pThrow;
            throw SQLException(aMsg,_rContext,::rtl::OUString(),-1,Any());
        }
        else
            pEnv->DeleteLocalRef( jThrow );
    }
}


