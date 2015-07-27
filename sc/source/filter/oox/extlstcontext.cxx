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
#include "calcconfig.hxx"
#include "document.hxx"

using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

ExtCfRuleContext::ExtCfRuleContext( WorksheetContextBase& rFragment, void* pTarget ):
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
        case XLS_EXT_TOKEN( dataBar ):
        {
            ExtCfRuleRef xRule = getCondFormats().createExtCfRule(mpTarget);
            xRule->importDataBar( rAttribs );
            break;
        }
        case XLS_EXT_TOKEN( negativeFillColor ):
        {
            ExtCfRuleRef xRule = getCondFormats().createExtCfRule(mpTarget);
            xRule->importNegativeFillColor( rAttribs );
            break;
        }
        case XLS_EXT_TOKEN( axisColor ):
        {
            ExtCfRuleRef xRule = getCondFormats().createExtCfRule(mpTarget);
            xRule->importAxisColor( rAttribs );
            break;
        }
        case XLS_EXT_TOKEN( cfvo ):
        {
            ExtCfRuleRef xRule = getCondFormats().createExtCfRule(mpTarget);
            xRule->importCfvo( rAttribs );
            xRule->getModel().mbIsLower = mbFirstEntry;
            mbFirstEntry = false;
            break;
        }
        default:
            break;
    }
}

ExtLstLocalContext::ExtLstLocalContext( WorksheetContextBase& rFragment, void* pTarget ):
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
            if (nElement == XLS_EXT_TOKEN( id ))
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
        case XLS_EXT_TOKEN( id ):
        break;
    }
}

void ExtLstLocalContext::onCharacters( const OUString& rChars )
{
    if (getCurrentElement() == XLS_EXT_TOKEN( id ))
    {
        getExtLst().insert( std::pair< OUString, void*>(rChars, mpTarget) );
    }
}

ExtGlobalContext::ExtGlobalContext( WorksheetContextBase& rFragment ):
    WorksheetContextBase(rFragment)
{
}

ContextHandlerRef ExtGlobalContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if(!rAttribs.hasAttribute( XML_id))
        return this;
    else
    {
        if(nElement == XLS_EXT_TOKEN( cfRule ))
        {
            OUString aId = rAttribs.getString( XML_id, OUString() );

            // an ext entrie does not need to have an existing corresponding entry
            ExtLst::const_iterator aExt = getExtLst().find( aId );
            if(aExt == getExtLst().end())
                return NULL;

            void* pInfo = aExt->second;
            if (!pInfo)
            {
                return NULL;
            }
            return new ExtCfRuleContext( *this, pInfo );
        }
        else
            return this;
    }
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
                aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_UNSPECIFIED);
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
