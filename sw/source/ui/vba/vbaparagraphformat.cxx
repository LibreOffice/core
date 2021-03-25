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
#include "vbaparagraphformat.hxx"
#include <vbahelper/vbahelper.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <ooo/vba/word/WdLineSpacing.hpp>
#include <ooo/vba/word/WdParagraphAlignment.hpp>
#include <ooo/vba/word/WdOutlineLevel.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "vbatabstops.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int16 CHARACTER_INDENT_FACTOR = 12;
const sal_Int16 PERCENT100 = 100;
const sal_Int16 PERCENT150 = 150;
const sal_Int16 PERCENT200 = 200;

SwVbaParagraphFormat::SwVbaParagraphFormat( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< beans::XPropertySet >& rParaProps ) : SwVbaParagraphFormat_BASE( rParent, rContext ), mxParaProps( rParaProps )
{
}

SwVbaParagraphFormat::~SwVbaParagraphFormat()
{
}

sal_Int32 SAL_CALL SwVbaParagraphFormat::getAlignment()
{
    style::ParagraphAdjust aParaAdjust = style::ParagraphAdjust_LEFT;
    mxParaProps->getPropertyValue("ParaAdjust") >>= aParaAdjust;
    return getMSWordAlignment( aParaAdjust );
}

void SAL_CALL SwVbaParagraphFormat::setAlignment( sal_Int32 _alignment )
{
    style::ParagraphAdjust aParaAdjust = getOOoAlignment( _alignment );
    mxParaProps->setPropertyValue("ParaAdjust", uno::makeAny( aParaAdjust ) );
}

float SAL_CALL SwVbaParagraphFormat::getFirstLineIndent()
{
    sal_Int32 indent = 0;
    mxParaProps->getPropertyValue("ParaFirstLineIndent") >>= indent;
    return static_cast<float>( Millimeter::getInPoints( indent ) );
}

void SAL_CALL SwVbaParagraphFormat::setFirstLineIndent( float _firstlineindent )
{
    sal_Int32 indent = Millimeter::getInHundredthsOfOneMillimeter( _firstlineindent );
    mxParaProps->setPropertyValue("ParaFirstLineIndent", uno::makeAny( indent ) );
}

uno::Any SAL_CALL SwVbaParagraphFormat::getKeepTogether()
{
    bool bKeep = false;
    mxParaProps->getPropertyValue("ParaKeepTogether") >>= bKeep;
    return uno::makeAny ( bKeep );
}

void SAL_CALL SwVbaParagraphFormat::setKeepTogether( const uno::Any& _keeptogether )
{
    bool bKeep = false;
    if( _keeptogether >>= bKeep )
    {
        mxParaProps->setPropertyValue("ParaKeepTogether", uno::makeAny( bKeep ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

uno::Any SAL_CALL SwVbaParagraphFormat::getKeepWithNext()
{
    bool bKeep = false;
    mxParaProps->getPropertyValue("ParaSplit") >>= bKeep;
    return uno::makeAny ( bKeep );
}

void SAL_CALL SwVbaParagraphFormat::setKeepWithNext( const uno::Any& _keepwithnext )
{
    bool bKeep = false;
    if( _keepwithnext >>= bKeep )
    {
        mxParaProps->setPropertyValue("ParaSplit", uno::makeAny( bKeep ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

uno::Any SAL_CALL SwVbaParagraphFormat::getHyphenation()
{
    bool bHypn = false;
    mxParaProps->getPropertyValue("ParaIsHyphenation") >>= bHypn;
    return uno::makeAny ( bHypn );
}

void SAL_CALL SwVbaParagraphFormat::setHyphenation( const uno::Any& _hyphenation )
{
    bool bHypn = false;
    if( _hyphenation >>= bHypn )
    {
        mxParaProps->setPropertyValue("ParaIsHyphenation", uno::makeAny( bHypn ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

float SAL_CALL SwVbaParagraphFormat::getLineSpacing()
{
    style::LineSpacing aLineSpacing;
    mxParaProps->getPropertyValue("ParaLineSpacing") >>= aLineSpacing;
    return getMSWordLineSpacing( aLineSpacing );
}

void SAL_CALL SwVbaParagraphFormat::setLineSpacing( float _linespacing )
{
    style::LineSpacing aLineSpacing;
    mxParaProps->getPropertyValue("ParaLineSpacing") >>= aLineSpacing;
    aLineSpacing = getOOoLineSpacing( _linespacing, aLineSpacing.Mode );
    mxParaProps->setPropertyValue("ParaLineSpacing", uno::makeAny( aLineSpacing ) );
}

sal_Int32 SAL_CALL SwVbaParagraphFormat::getLineSpacingRule()
{
    style::LineSpacing aLineSpacing;
    mxParaProps->getPropertyValue("ParaLineSpacing") >>= aLineSpacing;
    return getMSWordLineSpacingRule( aLineSpacing );
}

void SAL_CALL SwVbaParagraphFormat::setLineSpacingRule( sal_Int32 _linespacingrule )
{
    style::LineSpacing aLineSpacing = getOOoLineSpacingFromRule( _linespacingrule );
    mxParaProps->setPropertyValue("ParaLineSpacing", uno::makeAny( aLineSpacing ) );
}

uno::Any SAL_CALL SwVbaParagraphFormat::getNoLineNumber()
{
    bool noLineNum = false;
    mxParaProps->getPropertyValue("ParaLineNumberCount") >>= noLineNum;
    return uno::makeAny ( noLineNum );
}

void SAL_CALL SwVbaParagraphFormat::setNoLineNumber( const uno::Any& _nolinenumber )
{
    bool noLineNum = false;
    if( _nolinenumber >>= noLineNum )
    {
        mxParaProps->setPropertyValue("ParaLineNumberCount", uno::makeAny( noLineNum ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

sal_Int32 SAL_CALL SwVbaParagraphFormat::getOutlineLevel()
{
    sal_Int32 nLevel = word::WdOutlineLevel::wdOutlineLevelBodyText;
    OUString aHeading;
    static const OUStringLiteral HEADING = u"Heading";
    mxParaProps->getPropertyValue("ParaStyleName") >>= aHeading;
    if( aHeading.startsWith( HEADING ) )
    {
        // get the sub string after "Heading"
        nLevel = aHeading.copy( HEADING.getLength() ).toInt32();
    }
    return nLevel;
}

void SAL_CALL SwVbaParagraphFormat::setOutlineLevel( sal_Int32 _outlinelevel )
{
    if( _outlinelevel != getOutlineLevel() )
    {
        // TODO: in my test in msword, there is no effect for this function.
    }
}

uno::Any SAL_CALL SwVbaParagraphFormat::getPageBreakBefore()
{
    style::BreakType aBreakType;
    mxParaProps->getPropertyValue("BreakType") >>= aBreakType;
    bool bBreakBefore = ( aBreakType == style::BreakType_PAGE_BEFORE || aBreakType == style::BreakType_PAGE_BOTH );
    return uno::makeAny( bBreakBefore );
}

void SAL_CALL SwVbaParagraphFormat::setPageBreakBefore( const uno::Any& _breakbefore )
{
    bool bBreakBefore = false;
    if( _breakbefore >>= bBreakBefore )
    {
        style::BreakType aBreakType;
        mxParaProps->getPropertyValue("BreakType") >>= aBreakType;
        if( bBreakBefore )
        {
            if( aBreakType == style::BreakType_NONE )
                aBreakType = style::BreakType_PAGE_BEFORE;
            else if ( aBreakType == style::BreakType_PAGE_AFTER )
                aBreakType = style::BreakType_PAGE_BOTH;
        }
        else
        {
            if( aBreakType == style::BreakType_PAGE_BOTH )
                aBreakType = style::BreakType_PAGE_AFTER;
            else if ( aBreakType == style::BreakType_PAGE_BEFORE )
                aBreakType = style::BreakType_PAGE_AFTER;
        }
        mxParaProps->setPropertyValue("BreakType", uno::makeAny( aBreakType ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

float SAL_CALL SwVbaParagraphFormat::getSpaceBefore()
{
    sal_Int32 nSpace = 0;
    mxParaProps->getPropertyValue("ParaTopMargin") >>= nSpace;
    return static_cast<float>( Millimeter::getInPoints( nSpace ) );
}

void SAL_CALL SwVbaParagraphFormat::setSpaceBefore( float _space )
{
    sal_Int32 nSpace = Millimeter::getInHundredthsOfOneMillimeter( _space );
    mxParaProps->setPropertyValue("ParaTopMargin", uno::makeAny( nSpace ) );
}

float SAL_CALL SwVbaParagraphFormat::getSpaceAfter()
{
    sal_Int32 nSpace = 0;
    mxParaProps->getPropertyValue("ParaBottomMargin") >>= nSpace;
    return static_cast<float>( Millimeter::getInPoints( nSpace ) );
}

void SAL_CALL SwVbaParagraphFormat::setSpaceAfter( float _space )
{
    sal_Int32 nSpace = Millimeter::getInHundredthsOfOneMillimeter( _space );
    mxParaProps->setPropertyValue("ParaBottomMargin", uno::makeAny( nSpace ) );
}

float SAL_CALL SwVbaParagraphFormat::getLeftIndent()
{
    sal_Int32 nIndent = 0;
    mxParaProps->getPropertyValue("ParaLeftMargin") >>= nIndent;
    return static_cast<float>( Millimeter::getInPoints( nIndent ) );
}

void SAL_CALL SwVbaParagraphFormat::setLeftIndent( float _leftindent )
{
    sal_Int32 nIndent = Millimeter::getInHundredthsOfOneMillimeter( _leftindent );
    mxParaProps->setPropertyValue("ParaLeftMargin", uno::makeAny( nIndent ) );
}

float SAL_CALL SwVbaParagraphFormat::getRightIndent()
{
    sal_Int32 nIndent = 0;
    mxParaProps->getPropertyValue("ParaRightMargin") >>= nIndent;
    return static_cast<float>( Millimeter::getInPoints( nIndent ) );
}

void SAL_CALL SwVbaParagraphFormat::setRightIndent( float _rightindent )
{
    sal_Int32 nIndent = Millimeter::getInHundredthsOfOneMillimeter( _rightindent );
    mxParaProps->setPropertyValue("ParaRightMargin", uno::makeAny( nIndent ) );
}

uno::Any SAL_CALL SwVbaParagraphFormat::getTabStops()
{
    return uno::makeAny( uno::Reference< word::XTabStops >( new SwVbaTabStops( this, mxContext, mxParaProps ) ) );
}

void SAL_CALL SwVbaParagraphFormat::setTabStops( const uno::Any& /*_tabstops*/ )
{
    throw uno::RuntimeException("Not implemented" );
}

uno::Any SAL_CALL SwVbaParagraphFormat::getWidowControl()
{
    sal_Int8 nWidow = 0;
    mxParaProps->getPropertyValue("ParaWidows") >>= nWidow;
    sal_Int8 nOrphan = 0;
    mxParaProps->getPropertyValue("ParaOrphans") >>= nOrphan;
    // if the amount of single lines on one page > 1 and the same of start and end of the paragraph,
    // true is returned.
    bool bWidow = ( nWidow > 1 && nOrphan == nWidow );
    return uno::makeAny( bWidow );
}

void SAL_CALL SwVbaParagraphFormat::setWidowControl( const uno::Any& _widowcontrol )
{
    // if we get true, the part of the paragraph on one page has to be
    // at least two lines
    bool bWidow = false;
    if( _widowcontrol >>= bWidow )
    {
        sal_Int8 nControl = bWidow? 2:1;
        mxParaProps->setPropertyValue("ParaWidows", uno::makeAny( nControl ) );
        mxParaProps->setPropertyValue("ParaOrphans", uno::makeAny( nControl ) );
    }
    else
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
    }
}

style::LineSpacing SwVbaParagraphFormat::getOOoLineSpacing( float _lineSpace, sal_Int16 mode )
{
    style::LineSpacing aLineSpacing;
    if( mode != style::LineSpacingMode::MINIMUM && mode != style::LineSpacingMode::FIX )
    {
        // special behaviour of word: if the space is set to these values, the rule and
        // the height are changed accordingly
        if( _lineSpace == CHARACTER_INDENT_FACTOR )
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = PERCENT100;
        }
        else if( _lineSpace == CHARACTER_INDENT_FACTOR * 1.5 ) // no rounding issues, == 18
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = PERCENT150;
        }
        else if( _lineSpace == CHARACTER_INDENT_FACTOR * 2 )
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = PERCENT200;
        }
        else
        {
            aLineSpacing.Mode = style::LineSpacingMode::FIX;
            aLineSpacing.Height = static_cast<sal_Int16>( Millimeter::getInHundredthsOfOneMillimeter( _lineSpace ) );
        }
    }
    else
    {
        aLineSpacing.Mode = mode;
        aLineSpacing.Height = static_cast<sal_Int16>( Millimeter::getInHundredthsOfOneMillimeter( _lineSpace ) );
    }
    return aLineSpacing;
}

style::LineSpacing SwVbaParagraphFormat::getOOoLineSpacingFromRule( sal_Int32 _linespacingrule )
{
    style::LineSpacing aLineSpacing;
    switch( _linespacingrule )
    {
        case word::WdLineSpacing::wdLineSpace1pt5:
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = PERCENT150;
            break;
        }
        case word::WdLineSpacing::wdLineSpaceAtLeast:
        {
            aLineSpacing.Mode = style::LineSpacingMode::MINIMUM;
            aLineSpacing.Height = getCharHeight();
            break;
        }
        case word::WdLineSpacing::wdLineSpaceDouble:
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = getCharHeight();
            break;
        }
        case word::WdLineSpacing::wdLineSpaceExactly:
        case word::WdLineSpacing::wdLineSpaceMultiple:
        {
            aLineSpacing.Mode = style::LineSpacingMode::FIX;
            aLineSpacing.Height = getCharHeight();
            break;
        }
        case word::WdLineSpacing::wdLineSpaceSingle:
        {
            aLineSpacing.Mode = style::LineSpacingMode::PROP;
            aLineSpacing.Height = PERCENT100;
            break;
        }
        default:
        {
            DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
            break;
        }
    }
    return aLineSpacing;
}

float SwVbaParagraphFormat::getMSWordLineSpacing( style::LineSpacing const & rLineSpacing )
{
    float wdLineSpacing = 0;
    if( rLineSpacing.Mode != style::LineSpacingMode::PROP )
    {
        wdLineSpacing = static_cast<float>( Millimeter::getInPoints( rLineSpacing.Height ) );
    }
    else
    {
        wdLineSpacing = static_cast<float>( CHARACTER_INDENT_FACTOR * rLineSpacing.Height ) / PERCENT100;
    }
    return wdLineSpacing;
}

sal_Int32 SwVbaParagraphFormat::getMSWordLineSpacingRule( style::LineSpacing const & rLineSpacing )
{
    sal_Int32 wdLineSpacing = word::WdLineSpacing::wdLineSpaceSingle;
    switch( rLineSpacing.Mode )
    {
        case style::LineSpacingMode::PROP:
        {
            switch( rLineSpacing.Height )
            {
                case PERCENT100:
                {
                    wdLineSpacing = word::WdLineSpacing::wdLineSpaceSingle;
                    break;
                }
                case PERCENT150:
                {
                    wdLineSpacing = word::WdLineSpacing::wdLineSpace1pt5;
                    break;
                }
                case PERCENT200:
                {
                    wdLineSpacing = word::WdLineSpacing::wdLineSpaceDouble;
                    break;
                }
                default:
                {
                    wdLineSpacing = word::WdLineSpacing::wdLineSpaceMultiple;
                }
            }
            break;
        }
        case style::LineSpacingMode::MINIMUM:
        {
            wdLineSpacing = word::WdLineSpacing::wdLineSpaceAtLeast;
            break;
        }
        case style::LineSpacingMode::FIX:
        case style::LineSpacingMode::LEADING:
        {
            wdLineSpacing = word::WdLineSpacing::wdLineSpaceExactly;
            break;
        }
        default:
        {
            DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
        }
    }
    return wdLineSpacing;
}

sal_Int16 SwVbaParagraphFormat::getCharHeight()
{
    float fCharHeight = 0.0;
    mxParaProps->getPropertyValue("CharHeight") >>= fCharHeight;
    return static_cast<sal_Int16>( Millimeter::getInHundredthsOfOneMillimeter( fCharHeight ) );
}

style::ParagraphAdjust SwVbaParagraphFormat::getOOoAlignment( sal_Int32 _alignment )
{
    style::ParagraphAdjust nParaAjust = style::ParagraphAdjust_LEFT;
    switch( _alignment )
    {
        case word::WdParagraphAlignment::wdAlignParagraphCenter:
        {
            nParaAjust = style::ParagraphAdjust_CENTER;
            break;
        }
        case word::WdParagraphAlignment::wdAlignParagraphJustify:
        {
            nParaAjust = style::ParagraphAdjust_BLOCK;
            break;
        }
        case word::WdParagraphAlignment::wdAlignParagraphLeft:
        {
            nParaAjust = style::ParagraphAdjust_LEFT;
            break;
        }
        case word::WdParagraphAlignment::wdAlignParagraphRight:
        {
            nParaAjust = style::ParagraphAdjust_RIGHT;
            break;
        }
        default:
        {
            DebugHelper::runtimeexception( ERRCODE_BASIC_BAD_PARAMETER );
        }
    }
    return nParaAjust;
}

sal_Int32 SwVbaParagraphFormat::getMSWordAlignment( style::ParagraphAdjust _alignment )
{
    sal_Int32 wdAlignment = word::WdParagraphAlignment::wdAlignParagraphLeft;
    switch( _alignment )
    {
        case style::ParagraphAdjust_CENTER:
        {
            wdAlignment = word::WdParagraphAlignment::wdAlignParagraphCenter;
            break;
        }
        case style::ParagraphAdjust_LEFT:
        {
            wdAlignment = word::WdParagraphAlignment::wdAlignParagraphLeft;
            break;
        }
        case style::ParagraphAdjust_BLOCK:
        {
            wdAlignment = word::WdParagraphAlignment::wdAlignParagraphJustify;
            break;
        }
        case style::ParagraphAdjust_RIGHT:
        {
            wdAlignment = word::WdParagraphAlignment::wdAlignParagraphRight;
            break;
        }
        default:
        {
            DebugHelper::basicexception( ERRCODE_BASIC_BAD_PARAMETER, {} );
        }
    }
    return wdAlignment;
}

OUString
SwVbaParagraphFormat::getServiceImplName()
{
    return "SwVbaParagraphFormat";
}

uno::Sequence< OUString >
SwVbaParagraphFormat::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.ParagraphFormat"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
