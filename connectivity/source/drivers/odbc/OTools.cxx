/*************************************************************************
 *
 *  $RCSfile: OTools.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
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
#include "odbc/OTools.hxx"
#endif

#define __STL_IMPORT_VENDOR_CSTD
#include <cstring>
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;
#ifndef min
#define min(x,y) (x) < ( y) ? (x) : (y)
#endif
// -------------------------------------------------------------------------
void OTools::ThrowException(SQLRETURN _rRetCode,SQLHANDLE _pContext,SQLSMALLINT _nHandleType,const Reference< XInterface >& _xInterface,sal_Bool _bNoFound) throw(SQLException, RuntimeException)
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


        case SQL_INVALID_HANDLE:    OSL_ENSHURE(0,"SdbODBC3_SetStatus: SQL_INVALID_HANDLE");
                                    throw RuntimeException();
                                    break;
    }


    // Zusaetliche Informationen zum letzten ODBC-Funktionsaufruf vorhanden.
    // SQLError liefert diese Informationen.

    SDB_ODBC_CHAR szSqlState[5];
    SDWORD pfNativeError;
    SDB_ODBC_CHAR szErrorMessage[SQL_MAX_MESSAGE_LENGTH];
    SWORD pcbErrorMsg;

    // Informationen zur letzten Operation:
    // wenn hstmt != SQL_NULL_HSTMT ist (Benutzung von SetStatus in SdbCursor, SdbTable, ...),
    // dann wird der Status des letzten Statements erfragt, sonst der Status des letzten
    // Statements zu dieser Verbindung [was in unserem Fall wahrscheinlich gleichbedeutend ist,
    // aber das Reference Manual drueckt sich da nicht so klar aus ...].
    // Entsprechend bei hdbc.
    SQLRETURN n = N3SQLGetDiagRec(_nHandleType,_pContext,1,
                         szSqlState,
                         &pfNativeError,
                         szErrorMessage,sizeof szErrorMessage - 1,&pcbErrorMsg);
    OSL_ENSHURE(n != SQL_INVALID_HANDLE,"SdbODBC3_SetStatus: SQLError returned SQL_INVALID_HANDLE");
    OSL_ENSHURE(n == SQL_SUCCESS || n == SQL_SUCCESS_WITH_INFO || n == SQL_NO_DATA_FOUND || n == SQL_ERROR,"SdbODBC3_SetStatus: SQLError failed");

    // Zum Return Code von SQLError siehe ODBC 2.0 Programmer's Reference Seite 287ff
    throw SQLException( ::rtl::OUString((char *)szErrorMessage,pcbErrorMsg,RTL_TEXTENCODING_MS_1252),
                                    _xInterface,
                                    ::rtl::OUString((char *)szSqlState,5,RTL_TEXTENCODING_MS_1252),
                                    pfNativeError,
                                    Any()
                                );

}
// -------------------------------------------------------------------------
Sequence<sal_Int8> OTools::getBytesValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,SWORD  _fSqlType,sal_Bool &_bWasNull,
                const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    char aCharArray[2048];
    // Erstmal versuchen, die Daten mit dem kleinen Puffer
    // abzuholen:
    SQLINTEGER nMaxLen = sizeof aCharArray - 1;
    //  GETDATA(SQL_C_CHAR,aCharArray,nMaxLen);
    SQLINTEGER pcbValue = 0;
    OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                        (SQLUSMALLINT)columnIndex,
                                        SQL_C_BINARY,
                                        &aCharArray,
                                        (SQLINTEGER)nMaxLen,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface);

    _bWasNull = pcbValue == SQL_NULL_DATA;
    if(_bWasNull)
        return Sequence<sal_Int8>();

    SQLINTEGER nBytes = pcbValue != SQL_NO_TOTAL ? min(pcbValue, nMaxLen) : nMaxLen;
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
        OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                        (SQLUSMALLINT)columnIndex,
                                        SQL_C_BINARY,
                                        &aCharArray,
                                        (SQLINTEGER)nBytes,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
        sal_Int32 nLen = aData.getLength();
        aData.realloc(nLen + nBytes);
        ::std::memcpy(aData.getArray() + nLen, aCharArray, nBytes);
    }
    return aData;
}
// -------------------------------------------------------------------------
::rtl::OUString OTools::getStringValue(SQLHANDLE _aStatementHandle,sal_Int32 columnIndex,SWORD  _fSqlType,sal_Bool &_bWasNull,
                const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
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
            OTools::ThrowException(N3SQLGetData(_aStatementHandle,
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
            SQLINTEGER nLen = pcbValue != SQL_NO_TOTAL ? min(pcbValue, nMaxLen) : nMaxLen;
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
                OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_WCHAR,
                                                &waCharArray,
                                                (SQLINTEGER)nLen+1,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
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
            OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_CHAR,
                                                &aCharArray,
                                                (SQLINTEGER)nMaxLen,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            if(_bWasNull)
                return ::rtl::OUString();

            SQLINTEGER nLen = pcbValue != SQL_NO_TOTAL ? min(pcbValue, nMaxLen) : nMaxLen;
            aCharArray[nLen] = 0;
            aData = ::rtl::OUString((const sal_Char*)aCharArray,nLen, osl_getThreadTextEncoding());

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
                    nLen = pcbValue - nMaxLen;
                else
                    nLen = nMaxLen;

                // Solange eine "truncation"-Warnung vorliegt, weiter Daten abholen
                //  GETDATA(SQL_C_CHAR,aCharArray, nLen + 1);
                OTools::ThrowException(N3SQLGetData(_aStatementHandle,
                                                (SQLUSMALLINT)columnIndex,
                                                SQL_C_CHAR,
                                                &aCharArray,
                                                (SQLINTEGER)nLen +1,
                                                &pcbValue),
                                    _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
                aCharArray[nLen] = 0;

                aData += ::rtl::OUString((const sal_Char*)aCharArray,nLen,osl_getThreadTextEncoding());
            }

            // delete all blanks
            //  aData.EraseTrailingChars();
        }
    }

    return aData;
}
// -------------------------------------------------------------------------
void OTools::GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,::rtl::OUString &_rValue,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    char aValue[512];
    SQLSMALLINT nValueLen;
    OTools::ThrowException(
        N3SQLGetInfo(_aConnectionHandle,_nInfo,aValue,sizeof aValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);

    _rValue = ::rtl::OUString(aValue,nValueLen,RTL_TEXTENCODING_MS_1252);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,sal_Int32 &_rValue,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(
        N3SQLGetInfo(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
void OTools::GetInfo(SQLHANDLE _aConnectionHandle,SQLUSMALLINT _nInfo,sal_Bool &_rValue,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    SQLSMALLINT nValueLen;
    OTools::ThrowException(
        N3SQLGetInfo(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}
// -------------------------------------------------------------------------
sal_Int32 OTools::MapOdbcType2Jdbc(sal_Int32 _nType)
{
    sal_Int32 nValue;
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
            nValue = DataType::VARBINARY;
            break;
        case SQL_LONGVARBINARY:
            nValue = DataType::LONGVARBINARY;
            break;
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
void OTools::getBindTypes(sal_Bool _bUseWChar,sal_Bool _bUseOldTimeDate,
                          sal_Int32 jdbcType,SQLSMALLINT& fCType,SQLSMALLINT& fSqlType,
                          SQLUINTEGER& nColumnSize,SQLSMALLINT& nDecimalDigits)
{
    switch(jdbcTypeToOdbc(jdbcType))
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
        case SQL_TINYINT:           fCType      = SQL_C_SHORT;
                                    fSqlType    = SQL_TINYINT; break;
        case SQL_SMALLINT:          fCType      = SQL_C_SHORT;
                                    fSqlType    = SQL_SMALLINT; break;
        case SQL_INTEGER:           fCType      = SQL_C_LONG;
                                    fSqlType    = SQL_INTEGER; break;
        case SQL_BIGINT:            fCType      = _bUseWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                    fSqlType    = SQL_BIGINT; break;
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


