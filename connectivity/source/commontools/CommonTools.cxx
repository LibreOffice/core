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


#include <stdio.h>
#include "connectivity/CommonTools.hxx"
#include "connectivity/dbtools.hxx"
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/extract.hxx>
#include <cppuhelper/interfacecontainer.h>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/java/XJavaVM.hpp>
#include <rtl/process.h>

using namespace ::comphelper;
inline sal_Unicode rtl_ascii_toUpperCase( sal_Unicode ch )
{
    return ch >= 0x0061 && ch <= 0x007a ? ch + 0x20 : ch;
}

namespace connectivity
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace dbtools;
    namespace starjava  = com::sun::star::java;
    //------------------------------------------------------------------------------
    const sal_Unicode CHAR_PLACE = '_';
    const sal_Unicode CHAR_WILD  = '%';
    // -------------------------------------------------------------------------
    sal_Bool match(const sal_Unicode* pWild, const sal_Unicode* pStr, const sal_Unicode cEscape)
    {
        int    pos=0;
        int    flag=0;

        while ( *pWild || flag )
        {
            switch (*pWild)
            {
                case CHAR_PLACE:
                    if ( *pStr == 0 )
                        return sal_False;
                    break;
                default:
                    if (*pWild && (*pWild == cEscape) && ((*(pWild+1)== CHAR_PLACE) || (*(pWild+1) == CHAR_WILD)) )
                        pWild++;
                    if ( rtl_ascii_toUpperCase(*pWild) != rtl_ascii_toUpperCase(*pStr) )
                        if ( !pos )
                            return sal_False;
                        else
                            pWild += pos;
                    else
                        break;          // WARNING in certain circumstances
                // it will run into the next 'case'!!
                case CHAR_WILD:
                    while ( *pWild == CHAR_WILD )
                        pWild++;
                    if ( *pWild == 0 )
                        return sal_True;
                    flag = 1;
                    pos  = 0;
                    if ( *pStr == 0 )
                        return ( *pWild == 0 );
                    while ( *pStr && *pStr != *pWild )
                    {
                        if ( *pWild == CHAR_PLACE ) {
                            pWild++;
                            while ( *pWild == CHAR_WILD )
                                pWild++;
                        }
                        pStr++;
                        if ( *pStr == 0 )
                            return ( *pWild == 0 );
                    }
                    break;
            }
            if ( *pWild != 0 )
                pWild++;
            if ( *pStr != 0 )
                pStr++;
            else
                flag = 0;
            if ( flag )
                pos--;
        }
        return ( *pStr == 0 ) && ( *pWild == 0 );
    }
    //------------------------------------------------------------------
    rtl::OUString toDateString(const ::com::sun::star::util::Date& rDate)
    {
        sal_Char s[11];
        snprintf(s,
                sizeof(s),
                "%04d-%02d-%02d",
                (int)rDate.Year,
                (int)rDate.Month,
                (int)rDate.Day);
        s[10] = 0;
        return rtl::OUString::createFromAscii(s);
    }

    //------------------------------------------------------------------
    rtl::OUString toTimeString(const ::com::sun::star::util::Time& rTime)
    {
        sal_Char s[9];
        snprintf(s,
                sizeof(s),
                "%02d:%02d:%02d",
                (int)rTime.Hours,
                (int)rTime.Minutes,
                (int)rTime.Seconds);
        s[8] = 0;
        return rtl::OUString::createFromAscii(s);
    }

    //------------------------------------------------------------------
    rtl::OUString toDateTimeString(const ::com::sun::star::util::DateTime& rDateTime)
    {
        sal_Char s[20];
        snprintf(s,
                sizeof(s),
                "%04d-%02d-%02d %02d:%02d:%02d",
                (int)rDateTime.Year,
                (int)rDateTime.Month,
                (int)rDateTime.Day,
                (int)rDateTime.Hours,
                (int)rDateTime.Minutes,
                (int)rDateTime.Seconds);
        s[19] = 0;
        return rtl::OUString::createFromAscii(s);
    }

    // -----------------------------------------------------------------------------
    ::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM(const Reference<XMultiServiceFactory >& _rxFactory)
    {
        ::rtl::Reference< jvmaccess::VirtualMachine > aRet;
        OSL_ENSURE(_rxFactory.is(),"No XMultiServiceFactory a.v.!");
        if(!_rxFactory.is())
            return aRet;

        try
        {
            Reference< starjava::XJavaVM > xVM(_rxFactory->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine"))), UNO_QUERY);

            OSL_ENSURE(_rxFactory.is(),"InitJava: I have no factory!");
            if (!xVM.is() || !_rxFactory.is())
                throw Exception(); // -2;

            Sequence<sal_Int8> processID(16);
            rtl_getGlobalProcessId( (sal_uInt8*) processID.getArray() );
            processID.realloc(17);
            processID[16] = 0;

            Any uaJVM = xVM->getJavaVM( processID );

            if (!uaJVM.hasValue())
                throw Exception(); // -5
            else
            {
                sal_Int32 nValue = 0;
                jvmaccess::VirtualMachine* pJVM = NULL;
                if ( uaJVM >>= nValue )
                    pJVM = reinterpret_cast< jvmaccess::VirtualMachine* > (nValue);
                else
                {
                    sal_Int64 nTemp = 0;
                    uaJVM >>= nTemp;
                    pJVM = reinterpret_cast< jvmaccess::VirtualMachine* > (nTemp);
                }
                aRet = pJVM;
            }
        }
        catch (Exception&)
        {
        }

        return aRet;
    }
    //------------------------------------------------------------------------------
    sal_Bool existsJavaClassByName( const ::rtl::Reference< jvmaccess::VirtualMachine >& _pJVM,const ::rtl::OUString& _sClassName )
    {
        sal_Bool bRet = sal_False;
#ifdef SOLAR_JAVA
        if ( _pJVM.is() )
        {
            jvmaccess::VirtualMachine::AttachGuard aGuard(_pJVM);
            JNIEnv* pEnv = aGuard.getEnvironment();
            if( pEnv )
            {
                ::rtl::OString sClassName = ::rtl::OUStringToOString(_sClassName, RTL_TEXTENCODING_ASCII_US);
                sClassName = sClassName.replace('.','/');
                jobject out = pEnv->FindClass(sClassName.getStr());
                bRet = out != NULL;
                pEnv->DeleteLocalRef( out );
            }
        }
#else
        (void)_pJVM;
        (void)_sClassName;
#endif
        return bRet;
    }

}

#include <ctype.h>      //isdigit
namespace dbtools
{
//------------------------------------------------------------------
sal_Bool isCharOk(sal_Unicode c,const ::rtl::OUString& _rSpecials)
{

    return ( ((c >= 97) && (c <= 122)) || ((c >= 65) && (c <=  90)) || ((c >= 48) && (c <=  57)) ||
          c == '_' || _rSpecials.indexOf(c) != -1);
}

//------------------------------------------------------------------------------
sal_Bool isValidSQLName(const ::rtl::OUString& rName,const ::rtl::OUString& _rSpecials)
{
    // Test for correct naming (in SQL sense)
    // This is important for table names for example
    const sal_Unicode* pStr = rName.getStr();
    if (*pStr > 127 || isdigit(*pStr))
        return sal_False;

    for (; *pStr; ++pStr )
        if(!isCharOk(*pStr,_rSpecials))
            return sal_False;

    if  (   !rName.isEmpty()
        &&  (   (rName.toChar() == '_')
            ||  (   (rName.toChar() >= '0')
                &&  (rName.toChar() <= '9')
                )
            )
        )
        return sal_False;
    // the SQL-Standard requires the first character to be an alphabetic character, which
    // isn't easy to decide in UniCode ...
    // So we just prohibit the characters which already lead to problems ....
    // 11.04.00 - 74902 - FS

    return sal_True;
}
//------------------------------------------------------------------
// Creates a new name if necessary
::rtl::OUString convertName2SQLName(const ::rtl::OUString& rName,const ::rtl::OUString& _rSpecials)
{
    if(isValidSQLName(rName,_rSpecials))
        return rName;
    ::rtl::OUString aNewName(rName);
    const sal_Unicode* pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();
    sal_Bool bValid(*pStr < 128 && !isdigit(*pStr));
    for (sal_Int32 i=0; bValid && i < nLength; ++pStr,++i )
        if(!isCharOk(*pStr,_rSpecials))
        {
            aNewName = aNewName.replace(*pStr,'_');
            pStr = aNewName.getStr() + i;
        }

    if ( !bValid )
        aNewName = ::rtl::OUString();

    return aNewName;
}
//------------------------------------------------------------------------------
::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName)
{
    ::rtl::OUString sName = _rName;
    if( !_rQuote.isEmpty() && _rQuote.toChar() != ' ')
        sName = _rQuote + _rName + _rQuote;
    return sName;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
