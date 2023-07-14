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

#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>

#include <dbdocutl.hxx>
#include <document.hxx>
#include <formula/errorcodes.hxx>
#include <stringutil.hxx>

using namespace ::com::sun::star;

ScDatabaseDocUtil::StrData::StrData() :
    mbSimpleText(true), mnStrLength(0)
{
}

void ScDatabaseDocUtil::PutData(ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const uno::Reference<sdbc::XRow>& xRow, sal_Int32 nRowPos,
                                tools::Long nType, bool bCurrency, StrData* pStrData)
{
    OUString aString;
    double nVal = 0.0;
    bool bValue = false;
    bool bEmptyFlag = false;
    bool bError = false;
    sal_uInt32 nFormatIndex = 0;

    // wasNull calls only if null value was found?

    try
    {
        switch ( nType )
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
                //TODO: use language from doc (here, date/time and currency)?
                nFormatIndex = rDoc.GetFormatTable()->GetStandardFormat(
                                    SvNumFormatType::LOGICAL, ScGlobal::eLnge );
                nVal = (xRow->getBoolean(nRowPos) ? 1 : 0);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = true;
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
                //TODO: do the conversion here?
                nVal = xRow->getDouble(nRowPos);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = true;
                break;

            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
                aString = xRow->getString(nRowPos);
                bEmptyFlag = ( aString.isEmpty() ) && xRow->wasNull();
                break;

            case sdbc::DataType::DATE:
                {
                    util::Date aDate = xRow->getDate(nRowPos);
                    bEmptyFlag = xRow->wasNull();
                    if (bEmptyFlag)
                        nVal = 0.0;
                    else
                    {
                        SvNumberFormatter* pFormTable = rDoc.GetFormatTable();
                        nFormatIndex = pFormTable->GetStandardFormat(
                                SvNumFormatType::DATE, ScGlobal::eLnge );
                        nVal = Date( aDate ) - pFormTable->GetNullDate();
                    }
                    bValue = true;
                }
                break;

            case sdbc::DataType::TIME:
                {
                    SvNumberFormatter* pFormTable = rDoc.GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        SvNumFormatType::TIME, ScGlobal::eLnge );

                    util::Time aTime = xRow->getTime(nRowPos);
                    nVal = aTime.Hours       / static_cast<double>(::tools::Time::hourPerDay)   +
                           aTime.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                           aTime.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                           aTime.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
                    bEmptyFlag = xRow->wasNull();
                    bValue = true;
                }
                break;

            case sdbc::DataType::TIMESTAMP:
                {
                    SvNumberFormatter* pFormTable = rDoc.GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        SvNumFormatType::DATETIME, ScGlobal::eLnge );

                    util::DateTime aStamp = xRow->getTimestamp(nRowPos);
                    if (aStamp.Year != 0)
                    {
                        nVal = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) -
                                                    pFormTable->GetNullDate() ) +
                               aStamp.Hours       / static_cast<double>(::tools::Time::hourPerDay)   +
                               aStamp.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                               aStamp.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                               aStamp.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
                        bEmptyFlag = xRow->wasNull();
                        bValue = true;
                    }
                }
                break;

            case sdbc::DataType::SQLNULL:
                bEmptyFlag = true;
                break;

            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                bError = true;      // unknown type
        }
    }
    catch ( uno::Exception& )
    {
        bError = true;
    }

    if ( bValue && bCurrency )
        nFormatIndex = rDoc.GetFormatTable()->GetStandardFormat(
                            SvNumFormatType::CURRENCY, ScGlobal::eLnge );

    ScAddress aPos(nCol, nRow, nTab);
    if (bEmptyFlag)
        rDoc.SetEmptyCell(aPos);
    else if (bError)
    {
        rDoc.SetError( nCol, nRow, nTab, FormulaError::NotAvailable );
    }
    else if (bValue)
    {
        rDoc.SetValue(aPos, nVal);
        if (nFormatIndex)
            rDoc.SetNumberFormat(aPos, nFormatIndex);
    }
    else
    {
        if (!aString.isEmpty())
        {
            if (ScStringUtil::isMultiline(aString))
            {
                rDoc.SetEditText(aPos, aString);
                if (pStrData)
                    pStrData->mbSimpleText = false;
            }
            else
            {
                ScSetStringParam aParam;
                aParam.setTextInput();
                rDoc.SetString(aPos, aString, &aParam);
                if (pStrData)
                    pStrData->mbSimpleText = true;
            }

            if (pStrData)
                pStrData->mnStrLength = aString.getLength();
        }
        else
            rDoc.SetEmptyCell(aPos);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
