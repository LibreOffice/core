/*************************************************************************
 *
 *  $RCSfile: OTools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-14 11:34:08 $
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
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#define _CONNECTIVITY_OTOOLS_HXX_

#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
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
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

namespace connectivity
{
    namespace odbc
    {

        const sal_uInt32 ODBC_FRACTION_UNITS_PER_HSECOND = 10000000L;
        const sal_Int32 MAX_PUT_DATA_LENGTH = 2000;

        class OTools
        {
        public:
            static void ThrowException( SQLRETURN _rRetCode,SQLHANDLE _pContext,SQLSMALLINT _nHandleType,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,sal_Bool _bNoFound=sal_True,
                                        rtl_TextEncoding _nTextEncoding = RTL_TEXTENCODING_MS_1252)
                                        throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,::rtl::OUString &_rValue,
                                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                                                rtl_TextEncoding _nTextEncoding)
                                                                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,sal_Int32 &_rValue,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static void GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,sal_Bool &_rValue,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static sal_Int32 MapOdbcType2Jdbc(sal_Int32 _nType);
            static sal_Int32 jdbcTypeToOdbc(sal_Int32 jdbcType);

            static DATE_STRUCT DateToOdbcDate(const ::com::sun::star::util::Date& x)
            {
                DATE_STRUCT aVal;
                aVal.year   = x.Year;
                aVal.month  = x.Month;
                aVal.day    = x.Day;
                return aVal;
            }
            static TIME_STRUCT TimeToOdbcTime(const ::com::sun::star::util::Time& x)
            {
                TIME_STRUCT aVal;
                aVal.hour   = x.Hours;
                aVal.minute = x.Minutes;
                aVal.second = x.Seconds;
                return aVal;
            }
            static TIMESTAMP_STRUCT DateTimeToTimestamp(const ::com::sun::star::util::DateTime& x)
            {
                TIMESTAMP_STRUCT aVal;
                aVal.year       = x.Year;
                aVal.month      = x.Month;
                aVal.day        = x.Day;
                aVal.hour       = x.Hours;
                aVal.minute     = x.Minutes;
                aVal.second     = x.Seconds;
                aVal.fraction   = x.HundredthSeconds * ODBC_FRACTION_UNITS_PER_HSECOND;
                return aVal;
            }
            static void getBindTypes(sal_Bool _bUseWChar,sal_Bool _bUseOldTimeDate,
                              sal_Int32 jdbcType,SQLSMALLINT& fCType,SQLSMALLINT& fSqlType,
                              SQLUINTEGER& nColumnSize,SQLSMALLINT& nDecimalDigits);
            static ::rtl::OUString getStringValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,SWORD  _fSqlType,sal_Bool &_bWasNull,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                                rtl_TextEncoding _nTextEncoding) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static  ::com::sun::star::uno::Sequence<sal_Int8> OTools::getBytesValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,SWORD  _fSqlType,sal_Bool &_bWasNull,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static void getValue(   SQLHANDLE _aStatementHandle,
                                        sal_Int32 columnIndex,
                                        SQLSMALLINT _nType,
                                        sal_Bool &_bWasNull,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                        void* _pValue,
                                        SQLINTEGER _rSize) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void bindData(   SWORD fSqlType,
                                            sal_Bool _bUseWChar,
                                            void *&_pData,
                                            SDWORD*& pLen,
                                            const void* _pValue,
                                            rtl_TextEncoding _nTextEncoding);

            static void bindParameter(SQLHANDLE _hStmt,
                            sal_Int32 nPos,
                            sal_Int8* pDataBuffer,
                            sal_Int8* pLenBuffer,
                            SQLSMALLINT _nJDBCtype,
                            sal_Bool _bUseWChar,
                            sal_Bool _bUseOldTimeDate,
                            const void* _pValue,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                            rtl_TextEncoding _nTextEncoding)
                             throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static void bindValue(  SQLHANDLE _aStatementHandle,
                                            sal_Int32 columnIndex,
                                            SQLSMALLINT _nType,
                                            SQLSMALLINT _nMaxLen,
                                            SQLSMALLINT _nScale,
                                            const void* _pValue,
                                            void* _pData,
                                            SQLINTEGER *pLen,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                            rtl_TextEncoding _nTextEncoding) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };

        template <class T> void getValue(   SQLHANDLE _aStatementHandle,
                                        sal_Int32 columnIndex,
                                        SQLSMALLINT _nType,
                                        sal_Bool &_bWasNull,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                        T& _rValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            OTools::getValue(_aStatementHandle,columnIndex,_nType,_bWasNull,_xInterface,&_rValue,sizeof _rValue);
        }
        //-----------------------------------------------------------------------------


    }
}
#endif // _CONNECTIVITY_OTOOLS_HXX_

