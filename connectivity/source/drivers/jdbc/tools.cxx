/*************************************************************************
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-22 14:44:26 $
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

#include <cstdarg>
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_STRING_HXX_
#include "java/lang/String.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_LANG_CLASS_HXX_
#include "java/lang/Class.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_DRIVERPROPERTYINFO_HPP_
#include <com/sun/star/sdbc/DriverPropertyInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// --------------------------------------------------------------------------------
jstring connectivity::convertwchar_tToJavaString(JNIEnv *pEnv,const ::rtl::OUString& Temp)
{
    if (pEnv)
    {
        ::rtl::OString aT = ::rtl::OUStringToOString(Temp,RTL_TEXTENCODING_UTF8);
        return pEnv->NewString((const unsigned short *)aT.getStr(), aT.getLength());
    }
    return NULL;
}

// --------------------------------------------------------------------------------
jobjectArray connectivity::createStringPropertyArray(JNIEnv *pEnv,const Sequence< PropertyValue >& info )  throw(SQLException, RuntimeException)
{
    jobjectArray aArray = pEnv->NewObjectArray(info.getLength(),java_lang_String::getMyClass(),pEnv->NewStringUTF(""));

    const PropertyValue* pBegin = info.getConstArray();
    const PropertyValue* pEnd   = pBegin + info.getLength();

    sal_Bool bFound = sal_False;
    for(jsize i=0;pBegin != pEnd;++pBegin)
    {
        // this is a special property to find the jdbc driver
        if(!pBegin->Name.compareToAscii("JDBCDRV"))
        {
            ::rtl::OUString aStr;
            pBegin->Value >>= aStr;
            jstring a = convertwchar_tToJavaString(pEnv,aStr.getStr());
            pEnv->SetObjectArrayElement(aArray,i++,a);
        }
    }
    return aArray;
}
// --------------------------------------------------------------------------------
::rtl::OUString connectivity::JavaString2String(JNIEnv *pEnv,jstring _Str)
{
    ::rtl::OUString aStr;
    if(_Str)
    {
        jboolean bCopy(sal_True);
        const jchar* pChar = pEnv->GetStringChars(_Str,&bCopy);
        jsize len = pEnv->GetStringLength(_Str);
        aStr = ::rtl::OUString(pChar,RTL_TEXTENCODING_UTF8);

        if(bCopy)
            pEnv->ReleaseStringChars(_Str,pChar);
        pEnv->DeleteLocalRef(_Str);
    }
    return aStr;
}
// --------------------------------------------------------------------------------
jobject connectivity::XNameAccess2Map(JNIEnv *pEnv,const Reference< ::com::sun::star::container::XNameAccess > & _rMap)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq = _rMap->getElementNames();
    const ::rtl::OUString *pBegin   = aSeq.getConstArray();
    const ::rtl::OUString *pEnd     = pBegin + aSeq.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
    }
    return 0;
}


