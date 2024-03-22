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

#include <cellform.hxx>

#include <svl/numformat.hxx>
#include <svl/sharedstring.hxx>

#include <formulacell.hxx>
#include <document.hxx>
#include <cellvalue.hxx>
#include <formula/errorcodes.hxx>
#include <editutil.hxx>

OUString ScCellFormat::GetString( const ScRefCellValue& rCell, sal_uInt32 nFormat,
                              const Color** ppColor, ScInterpreterContext* pContext, const ScDocument& rDoc,
                              bool bNullVals, bool bFormula, bool bUseStarFormat )
{
    *ppColor = nullptr;

    ScInterpreterContext& rContext = pContext ? *pContext : rDoc.GetNonThreadedContext();

    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
        {
            OUString str;
            rContext.NFGetOutputString(rCell.getSharedString()->getString(), nFormat, str, ppColor, bUseStarFormat);
            return str;
        }
        case CELLTYPE_EDIT:
        {
            OUString str;
            rContext.NFGetOutputString(rCell.getString(&rDoc), nFormat, str, ppColor );
            return str;
        }
        case CELLTYPE_VALUE:
        {
            const double nValue = rCell.getDouble();
            if (!bNullVals && nValue == 0.0)
                return OUString();
            else
            {
                OUString str;
                rContext.NFGetOutputString( nValue, nFormat, str, ppColor, bUseStarFormat );
                return str;
            }
        }
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell*  pFCell = rCell.getFormula();
            if ( bFormula )
            {
                return pFCell->GetFormula();
            }
            else
            {
                // A macro started from the interpreter, which has
                // access to Formula Cells, becomes a CellText, even if
                // that triggers further interpretation, except if those
                // cells are already being interpreted.
                // IdleCalc generally doesn't trigger further interpretation,
                // as not to get Err522 (circular).
                if ( pFCell->GetDocument().IsInInterpreter() &&
                        (!pFCell->GetDocument().GetMacroInterpretLevel()
                        || pFCell->IsRunning()) )
                {
                    return "...";
                }
                else
                {
                    const FormulaError nErrCode = pFCell->GetErrCode();

                    if (nErrCode != FormulaError::NONE)
                        return ScGlobal::GetErrorString(nErrCode);
                    else if ( pFCell->IsEmptyDisplayedAsString() )
                        return OUString();
                    else if ( pFCell->IsValue() )
                    {
                        double fValue = pFCell->GetValue();
                        if ( !bNullVals && fValue == 0.0 )
                            return OUString();
                        else
                        {
                            OUString str;
                            rContext.NFGetOutputString( fValue, nFormat, str, ppColor, bUseStarFormat );
                            return str;
                        }
                    }
                    else
                    {
                        OUString str;
                        rContext.NFGetOutputString( pFCell->GetString().getString(),
                                                    nFormat, str, ppColor, bUseStarFormat );
                        return str;
                    }
                }
            }
        }
        default:
            return OUString();
    }
}

OUString ScCellFormat::GetString(
    ScDocument& rDoc, const ScAddress& rPos, sal_uInt32 nFormat, const Color** ppColor,
    ScInterpreterContext* pContext, bool bNullVals, bool bFormula )
{
    *ppColor = nullptr;

    ScRefCellValue aCell(rDoc, rPos);
    return GetString(aCell, nFormat, ppColor, pContext, rDoc, bNullVals, bFormula);
}

OUString ScCellFormat::GetInputString(
    const ScRefCellValue& rCell, sal_uInt32 nFormat, ScInterpreterContext* pContext, const ScDocument& rDoc,
    const svl::SharedString** pShared, bool bFiltering, bool bForceSystemLocale )
{
    ScInterpreterContext& rContext = pContext ? *pContext : rDoc.GetNonThreadedContext();

    if(pShared != nullptr)
        *pShared = nullptr;
    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return rCell.getString(&rDoc);
        case CELLTYPE_VALUE:
        {
            OUString str;
            rContext.NFGetInputLineString(rCell.getDouble(), nFormat, str, bFiltering, bForceSystemLocale);
            return str;
        }
        break;
        case CELLTYPE_FORMULA:
        {
            std::optional<OUString> str;
            ScFormulaCell* pFC = rCell.getFormula();
            if (pFC->IsEmptyDisplayedAsString())
                ; // empty
            else if (pFC->IsValue())
            {
                str.emplace();
                rContext.NFGetInputLineString(pFC->GetValue(), nFormat, *str, bFiltering, bForceSystemLocale);
            }
            else
            {
                const svl::SharedString& shared = pFC->GetString();
                // Allow callers to optimize by avoiding converting later back to OUString.
                // To avoid refcounting that won't be needed, do not even return the OUString.
                if( pShared != nullptr )
                    *pShared = &shared;
                else
                    str = shared.getString();
            }

            const FormulaError nErrCode = pFC->GetErrCode();
            if (nErrCode != FormulaError::NONE)
            {
                str.reset();
                if( pShared != nullptr )
                    *pShared = nullptr;
            }

            return str ? std::move(*str) : svl::SharedString::EMPTY_STRING;
        }
        case CELLTYPE_NONE:
            if( pShared != nullptr )
                *pShared = &svl::SharedString::getEmptyString();
            return svl::SharedString::EMPTY_STRING;
        default:
            return svl::SharedString::EMPTY_STRING;
    }
}

OUString ScCellFormat::GetOutputString( ScDocument& rDoc, const ScAddress& rPos, const ScRefCellValue& rCell )
{
    if (rCell.isEmpty())
        return OUString();

    if (rCell.getType() == CELLTYPE_EDIT)
    {
        //  GetString converts line breaks into spaces in EditCell,
        //  but here we need the line breaks
        const EditTextObject* pData = rCell.getEditText();
        if (pData)
        {
            ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
            rEngine.SetTextCurrentDefaults(*pData);
            return rEngine.GetText();
        }
        //  also do not format EditCells as numbers
        //  (fitting to output)
        return OUString();
    }
    else
    {
        //  like in GetString for document (column)
        const Color* pColor;
        sal_uInt32 nNumFmt = rDoc.GetNumberFormat(rPos);
        return GetString(rCell, nNumFmt, &pColor, nullptr, rDoc);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
