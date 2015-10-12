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

#include "scitems.hxx"
#include <editeng/scripttypeitem.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include "document.hxx"
#include "cellform.hxx"
#include "patattr.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"
#include "attrib.hxx"
#include "globalnames.hxx"
#include "columnspanset.hxx"
#include "table.hxx"

using namespace com::sun::star;

//  this file is compiled with exceptions enabled
//  put functions here that need exceptions!

const uno::Reference< i18n::XBreakIterator >& ScDocument::GetBreakIterator()
{
    if ( !pScriptTypeData )
        pScriptTypeData = new ScScriptTypeData;
    if ( !pScriptTypeData->xBreakIter.is() )
    {
        pScriptTypeData->xBreakIter = i18n::BreakIterator::create( comphelper::getProcessComponentContext() );
    }
    return pScriptTypeData->xBreakIter;
}

bool ScDocument::HasStringWeakCharacters( const OUString& rString )
{
    if (!rString.isEmpty())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            sal_Int32 nLen = rString.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( rString, nPos );
                if ( nType == i18n::ScriptType::WEAK )
                    return true;                            // found

                nPos = xBreakIter->endOfScript( rString, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }

    return false;       // none found
}

SvtScriptType ScDocument::GetStringScriptType( const OUString& rString )
{
    SvtScriptType nRet = SvtScriptType::NONE;
    if (!rString.isEmpty())
    {
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
        if ( xBreakIter.is() )
        {
            sal_Int32 nLen = rString.getLength();

            sal_Int32 nPos = 0;
            do
            {
                sal_Int16 nType = xBreakIter->getScriptType( rString, nPos );
                switch ( nType )
                {
                    case i18n::ScriptType::LATIN:
                        nRet |= SvtScriptType::LATIN;
                        break;
                    case i18n::ScriptType::ASIAN:
                        nRet |= SvtScriptType::ASIAN;
                        break;
                    case i18n::ScriptType::COMPLEX:
                        nRet |= SvtScriptType::COMPLEX;
                        break;
                    // WEAK is ignored
                }
                nPos = xBreakIter->endOfScript( rString, nPos, nType );
            }
            while ( nPos >= 0 && nPos < nLen );
        }
    }
    return nRet;
}

SvtScriptType ScDocument::GetCellScriptType( const ScAddress& rPos, sal_uLong nNumberFormat )
{
    SvtScriptType nStored = GetScriptType(rPos);
    if ( nStored != SvtScriptType::UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    Color* pColor;
    OUString aStr = ScCellFormat::GetString(*this, rPos, nNumberFormat, &pColor, *xPoolHelper->GetFormTable());

    SvtScriptType nRet = GetStringScriptType( aStr );

    SetScriptType(rPos, nRet);       // store for later calls

    return nRet;
}

SvtScriptType ScDocument::GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    // if script type is set, don't have to get number formats

    ScAddress aPos(nCol, nRow, nTab);
    SvtScriptType nStored = GetScriptType(aPos);
    if ( nStored != SvtScriptType::UNKNOWN )         // stored value valid?
        return nStored;                             // use stored value

    // include number formats from conditional formatting

    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    if (!pPattern) return SvtScriptType::NONE;
    const SfxItemSet* pCondSet = NULL;
    if ( !static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData().empty() )
        pCondSet = GetCondResult( nCol, nRow, nTab );

    sal_uLong nFormat = pPattern->GetNumberFormat( xPoolHelper->GetFormTable(), pCondSet );

    return GetCellScriptType(aPos, nFormat);
}

namespace {

class ScriptTypeAggregator : public sc::ColumnSpanSet::Action
{
    ScDocument& mrDoc;
    sc::ColumnBlockPosition maBlockPos;
    SvtScriptType mnScriptType;

public:
    ScriptTypeAggregator(ScDocument& rDoc) : mrDoc(rDoc), mnScriptType(SvtScriptType::NONE) {}

    virtual void startColumn(SCTAB nTab, SCCOL nCol) override
    {
        mrDoc.InitColumnBlockPosition(maBlockPos, nTab, nCol);
    }

    virtual void execute(const ScAddress& rPos, SCROW nLength, bool bVal) override
    {
        if (!bVal)
            return;

        mnScriptType |= mrDoc.GetRangeScriptType(maBlockPos, rPos, nLength);
    };

    SvtScriptType getScriptType() const { return mnScriptType; }
};

}

SvtScriptType ScDocument::GetRangeScriptType(
    sc::ColumnBlockPosition& rBlockPos, const ScAddress& rPos, SCROW nLength )
{
    if (!TableExists(rPos.Tab()))
        return SvtScriptType::NONE;

    return maTabs[rPos.Tab()]->GetRangeScriptType(rBlockPos, rPos.Col(), rPos.Row(), rPos.Row()+nLength-1);
}

SvtScriptType ScDocument::GetRangeScriptType( const ScRangeList& rRanges )
{
    sc::ColumnSpanSet aSet(false);
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange& rRange = *rRanges[i];
        SCTAB nTab = rRange.aStart.Tab();
        SCROW nRow1 = rRange.aStart.Row();
        SCROW nRow2 = rRange.aEnd.Row();
        for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
            aSet.set(nTab, nCol, nRow1, nRow2, true);
    }

    ScriptTypeAggregator aAction(*this);
    aSet.executeAction(aAction);
    return aAction.getScriptType();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
