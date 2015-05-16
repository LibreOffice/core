/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "extlstcontext.hxx"
#include "worksheethelper.hxx"
#include <oox/core/contexthandler.hxx>
#include "colorscale.hxx"
#include "condformatbuffer.hxx"
#include "condformatcontext.hxx"
#include "document.hxx"

#include "rangeutl.hxx"

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
    WorksheetContextBase(rFragment),
    mpCurrentRule(NULL)
{
}

ContextHandlerRef ExtConditionalFormattingContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    if (mpCurrentRule)
    {
        ScFormatEntry& rFormat = *maEntries.rbegin();
        assert(rFormat.GetType() == condformat::ICONSET);
        ScIconSetFormat& rIconSet = static_cast<ScIconSetFormat&>(rFormat);
        ScDocument* pDoc = &getScDocument();
        SCTAB nTab = getCurrentSheetIndex();
        ScAddress aPos(0, 0, nTab);
        mpCurrentRule->SetData(&rIconSet, pDoc, aPos);
        delete mpCurrentRule;
        mpCurrentRule = NULL;
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
                return NULL;

            ScDataBarFormatData* pInfo = aExt->second;
            if (!pInfo)
            {
                return NULL;
            }
            return new ExtCfRuleContext( *this, pInfo );
        }
        else if (aType == "iconSet")
        {
            ScDocument* pDoc = &getScDocument();
            mpCurrentRule = new IconSetRule(*this);
            ScIconSetFormat* pIconSet = new ScIconSetFormat(pDoc);
            maEntries.push_back(pIconSet);
            return new IconSetContext(*this, mpCurrentRule);
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

    return NULL;
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

            boost::ptr_vector<ExtCfCondFormat>& rExtFormats =  getCondFormats().importExtCondFormat();
            rExtFormats.push_back(new ExtCfCondFormat(aRange, maEntries));
        }
        break;
        case XLS14_TOKEN(cfRule):
            if (mpCurrentRule)
            {
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
                return 0;
            break;
        case XLS_TOKEN( ext ):
            if (nElement == XLS14_TOKEN( id ))
                return this;
            else
                return 0;
    }
    return 0;
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
    if (nElement == XLS14_TOKEN(conditionalFormatting))
        return new ExtConditionalFormattingContext(*this);

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

} //namespace oox
} //namespace xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
