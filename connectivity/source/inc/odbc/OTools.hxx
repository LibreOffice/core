/*************************************************************************
 *
 *  $RCSfile: OTools.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
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
                                                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,sal_Bool _bNoFound=sal_True)
                                                                                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,::rtl::OUString &_rValue,
                                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface)
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
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static  ::com::sun::star::uno::Sequence<sal_Int8> OTools::getBytesValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,SWORD  _fSqlType,sal_Bool &_bWasNull,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };

        template <class T> T getValue(  SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,
                                        SQLSMALLINT _nType,sal_Bool &_bWasNull,
                                                                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,const T& _rValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            SQLINTEGER pcbValue;
            T nValue = _rValue;
            OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                                columnIndex,
                                                _nType,
                                                &nValue,
                                                (SQLINTEGER)sizeof nValue,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface,sal_False);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            return nValue;
        }

        //-----------------------------------------------------------------------------
        template < class T > void bindData(SWORD fSqlType,sal_Bool _bUseWChar,void *&_pData,SDWORD*& pLen,const T* _pValue)
        {
            SDWORD  nMaxLen = 0;

            switch (fSqlType)
            {
                case SQL_CHAR:
                case SQL_VARCHAR:
                    if(_bUseWChar)
                    {
                        *pLen = SQL_NTS;
                        *((rtl::OUString*)_pData) = *(::rtl::OUString*)_pValue;

                        // Zeiger auf Char*
                        _pData = (void*)((rtl::OUString*)_pData)->getStr();
                    }
                    else
                    {
                        ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,osl_getThreadTextEncoding()));
                        *pLen = SQL_NTS;
                        memcpy(_pData,aString.getStr(),aString.getLength());
                        ((sal_Int8*)_pData)[aString.getLength()] = '\0';
                    }   break;

                case SQL_BIGINT:
                case SQL_DECIMAL:
                case SQL_NUMERIC:
                    if(_bUseWChar)
                    {
                        ::rtl::OUString aString = rtl::OUString::valueOf(*(double*)_pValue);
                        nMaxLen = aString.getLength();
                        *pLen = nMaxLen;
                        *((rtl::OUString*)_pData) = aString;
                        // Zeiger auf Char*
                        _pData = (void*)((rtl::OUString*)_pData)->getStr();
                    }
                    else
                    {
                        ::rtl::OString aString = ::rtl::OString::valueOf(*(double*)_pValue);
                        nMaxLen = aString.getLength();
                        *pLen = nMaxLen;
                        memcpy(_pData,aString.getStr(),aString.getLength());
                        ((sal_Int8*)_pData)[aString.getLength()] = '\0';
                    }   break;
                case SQL_BIT:
                    *((sal_Int8*)_pData) = *(sal_Int8*)_pValue;
                    *pLen = sizeof(sal_Int8);
                    break;
                case SQL_TINYINT:
                case SQL_SMALLINT:
                    *((sal_Int16*)_pData) = *(sal_Int16*)_pValue;
                    *pLen = sizeof(sal_Int16);
                    break;
                case SQL_INTEGER:
                    *((sal_Int32*)_pData) = *(sal_Int32*)_pValue;
                    *pLen = sizeof(sal_Int32);
                    break;
                case SQL_REAL:
                    *((float*)_pData) = *(float*)_pValue;
                    *pLen = sizeof(float);
                    break;
                case SQL_DOUBLE:
                    *((double*)_pData) = *(double*)_pValue;
                    *pLen = sizeof(double);
                    break;
                case SQL_BINARY:
                case SQL_VARBINARY:
                    _pData = (void*)((const ::com::sun::star::uno::Sequence< sal_Int8 >  *)_pValue)->getConstArray();
                    *pLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 >  *)_pValue)->getLength();
                    break;
                case SQL_LONGVARBINARY:
                    {
                        sal_Int32 nLen = 0;
                        nLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 > *)_pValue)->getLength();
                        *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
                    }
                    break;
                case SQL_LONGVARCHAR:
                {
                    _pData  = 0;//(void*)&rCol;
                    sal_Int32 nLen = 0;
                    if(_bUseWChar)
                        nLen = sizeof(sal_Unicode) * ((::rtl::OUString*)_pValue)->getLength();
                    else
                    {
                        ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,
                            osl_getThreadTextEncoding()
                            ));
                        nLen = aString.getLength();
                    }
                    *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
                }   break;
                case SQL_DATE:
                    *(DATE_STRUCT*)_pData = *(DATE_STRUCT*)_pValue;
                    *pLen = (SDWORD)sizeof(DATE_STRUCT);
                    break;
                case SQL_TIME:
                    *(TIME_STRUCT*)_pData = *(TIME_STRUCT*)_pValue;
                    *pLen = (SDWORD)sizeof(TIME_STRUCT);
                    break;
                case SQL_TIMESTAMP:
                    *(TIMESTAMP_STRUCT*)_pData = *(TIMESTAMP_STRUCT*)_pValue;
                    *pLen = (SDWORD)sizeof(TIMESTAMP_STRUCT);
                    break;
            }
        }

        //-----------------------------------------------------------------------------
        template < class T > sal_Bool bindParameter(    SQLHANDLE _hStmt,sal_Int32 nPos, sal_Int8* pDataBuffer,
                                                    sal_Int8* pLenBuffer,SQLSMALLINT _nJDBCtype,
                                                    sal_Bool _bUseWChar,sal_Bool _bUseOldTimeDate,const T* _pValue,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface)
                                                     throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            SQLRETURN nRetcode;
            SWORD   fSqlType;
            SWORD   fCType;
            SDWORD  nMaxLen = 0;
            void*   pData   = pDataBuffer;
            SDWORD* pLen    = (SDWORD*)pLenBuffer;
            SQLUINTEGER nColumnSize=0;
            SQLSMALLINT nDecimalDigits=0;
            SQLSMALLINT nNullable=0;

            OTools::getBindTypes(_bUseWChar,_bUseOldTimeDate,_nJDBCtype,fSqlType,fCType,nColumnSize,nDecimalDigits);

            bindData< T >(fSqlType,_bUseWChar,pData,pLen,_pValue);
            if(fSqlType == SQL_LONGVARCHAR || fSqlType == SQL_LONGVARBINARY)
                memcpy(pData,&nPos,sizeof(nPos));

            nRetcode = N3SQLDescribeParam(_hStmt,nPos,&fSqlType,&nColumnSize,&nDecimalDigits,&nNullable);

            nRetcode = N3SQLBindParameter(_hStmt,
                          nPos,
                          SQL_PARAM_INPUT,
                          fCType,
                          fSqlType,
                          nColumnSize,
                          nDecimalDigits,
                          pData,
                          nMaxLen,
                          pLen);

            OTools::ThrowException(nRetcode,_hStmt,SQL_HANDLE_STMT,_xInterface);
            return sal_True;
        }


        template <class T> void bindValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,
                                                   SQLSMALLINT _nType,SQLSMALLINT _nMaxLen,SQLSMALLINT _nScale,
                                                   const T* _pValue,void* _pData,SQLINTEGER *pLen,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            SQLRETURN nRetcode;
            SWORD   fSqlType;
            SWORD   fCType;

            switch(_nType)
            {
                case SQL_CHAR:          //if(GetODBCConnection()->m_bUserWChar)
                                                //  {
        //                                          fCType   = SQL_C_WCHAR;
        //                                          fSqlType = SQL_WCHAR;
        //                                      }
                                                //  else
                                                {
                                                    fCType   = SQL_C_CHAR;
                                                    fSqlType = SQL_CHAR;
                                                }
                                                break;
                case SQL_VARCHAR:       //if(GetODBCConnection()->m_bUserWChar)
        //                                      {
        //                                          fCType   = SQL_C_WCHAR;
        //                                          fSqlType = SQL_WVARCHAR;
        //                                      }
        //                                      else
                                                {
                                                    fCType   = SQL_C_CHAR;
                                                    fSqlType = SQL_VARCHAR;
                                                }
                                                break;
                case SQL_LONGVARCHAR:   //if(GetODBCConnection()->m_bUserWChar)
        //                                      {
        //                                          fCType   = SQL_C_WCHAR;
        //                                          fSqlType = SQL_WLONGVARCHAR;
        //                                      }
        //                                      else
                                                {
                                                    fCType   = SQL_C_CHAR;
                                                    fSqlType = SQL_LONGVARCHAR;
                                                }
                                                break;
                case SQL_DECIMAL:               fCType      = SQL_C_CHAR;//GetODBCConnection()->m_bUserWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                                fSqlType    = SQL_DECIMAL; break;
                case SQL_NUMERIC:               fCType      = SQL_C_CHAR;//GetODBCConnection()->m_bUserWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                                fSqlType    = SQL_NUMERIC; break;
                case SQL_BIT:                   fCType      = SQL_C_TINYINT;
                                                fSqlType    = SQL_INTEGER; break;
                case SQL_TINYINT:               fCType      = SQL_C_SHORT;
                                                fSqlType    = SQL_TINYINT; break;
                case SQL_SMALLINT:              fCType      = SQL_C_SHORT;
                                                fSqlType    = SQL_SMALLINT; break;
                case SQL_INTEGER:               fCType      = SQL_C_LONG;
                                                fSqlType    = SQL_INTEGER; break;
                case SQL_BIGINT:                fCType      = SQL_C_CHAR;//GetODBCConnection()->m_bUserWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                                fSqlType    = SQL_BIGINT; break;
                case SQL_REAL:                  fCType      = SQL_C_FLOAT;
                                                fSqlType    = SQL_REAL; break;
                case SQL_DOUBLE:                fCType      = SQL_C_DOUBLE;
                                                fSqlType    = SQL_DOUBLE; break;
                case SQL_BINARY:                fCType      = SQL_C_BINARY;
                                                fSqlType    = SQL_BINARY; break;
                case SQL_VARBINARY:
                                                fCType      = SQL_C_BINARY;
                                                fSqlType    = SQL_VARBINARY; break;
                case SQL_LONGVARBINARY:         fCType      = SQL_C_BINARY;
                                                fSqlType    = SQL_LONGVARBINARY; break;
                case SQL_DATE:
                                            //  if(((SdbODBC3Connection*)GetODBCConnection())->m_bUseOldTimeDate)
                                            {
                                                fCType      = SQL_C_DATE;
                                                fSqlType    = SQL_DATE;
                                            }
        //                                  else
        //                                  {
        //                                      fCType      = SQL_C_TYPE_DATE;
        //                                      fSqlType    = SQL_TYPE_DATE;
        //                                  }
                                            break;
                case SQL_TIME:
                                            //  if(((SdbODBC3Connection*)GetODBCConnection())->m_bUseOldTimeDate)
                                            {
                                                fCType      = SQL_C_TIME;
                                                fSqlType    = SQL_TIME;
                                            }
        //                                  else
        //                                  {
        //                                      fCType      = SQL_C_TYPE_TIME;
        //                                      fSqlType    = SQL_TYPE_TIME;
        //                                  }
                                            break;
                case SQL_TIMESTAMP:
                                            //  if(((SdbODBC3Connection*)GetODBCConnection())->m_bUseOldTimeDate)
                                            {
                                                fCType      = SQL_C_TIMESTAMP;
                                                fSqlType    = SQL_TIMESTAMP;
                                            }
        //                                  else
        //                                  {
        //                                      fCType      = SQL_C_TYPE_TIMESTAMP;
        //                                      fSqlType    = SQL_TYPE_TIMESTAMP;
        //                                  }
                                            break;
                default:                        fCType      = SQL_C_BINARY;
                                                fSqlType    = SQL_LONGVARBINARY; break;
            }

            if (columnIndex != 0 && !_pValue)
            {
                *pLen = SQL_NULL_DATA;
                nRetcode = N3SQLBindCol(_aStatementHandle,
                                        columnIndex,
                                        fCType,
                                        _pData,
                                        _nMaxLen,
                                        pLen
                                        );
            }
            else
            {
                try
                {
                    switch (_nType)
                    {
                        case SQL_CHAR:
                        case SQL_VARCHAR:
                        //if(GetODBCConnection()->m_bUserWChar)
        //              {
        //                  _nMaxLen = rCol.GetPrecision();
        //                  *pLen = SQL_NTS;
        //                  *((rtl::OUString*)pData) = (rtl::OUString)_aValue;
        //
        //                  // Zeiger auf Char*
        //                  pData = (void*)((rtl::OUString*)pData)->getStr();
        //              }
        //              else
                        {
                            ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,osl_getThreadTextEncoding()));
                            *pLen = SQL_NTS;
                            *((::rtl::OString*)_pData) = aString;
                            _nMaxLen = aString.getLength();

                            // Zeiger auf Char*
                            _pData = (void*)aString.getStr();
                        }   break;
                        case SQL_BIGINT:
                        case SQL_DECIMAL:
                        case SQL_NUMERIC:
                        //if(GetODBCConnection()->m_bUserWChar)
        //              {
        //                  rtl::OUString aString(rtl::OUString(SdbTools::ToString(ODbTypeConversion::toDouble(*pVariable),rCol.GetScale())));
        //                  *pLen = _nMaxLen;
        //                  *((rtl::OUString*)_pData) = aString;
        //                  // Zeiger auf Char*
        //                  _pData = (void*)((rtl::OUString*)_pData)->getStr();
        //              }
        //              else
                        {
                            ::rtl::OString aString = ::rtl::OString::valueOf(*(double*)_pValue);
                            _nMaxLen = aString.getLength();
                            *pLen = _nMaxLen;
                            *((::rtl::OString*)_pData) = aString;
                            // Zeiger auf Char*
                            _pData = (void*)((::rtl::OString*)_pData)->getStr();
                        }   break;
                        case SQL_BIT:
                            *((sal_Int8*)_pData) = *(sal_Int8*)_pValue;
                            *pLen = sizeof(sal_Int8);
                            break;
                        case SQL_TINYINT:
                        case SQL_SMALLINT:
                            *((sal_Int16*)_pData) = *(sal_Int16*)_pValue;
                            *pLen = sizeof(sal_Int16);
                            break;
                        case SQL_INTEGER:
                            *((sal_Int32*)_pData) = *(sal_Int32*)_pValue;
                            *pLen = sizeof(sal_Int32);
                            break;
                        case SQL_REAL:
                            *((float*)_pData) = *(float*)_pValue;
                            *pLen = sizeof(float);
                            break;
                        case SQL_DOUBLE:
                            *((double*)_pData) = *(double*)_pValue;
                            *pLen = sizeof(double);
                            break;
                        case SQL_BINARY:
                        case SQL_VARBINARY:
                                                        //      if (_pValue == ::getCppuType((const ::com::sun::star::uno::Sequence< sal_Int8 > *)0))
                            {
                                _pData = (void*)((const ::com::sun::star::uno::Sequence< sal_Int8 > *)_pValue)->getConstArray();
                                *pLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 > *)_pValue)->getLength();
                            }   break;
                        case SQL_LONGVARBINARY:
                        {
                            sal_Int32 nLen = 0;
                            nLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 > *)_pValue)->getLength();
                            *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
                        }
                            break;
                        case SQL_LONGVARCHAR:
                        {
                            sal_Int32 nLen = 0;
                            nLen = ((::rtl::OUString*)_pValue)->getLength();
                            *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
                        }   break;
                        case SQL_DATE:
                            *pLen = sizeof(DATE_STRUCT);
                            *((DATE_STRUCT*)_pData) = *(DATE_STRUCT*)_pValue;
                            break;
                        case SQL_TIME:
                            *pLen = sizeof(TIME_STRUCT);
                            *((TIME_STRUCT*)_pData) = *(TIME_STRUCT*)_pValue;
                            break;
                        case SQL_TIMESTAMP:
                            *pLen = sizeof(TIMESTAMP_STRUCT);
                            *((TIMESTAMP_STRUCT*)_pData) = *(TIMESTAMP_STRUCT*)_pValue;
                    }
                }
                catch ( ... )
                {
                }
//              SQLINTEGER *pLen = &aLen;
//              bindData< T >(fSqlType,sal_False,_pData,pLen,_pValue);


                nRetcode = N3SQLBindCol(_aStatementHandle,
                                        columnIndex,
                                        fCType,
                                        _pData,
                                        _nMaxLen,
                                        pLen
                                        );
            }

            OTools::ThrowException(nRetcode,_aStatementHandle,SQL_HANDLE_STMT,_xInterface);
        }
    }
}
#endif // _CONNECTIVITY_OTOOLS_HXX_

