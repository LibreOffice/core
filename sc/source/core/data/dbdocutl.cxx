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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <svl/zforlist.hxx>

#include "dbdocutl.hxx"
#include "document.hxx"
#include "formula/errorcodes.hxx"
#include "stringutil.hxx"

using namespace ::com::sun::star;

#define D_TIMEFACTOR              86400.0

// ----------------------------------------------------------------------------

ScDatabaseDocUtil::StrData::StrData() :
    mbSimpleText(true), mnStrLength(0)
{
}

// ----------------------------------------------------------------------------

void ScDatabaseDocUtil::PutData( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const uno::Reference<sdbc::XRow>& xRow, long nRowPos,
                                long nType, sal_Bool bCurrency, StrData* pStrData )
{
    String aString;
    double nVal = 0.0;
    sal_Bool bValue = false;
    sal_Bool bEmptyFlag = false;
    sal_Bool bError = false;
    sal_uLong nFormatIndex = 0;

    //! wasNull calls only if null value was found?

    try
    {
        switch ( nType )
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
                //! use language from doc (here, date/time and currency)?
                nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
                                    NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
                nVal = (xRow->getBoolean(nRowPos) ? 1 : 0);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = sal_True;
                break;

            case sdbc::DataType::TINYINT:
            case sdbc::DataType::SMALLINT:
            case sdbc::DataType::INTEGER:
            case sdbc::DataType::BIGINT:
            case sdbc::DataType::FLOAT:
            case sdbc::DataType::REAL:
            case sdbc::DataType::DOUBLE:
            case sdbc::DataType::NUMERIC:
            case sdbc::DataType::DECIMAL:
                //! do the conversion here?
                nVal = xRow->getDouble(nRowPos);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = sal_True;
                break;

            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
                aString = xRow->getString(nRowPos);
                bEmptyFlag = ( aString.Len() == 0 ) && xRow->wasNull();
                break;

            case sdbc::DataType::DATE:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_DATE, ScGlobal::eLnge );

                    util::Date aDate = xRow->getDate(nRowPos);
                    nVal = Date( aDate.Day, aDate.Month, aDate.Year ) -
                                                *pFormTable->GetNullDate();
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::TIME:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_TIME, ScGlobal::eLnge );

                    util::Time aTime = xRow->getTime(nRowPos);
                    nVal = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                             aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::TIMESTAMP:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_DATETIME, ScGlobal::eLnge );

                    util::DateTime aStamp = xRow->getTimestamp(nRowPos);
                    nVal = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) -
                                                *pFormTable->GetNullDate() ) +
                           ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                             aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::SQLNULL:
                bEmptyFlag = sal_True;
                break;

            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                bError = sal_True;      // unknown type
        }
    }
    catch ( uno::Exception& )
    {
        bError = sal_True;
    }

    if ( bValue && bCurrency )
        nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
                            NUMBERFORMAT_CURRENCY, ScGlobal::eLnge );

    ScAddress aPos(nCol, nRow, nTab);
    if (bEmptyFlag)
        pDoc->SetEmptyCell(aPos);
    else if (bError)
    {
        pDoc->SetError( nCol, nRow, nTab, NOTAVAILABLE );
    }
    else if (bValue)
    {
        pDoc->SetValue(aPos, nVal);
        if (nFormatIndex)
            pDoc->SetNumberFormat(aPos, nFormatIndex);
    }
    else
    {
        if (aString.Len())
        {
            if (ScStringUtil::isMultiline(aString))
            {
                pDoc->SetEditText(aPos, aString);
                if (pStrData)
                    pStrData->mbSimpleText = false;
            }
            else
            {
                ScSetStringParam aParam;
                aParam.mbDetectNumberFormat = false;
                aParam.mbHandleApostrophe = false;
                aParam.meSetTextNumFormat = ScSetStringParam::Always;
                pDoc->SetString(aPos, aString, &aParam);
                if (pStrData)
                    pStrData->mbSimpleText = true;
            }

            if (pStrData)
                pStrData->mnStrLength = aString.Len();
        }
        else
            pDoc->SetEmptyCell(aPos);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
