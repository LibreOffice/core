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

#include <xmloff/controlpropertyhdl.hxx>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltypes.hxx>
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include "formenums.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include "callbacks.hxx"
#include <xmloff/XMLConstantsPropertyHandler.hxx>

namespace xmloff
{

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;
    using namespace ::xmloff::token;

    //= OControlPropertyHandlerFactory
    OControlPropertyHandlerFactory::OControlPropertyHandlerFactory()
    {
    }

    const XMLPropertyHandler* OControlPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
    {
        const XMLPropertyHandler* pHandler = nullptr;

        switch (_nType)
        {
            case XML_TYPE_TEXT_ALIGN:
                if (!m_pTextAlignHandler)
                    m_pTextAlignHandler = std::make_unique<XMLConstantsPropertyHandler>(aTextAlignMap, XML_TOKEN_INVALID );
                pHandler = m_pTextAlignHandler.get();
                break;

            case XML_TYPE_CONTROL_BORDER:
                if (!m_pControlBorderStyleHandler)
                    m_pControlBorderStyleHandler = std::make_unique<OControlBorderHandler>( OControlBorderHandler::STYLE );
                pHandler = m_pControlBorderStyleHandler.get();
                break;

            case XML_TYPE_CONTROL_BORDER_COLOR:
                if ( !m_pControlBorderColorHandler )
                    m_pControlBorderColorHandler = std::make_unique<OControlBorderHandler>( OControlBorderHandler::COLOR );
                pHandler = m_pControlBorderColorHandler.get();
                break;

            case XML_TYPE_ROTATION_ANGLE:
                if (!m_pRotationAngleHandler)
                    m_pRotationAngleHandler = std::make_unique<ORotationAngleHandler>();
                pHandler = m_pRotationAngleHandler.get();
                break;

            case XML_TYPE_FONT_WIDTH:
                if (!m_pFontWidthHandler)
                    m_pFontWidthHandler = std::make_unique<OFontWidthHandler>();
                pHandler = m_pFontWidthHandler.get();
                break;

            case XML_TYPE_CONTROL_TEXT_EMPHASIZE:
                if (!m_pFontEmphasisHandler)
                    m_pFontEmphasisHandler = std::make_unique<XMLConstantsPropertyHandler>( aFontEmphasisMap, XML_NONE );
                pHandler = m_pFontEmphasisHandler.get();
                break;

            case XML_TYPE_TEXT_FONT_RELIEF:
                if (!m_pFontReliefHandler)
                    m_pFontReliefHandler = std::make_unique<XMLConstantsPropertyHandler>( aFontReliefMap, XML_NONE );
                pHandler = m_pFontReliefHandler.get();
                break;
            case XML_TYPE_TEXT_LINE_MODE:
                if (!m_pTextLineModeHandler)
                {
                    m_pTextLineModeHandler = std::make_unique<XMLNamedBoolPropertyHdl>(
                            ::xmloff::token::XML_SKIP_WHITE_SPACE,
                            ::xmloff::token::XML_CONTINUOUS);
                }
                pHandler = m_pTextLineModeHandler.get();
                break;
        }

        if (!pHandler)
            pHandler = XMLPropertyHandlerFactory::GetPropertyHandler(_nType);
        return pHandler;
    }

    //= OControlTextEmphasisHandler
    OControlTextEmphasisHandler::OControlTextEmphasisHandler()
    {
    }

    bool OControlTextEmphasisHandler::exportXML( OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        OUStringBuffer aReturn;
        bool bSuccess = false;
        sal_Int16 nFontEmphasis = sal_Int16();
        if (_rValue >>= nFontEmphasis)
        {
            // the type
            sal_uInt16 nType = nFontEmphasis & ~(awt::FontEmphasisMark::ABOVE | awt::FontEmphasisMark::BELOW);
            // the position of the mark
            bool bBelow = 0 != (nFontEmphasis & awt::FontEmphasisMark::BELOW);

            // convert
            bSuccess = SvXMLUnitConverter::convertEnum(aReturn, nType, aFontEmphasisMap, XML_NONE);
            if (bSuccess)
            {
                aReturn.append( ' ' );
                aReturn.append( GetXMLToken(bBelow ? XML_BELOW : XML_ABOVE) );

                _rStrExpValue = aReturn.makeStringAndClear();
            }
        }

        return bSuccess;
    }

    bool OControlTextEmphasisHandler::importXML( const OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        bool bSuccess = true;
        sal_uInt16 nEmphasis = awt::FontEmphasisMark::NONE;

        bool bBelow = false;
        bool bHasPos = false, bHasType = false;

        OUString sToken;
        SvXMLTokenEnumerator aTokenEnum(_rStrImpValue);
        while (aTokenEnum.getNextToken(sToken))
        {
            if (!bHasPos)
            {
                if (IsXMLToken(sToken, XML_ABOVE))
                {
                    bBelow = false;
                    bHasPos = true;
                }
                else if (IsXMLToken(sToken, XML_BELOW))
                {
                    bBelow = true;
                    bHasPos = true;
                }
            }
            if (!bHasType)
            {
                if (SvXMLUnitConverter::convertEnum(nEmphasis, sToken, aFontEmphasisMap))
                {
                    bHasType = true;
                }
                else
                {
                    bSuccess = false;
                    break;
                }
            }
        }

        if (bSuccess)
        {
            nEmphasis |= bBelow ? awt::FontEmphasisMark::BELOW : awt::FontEmphasisMark::ABOVE;
            _rValue <<= nEmphasis;
        }

        return bSuccess;
    }

    //= OControlBorderHandlerBase
    OControlBorderHandler::OControlBorderHandler( const OControlBorderHandler::BorderFacet _eFacet )
        :m_eFacet( _eFacet )
    {
    }

    bool OControlBorderHandler::importXML( const OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        OUString sToken;
        SvXMLTokenEnumerator aTokens(_rStrImpValue);

        sal_uInt16 nStyle = 1;

        while   (   aTokens.getNextToken(sToken)    // have a new token
                &&  (!sToken.isEmpty())       // really have a new token
                )
        {
            // try interpreting the token as border style
            if ( m_eFacet == STYLE )
            {
                // is it a valid enum value?
                if ( SvXMLUnitConverter::convertEnum( nStyle, sToken, aBorderTypeMap ) )
                {
                    _rValue <<= nStyle;
                    return true;
                }
            }

            // try interpreting it as color value
            if ( m_eFacet == COLOR )
            {
                sal_Int32 nColor(0);
                if (::sax::Converter::convertColor( nColor, sToken ))
                {
                    _rValue <<= nColor;
                    return true;
                }
            }
        }

        return false;
    }

    bool OControlBorderHandler::exportXML( OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        bool bSuccess = false;

        OUStringBuffer aOut;
        switch ( m_eFacet )
        {
        case STYLE:
        {
            sal_uInt16 nBorder = 0;
            bSuccess =  (_rValue >>= nBorder)
                    &&  SvXMLUnitConverter::convertEnum( aOut, nBorder, aBorderTypeMap );
        }
        break;
        case COLOR:
        {
            sal_Int32 nBorderColor = 0;
            if ( _rValue >>= nBorderColor )
            {
                ::sax::Converter::convertColor(aOut, nBorderColor);
                bSuccess = true;
            }
        }
        break;
        }   // switch ( m_eFacet )

        if ( !bSuccess )
            return false;

        if ( !_rStrExpValue.isEmpty() )
            _rStrExpValue += " ";
        _rStrExpValue += aOut;

        return true;
    }

    //= OFontWidthHandler
    OFontWidthHandler::OFontWidthHandler()
    {
    }

    bool OFontWidthHandler::importXML( const OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Int32 nWidth = 0;
        bool const bSuccess = ::sax::Converter::convertMeasure(
                nWidth, _rStrImpValue, util::MeasureUnit::POINT);
        if (bSuccess)
            _rValue <<= static_cast<sal_Int16>(nWidth);

        return bSuccess;
    }

    bool OFontWidthHandler::exportXML( OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Int16 nWidth = 0;
        OUStringBuffer aResult;
        if (_rValue >>= nWidth)
        {
            ::sax::Converter::convertMeasure(aResult, nWidth,
                    util::MeasureUnit::POINT, util::MeasureUnit::POINT);
        }
        _rStrExpValue = aResult.makeStringAndClear();

        return !_rStrExpValue.isEmpty();
    }

    //= ORotationAngleHandler
    ORotationAngleHandler::ORotationAngleHandler()
    {
    }

    bool ORotationAngleHandler::importXML( const OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        double fValue;
        bool const bSucces =
            ::sax::Converter::convertDouble(fValue, _rStrImpValue);
        if (bSucces)
        {
            fValue *= 10;
            _rValue <<= static_cast<float>(fValue);
        }

        return bSucces;
    }

    bool ORotationAngleHandler::exportXML( OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        float fAngle = 0;
        bool bSuccess = (_rValue >>= fAngle);

        if (bSuccess)
        {
            OUStringBuffer sValue;
            ::sax::Converter::convertDouble(sValue, static_cast<double>(fAngle) / 10);
            _rStrExpValue = sValue.makeStringAndClear();
        }

        return bSuccess;
    }

    //= ImageScaleModeHandler
    ImageScaleModeHandler::ImageScaleModeHandler()
        :XMLConstantsPropertyHandler( aScaleModeMap, XML_STRETCH )
    {
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
