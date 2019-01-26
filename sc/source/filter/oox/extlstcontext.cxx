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

#include <rangeutl.hxx>
#include <sal/log.hxx>

using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

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
            break;
        }
        default:
            break;
    }
}

ExtConditionalFormattingContext::ExtConditionalFormattingContext(WorksheetContextBase& rFragment):
    WorksheetContextBase(rFragment)
{
}

ContextHandlerRef ExtConditionalFormattingContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    if (mpCurrentRule)
    {
        ScFormatEntry& rFormat = **maEntries.rbegin();
        assert(rFormat.GetType() == ScFormatEntry::Type::Iconset);
        ScIconSetFormat& rIconSet = static_cast<ScIconSetFormat&>(rFormat);
        ScDocument* pDoc = &getScDocument();
        SCTAB nTab = getSheetIndex();
        ScAddress aPos(0, 0, nTab);
        mpCurrentRule->SetData(&rIconSet, pDoc, aPos);
        mpCurrentRule.reset();
    }
    if (nElement == XLS14_TOKEN(cfRule))
    {
        OUString aType = rAttribs.getString(XML_type, OUString());
        OUString aId = rAttribs.getString(XML_id, OUString());
        if (aType == "dataBar")
        {
            // an ext entry does not need to have an existing corresponding entry
            ExtLst::const_iterator aExt = getExtLst().find( aId );
            if(aExt == getExtLst().end())
                return nullptr;

            ScDataBarFormatData* pInfo = aExt->second;
            if (!pInfo)
            {
                return nullptr;
            }
            return new ExtCfRuleContext( *this, pInfo );
        }
        else if (aType == "iconSet")
        {
            ScDocument* pDoc = &getScDocument();
            mpCurrentRule.reset(new IconSetRule(*this));
            maEntries.push_back(std::make_unique<ScIconSetFormat>(pDoc));
            return new IconSetContext(*this, mpCurrentRule.get());
        }
        else
        {
            SAL_WARN("sc", "unhandled XLS14_TOKEN(cfRule) with type: " << aType);
        }
    }
    else if (nElement == XM_TOKEN(sqref))
    {
        return this;
    }

    return nullptr;
}

void ExtConditionalFormattingContext::onStartElement(const AttributeList& /*rAttribs*/)
{
    switch (getCurrentElement())
    {
        case XM_TOKEN(sqref):
        {
        }
        break;
    }
}

void ExtConditionalFormattingContext::onCharacters(const OUString& rCharacters)
{
    aChars = rCharacters;
}

void ExtConditionalFormattingContext::onEndElement()
{
    switch (getCurrentElement())
    {
        case XM_TOKEN(sqref):
        {
            ScRangeList aRange;
            ScDocument* pDoc = &getScDocument();
            bool bSuccess = ScRangeStringConverter::GetRangeListFromString(aRange, aChars, pDoc, formula::FormulaGrammar::CONV_XL_OOX);
            if (!bSuccess || aRange.empty())
                break;

            SCTAB nTab = getSheetIndex();
            for (size_t i = 0; i < aRange.size(); ++i)
            {
                aRange[i].aStart.SetTab(nTab);
                aRange[i].aEnd.SetTab(nTab);
            }

            std::vector< std::unique_ptr<ExtCfCondFormat> >& rExtFormats =  getCondFormats().importExtCondFormat();
            rExtFormats.push_back(std::make_unique<ExtCfCondFormat>(aRange, maEntries));
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
            break;
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
        ScDocument* pDoc = &getScDocument();
        sal_Int32 nToken = rAttribs.getToken( XML_stringRefSyntax, XML_CalcA1 );
        ScCalcConfig aCalcConfig = pDoc->GetCalcConfig();

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
        pDoc->SetCalcConfig(aCalcConfig);
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

} //namespace oox
} //namespace xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
