/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    mpTarget( pTarget )
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

        default:
            break;
    }
}

void ExtCfRuleContext::importDataBar( const AttributeList& rAttribs )
{
    ScDataBarFormatData* pDataBar = static_cast<ScDataBarFormatData*>(mpTarget);
    pDataBar->mbGradient = rAttribs.getBool( XML_gradient, true );

    rtl::OUString aAxisPosition = rAttribs.getString( XML_axisPosition, "automatic" );
    if( aAxisPosition == "none" )
        pDataBar->meAxisPosition = databar::NONE;
    else if( aAxisPosition == "middle" )
        pDataBar->meAxisPosition = databar::MIDDLE;
    else
        pDataBar->meAxisPosition = databar::AUTOMATIC;

    pDataBar->mbNeg = !rAttribs.getBool( XML_negativeBarColorSameAsPositive, false );
}

namespace {

::Color RgbToRgbComponents( sal_Int32& nRgb )
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

void ExtLstLocalContext::onCharacters( const rtl::OUString& rChars )
{
    if (getCurrentElement() == XLS_EXT_TOKEN( id ))
    {
        getExtLst().insert( std::pair< rtl::OUString, void*>(rChars, mpTarget) );
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
            rtl::OUString aId = rAttribs.getString( XML_id, rtl::OUString() );
            void* pInfo = getExtLst().find( aId )->second;
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

void ExtGlobalContext::onStartElement( const AttributeList& rAttribs )
{
    if(rAttribs.hasAttribute( XML_id ))
    {
        rtl::OUString aId = rAttribs.getString( XML_id, rtl::OUString() );
    }

    if(rAttribs.hasAttribute( XML_uri ))
    {
        rtl::OUString aUri = rAttribs.getString( XML_uri, rtl::OUString() );
    }
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

void ExtLstGlobalContext::onStartElement( const AttributeList& )
{
}

void ExtLstGlobalContext::onCharacters( const rtl::OUString&)
{
}

} //namespace oox
} //namespace xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
