/*************************************************************************
 *
 *  $RCSfile: Reader.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 17:03:17 $
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

#ifndef _CONNECTIVITY_JAVA_IO_READER_HXX_
#include "java/io/Reader.hxx"
#endif
#ifndef _INC_MEMORY
#include <memory.h>
#endif
using namespace connectivity;
//**************************************************************
//************ Class: java.io.Reader
//**************************************************************

jclass java_io_Reader::theClass = 0;

java_io_Reader::~java_io_Reader()
{}

jclass java_io_Reader::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/io/Reader" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_io_Reader::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

sal_Int32 SAL_CALL java_io_Reader::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_Reader::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "(I)I";
        char * cMethodName = "skip";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallIntMethod( object, mID,nBytesToSkip);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
}

sal_Int32 SAL_CALL java_io_Reader::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "()Z";
        char * cMethodName = "available";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
    return out;
}
void SAL_CALL java_io_Reader::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "()V";
        char * cMethodName = "close";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
}
// -----------------------------------------------------
sal_Int32 SAL_CALL java_io_Reader::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(aData.getLength() < nBytesToRead," Sequence is smaller than BytesToRead");
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jcharArray pCharArray = t.pEnv->NewCharArray(nBytesToRead);
        char * cSignature = "([CII)I";
        char * cMethodName = "read";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            while(!(out = t.pEnv->CallIntMethod( object, mID,pCharArray,0,nBytesToRead)))
                ThrowSQLException(t.pEnv,*this);
            if(out > 0)
            {
                jboolean p = sal_False;
                if(aData.getLength() < out)
                    aData.realloc(out-aData.getLength());

                memcpy(aData.getArray(),t.pEnv->GetCharArrayElements(pCharArray,&p),out);
            }
        }
        t.pEnv->DeleteLocalRef((jcharArray)pCharArray);
    } //t.pEnv
    return out;
}

