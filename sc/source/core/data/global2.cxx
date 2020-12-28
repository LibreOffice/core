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

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <svl/zforlist.hxx>
#include <formula/errorcodes.hxx>
#include <sal/log.hxx>
#include <rtl/character.hxx>

#include <global.hxx>
#include <rangeutl.hxx>
#include <compiler.hxx>
#include <paramisc.hxx>
#include <calcconfig.hxx>

// struct ScImportParam:

ScImportParam::ScImportParam() :
    nCol1(0),
    nRow1(0),
    nCol2(0),
    nRow2(0),
    bImport(false),
    bNative(false),
    bSql(true),
    nType(ScDbTable)
{
}

ScImportParam::ScImportParam( const ScImportParam& r ) :
    nCol1       (r.nCol1),
    nRow1       (r.nRow1),
    nCol2       (r.nCol2),
    nRow2       (r.nRow2),
    bImport     (r.bImport),
    aDBName     (r.aDBName),
    aStatement  (r.aStatement),
    bNative     (r.bNative),
    bSql        (r.bSql),
    nType       (r.nType)
{
}

ScImportParam::~ScImportParam()
{
}

ScImportParam& ScImportParam::operator=( const ScImportParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    bImport         = r.bImport;
    aDBName         = r.aDBName;
    aStatement      = r.aStatement;
    bNative         = r.bNative;
    bSql            = r.bSql;
    nType           = r.nType;

    return *this;
}

bool ScImportParam::operator==( const ScImportParam& rOther ) const
{
    return( nCol1       == rOther.nCol1 &&
            nRow1       == rOther.nRow1 &&
            nCol2       == rOther.nCol2 &&
            nRow2       == rOther.nRow2 &&
            bImport     == rOther.bImport &&
            aDBName     == rOther.aDBName &&
            aStatement  == rOther.aStatement &&
            bNative     == rOther.bNative &&
            bSql        == rOther.bSql &&
            nType       == rOther.nType );

    //TODO: are nQuerySh and pConnection equal ?
}

// struct ScConsolidateParam:

ScConsolidateParam::ScConsolidateParam()
{
    Clear();
}

ScConsolidateParam::ScConsolidateParam( const ScConsolidateParam& r )
{
    operator=(r);
}

ScConsolidateParam::~ScConsolidateParam()
{
}

void ScConsolidateParam::ClearDataAreas()
{
    pDataAreas.reset();
    nDataAreaCount = 0;
}

void ScConsolidateParam::Clear()
{
    ClearDataAreas();

    nCol = 0;
    nRow = 0;
    nTab = 0;
    bByCol = bByRow = bReferenceData    = false;
    eFunction                           = SUBTOTAL_FUNC_SUM;
}

ScConsolidateParam& ScConsolidateParam::operator=( const ScConsolidateParam& r )
{
    if (this != &r)
    {
        nCol            = r.nCol;
        nRow            = r.nRow;
        nTab            = r.nTab;
        bByCol          = r.bByCol;
        bByRow          = r.bByRow;
        bReferenceData  = r.bReferenceData;
        eFunction       = r.eFunction;
        nDataAreaCount  = r.nDataAreaCount;
        if ( r.nDataAreaCount > 0 )
        {
            nDataAreaCount = r.nDataAreaCount;
            pDataAreas.reset( new ScArea[nDataAreaCount] );
            for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
                pDataAreas[i] = r.pDataAreas[i];
        }
        else
            pDataAreas.reset();
    }
    return *this;
}

bool ScConsolidateParam::operator==( const ScConsolidateParam& r ) const
{
    bool bEqual =   (nCol           == r.nCol)
                 && (nRow           == r.nRow)
                 && (nTab           == r.nTab)
                 && (bByCol         == r.bByCol)
                 && (bByRow         == r.bByRow)
                 && (bReferenceData == r.bReferenceData)
                 && (nDataAreaCount == r.nDataAreaCount)
                 && (eFunction      == r.eFunction);

    if ( nDataAreaCount == 0 )
        bEqual = bEqual && (pDataAreas == nullptr) && (r.pDataAreas == nullptr);
    else
        bEqual = bEqual && (pDataAreas != nullptr) && (r.pDataAreas != nullptr);

    if ( bEqual && (nDataAreaCount > 0) )
        for ( sal_uInt16 i=0; i<nDataAreaCount && bEqual; i++ )
            bEqual = pDataAreas[i] == r.pDataAreas[i];

    return bEqual;
}

void ScConsolidateParam::SetAreas( std::unique_ptr<ScArea[]> pAreas, sal_uInt16 nCount )
{
    pDataAreas = std::move(pAreas);
    nDataAreaCount = nCount;
}

// struct ScSolveParam

ScSolveParam::ScSolveParam()
{
}

ScSolveParam::ScSolveParam( const ScSolveParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefVariableCell( r.aRefVariableCell ),
        pStrTargetVal   ( r.pStrTargetVal )
{
}

ScSolveParam::ScSolveParam( const ScAddress& rFormulaCell,
                            const ScAddress& rVariableCell,
                            const OUString&   rTargetValStr )
    :   aRefFormulaCell ( rFormulaCell ),
        aRefVariableCell( rVariableCell ),
        pStrTargetVal   ( rTargetValStr )
{
}

ScSolveParam::~ScSolveParam()
{
}

ScSolveParam& ScSolveParam::operator=( const ScSolveParam& r )
{
    aRefFormulaCell  = r.aRefFormulaCell;
    aRefVariableCell = r.aRefVariableCell;
    pStrTargetVal = r.pStrTargetVal;
    return *this;
}

bool ScSolveParam::operator==( const ScSolveParam& r ) const
{
    bool bEqual =   (aRefFormulaCell  == r.aRefFormulaCell)
                 && (aRefVariableCell == r.aRefVariableCell);

    if ( bEqual )
    {
        if ( !pStrTargetVal && !r.pStrTargetVal )
            bEqual = true;
        else if ( !pStrTargetVal || !r.pStrTargetVal )
            bEqual = false;
        else
            bEqual = ( *pStrTargetVal == *(r.pStrTargetVal) );
    }

    return bEqual;
}

// struct ScTabOpParam

ScTabOpParam::ScTabOpParam() : meMode(Column) {}

ScTabOpParam::ScTabOpParam( const ScTabOpParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefFormulaEnd  ( r.aRefFormulaEnd ),
        aRefRowCell     ( r.aRefRowCell ),
        aRefColCell     ( r.aRefColCell ),
    meMode(r.meMode)
{
}

ScTabOpParam::ScTabOpParam( const ScRefAddress& rFormulaCell,
                            const ScRefAddress& rFormulaEnd,
                            const ScRefAddress& rRowCell,
                            const ScRefAddress& rColCell,
                            Mode eMode )
    :   aRefFormulaCell ( rFormulaCell ),
        aRefFormulaEnd  ( rFormulaEnd ),
        aRefRowCell     ( rRowCell ),
        aRefColCell     ( rColCell ),
    meMode(eMode)
{
}

ScTabOpParam& ScTabOpParam::operator=( const ScTabOpParam& r )
{
    aRefFormulaCell  = r.aRefFormulaCell;
    aRefFormulaEnd   = r.aRefFormulaEnd;
    aRefRowCell      = r.aRefRowCell;
    aRefColCell      = r.aRefColCell;
    meMode = r.meMode;
    return *this;
}

bool ScTabOpParam::operator==( const ScTabOpParam& r ) const
{
    return (        (aRefFormulaCell == r.aRefFormulaCell)
                 && (aRefFormulaEnd  == r.aRefFormulaEnd)
                 && (aRefRowCell     == r.aRefRowCell)
                 && (aRefColCell     == r.aRefColCell)
                 && (meMode == r.meMode) );
}

OUString ScGlobal::GetAbsDocName( const OUString& rFileName,
                                  const SfxObjectShell* pShell )
{
    OUString aAbsName;
    if (!pShell || !pShell->HasName())
    {   // maybe relative to document path working directory
        INetURLObject aObj;
        if (!utl::ConfigManager::IsFuzzing())
        {
            aObj.SetSmartURL(SvtPathOptions().GetWorkPath());
            aObj.setFinalSlash();       // it IS a path
        }
        else
            aObj.SetSmartURL("file:///tmp/document");
        bool bWasAbs = true;
        aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::DecodeMechanism::NONE);
        //  returned string must be encoded because it's used directly to create SfxMedium
    }
    else
    {
        const SfxMedium* pMedium = pShell->GetMedium();
        if ( pMedium )
        {
            bool bWasAbs = true;
            aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::DecodeMechanism::NONE);
        }
        else
        {   // This can't happen, but ...
            // just to be sure to have the same encoding
            INetURLObject aObj;
            aObj.SetSmartURL( aAbsName );
            aAbsName = aObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        }
    }
    return aAbsName;
}

OUString ScGlobal::GetDocTabName( std::u16string_view rFileName,
                                std::u16string_view rTabName )
{
    OUString  aDocTab = OUString::Concat("'") + rFileName;
    sal_Int32 nPos = 1;
    while( (nPos = aDocTab.indexOf( '\'', nPos )) != -1 )
    {   // escape Quotes
        aDocTab = aDocTab.replaceAt( nPos, 0, "\\" );
        nPos += 2;
    }
    aDocTab += "'" + OUStringChar(SC_COMPILER_FILE_TAB_SEP) + rTabName;
        // "'Doc'#Tab"
    return aDocTab;
}

namespace
{
bool isEmptyString( const OUString& rStr )
{
    if (rStr.isEmpty())
        return true;
    else if (rStr[0] == ' ')
    {
        const sal_Unicode* p = rStr.getStr() + 1;
        const sal_Unicode* const pStop = p - 1 + rStr.getLength();
        while (p < pStop && *p == ' ')
            ++p;
        if (p == pStop)
            return true;
    }
    return false;
}
}

double ScGlobal::ConvertStringToValue( const OUString& rStr, const ScCalcConfig& rConfig,
        FormulaError & rError, FormulaError nStringNoValueError,
        SvNumberFormatter* pFormatter, SvNumFormatType & rCurFmtType )
{
    // We keep ScCalcConfig::StringConversion::LOCALE default until
    // we provide a friendly way to convert string numbers into numbers in the UI.

    double fValue = 0.0;
    if (nStringNoValueError == FormulaError::CellNoValue)
    {
        // Requested that all strings result in 0, error handled by caller.
        rError = nStringNoValueError;
        return fValue;
    }

    switch (rConfig.meStringConversion)
    {
        case ScCalcConfig::StringConversion::ILLEGAL:
            rError = nStringNoValueError;
            return fValue;
        case ScCalcConfig::StringConversion::ZERO:
            return fValue;
        case ScCalcConfig::StringConversion::LOCALE:
            {
                if (rConfig.mbEmptyStringAsZero)
                {
                    // The number scanner does not accept empty strings or strings
                    // containing only spaces, be on par in these cases with what was
                    // accepted in OOo and is in AOO (see also the
                    // StringConversion::UNAMBIGUOUS branch) and convert to 0 to prevent
                    // interoperability nightmares.

                    if (isEmptyString( rStr))
                        return fValue;
                }

                if (!pFormatter)
                    goto Label_fallback_to_unambiguous;

                sal_uInt32 nFIndex = 0;
                if (!pFormatter->IsNumberFormat(rStr, nFIndex, fValue))
                {
                    rError = nStringNoValueError;
                    fValue = 0.0;
                }
                return fValue;
            }
            break;
        case ScCalcConfig::StringConversion::UNAMBIGUOUS:
Label_fallback_to_unambiguous:
            {
                if (!rConfig.mbEmptyStringAsZero)
                {
                    if (isEmptyString( rStr))
                    {
                        rError = nStringNoValueError;
                        return fValue;
                    }
                }
            }
            // continue below, pulled from switch case for better readability
            break;
    }

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nParseEnd;
    // Decimal and group separator 0 => only integer and possibly exponent,
    // stops at first non-digit non-sign.
    fValue = ::rtl::math::stringToDouble( rStr, 0, 0, &eStatus, &nParseEnd);
    sal_Int32 nLen = rStr.getLength();
    if (eStatus == rtl_math_ConversionStatus_Ok && nParseEnd < nLen)
    {
        // Not at string end, check for trailing blanks or switch to date or
        // time parsing or bail out.
        const sal_Unicode* const pStart = rStr.getStr();
        const sal_Unicode* p = pStart + nParseEnd;
        const sal_Unicode* const pStop = pStart + nLen;
        switch (*p++)
        {
            case ' ':
                while (p < pStop && *p == ' ')
                    ++p;
                if (p < pStop)
                    rError = nStringNoValueError;
                break;
            case '-':
            case ':':
                {
                    bool bDate = (*(p-1) == '-');
                    enum State { year = 0, month, day, hour, minute, second, fraction, done, blank, stop };
                    sal_Int32 nUnit[done] = {0,0,0,0,0,0,0};
                    const sal_Int32 nLimit[done] = {0,12,31,0,59,59,0};
                    State eState = (bDate ? month : minute);
                    rCurFmtType = (bDate ? SvNumFormatType::DATE : SvNumFormatType::TIME);
                    nUnit[eState-1] = rStr.copy( 0, nParseEnd).toInt32();
                    const sal_Unicode* pLastStart = p;
                    // Ensure there's no preceding sign. Negative dates
                    // currently aren't handled correctly. Also discard
                    // +CCYY-MM-DD
                    p = pStart;
                    while (p < pStop && *p == ' ')
                        ++p;
                    if (p < pStop && !rtl::isAsciiDigit(*p))
                        rError = nStringNoValueError;
                    p = pLastStart;
                    while (p < pStop && rError == FormulaError::NONE && eState < blank)
                    {
                        if (eState == minute)
                            rCurFmtType |= SvNumFormatType::TIME;
                        if (rtl::isAsciiDigit(*p))
                        {
                            // Maximum 2 digits per unit, except fractions.
                            if (p - pLastStart >= 2 && eState != fraction)
                                rError = nStringNoValueError;
                        }
                        else if (p > pLastStart)
                        {
                            // We had at least one digit.
                            if (eState < done)
                            {
                                nUnit[eState] = rStr.copy( pLastStart - pStart, p - pLastStart).toInt32();
                                if (nLimit[eState] && nLimit[eState] < nUnit[eState])
                                    rError = nStringNoValueError;
                            }
                            pLastStart = p + 1;     // hypothetical next start
                            // Delimiters must match, a trailing delimiter
                            // yields an invalid date/time.
                            switch (eState)
                            {
                                case month:
                                    // Month must be followed by separator and
                                    // day, no trailing blanks.
                                    if (*p != '-' || (p+1 == pStop))
                                        rError = nStringNoValueError;
                                    break;
                                case day:
                                    if ((*p != 'T' || (p+1 == pStop)) && *p != ' ')
                                        rError = nStringNoValueError;
                                    // Take one blank as a valid delimiter
                                    // between date and time.
                                    break;
                                case hour:
                                    // Hour must be followed by separator and
                                    // minute, no trailing blanks.
                                    if (*p != ':' || (p+1 == pStop))
                                        rError = nStringNoValueError;
                                    break;
                                case minute:
                                    if ((*p != ':' || (p+1 == pStop)) && *p != ' ')
                                        rError = nStringNoValueError;
                                    if (*p == ' ')
                                        eState = done;
                                    break;
                                case second:
                                    if (((*p != ',' && *p != '.') || (p+1 == pStop)) && *p != ' ')
                                        rError = nStringNoValueError;
                                    if (*p == ' ')
                                        eState = done;
                                    break;
                                case fraction:
                                    eState = done;
                                    break;
                                default:
                                    rError = nStringNoValueError;
                                    break;
                            }
                            eState = static_cast<State>(eState + 1);
                        }
                        else
                            rError = nStringNoValueError;
                        ++p;
                    }
                    if (eState == blank)
                    {
                        while (p < pStop && *p == ' ')
                            ++p;
                        if (p < pStop)
                            rError = nStringNoValueError;
                        eState = stop;
                    }

                    // Month without day, or hour without minute.
                    if (eState == month || (eState == day && p <= pLastStart) ||
                            eState == hour || (eState == minute && p <= pLastStart))
                        rError = nStringNoValueError;

                    if (rError == FormulaError::NONE)
                    {
                        // Catch the very last unit at end of string.
                        if (p > pLastStart && eState < done)
                        {
                            nUnit[eState] = rStr.copy( pLastStart - pStart, p - pLastStart).toInt32();
                            if (nLimit[eState] && nLimit[eState] < nUnit[eState])
                                rError = nStringNoValueError;
                        }
                        if (bDate && nUnit[hour] > 23)
                            rError = nStringNoValueError;
                        if (rError == FormulaError::NONE)
                        {
                            if (bDate && nUnit[day] == 0)
                                nUnit[day] = 1;
                            double fFraction = (nUnit[fraction] <= 0 ? 0.0 :
                                    ::rtl::math::pow10Exp( nUnit[fraction],
                                        static_cast<int>( -ceil( log10( static_cast<double>( nUnit[fraction]))))));
                            if (!bDate)
                                fValue = 0.0;
                            else
                            {
                                Date aDate(
                                        sal::static_int_cast<sal_Int16>(nUnit[day]),
                                        sal::static_int_cast<sal_Int16>(nUnit[month]),
                                        sal::static_int_cast<sal_Int16>(nUnit[year]));
                                if (!aDate.IsValidDate())
                                    rError = nStringNoValueError;
                                else
                                {
                                    if (pFormatter)
                                        fValue = aDate - pFormatter->GetNullDate();
                                    else
                                    {
                                        SAL_WARN("sc.core","ScGlobal::ConvertStringToValue - fixed null date");
                                        static Date aDefaultNullDate( 30, 12, 1899);
                                        fValue = aDate - aDefaultNullDate;
                                    }
                                }
                            }
                            fValue += ((nUnit[hour] * 3600) + (nUnit[minute] * 60) + nUnit[second] + fFraction) / 86400.0;
                        }
                    }
                }
                break;
            default:
                rError = nStringNoValueError;
        }
        if (rError != FormulaError::NONE)
            fValue = 0.0;
    }
    return fValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
