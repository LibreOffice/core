/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <extlstcontext.hxx>
#include <worksheethelper.hxx>
#include <oox/core/contexthandler.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <colorscale.hxx>
#include <condformatbuffer.hxx>
#include <condformatcontext.hxx>
#include <document.hxx>
#include <worksheetfragment.hxx>
#include <workbookfragment.hxx>
#include <stylesbuffer.hxx>
#include <stylesfragment.hxx>
#include <SparklineFragment.hxx>

#include <rangeutl.hxx>
#include <sal/log.hxx>

using ::oox::core::ContextHandlerRef;
using ::oox::xls::CondFormatBuffer;

sal_Int32 gnStyleIdx = 0; // Holds index of the <extlst> <cfRule> style (Will be reset by finalize import)

namespace oox::xls {

ExtCfRuleContext::ExtCfRuleContext( WorksheetContextBase& rFragment, ScDataBarFormatData* pTarget ):
    WorksheetContextBase( rFragment ),
    mpTarget( pTarget ),
    mbFirstEntry(true)
{
}

ContextHandlerRef ExtCfRuleContext::onCreateContext( sal_Int32 , const AttributeList& )
{
    return this;
}

void ExtCfRuleContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS14_TOKEN( dataBar ):
        {
            ExtCfDataBarRuleRef xRule = getCondFormats().createExtCfDataBarRule(mpTarget);
            xRule->importDataBar( rAttribs );
            break;
        }
        case XLS14_TOKEN( fillColor ):
        {
            ExtCfDataBarRuleRef xRule = getCondFormats().createExtCfDataBarRule(mpTarget);
            xRule->importPositiveFillColor( rAttribs );
            break;
        }
        case XLS14_TOKEN( negativeFillColor ):
        {
            ExtCfDataBarRuleRef xRule = getCondFormats().createExtCfDataBarRule(mpTarget);
            xRule->importNegativeFillColor( rAttribs );
            break;
        }
        case XLS14_TOKEN( axisColor ):
        {
            ExtCfDataBarRuleRef xRule = getCondFormats().createExtCfDataBarRule(mpTarget);
            xRule->importAxisColor( rAttribs );
            break;
        }
        case XLS14_TOKEN( cfvo ):
        {
            ExtCfDataBarRuleRef xRule = getCondFormats().createExtCfDataBarRule(mpTarget);
            xRule->importCfvo( rAttribs );
            xRule->getModel().mbIsLower = mbFirstEntry;
            mbFirstEntry = false;
            mpRule = xRule;
            break;
        }
        default:
            break;
    }
}

void ExtCfRuleContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XM_TOKEN( f ):
        {
            if (mpRule)
            {
                mpRule->getModel().msScaleTypeValue = rChars;
            }
        }
        break;
    }
}

void ExtCfRuleContext::onEndElement()
{
    switch( getCurrentElement() )
    {
        case XLS14_TOKEN( cfvo ):
        {
            mpRule.reset();
            break;
        }
    }
}

namespace {
    bool IsSpecificTextCondMode(ScConditionMode eMode)
    {
        switch (eMode)
        {
        case ScConditionMode::BeginsWith:
        case ScConditionMode::EndsWith:
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
            return true;
        default:
            break;
        }
        return false;
    }
}

ExtConditionalFormattingContext::ExtConditionalFormattingContext(WorksheetContextBase& rFragment)
    : WorksheetContextBase(rFragment)
    , nFormulaCount(0)
    , nPriority(-1)
    , eOperator(ScConditionMode::NONE)
    , isPreviousElementF(false)
{
}

ContextHandlerRef ExtConditionalFormattingContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    if (mpCurrentRule)
    {
        ScFormatEntry& rFormat = **maEntries.rbegin();
        assert(rFormat.GetType() == ScFormatEntry::Type::Iconset);
        ScIconSetFormat& rIconSet = static_cast<ScIconSetFormat&>(rFormat);
        ScDocument& rDoc = getScDocument();
        SCTAB nTab = getSheetIndex();
        ScAddress aPos(0, 0, nTab);
        mpCurrentRule->SetData(&rIconSet, &rDoc, aPos);
        mpCurrentRule.reset();
    }
    if (nElement == XLS14_TOKEN(cfRule))
    {
        OUString aType = rAttribs.getString(XML_type, OUString());
        OUString aId = rAttribs.getString(XML_id, OUString());
        nPriority = rAttribs.getInteger( XML_priority, -1 );
        maPriorities.push_back(nPriority);
        maModel.nPriority = nPriority;

        if (aType == "dataBar")
        {
            // an ext entry does not need to have an existing corresponding entry
            ScDataBarFormatData* pInfo;
            ExtLst::const_iterator aExt = getExtLst().find( aId );
            if (aExt == getExtLst().end())
            {
                pInfo = new ScDataBarFormatData();
                if (pInfo)
                {
                    auto pFormat = std::make_unique<ScDataBarFormat>(&getScDocument());
                    pFormat->SetDataBarData(pInfo);
                    getCondFormats().importExtFormatEntries().push_back(std::move(pFormat));
                }
            }
            else
            {
                pInfo = aExt->second;
            }

            if (!pInfo)
            {
                return nullptr;
            }
            return new ExtCfRuleContext( *this, pInfo );
        }
        else if (aType == "iconSet")
        {
            ScDocument& rDoc = getScDocument();
            mpCurrentRule.reset(new IconSetRule(*this));
            maEntries.push_back(std::make_unique<ScIconSetFormat>(&rDoc));
            return new IconSetContext(*this, mpCurrentRule.get());
        }
        else if (aType == "cellIs")
        {
            sal_Int32 aToken = rAttribs.getToken( XML_operator, XML_TOKEN_INVALID );
            eOperator =  CondFormatBuffer::convertToInternalOperator(aToken);
            maModel.eOperator = eOperator;
            return this;
        }
        else if (aType == "containsText")
        {
            eOperator = ScConditionMode::ContainsText;
            maModel.eOperator = eOperator;
            return this;
        }
        else if (aType == "notContainsText")
        {
            eOperator = ScConditionMode::NotContainsText;
            maModel.eOperator = eOperator;
            return this;
        }
        else if (aType == "beginsWith")
        {
            eOperator = ScConditionMode::BeginsWith;
            maModel.eOperator = eOperator;
            return this;
        }
        else if (aType == "endsWith")
        {
            eOperator = ScConditionMode::EndsWith;
            maModel.eOperator = eOperator;
            return this;
        }
        else if (aType == "expression")
        {
            eOperator = ScConditionMode::Direct;
            maModel.eOperator = eOperator;
            return this;
        }
        else
        {
            SAL_WARN("sc", "unhandled XLS14_TOKEN(cfRule) with type: " << aType);
        }
    }
    else if (nElement == XLS14_TOKEN( dxf ))
    {
        return new DxfContext( *this, getStyles().createExtDxf() );
    }
    else if (nElement == XM_TOKEN( sqref ) || nElement == XM_TOKEN( f ))
    {
        if(nElement == XM_TOKEN( f ))
           nFormulaCount++;
        return this;
    }

    return nullptr;
}

void ExtConditionalFormattingContext::onStartElement(const AttributeList& /*Attribs*/)
{
}

void ExtConditionalFormattingContext::onCharacters(const OUString& rCharacters)
{
    switch (getCurrentElement())
    {
        case XM_TOKEN(f):
        {
            aChars = rCharacters;
            isPreviousElementF = true;
        }
        break;
        case XM_TOKEN(sqref):
        {
            aChars = rCharacters;
        }
        break;
    }

}

void ExtConditionalFormattingContext::onEndElement()
{
    switch (getCurrentElement())
    {
        case XM_TOKEN(f):
        {
            if(!IsSpecificTextCondMode(eOperator) || nFormulaCount == 2)
               maModel.aFormula = aChars;
        }
        break;
        case XLS14_TOKEN( cfRule ):
        {
            if (IsSpecificTextCondMode(maModel.eOperator) && nFormulaCount == 1)
            {
                maModel.aFormula = aChars;
                maModel.eOperator = ScConditionMode::Direct;
            }

            if (Dxf* pDxf = getStyles().getExtDxfs().get(gnStyleIdx).get())
                pDxf->finalizeImport();
            maModel.aStyle = getStyles().createExtDxfStyle(gnStyleIdx);
            gnStyleIdx++;
            nFormulaCount = 0;
            maModels.push_back(maModel);
        }
        break;
        case XM_TOKEN(sqref):
        {
            ScRangeList aRange;
            ScDocument& rDoc = getScDocument();
            bool bSuccess = ScRangeStringConverter::GetRangeListFromString(aRange, aChars, rDoc, formula::FormulaGrammar::CONV_XL_OOX);
            if (!bSuccess || aRange.empty())
                break;

            SCTAB nTab = getSheetIndex();
            for (size_t i = 0; i < aRange.size(); ++i)
            {
                aRange[i].aStart.SetTab(nTab);
                aRange[i].aEnd.SetTab(nTab);
            }

            if (maModels.size() > 1)
            {
                std::sort(maModels.begin(), maModels.end(),
                          [](const ExtCondFormatRuleModel& lhs, const ExtCondFormatRuleModel& rhs) {
                              return lhs.nPriority < rhs.nPriority;
                          });
            }

            if (isPreviousElementF) // sqref can be alone in some cases.
            {
                for (size_t i = 0; i < maModels.size(); ++i)
                {
                    ScAddress rPos = aRange.GetTopLeftCorner();
                    ScCondFormatEntry* pEntry = new ScCondFormatEntry(maModels[i].eOperator, maModels[i].aFormula, "", rDoc,
                                                                      rPos, maModels[i].aStyle, "", "",
                                                                      formula::FormulaGrammar::GRAM_OOXML ,
                                                                      formula::FormulaGrammar::GRAM_OOXML,
                                                                      ScFormatEntry::Type::ExtCondition );
                    maEntries.push_back(std::unique_ptr<ScFormatEntry>(pEntry));
                }

                assert(maPriorities.size() >= maModels.size());
                maModels.clear();
            }

            std::vector< std::unique_ptr<ExtCfCondFormat> >& rExtFormats =  getCondFormats().importExtCondFormat();
            rExtFormats.push_back(std::make_unique<ExtCfCondFormat>(aRange, maEntries, &maPriorities));

            maPriorities.clear();
            isPreviousElementF = false;
        }
        break;
        default:
        break;
    }
}

ExtLstLocalContext::ExtLstLocalContext( WorksheetContextBase& rFragment, ScDataBarFormatData* pTarget ):
    WorksheetContextBase(rFragment),
    mpTarget(pTarget)
{
}

ContextHandlerRef ExtLstLocalContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( extLst ):
            if(nElement == XLS_TOKEN( ext ))
                return this;
            else
                return nullptr;
        case XLS_TOKEN( ext ):
            if (nElement == XLS14_TOKEN( id ))
                return this;
            else
                return nullptr;
    }
    return nullptr;
}

void ExtLstLocalContext::onStartElement( const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS14_TOKEN( id ):
        break;
    }
}

void ExtLstLocalContext::onCharacters( const OUString& rChars )
{
    if (getCurrentElement() == XLS14_TOKEN( id ))
    {
        getExtLst().insert( std::pair< OUString, ScDataBarFormatData*>(rChars, mpTarget) );
    }
}

ExtGlobalContext::ExtGlobalContext( WorksheetContextBase& rFragment ):
    WorksheetContextBase(rFragment)
{
}

ContextHandlerRef ExtGlobalContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch (nElement)
    {
        case XLS14_TOKEN(conditionalFormatting): return new ExtConditionalFormattingContext(*this);
        case XLS14_TOKEN(dataValidations):       return new ExtDataValidationsContext(*this);
        case XLS14_TOKEN(sparklineGroups): return new SparklineGroupsContext(*this);
    }
    return this;
}

void ExtGlobalContext::onStartElement( const AttributeList& /*rAttribs*/ )
{
}

ExtLstGlobalContext::ExtLstGlobalContext( WorksheetFragment& rFragment ):
    WorksheetContextBase(rFragment)
{
}

ContextHandlerRef ExtLstGlobalContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    if (nElement == XLS_TOKEN( ext ))
        return new ExtGlobalContext( *this );

    return this;
}

ExtGlobalWorkbookContext::ExtGlobalWorkbookContext( WorkbookContextBase& rFragment ):
    WorkbookContextBase(rFragment)
{
}

ContextHandlerRef ExtGlobalWorkbookContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if (nElement == LOEXT_TOKEN(extCalcPr))
    {
        ScDocument& rDoc = getScDocument();
        sal_Int32 nToken = rAttribs.getToken( XML_stringRefSyntax, XML_CalcA1 );
        ScCalcConfig aCalcConfig = rDoc.GetCalcConfig();

        switch( nToken )
        {
             case XML_CalcA1:
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_OOO );
                break;
             case XML_ExcelA1:
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_XL_A1 );
                break;
             case XML_ExcelR1C1:
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_XL_R1C1 );
                break;
             case XML_CalcA1ExcelA1:
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_A1_XL_A1 );
                break;
             default:
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_UNSPECIFIED );
               break;
        }
        rDoc.SetCalcConfig(aCalcConfig);
    }

    return this;
}

void ExtGlobalWorkbookContext::onStartElement( const AttributeList& /*rAttribs*/ )
{
}

ExtLstGlobalWorkbookContext::ExtLstGlobalWorkbookContext( WorkbookFragment& rFragment ):
    WorkbookContextBase(rFragment)
{
}

ContextHandlerRef ExtLstGlobalWorkbookContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    if (nElement == XLS_TOKEN( ext ))
        return new ExtGlobalWorkbookContext( *this );

    return this;
}

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
