/*************************************************************************
 *
 *  $RCSfile: OTools.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:10:31 $
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

#include <cstring>
#include <string>
#include <algorithm>

#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif

#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

void OTools::getValue(  OConnection* _pConnection,
                        SQLHANDLE _aStatementHandle,
                        sal_Int32 columnIndex,
                        SQLSMALLINT _nType,
                        sal_Bool &_bWasNull,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                        void* _pValue,
                        SQLINTEGER _rSize) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SQLINTEGER pcbValue = SQL_NULL_DATA;
    OTools::ThrowException(_pConnection,
                            (*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                        (SQLUSMALLINT)columnIndex,
                                        _nType,
                                        _pValue,
                                        (SQLINTEGER)_rSize,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface,sal_False);
    _bWasNull = pcbValue == SQL_NULL_DATA;
}
// -----------------------------------------------------------------------------
void OTools::bindParameter( OConnection* _pConnection,
                            SQLHANDLE _hStmt,
                            sal_Int32 nPos,
                            sal_Int8*& pDataBuffer,
                            sal_Int8* pLenBuffer,
                            SQLSMALLINT _nODBCtype,
                            sal_Bool _bUseWChar,
                            sal_Bool _bUseOldTimeDate,
                            const void* _pValue,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                            rtl_TextEncoding _nTextEncoding)
                             throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SQLRETURN nRetcode;
    SWORD   fSqlType;
    SWORD   fCType;
    SDWORD  nMaxLen = 0;
    //  void*&   pData   = pDataBuffer;
    SQLINTEGER* pLen    = (SQLINTEGER*)pLenBuffer;
    SQLUINTEGER nColumnSize=0;
    SQLSMALLINT nDecimalDigits=0;
    SQLSMALLINT nNullable=0;

    OTools::getBindTypes(_bUseWChar,_bUseOldTimeDate,_nODBCtype,fCType,fSqlType);

    OTools::bindData(_nODBCtype,_bUseWChar,pDataBuffer,pLen,_pValue,_nTextEncoding,nColumnSize);
    if ((nColumnSize == 0) && (fSqlType == SQL_CHAR || fSqlType == SQL_VARCHAR || fSqlType == SQL_LONGVARCHAR))
        nColumnSize = 1;

    if(fSqlType == SQL_LONGVARCHAR || fSqlType == SQL_LONGVARBINARY)
        memcpy(pDataBuffer,&nPos,sizeof(nPos));

    // 20.09.2001 OJ: Problems with mysql. mysql returns only CHAR as parameter type
    //  nRetcode = (*(T3SQLDescribeParam)_pConnection->getOdbcFunction(ODBC3SQLDescribeParam))(_hStmt,(SQLUSMALLINT)nPos,&fSqlType,&nColumnSize,&nDecimalDigits,&nNullable);

    nRetcode = (*(T3SQLBindParameter)_pConnection->getOdbcFunction(ODBC3SQLBindParameter))(_hStmt,
                  (SQLUSMALLINT)nPos,
                  SQL_PARAM_INPUT,
                  fCType,
                  fSqlType,
                  nColumnSize,
                  nDecimalDigits,
                  pDataBuffer,
                  nMaxLen,
                  pLen);

    OTools::ThrowException(_pConnection,nRetcode,_hStmt,SQL_HANDLE_STMT,_xInterface);
}
// -----------------------------------------------------------------------------
void OTools::bindData(  SQLSMALLINT _nOdbcType,
                        sal_Bool _bUseWChar,
                        sal_Int8 *&_pData,
                        SQLINTEGER*& pLen,
                        const void* _pValue,
                        rtl_TextEncoding _nTextEncoding,
                        SQLUINTEGER& _nColumnSize)
{
    _nColumnSize = 0;

    switch (_nOdbcType)
    {
        case SQL_CHAR:
        case SQL_VARCHAR:
            if(_bUseWChar)
            {
                *pLen = SQL_NTS;
                ::rtl::OUString sStr(*(::rtl::OUString*)_pValue);
                _nColumnSize = sStr.getLength();
                *((rtl::OUString*)_pData) = sStr;

                // Zeiger auf Char*
                _pData = (sal_Int8*)((rtl::OUString*)_pData)->getStr();
            }
            else
            {
                ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,_nTextEncoding));
                *pLen = SQL_NTS;
                _nColumnSize = aString.getLength();
                memcpy(_pData,aString.getStr(),aString.getLength());
                ((sal_Int8*)_pData)[aString.getLength()] = '\0';
            }
            break;

        case SQL_BIGINT:
            *((sal_Int64*)_pData) = *(sal_Int64*)_pValue;
            *pLen = sizeof(sal_Int64);
            _nColumnSize = *pLen;
            break;
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            if(_bUseWChar)
            {
                ::rtl::OUString aString = rtl::OUString::valueOf(*(double*)_pValue);
                _nColumnSize = aString.getLength();
                *pLen = _nColumnSize;
                *((rtl::OUString*)_pData) = aString;
                // Zeiger auf Char*
                _pData = (sal_Int8*)((rtl::OUString*)_pData)->getStr();
            }
            else
            {
                ::rtl::OString aString = ::rtl::OString::valueOf(*(double*)_pValue);
                _nColumnSize = aString.getLength();
                *pLen = _nColumnSize;
                memcpy(_pData,aString.getStr(),aString.getLength());
                ((sal_Int8*)_pData)[_nColumnSize] = '\0';
            }   break;
        case SQL_BIT:
        case SQL_TINYINT:
            *((sal_Int8*)_pData) = *(sal_Int8*)_pValue;
            *pLen = sizeof(sal_Int8);
            break;

        case SQL_SMALLINT:
            *((sal_Int16*)_pData) = *(sal_Int16*)_pValue;
            *pLen = sizeof(sal_Int16);
            break;
        case SQL_INTEGER:
            *((sal_Int32*)_pData) = *(sal_Int32*)_pValue;
            *pLen = sizeof(sal_Int32);
            break;
        case SQL_FLOAT:
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
            {
                const ::com::sun::star::uno::Sequence< sal_Int8 >* pSeq = static_cast< const ::com::sun::star::uno::Sequence< sal_Int8 >* >(_pValue);
                OSL_ENSURE(pSeq,"OTools::bindData: Sequence is null!");

                if(pSeq)
                {
                    //  memcpy(_pData,pSeq->getConstArray(),pSeq->getLength());
                    _pData = (sal_Int8*)((const ::com::sun::star::uno::Sequence< sal_Int8 >  *)_pValue)->getConstArray();
                    *pLen = pSeq->getLength();
                }
                //  _pData = (sal_Int8*)((const ::com::sun::star::uno::Sequence< sal_Int8 >  *)_pValue)->getConstArray();
                //  *pLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 >  *)_pValue)->getLength();
            }
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
            sal_Int32 nLen = 0;
            if(_bUseWChar)
                nLen = sizeof(sal_Unicode) * ((::rtl::OUString*)_pValue)->getLength();
            else
            {
                ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,_nTextEncoding));
                nLen = aString.getLength();
            }
            *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
        }   break;
        case SQL_DATE:
            *(DATE_STRUCT*)_pData = *(DATE_STRUCT*)_pValue;
            *pLen = (SDWORD)sizeof(DATE_STRUCT);
            _nColumnSize = 10;
            break;
        case SQL_TIME:
            *(TIME_STRUCT*)_pData = *(TIME_STRUCT*)_pValue;
            *pLen = (SDWORD)sizeof(TIME_STRUCT);
            _nColumnSize = 8;
            break;
        case SQL_TIMESTAMP:
            *(TIMESTAMP_STRUCT*)_pData = *(TIMESTAMP_STRUCT*)_pValue;
            *pLen = (SDWORD)sizeof(TIMESTAMP_STRUCT);
            _nColumnSize = 19;
            break;
    }
}
// -------------------------------------------------------------------------
void OTools::bindValue( OConnection* _pConnection,
                        SQLHANDLE _aStatementHandle,
                        sal_Int32 columnIndex,
                        SQLSMALLINT _nType,
                        SQLSMALLINT _nMaxLen,
                        SQLSMALLINT _nScale,
                        const void* _pValue,
                        void* _pData,
                        SQLINTEGER *pLen,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                        rtl_TextEncoding _nTextEncoding,
                        sal_Bool _bUseOldTimeDate) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SQLRETURN nRetcode;
    SQLSMALLINT   fSqlType;
    SQLSMALLINT   fCType;
    SQLSMALLINT   nMaxLen = _nMaxLen;

    OTools::getBindTypes(   sal_False,
                            _bUseOldTimeDate,
                            _nType,
                            fCType,
                            fSqlType);

    if (columnIndex != 0 && !_pValue)
    {
        *pLen = SQL_NULL_DATA;
        nRetcode = (*(T3SQLBindCol)_pConnection->getOdbcFunction(ODBC3SQLBindCol))(_aStatementHandle,
                                columnIndex,
                                fCType,
                                _pData,
                                nMaxLen,
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
                    ::rtl::OString aString(::rtl::OUStringToOString(*(::rtl::OUString*)_pValue,_nTextEncoding));
                    *pLen = SQL_NTS;
                    *((::rtl::OString*)_pData) = aString;
                    _nMaxLen = aString.getLength();

                    // Zeiger auf Char*
                    _pData = (void*)aString.getStr();
                }   break;
                case SQL_BIGINT:
                    *((sal_Int64*)_pData) = *(sal_Int64*)_pValue;
                    *pLen = sizeof(sal_Int64);
                    break;
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
                case SQL_TINYINT:
                    *((sal_Int8*)_pData) = *(sal_Int8*)_pValue;
                    *pLen = sizeof(sal_Int8);
                    break;

                case SQL_SMALLINT:
                    *((sal_Int16*)_pData) = *(sal_Int16*)_pValue;
                    *pLen = sizeof(sal_Int16);
                    break;
                case SQL_INTEGER:
                    *((sal_Int32*)_pData) = *(sal_Int32*)_pValue;
                    *pLen = sizeof(sal_Int32);
                    break;
                case SQL_FLOAT:
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
                    _pData = (void*)(columnIndex);
                    sal_Int32 nLen = 0;
                    nLen = ((const ::com::sun::star::uno::Sequence< sal_Int8 > *)_pValue)->getLength();
                    *pLen = (SDWORD)SQL_LEN_DATA_AT_EXEC(nLen);
                }
                    break;
                case SQL_LONGVARCHAR:
                {
                    _pData = (void*)(columnIndex);
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

        nRetcode = (*(T3SQLBindCol)_pConnection->getOdbcFunction(ODBC3SQLBindCol))(_aStatementHandle,
                                columnIndex,
                                fCType,
                                _pData,
                                nMaxLen,
                                pLen
                                );
    }

    OTools::ThrowException(_pConnection,nRetcode,_aStatementHandle,SQL_HANDLE_STMT,_xInterface);
}
// -----------------------------------------------------------------------------
void OTools::ThrowException(OConnection* _pConnection,
                            SQLRETURN _rRetCode,
                            SQLHANDLE _pContext,
                            SQLSMALLINT _nHandleType,
                            const Reference< XInterface >& _xInterface,
                            sal_Bool _bNoFound,
                            rtl_TextEncoding _nTextEncoding) throw(SQLException)
{
    switch(_rRetCode)
    {
        case SQL_NEED_DATA:
        case SQL_STILL_EXECUTING:
        case SQL_SUCCESS:

        case SQL_SUCCESS_WITH_INFO:
                        return;
        case SQL_NO_DATA_FOUND:
                                if(_bNoFound)
                                    return; // no need to throw a exception
        case SQL_ERROR:             break;


        case SQL_INVALID_HANDLE:    OSL_ENSURE(0,"SdbODBC3_SetStatus: SQL_INVALID_HANDLE");
                                    throw SQLException();
                                    break;
    }


    // Zusaetliche Informationen zum letzten ODBC-Funktionsaufruf vorhanden.
    // SQLError liefert diese Informationen.

    SDB_ODBC_CHAR szSqlState[5];
    SDWORD pfNativeError;
    SDB_ODBC_CHAR szErrorMessage[SQL_MAX_MESSAGE_LENGTH];
    szErrorMessage[0] = '\0';
    SWORD pcbErrorMsg = 0;

    // Informationen zur letzten Operation:
    // wenn hstmt != SQL_NULL_HSTMT ist (Benutzung von SetStatus in SdbCursor, SdbTable, ...),
    // dann wird der Status des letzten Statements erfragt, sonst der Status des letzten
    // Statements zu dieser Verbindung [was in unserem Fall wahrscheinlich gleichbedeutend ist,
    // aber das Reference Manual drueckt sich da nicht so klar aus ...].
    // Entsprechend bei hdbc.
    SQLRETURN n = (*(T3SQLGetDiagRec)_pConnection->getOdbcFunction(ODBC3SQLGetDiagRec))(_nHandleType,_pContext,1,
                         szSqlState,
                         &pfNativeError,
                         szErrorMessage,sizeof szErrorMessage - 1,&pcbErrorMsg);
    OSL_ENSURE(n != SQL_INVALID_HANDLE,"SdbODBC3_SetStatus: SQLError returned SQL_INVALID_HANDLE");
    OSL_ENSURE(n == SQL_SUCCESS || n == SQL_SUCCESS_WITH_INFO || n == SQL_NO_DATA_FOUND || n == SQL_ERROR,"SdbODBC3_SetStatus: SQLError failed");

    // Zum Return Code von SQLError siehe ODBC 2.0 Programmer's Reference Seite 287ff
    throw SQLException( ::rtl::OUString((char *)szErrorMessage,pcbErrorMsg,_nTextEncoding),
                                    _xInterface,
                                    ::rtl::OUString((char *)szSqlState,5,_nTextEncoding),
                                    pfNativeError,
                                    Any()
                                );

}
// -------------------------------------------------------------------------
Sequence<sal_Int8> OTools::getBytesValue(OConnection* _pConnection,
                                         SQLHANDLE _aStatementHandle,
                                         sal_Int32 columnIndex,
                                         SWORD  _fSqlType,
                                         sal_Bool &_bWasNull,
                                         const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    char aCharArray[2048];
    // Erstmal versuchen, die Daten mit dem kleinen Puffer
    // abzuholen:
    SQLINTEGER nMaxLen = sizeof aCharArray - 1;
    //  GETDATA(SQL_C_CHAR,aCharArray,nMaxLen);
    SQLINTEGER pcbValue = 0;
    OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                        (SQLUSMALLINT)columnIndex,
                                        _fSqlType,
                                        (SQLPOINTER)aCharArray,
                                        (SQLINTEGER)nMaxLen,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface);

    _bWasNull = pcbValue == SQL_NULL_DATA;
    if(_bWasNull)
        return Sequence<sal_Int8>();

    SQLINTEGER nBytes = pcbValue != SQL_NO_TOTAL ? std::min(pcbValue, nMaxLen) : nMaxLen;
    if ( ((pcbValue == SQL_NO_TOTAL) || pcbValue > nMaxLen) && aCharArray[nBytes-1] == 0  && nBytes > 0 )
        --nBytes;
    Sequence<sal_Int8> aData((sal_Int8*)aCharArray, nBytes);


    // Es handelt sich um Binaerdaten, um einen String, der fuer
    // StarView zu lang ist oder der Treiber kann die Laenge der
    // Daten nicht im voraus bestimmen - also als MemoryStream
    // speichern.
    while ((pcbValue == SQL_NO_TOTAL) || pcbValue > nMaxLen)
    {
        // Bei Strings wird der Puffer nie ganz ausgenutzt
        // (das letzte Byte ist immer ein NULL-Byte, das
        // aber bei pcbValue nicht mitgezaehlt wird)
        if (pcbValue != SQL_NO_TOTAL && (pcbValue - nMaxLen) < nMaxLen)
            nBytes = pcbValue - nMaxLen;
        else
            nBytes = nMaxLen;

        // Solange eine "truncation"-Warnung vorliegt, weiter Daten abholen
        //  GETDATA(SQL_C_CHAR,aCharArray, nLen + 1);
        OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                        (SQLUSMALLINT)columnIndex,
                                        SQL_C_BINARY,
                                        &aCharArray,
                                        (SQLINTEGER)nBytes,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
        sal_Int32 nLen = aData.getLength();
        aData.realloc(nLen + nBytes);
        memcpy(aData.getArray() + nLen, aCharArray, nBytes);
    }
    return aData;
}
// -------------------------------------------------------------------------
::rtl::OUString OTools::getStringValue(OConnection* _pConnection,
                                       SQLHANDLE _aStatementHandle,
                                       sal_Int32 columnIndex,
                                       SWORD  _fSqlType,
                                       sal_Bool &_bWasNull,
                                       const Reference< XInterface >& _xInterface,
                                       rtl_TextEncoding _nTextEncoding) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aData;
    switch(_fSqlType)
    {
    case SQL_WVARCHAR:
    case SQL_WCHAR:
    case SQL_WLONGVARCHAR:
        {
            sal_Unicode waCharArray[2048];
            // read the unicode data
            sal_Int32 nMaxLen = sizeof(waCharArray) - sizeof(sal_Unicode);
            //  GETDATA(SQL_C_WCHAR, waCharArray, nMaxLen + sizeof(sal_Unicode));

            SQLINTEGER pcbValue=0;
            OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_WCHAR,
                                                &waCharArray,
                                                (SQLINTEGER)nMaxLen*sizeof(sal_Unicode),
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            if(_bWasNull)
                return ::rtl::OUString();
            // Bei Fehler bricht der GETDATA-Makro mit return ab,
            // bei NULL mit break!
            SQLINTEGER nLen = pcbValue != SQL_NO_TOTAL ? std::min(pcbValue, nMaxLen) : (nMaxLen-1);
            waCharArray[nLen] = 0;
            aData = ::rtl::OUString(waCharArray);

            // Es handelt sich um Binaerdaten, um einen String, der fuer
            // StarView zu lang ist oder der Treiber kann die Laenge der
            // Daten nicht im voraus bestimmen - also als MemoryStream
            // speichern.
            while ((pcbValue == SQL_NO_TOTAL ) || pcbValue > nMaxLen)
            {
                // Bei Strings wird der Puffer nie ganz ausgenutzt
                // (das letzte Byte ist immer ein NULL-Byte, das
                // aber bei pcbValue nicht mitgezaehlt wird)
                if (pcbValue != SQL_NO_TOTAL && (pcbValue - nMaxLen) < nMaxLen)
                    nLen = pcbValue - nMaxLen;
                else
                    nLen = nMaxLen;

                // Solange eine "truncation"-Warnung vorliegt, weiter Daten abholen
                //  GETDATA(SQL_C_CHAR,waCharArray, nLen + 1);
                OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_WCHAR,
                                                &waCharArray,
                                                (SQLINTEGER)nLen+1,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
                nLen = pcbValue != SQL_NO_TOTAL ? std::min(pcbValue, nMaxLen) : (nMaxLen-1);
                waCharArray[nLen] = 0;

                aData += ::rtl::OUString(waCharArray);
            }
        }
        break;
        default:
        {
            char aCharArray[2048];
            // Erstmal versuchen, die Daten mit dem kleinen Puffer
            // abzuholen:
            SDWORD nMaxLen = sizeof aCharArray - 1;
            //  GETDATA(SQL_C_CHAR,aCharArray,nMaxLen);
            SQLINTEGER pcbValue = 0;
            OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_CHAR,
                                                &aCharArray,
                                                (SQLINTEGER)nMaxLen,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            if(_bWasNull)
                return ::rtl::OUString();

            SQLINTEGER nLen = pcbValue != SQL_NO_TOTAL ? std::min(pcbValue, nMaxLen) : (nMaxLen-1);
            aCharArray[nLen] = 0;
            if ( ((pcbValue == SQL_NO_TOTAL) || pcbValue > nMaxLen) && aCharArray[nLen-1] == 0 && nLen > 0 )
                --nLen;
            aData = ::rtl::OUString((const sal_Char*)aCharArray,nLen, _nTextEncoding);

            // Es handelt sich um Binaerdaten, um einen String, der fuer
            // StarView zu lang ist oder der Treiber kann die Laenge der
            // Daten nicht im voraus bestimmen - also als MemoryStream
            // speichern.
            while ((pcbValue == SQL_NO_TOTAL) || pcbValue > nMaxLen)
            {
                // Solange eine "truncation"-Warnung vorliegt, weiter Daten abholen
                //  GETDATA(SQL_C_CHAR,aCharArray, nLen + 1);
                OTools::ThrowException(_pConnection,(*(T3SQLGetData)_pConnection->getOdbcFunction(ODBC3SQLGetData))(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_CHAR,
                                                &aCharArray,
                                                (SQLINTEGER)nMaxLen,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
                nLen = pcbValue != SQL_NO_TOTAL ? std::min(pcbValue, nMaxLen) : (nMaxLen-1);
                if ( ((pcbValue == SQL_NO_TOTAL) || pcbValue > nMaxLen) && aCharArray[nLen-1] == 0 && nLen > 0 )
                    --nLen;
                aCharArray[nLen] = 0;

                aData += ::rtl::OUString((const sal_Char*)aCharArray,nLen,_nTextEncoding);
            }

            // delete all blanks
            //  aData.EraseTrailingChars();
        }
    }

    return aData;
}
// -------------------------------------------------------------------------
void OTools::GetInfo(OConnection* _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     ::rtl::OUString &_rValue,
                     const Reference< XInterface >& _xInterface,
                     rtl_TextEncoding _nTextEncoding) throw(SQLException, RuntimeException)
{
    char aValue[512];
    SQLSMALLINT nValueLen=0;
    OTools::ThrowException(_pConnection,
        (*(T3SQLGetInfo)_pConnection->getOdbcFunction(ODBC3SQLGetInfo))(_aConnectionHandle,_nInfo,aValue,sizeof aValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);

    _rValue = ::rtl::OUString(aValue,nValueLen,_nTextEncoding);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(OConnection* _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     sal_Int32 &_rValue,
                     const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        (*(T3SQLGetInfo)_pConnection->getOdbcFunction(ODBC3SQLGetInfo))(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(OConnection* _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     SQLUINTEGER &_rValue,
                     const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        (*(T3SQLGetInfo)_pConnection->getOdbcFunction(ODBC3SQLGetInfo))(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(OConnection* _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     SQLUSMALLINT &_rValue,
                     const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        (*(T3SQLGetInfo)_pConnection->getOdbcFunction(ODBC3SQLGetInfo))(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(OConnection* _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     sal_Bool &_rValue,
                     const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    OTools::ThrowException(_pConnection,
                            (*(T3SQLGetInfo)_pConnection->getOdbcFunction(ODBC3SQLGetInfo))(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
                            _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
sal_Int32 OTools::MapOdbcType2Jdbc(sal_Int32 _nType)
{
    sal_Int32 nValue = DataType::VARCHAR;
    switch(_nType)
    {
        case SQL_BIT:
            nValue = DataType::BIT;
            break;
        case SQL_TINYINT:
            nValue = DataType::TINYINT;
            break;
        case SQL_SMALLINT:
            nValue = DataType::SMALLINT;
            break;
        case SQL_INTEGER:
            nValue = DataType::INTEGER;
            break;
        case SQL_BIGINT:
            nValue = DataType::BIGINT;
            break;
        case SQL_FLOAT:
            nValue = DataType::FLOAT;
            break;
        case SQL_REAL:
            nValue = DataType::REAL;
            break;
        case SQL_DOUBLE:
            nValue = DataType::DOUBLE;
            break;
        case SQL_NUMERIC:
            nValue = DataType::NUMERIC;
            break;
        case SQL_DECIMAL:
            nValue = DataType::DECIMAL;
            break;
        case SQL_WCHAR:
        case SQL_CHAR:
            nValue = DataType::CHAR;
            break;
        case SQL_WVARCHAR:
        case SQL_VARCHAR:
            nValue = DataType::VARCHAR;
            break;
        case SQL_WLONGVARCHAR:
        case SQL_LONGVARCHAR:
            nValue = DataType::LONGVARCHAR;
            break;
        case SQL_TYPE_DATE:
        case SQL_DATE:
            nValue = DataType::DATE;
            break;
        case SQL_TYPE_TIME:
        case SQL_TIME:
            nValue = DataType::TIME;
            break;
        case SQL_TYPE_TIMESTAMP:
        case SQL_TIMESTAMP:
            nValue = DataType::TIMESTAMP;
            break;
        case SQL_BINARY:
            nValue = DataType::BINARY;
            break;
        case SQL_VARBINARY:
        case SQL_GUID:
            nValue = DataType::VARBINARY;
            break;
        case SQL_LONGVARBINARY:
            nValue = DataType::LONGVARBINARY;
            break;
        default:
            OSL_ASSERT(!"Invalid type");
    }
    return nValue;
}
//--------------------------------------------------------------------
// jdbcTypeToOdbc
// Convert the JDBC SQL type to the correct ODBC type
//--------------------------------------------------------------------
sal_Int32 OTools::jdbcTypeToOdbc(sal_Int32 jdbcType)
{
    // For the most part, JDBC types match ODBC types.  We'll
    // just convert the ones that we know are different

    sal_Int32 odbcType = jdbcType;

    switch (jdbcType)
    {
    case DataType::DATE:
        odbcType = SQL_DATE;
        break;
    case DataType::TIME:
        odbcType = SQL_TIME;
        break;
    case DataType::TIMESTAMP:
        odbcType = SQL_TIMESTAMP;
        break;
    }

    return odbcType;
}
//-----------------------------------------------------------------------------
void OTools::getBindTypes(sal_Bool _bUseWChar,
                          sal_Bool _bUseOldTimeDate,
                          SQLSMALLINT _nOdbcType,
                          SQLSMALLINT& fCType,
                          SQLSMALLINT& fSqlType
                          )
{
    switch(_nOdbcType)
    {
        case SQL_CHAR:              if(_bUseWChar)
                                    {
                                        fCType   = SQL_C_WCHAR;
                                        fSqlType = SQL_WCHAR;
                                    }
                                    else
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_CHAR;
                                    }
                                    break;
        case SQL_VARCHAR:           if(_bUseWChar)
                                    {
                                        fCType   = SQL_C_WCHAR;
                                        fSqlType = SQL_WVARCHAR;
                                    }
                                    else
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_VARCHAR;
                                    }
                                    break;
        case SQL_LONGVARCHAR:       if(_bUseWChar)
                                    {
                                        fCType   = SQL_C_WCHAR;
                                        fSqlType = SQL_WLONGVARCHAR;
                                    }
                                    else
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_LONGVARCHAR;
                                    }
                                    break;
        case SQL_DECIMAL:           fCType      = _bUseWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                    fSqlType    = SQL_DECIMAL; break;
        case SQL_NUMERIC:           fCType      = _bUseWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                    fSqlType    = SQL_NUMERIC; break;
        case SQL_BIT:               fCType      = SQL_C_TINYINT;
                                    fSqlType    = SQL_INTEGER; break;
        case SQL_TINYINT:           fCType      = SQL_C_TINYINT;
                                    fSqlType    = SQL_TINYINT; break;
        case SQL_SMALLINT:          fCType      = SQL_C_SHORT;
                                    fSqlType    = SQL_SMALLINT; break;
        case SQL_INTEGER:           fCType      = SQL_C_LONG;
                                    fSqlType    = SQL_INTEGER; break;
        case SQL_BIGINT:            fCType      = SQL_C_SBIGINT;
                                    fSqlType    = SQL_BIGINT; break;
        case SQL_FLOAT:             fCType      = SQL_C_FLOAT;
                                    fSqlType    = SQL_FLOAT; break;
        case SQL_REAL:              fCType      = SQL_C_FLOAT;
                                    fSqlType    = SQL_REAL; break;
        case SQL_DOUBLE:            fCType      = SQL_C_DOUBLE;
                                    fSqlType    = SQL_DOUBLE; break;
        case SQL_BINARY:            fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_BINARY; break;
        case SQL_VARBINARY:
                                    fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_VARBINARY; break;
        case SQL_LONGVARBINARY:     fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_LONGVARBINARY; break;
        case SQL_DATE:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_DATE;
                                        fSqlType    = SQL_DATE;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_DATE;
                                        fSqlType    = SQL_TYPE_DATE;
                                    }
                                    break;
        case SQL_TIME:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_TIME;
                                        fSqlType    = SQL_TIME;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_TIME;
                                        fSqlType    = SQL_TYPE_TIME;
                                    }
                                    break;
        case SQL_TIMESTAMP:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_TIMESTAMP;
                                        fSqlType    = SQL_TIMESTAMP;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_TIMESTAMP;
                                        fSqlType    = SQL_TYPE_TIMESTAMP;
                                    }
                                    break;
        default:                        fCType      = SQL_C_BINARY;
                                        fSqlType    = SQL_LONGVARBINARY; break;
    }
}


