/*************************************************************************
 *
 *  $RCSfile: CommonTools.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:51:00 $
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

#include <stdio.h>

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVAVM_HPP_
#include <com/sun/star/java/XJavaVM.hpp>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

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
                        break;          // ACHTUNG laeuft unter bestimmten
                                        // Umstaenden in den nachsten case rein!!
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
        sal_Char s[21];
        snprintf(s,
                sizeof(s),
                "%04d-%02d-%02d %02d:%02d:%02d",
                (int)rDateTime.Year,
                (int)rDateTime.Month,
                (int)rDateTime.Day,
                (int)rDateTime.Hours,
                (int)rDateTime.Minutes,
                (int)rDateTime.Seconds);
        s[20] = 0;
        return rtl::OUString::createFromAscii(s);
    }


    //--------------------------------------------------------------------------------------------------
    rtl::OUString toString(const Any& rValue)
    {
        rtl::OUString aRes;
        TypeClass aDestinationClass = rValue.getValueType().getTypeClass();

        switch (aDestinationClass)
        {
            case TypeClass_CHAR:
                aRes = ::rtl::OUString::valueOf(*(sal_Unicode*)rValue.getValue());
                break;
            case TypeClass_FLOAT:
                aRes = ::rtl::OUString::valueOf(*(float*)rValue.getValue());
                break;
            case TypeClass_DOUBLE:
                aRes = ::rtl::OUString::valueOf(*(double*)rValue.getValue());
                break;
            case TypeClass_BOOLEAN:
                aRes = ::rtl::OUString::valueOf((sal_Int32)*(sal_Bool*)rValue.getValue());
                break;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                aRes = ::rtl::OUString::valueOf(*(sal_Int32*)rValue.getValue());
                break;
            case TypeClass_STRING:
                rValue >>= aRes;
                break;
            case TypeClass_STRUCT:
                if (rValue.getValueType() == ::getCppuType((const ::com::sun::star::util::Date*)0))
                {
                    ::com::sun::star::util::Date aDate;
                    rValue >>= aDate;
                    aRes = toDateString(aDate);
                }
                else if (rValue.getValueType() == ::getCppuType((const ::com::sun::star::util::DateTime*)0))
                {
                    ::com::sun::star::util::DateTime aDT;
                    rValue >>= aDT;
                    aRes = toDateTimeString(aDT);
                }
                else if (rValue.getValueType() == ::getCppuType((const ::com::sun::star::util::Time*)0))
                {
                    ::com::sun::star::util::Time aTime;
                    rValue >>= aTime;
                    aRes = toTimeString(aTime);
                }

                break;
            default:
                ;
//              throw( CannotConvertException( ::rtl::OUString::createFromAscii("TYPE is not supported!"), Reference< XInterface > (),
//                                         aDestinationClass, FailReason::TYPE_NOT_SUPPORTED, 0 ) );

        }
        return aRes;
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
                rtl::OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);

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
                sal_Int32 nValue;
                jvmaccess::VirtualMachine* pJVM = NULL;
                if ( uaJVM >>= nValue )
                    pJVM = reinterpret_cast< jvmaccess::VirtualMachine* > (nValue);
                else
                {
                    sal_Int64 nTemp;
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
        jvmaccess::VirtualMachine::AttachGuard aGuard(_pJVM);
        JNIEnv* pEnv = aGuard.getEnvironment();
        if( pEnv )
        {
            ::rtl::OString sClassName = ::rtl::OUStringToOString(_sClassName, RTL_TEXTENCODING_ASCII_US);
            sClassName = sClassName.replace('.','/');
            jobject out = pEnv->FindClass(sClassName);
            bRet = out != NULL;
            pEnv->DeleteLocalRef( out );
        }
        return bRet;
    }

}

#include <ctype.h>      //isdigit
namespace dbtools
{
sal_Bool isCharOk(char c,const ::rtl::OUString& _rSpecials);

//------------------------------------------------------------------------------
sal_Bool isValidSQLName(const ::rtl::OUString& rName,const ::rtl::OUString& _rSpecials)
{
    // Überprüfung auf korrekte Namensgebung im SQL Sinne
    // Dieses ist wichtig für Tabellennamen beispielsweise
    ::rtl::OString aName(rName,rName.getLength(),RTL_TEXTENCODING_ASCII_US);
    const char* pStr = aName.getStr();
    if (isdigit(*pStr))
        return sal_False;

    for (; *pStr; ++pStr )
        if(!isCharOk(*pStr,_rSpecials))
            return sal_False;

    if  (   rName.getLength()
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
sal_Bool isCharOk(char c,const ::rtl::OUString& _rSpecials)
{

    return ( ((c >= 97) && (c <= 122)) || ((c >= 65) && (c <=  90)) || ((c >= 48) && (c <=  57)) ||
          c == '_' || _rSpecials.indexOf(c) != -1);
}
//------------------------------------------------------------------
// Erzeugt einen neuen Namen falls noetig
::rtl::OUString convertName2SQLName(const ::rtl::OUString& rName,const ::rtl::OUString& _rSpecials)
{
    if(isValidSQLName(rName,_rSpecials))
        return rName;
    ::rtl::OUString aNewName(rName);
    const sal_Unicode* pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();
    sal_Bool bValid(!isdigit(*pStr));
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
    if(_rQuote.getLength() && _rQuote.toChar() != ' ')
        sName = _rQuote + _rName + _rQuote;
    return sName;
}


}
