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
                              const Color** ppColor, SvNumberFormatter& rFormatter, const ScDocument& rDoc,
                              bool bNullVals, bool bFormula, bool bUseStarFormat )
{
    *ppColor = nullptr;

    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
        {
            OUString str;
            rFormatter.GetOutputString(rCell.mpString->getString(), nFormat, str, ppColor, bUseStarFormat);
            return str;
        }
        case CELLTYPE_EDIT:
        {
            OUString str;
            rFormatter.GetOutputString(rCell.getString(&rDoc), nFormat, str, ppColor );
            return str;
        }
        case CELLTYPE_VALUE:
        {
            const double & nValue = rCell.mfValue;
            if (!bNullVals && nValue == 0.0)
                return OUString();
            else
            {
                OUString str;
                rFormatter.GetOutputString( nValue, nFormat, str, ppColor, bUseStarFormat );
                return str;
            }
        }
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell*  pFCell = rCell.mpFormula;
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
                            rFormatter.GetOutputString( fValue, nFormat, str, ppColor, bUseStarFormat );
                            return str;
                        }
                    }
                    else
                    {
                        OUString str;
                        rFormatter.GetOutputString( pFCell->GetString().getString(),
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
    SvNumberFormatter& rFormatter, bool bNullVals, bool bFormula )
{
    *ppColor = nullptr;

    ScRefCellValue aCell(rDoc, rPos);
    return GetString(aCell, nFormat, ppColor, rFormatter, rDoc, bNullVals, bFormula);
}

OUString ScCellFormat::GetInputString(
    const ScRefCellValue& rCell, sal_uInt32 nFormat, SvNumberFormatter& rFormatter, const ScDocument& rDoc,
    const svl::SharedString** pShared, bool bFiltering )
{
    if(pShared != nullptr)
        *pShared = nullptr;
    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return rCell.getString(&rDoc);
        case CELLTYPE_VALUE:
        {
            OUString str;
            rFormatter.GetInputLineString(rCell.mfValue, nFormat, str, bFiltering);
            return str;
        }
        break;
        case CELLTYPE_FORMULA:
        {
            OUString str;
            ScFormulaCell* pFC = rCell.mpFormula;
            if (pFC->IsEmptyDisplayedAsString())
                ; // empty
            else if (pFC->IsValue())
                rFormatter.GetInputLineString(pFC->GetValue(), nFormat, str, bFiltering);
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
                str.clear();
                if( pShared != nullptr )
                    *pShared = nullptr;
            }

            return str;
        }
        case CELLTYPE_NONE:
            if( pShared != nullptr )
                *pShared = &svl::SharedString::getEmptyString();
            return OUString();
        default:
            return OUString();
    }
}

OUString ScCellFormat::GetOutputString( ScDocument& rDoc, const ScAddress& rPos, const ScRefCellValue& rCell )
{
    if (rCell.isEmpty())
        return OUString();

    if (rCell.meType == CELLTYPE_EDIT)
    {
        //  GetString converts line breaks into spaces in EditCell,
        //  but here we need the line breaks
        const EditTextObject* pData = rCell.mpEditText;
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
        return GetString(rCell, nNumFmt, &pColor, *rDoc.GetFormatTable(), rDoc);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
