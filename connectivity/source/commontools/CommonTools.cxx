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

#include <config_features.h>

#include <connectivity/CommonTools.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/extract.hxx>
#include <cppuhelper/interfacecontainer.h>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/java/JavaVirtualMachine.hpp>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
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
    using namespace ::com::sun::star::java;
    using namespace dbtools;

    const sal_Unicode CHAR_PLACE = '_';
    const sal_Unicode CHAR_WILD  = '%';

    bool match(const sal_Unicode* pWild, const sal_Unicode* pStr, const sal_Unicode cEscape)
    {
        int    pos=0;
        int    flag=0;

        while ( *pWild || flag )
        {
            switch (*pWild)
            {
                case CHAR_PLACE:
                    if ( *pStr == 0 )
                        return false;
                    break;
                default:
                    if (*pWild && (*pWild == cEscape) && ((*(pWild+1)== CHAR_PLACE) || (*(pWild+1) == CHAR_WILD)) )
                        pWild++;
                    if ( rtl_ascii_toUpperCase(*pWild) != rtl_ascii_toUpperCase(*pStr) )
                        if ( !pos )
                            return false;
                        else
                            pWild += pos;
                    else
                        break;
                    // WARNING/TODO: in certain circumstances it will run into
                    // the next 'case'!
                    SAL_FALLTHROUGH;
                case CHAR_WILD:
                    while ( *pWild == CHAR_WILD )
                        pWild++;
                    if ( *pWild == 0 )
                        return true;
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

#if HAVE_FEATURE_JAVA
    ::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM(const Reference<XComponentContext >& _rxContext)
    {
        ::rtl::Reference< jvmaccess::VirtualMachine > aRet;
        OSL_ENSURE(_rxContext.is(),"No XMultiServiceFactory a.v.!");
        if(!_rxContext.is())
            return aRet;

        try
        {
            Reference< XJavaVM > xVM = JavaVirtualMachine::create(_rxContext);

            Sequence<sal_Int8> processID(16);
            rtl_getGlobalProcessId( reinterpret_cast<sal_uInt8*>(processID.getArray()) );
            processID.realloc(17);
            processID[16] = 0;

            Any uaJVM = xVM->getJavaVM( processID );

            if (!uaJVM.hasValue())
                throw Exception(); // -5
            else
            {
                sal_Int32 nValue = 0;
                jvmaccess::VirtualMachine* pJVM = nullptr;
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

    bool existsJavaClassByName( const ::rtl::Reference< jvmaccess::VirtualMachine >& _pJVM,const OUString& _sClassName )
    {
        bool bRet = false;
        if ( _pJVM.is() )
        {
            jvmaccess::VirtualMachine::AttachGuard aGuard(_pJVM);
            JNIEnv* pEnv = aGuard.getEnvironment();
            if( pEnv )
            {
                OString sClassName = OUStringToOString(_sClassName, RTL_TEXTENCODING_ASCII_US);
                sClassName = sClassName.replace('.','/');
                jobject out = pEnv->FindClass(sClassName.getStr());
                bRet = out != nullptr;
                pEnv->DeleteLocalRef( out );
            }
        }
        return bRet;
    }
#endif
}

#include <ctype.h>
namespace dbtools
{

bool isCharOk(sal_Unicode c,const OUString& _rSpecials)
{

    return ( ((c >= 97) && (c <= 122)) || ((c >= 65) && (c <=  90)) || ((c >= 48) && (c <=  57)) ||
          c == '_' || _rSpecials.indexOf(c) != -1);
}


bool isValidSQLName(const OUString& rName,const OUString& _rSpecials)
{
    // Test for correct naming (in SQL sense)
    // This is important for table names for example
    const sal_Unicode* pStr = rName.getStr();
    if (*pStr > 127 || isdigit(*pStr))
        return false;

    for (; *pStr; ++pStr )
        if(!isCharOk(*pStr,_rSpecials))
            return false;

    if  (   !rName.isEmpty()
        &&  (   (rName.toChar() == '_')
            ||  (   (rName.toChar() >= '0')
                &&  (rName.toChar() <= '9')
                )
            )
        )
        return false;
    // the SQL-Standard requires the first character to be an alphabetic character, which
    // isn't easy to decide in UniCode ...
    // So we just prohibit the characters which already lead to problems ....
    // 11.04.00 - 74902 - FS

    return true;
}

// Creates a new name if necessary
OUString convertName2SQLName(const OUString& rName,const OUString& _rSpecials)
{
    if(isValidSQLName(rName,_rSpecials))
        return rName;
    OUString aNewName(rName);
    const sal_Unicode* pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();
    bool bValid(*pStr < 128 && !isdigit(*pStr));
    for (sal_Int32 i=0; bValid && i < nLength; ++pStr,++i )
        if(!isCharOk(*pStr,_rSpecials))
        {
            aNewName = aNewName.replace(*pStr,'_');
            pStr = aNewName.getStr() + i;
        }

    if ( !bValid )
        aNewName.clear();

    return aNewName;
}

OUString quoteName(const OUString& _rQuote, const OUString& _rName)
{
    OUString sName = _rName;
    if( !_rQuote.isEmpty() && _rQuote.toChar() != ' ')
        sName = _rQuote + _rName + _rQuote;
    return sName;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
