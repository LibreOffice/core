/*************************************************************************
 *
 *  $RCSfile: pyuno_util.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-24 14:25:40 $
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
 *  The Initial Developer of the Original Code is: Ralph Thomas
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Ralph Thomas, Joerg Budischewski
 *
 *
 ************************************************************************/
#include <osl/thread.h>

#include <typelib/typedescription.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/beans/XMaterialHolder.hpp>

#include "pyuno_impl.hxx"


using rtl::OUStringToOString;
using rtl::OUString;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUStringBuffer;


using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::script::XTypeConverter;
using com::sun::star::beans::XMaterialHolder;

#define USTR_ASCII(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )
namespace pyuno
{
PyRef ustring2PyUnicode( const OUString & str )
{
    PyRef ret;
#if Py_UNICODE_SIZE == 2
    ret = PyRef( PyUnicode_FromUnicode( str.getStr(), str.getLength() ), SAL_NO_ACQUIRE );
#else
    OString sUtf8(OUStringToOString(str, RTL_TEXTENCODING_UTF8));
    ret = PyRef( PyUnicode_DecodeUTF8( sUtf8.getStr(), sUtf8.getLength(), NULL) , SAL_NO_ACQUIRE );
#endif
    return ret;
}

PyRef ustring2PyString( const OUString &str )
{
    OString o = OUStringToOString( str, osl_getThreadTextEncoding() );
    return PyRef( PyString_FromString( o.getStr() ), SAL_NO_ACQUIRE );
}

OUString pyString2ustring( PyObject *pystr )
{
    OUString ret;
    if( PyUnicode_Check( pystr ) )
    {
#if Py_UNICODE_SIZE == 2
    ret = OUString( (sal_Unicode * ) PyUnicode_AS_UNICODE( pystr ) );
#else
    PyObject* pUtf8 = PyUnicode_AsUTF8String(pystr);
    ret = OUString(PyString_AsString(pUtf8), PyString_Size(pUtf8), RTL_TEXTENCODING_UTF8);
    Py_DECREF(pUtf8);
#endif
    }
    else
    {
        char *name = PyString_AsString(pystr );
        ret = OUString( name, strlen(name), osl_getThreadTextEncoding() );
    }
    return ret;
}

PyRef getObjectFromUnoModule( const Runtime &runtime, const char * func )
    throw ( RuntimeException )
{
    PyRef object(PyDict_GetItemString( runtime.getImpl()->cargo->getUnoModule().get(), (char*)func ) );
    if( !object.is() )
    {
        OUStringBuffer buf;
        buf.appendAscii( "couldn't find core function " );
        buf.appendAscii( func );
        throw RuntimeException(buf.makeStringAndClear(),Reference< XInterface >());
    }
    return object;
}

}
