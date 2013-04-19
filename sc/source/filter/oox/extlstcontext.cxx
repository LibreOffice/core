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
            importDataBar( rAttribs );
            break;
        case XLS_EXT_TOKEN( negativeFillColor ):
            importNegativeFillColor( rAttribs );
            break;
        case XLS_EXT_TOKEN( axisColor ):
            importAxisColor( rAttribs );
            break;
        case XLS_EXT_TOKEN( cfvo ):
            importCfvo( rAttribs );
            break;

        default:
            break;
    }
}

void ExtCfRuleContext::importDataBar( const AttributeList& rAttribs )
{
    ScDataBarFormatData* pDataBar = static_cast<ScDataBarFormatData*>(mpTarget);
    pDataBar->mbGradient = rAttribs.getBool( XML_gradient, true );

    OUString aAxisPosition = rAttribs.getString( XML_axisPosition, "automatic" );
    if( aAxisPosition == "none" )
        pDataBar->meAxisPosition = databar::NONE;
    else if( aAxisPosition == "middle" )
        pDataBar->meAxisPosition = databar::MIDDLE;
    else
        pDataBar->meAxisPosition = databar::AUTOMATIC;

    pDataBar->mbNeg = !rAttribs.getBool( XML_negativeBarColorSameAsPositive, false );
}

namespace {

::Color RgbToRgbComponents( sal_Int32 nRgb )
{
    sal_Int32 ornR = (nRgb >> 16) & 0xFF;
    sal_Int32 ornG = (nRgb >> 8) & 0xFF;
    sal_Int32 ornB = nRgb & 0xFF;

    return ::Color(ornR, ornG, ornB);
}

}

void ExtCfRuleContext::importAxisColor( const AttributeList& rAttribs )
{
    ScDataBarFormatData* pDataBar = static_cast<ScDataBarFormatData*>(mpTarget);

    sal_Int32 nColor = rAttribs.getIntegerHex( XML_rgb, API_RGB_TRANSPARENT );
    ::Color aColor = RgbToRgbComponents(nColor);
    pDataBar->maAxisColor = aColor;
}

void ExtCfRuleContext::importNegativeFillColor( const AttributeList& rAttribs )
{
    sal_Int32 nColor = rAttribs.getIntegerHex( XML_rgb, API_RGB_TRANSPARENT );
    ::Color aColor = RgbToRgbComponents(nColor);
    ::Color* pColor = new Color(aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue());
    static_cast<ScDataBarFormatData*>(mpTarget)->mpNegativeColor.reset(pColor);
}

void ExtCfRuleContext::importCfvo( const AttributeList& rAttribs )
{
    ScDataBarFormatData* pDataBar = static_cast<ScDataBarFormatData*>(mpTarget);
    ScColorScaleEntry* pEntry = NULL;
    if(mbFirstEntry)
        pEntry = pDataBar->mpLowerLimit.get();
    else
        pEntry = pDataBar->mpUpperLimit.get();

    OUString aColorScaleType = rAttribs.getString( XML_type, OUString() );
    if(aColorScaleType == "min")
        pEntry->SetType(COLORSCALE_MIN);
    else if (aColorScaleType == "max")
        pEntry->SetType(COLORSCALE_MAX);
    else if (aColorScaleType == "autoMin")
        pEntry->SetType(COLORSCALE_AUTO);
    else if (aColorScaleType == "autoMax")
        pEntry->SetType(COLORSCALE_AUTO);
    else if (aColorScaleType == "percentile")
        pEntry->SetType(COLORSCALE_PERCENTILE);
    else if (aColorScaleType == "percent")
        pEntry->SetType(COLORSCALE_PERCENT);
    else if (aColorScaleType == "formula")
        pEntry->SetType(COLORSCALE_FORMULA);

    mbFirstEntry = false;
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

} //namespace oox
} //namespace xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
