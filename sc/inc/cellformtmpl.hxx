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

#include "cellform.hxx"

#include <svl/numformat.hxx>
#include <svl/sharedstring.hxx>
#include <svl/sharedstringpool.hxx>

#include "formulacell.hxx"
#include "document.hxx"
#include "cellvalue.hxx"
#include <formula/errorcodes.hxx>
#include "editutil.hxx"

template <typename TFunctor>
auto ScCellFormat::visitInputSharedString(const ScRefCellValue& rCell, sal_uInt32 nFormat,
                                          ScInterpreterContext* pContext, const ScDocument& rDoc,
                                          svl::SharedStringPool& rStrPool, bool bFiltering,
                                          bool bForceSystemLocale, const TFunctor& rOper)
{
    ScInterpreterContext& rContext = pContext ? *pContext : rDoc.GetNonThreadedContext();

    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return rOper(rCell.getSharedString(rDoc, rStrPool));
        case CELLTYPE_VALUE:
            return rOper(rStrPool.intern(rContext.NFGetInputLineString(
                rCell.getDouble(), nFormat, bFiltering, bForceSystemLocale)));
            break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFC = rCell.getFormula();
            const FormulaError nErrCode = pFC->GetErrCode();
            if (nErrCode != FormulaError::NONE)
                return rOper(svl::SharedString::getEmptyString());
            else if (pFC->IsEmptyDisplayedAsString())
                return rOper(svl::SharedString::getEmptyString());
            else if (pFC->IsValue())
                return rOper(rStrPool.intern(rContext.NFGetInputLineString(
                    pFC->GetValue(), nFormat, bFiltering, bForceSystemLocale)));
            else
                return rOper(pFC->GetString());
        }
        case CELLTYPE_NONE:
        default:
            return rOper(svl::SharedString::getEmptyString());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
