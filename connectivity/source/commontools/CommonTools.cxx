/*************************************************************************
 *
 *  $RCSfile: CommonTools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-30 07:46:12 $
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

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
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
#include <stdio.h>
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

namespace connectivity
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace dbtools;
    //------------------------------------------------------------------------------
    sal_Int32 getINT32(const Any& _rAny)
    {
        sal_Int32 nReturn = 0;
        _rAny >>= nReturn;
        return nReturn;
    }

    //------------------------------------------------------------------------------
    sal_Int16 getINT16(const Any& _rAny)
    {
        sal_Int16 nReturn = 0;
        _rAny >>= nReturn;
        return nReturn;
    }

    //------------------------------------------------------------------------------
    double getDouble(const Any& _rAny)
    {
        double nReturn = 0.0;
        _rAny >>= nReturn;
        return nReturn;
    }

    //------------------------------------------------------------------------------
    ::rtl::OUString getString(const Any& _rAny)
    {
        ::rtl::OUString nReturn;
        _rAny >>= nReturn;
        return nReturn;
    }

    //------------------------------------------------------------------------------
    sal_Bool getBOOL(const Any& _rAny)
    {
        return ::cppu::any2bool(_rAny);
    }
    //------------------------------------------------------------------
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
                    if ( rtl_char_toUpperCase(*pWild) != rtl_char_toUpperCase(*pStr) )
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
        sprintf(s,"%04d-%02d-%02d",
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
        sprintf(s,"%02d:%02d:%02d",
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
        sprintf(s,"%04d-%02d-%02d %02d:%02d:%02d",
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
                aRes = ::rtl::OUString::valueOf(*(sal_Bool*)rValue.getValue());
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
    // -------------------------------------------------------------------------
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        while (__first != __last && !_rCase(getString((*__first)->getPropertyValue(PROPERTY_NAME)),_rVal))
            ++__first;
        return __first;
    }
    // -------------------------------------------------------------------------
    OSQLColumns::const_iterator findRealName(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        while (__first != __last && !_rCase(getString((*__first)->getPropertyValue(PROPERTY_REALNAME)),_rVal))
            ++__first;
        return __first;
    }
    // -------------------------------------------------------------------------
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rProp,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        while (__first != __last && !_rCase(getString(Reference<XPropertySet>((*__first),UNO_QUERY)->getPropertyValue(_rProp)),_rVal))
            ++__first;
        return __first;
    }
}
