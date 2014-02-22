/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "compare.hxx"

#include "document.hxx"
#include "docoptio.hxx"

#include "unotools/textsearch.hxx"

namespace sc {

Compare::Cell::Cell() :
    mfValue(0.0), mbValue(false), mbEmpty(false) {}

Compare::Compare() :
    meOp(SC_EQUAL), mbIgnoreCase(true) {}

CompareOptions::CompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg ) :
    aQueryEntry(rEntry),
    bRegEx(bReg),
    bMatchWholeCell(pDoc->GetDocOptions().IsMatchWholeCell())
{
    bRegEx = (bRegEx && (aQueryEntry.eOp == SC_EQUAL || aQueryEntry.eOp == SC_NOT_EQUAL));
    
    
    
}

double CompareFunc( const Compare::Cell& rCell1, const Compare::Cell& rCell2, bool bIgnoreCase, CompareOptions* pOptions )
{
    
    
    if (!rCell1.mbEmpty && rCell1.mbValue && !rtl::math::isFinite(rCell1.mfValue))
        return rCell1.mfValue;
    if (!rCell2.mbEmpty && rCell2.mbValue && !rtl::math::isFinite(rCell2.mfValue))
        return rCell2.mfValue;

    size_t nStringQuery = 0;    
    double fRes = 0;
    if (rCell1.mbEmpty)
    {
        if (rCell2.mbEmpty)
            ;       
        else if (rCell2.mbValue)
        {
            if (rCell2.mfValue != 0.0)
            {
                if (rCell2.mfValue < 0.0)
                    fRes = 1;       
                else
                    fRes = -1;      
            }
            
        }
        else
        {
            if (!rCell2.maStr.isEmpty())
                fRes = -1;      
            
        }
    }
    else if (rCell2.mbEmpty)
    {
        if (rCell1.mbValue)
        {
            if (rCell1.mfValue != 0.0)
            {
                if (rCell1.mfValue < 0.0)
                    fRes = -1;      
                else
                    fRes = 1;       
            }
            
        }
        else
        {
            if (!rCell1.maStr.isEmpty())
                fRes = 1;       
            
        }
    }
    else if (rCell1.mbValue)
    {
        if (rCell2.mbValue)
        {
            if (!rtl::math::approxEqual(rCell1.mfValue, rCell2.mfValue))
            {
                if (rCell1.mfValue - rCell2.mfValue < 0)
                    fRes = -1;
                else
                    fRes = 1;
            }
        }
        else
        {
            fRes = -1;          
            nStringQuery = 2;   
        }
    }
    else if (rCell2.mbValue)
    {
        fRes = 1;               
        nStringQuery = 1;       
    }
    else
    {
        
        if (pOptions)
        {
            
            
            
            ScQueryEntry& rEntry = pOptions->aQueryEntry;
            OSL_ENSURE(rEntry.GetQueryItem().maString == rCell2.maStr, "ScInterpreter::CompareFunc: broken options");
            if (pOptions->bRegEx)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nStop  = rCell1.maStr.getLength();
                bool bMatch = rEntry.GetSearchTextPtr(
                        !bIgnoreCase)->SearchForward(
                            rCell1.maStr.getString(), &nStart, &nStop);
                if (bMatch && pOptions->bMatchWholeCell && (nStart != 0 || nStop != rCell1.maStr.getLength()))
                    bMatch = false;     
                fRes = (bMatch ? 0 : 1);
            }
            else if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
            {
                ::utl::TransliterationWrapper* pTransliteration =
                    (bIgnoreCase ? ScGlobal::GetpTransliteration() :
                     ScGlobal::GetCaseTransliteration());
                bool bMatch = false;
                if (pOptions->bMatchWholeCell)
                {
                    if (bIgnoreCase)
                        bMatch = rCell1.maStr.getDataIgnoreCase() == rCell2.maStr.getDataIgnoreCase();
                    else
                        bMatch = rCell1.maStr.getData() == rCell2.maStr.getData();
                }
                else
                {
                    OUString aCell( pTransliteration->transliterate(
                                rCell1.maStr.getString(), ScGlobal::eLnge, 0,
                                rCell1.maStr.getLength(), NULL));
                    OUString aQuer( pTransliteration->transliterate(
                                rCell2.maStr.getString(), ScGlobal::eLnge, 0,
                                rCell2.maStr.getLength(), NULL));
                    bMatch = (aCell.indexOf( aQuer ) != -1);
                }
                fRes = (bMatch ? 0 : 1);
            }
            else if (bIgnoreCase)
                fRes = (double) ScGlobal::GetCollator()->compareString(
                        rCell1.maStr.getString(), rCell2.maStr.getString());
            else
                fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                        rCell1.maStr.getString(), rCell2.maStr.getString());
        }
        else if (bIgnoreCase)
            fRes = (double) ScGlobal::GetCollator()->compareString(
                rCell1.maStr.getString(), rCell2.maStr.getString());
        else
            fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                rCell1.maStr.getString(), rCell2.maStr.getString());
    }

    if (nStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                
                
                
                bool bEqual = false;
                if (nStringQuery == 1)
                    bEqual = rCell1.maStr == rItem.maString;
                else
                    bEqual = rCell2.maStr == rItem.maString;

                
                fRes = double((rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual);
            }
        }
    }

    return fRes;
}

double CompareFunc( double fCell1, const Compare::Cell& rCell2, CompareOptions* pOptions )
{
    
    
    if (!rtl::math::isFinite(fCell1))
        return fCell1;
    if (!rCell2.mbEmpty && rCell2.mbValue && !rtl::math::isFinite(rCell2.mfValue))
        return rCell2.mfValue;

    bool bStringQuery = false;
    double fRes = 0;
    if (rCell2.mbEmpty)
    {
        if (fCell1 != 0.0)
        {
            if (fCell1 < 0.0)
                fRes = -1;      
            else
                fRes = 1;       
        }
        
    }
    else
    {
        if (rCell2.mbValue)
        {
            if (!rtl::math::approxEqual(fCell1, rCell2.mfValue))
            {
                if (fCell1 - rCell2.mfValue < 0)
                    fRes = -1;
                else
                    fRes = 1;
            }
        }
        else
        {
            fRes = -1;          
            bStringQuery = true;
        }
    }

    if (bStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                
                
                
                bool bEqual = rCell2.maStr == rItem.maString;

                
                fRes = double((rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual);
            }
        }
    }

    return fRes;
}

double CompareFunc( const Compare::Cell& rCell1, double fCell2, CompareOptions* pOptions )
{
    
    
    if (!rCell1.mbEmpty && rCell1.mbValue && !rtl::math::isFinite(rCell1.mfValue))
        return rCell1.mfValue;
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    bool bStringQuery = false;
    double fRes = 0;
    if (rCell1.mbEmpty)
    {
        if (fCell2 != 0.0)
        {
            if (fCell2 < 0.0)
                fRes = 1;       
            else
                fRes = -1;      
        }
        
    }
    else if (rCell1.mbValue)
    {
        if (!rtl::math::approxEqual(rCell1.mfValue, fCell2))
        {
            if (rCell1.mfValue - fCell2 < 0)
                fRes = -1;
            else
                fRes = 1;
        }
    }
    else
    {
        fRes = 1;               
        bStringQuery = true;
    }

    if (bStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                
                
                
                bool bEqual = rCell1.maStr == rItem.maString;

                
                fRes = double((rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual);
            }
        }
    }

    return fRes;
}

double CompareFunc( double fCell1, double fCell2 )
{
    
    
    if (!rtl::math::isFinite(fCell1))
        return fCell1;
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    double fRes = 0.0;

    if (!rtl::math::approxEqual(fCell1, fCell2))
    {
        if (fCell1 - fCell2 < 0.0)
            fRes = -1;
        else
            fRes = 1;
    }

    return fRes;
}

double CompareEmptyToNumericFunc( double fCell2 )
{
    
    
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    double fRes = 0;
    if (fCell2 != 0.0)
    {
        if (fCell2 < 0.0)
            fRes = 1;       
        else
            fRes = -1;      
    }
    

    return fRes;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
